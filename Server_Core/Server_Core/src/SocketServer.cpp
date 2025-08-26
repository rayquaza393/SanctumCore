#include "SocketServer.hpp"
#include "CommandHandler.hpp"
#include "ScriptRunner.hpp"
#include "SessionManager.hpp"
#include "Logger.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <vector>
#include "json.hpp"

SocketServer::SocketServer(int port, MYSQL* db)
    : port(port), conn(db), serverSocket(INVALID_SOCKET), running(false)
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        GlobalLogger.Error("WSAStartup failed: " + std::to_string(result));
    }
    else {
        GlobalLogger.Success("WSAStartup OK.");
    }
}

SocketServer::~SocketServer()
{
    stop();
    WSACleanup();
    GlobalLogger.Info("WSACleanup complete.");
}

bool SocketServer::start()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        GlobalLogger.Error("Socket creation failed: " + std::to_string(WSAGetLastError()));
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        GlobalLogger.Error("Bind failed: " + std::to_string(WSAGetLastError()));
        return false;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        GlobalLogger.Error("Listen failed: " + std::to_string(WSAGetLastError()));
        return false;
    }

    running = true;
    std::thread(&SocketServer::acceptLoop, this).detach();
    GlobalLogger.Success("SocketServer listening on port " + std::to_string(port));
    return true;
}

void SocketServer::stop()
{
    running = false;

    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
        GlobalLogger.Info("Server socket closed.");
    }

    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto& t : clientThreads) {
        if (t.joinable()) t.join();
    }
    clientThreads.clear();

    GlobalLogger.Info("Client threads cleaned up.");
}

void SocketServer::acceptLoop()
{
    while (running) {
        sockaddr_in clientAddr{};
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientLen);

        if (clientSocket != INVALID_SOCKET) {
            GlobalLogger.Info("New client connected.");
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientThreads.emplace_back(&SocketServer::handleClient, this, clientSocket);
        }
        else {
            GlobalLogger.Warning("Accept failed: " + std::to_string(WSAGetLastError()));
        }
    }
}

void SocketServer::handleClient(SOCKET clientSocket)
{
    GlobalLogger.Info("Client thread started. Awaiting data...");

    std::string buffer;
    char temp[1024];
    int bytesRead = 0;

    while ((bytesRead = recv(clientSocket, temp, sizeof(temp), 0)) > 0 && running)
    {
        buffer.append(temp, bytesRead);

        size_t pos = 0;
        while ((pos = buffer.find('\n')) != std::string::npos)
        {
            std::string line = buffer.substr(0, pos);
            buffer.erase(0, pos + 1);
            if (line.empty()) continue;

            GlobalLogger.Debug("Raw input: " + line);

            try {
                auto parsed = nlohmann::json::parse(line);

                if (!parsed.contains("type") || !parsed["type"].is_string()) {
                    nlohmann::json bad = {
                        {"type", "error"},
                        {"success", false},
                        {"reason", "Missing or invalid 'type' field"},
                        {"data", nlohmann::json::object()}
                    };
                    std::string errStr = bad.dump() + "\n";
                    send(clientSocket, errStr.c_str(), static_cast<int>(errStr.length()), 0);
                    continue;
                }

                std::string type = parsed["type"];
                GlobalLogger.Info("Received packet: " + type);

                nlohmann::json result;

                try {
                    result = ScriptRunner::RunScript(type, parsed);

                    if (!result.contains("type") || !result.contains("success") ||
                        !result.contains("reason") || !result.contains("data")) {
                        result = {
                            {"type", type},
                            {"success", false},
                            {"reason", "Script returned malformed response."},
                            {"data", nlohmann::json::object()}
                        };
                    }

                    std::string jsonStr = result.dump() + "\n";
                    send(clientSocket, jsonStr.c_str(), static_cast<int>(jsonStr.length()), 0);
                    GlobalLogger.Info("ScriptRunner handled packet: " + type);

                }
                catch (const std::exception& ex) {
                    nlohmann::json err = {
                        {"type", "error"},
                        {"success", false},
                        {"reason", "Script failure"},
                        {"data", {
                            {"details", ex.what()}
                        }}
                    };
                    std::string errStr = err.dump() + "\n";
                    send(clientSocket, errStr.c_str(), static_cast<int>(errStr.length()), 0);
                    GlobalLogger.Error("Script failure: " + std::string(ex.what()));
                }

            }
            catch (...) {
                GlobalLogger.Warning("Non-JSON input received. Routing to CommandHandler: " + line);
                handleCommand(conn, line);
                std::string ack = "[SERVER] Command processed\n";
                send(clientSocket, ack.c_str(), static_cast<int>(ack.length()), 0);
            }
        }
    }

    GlobalLogger.Warning("Client disconnected.");
    GlobalSessionManager.Remove(clientSocket);
    closesocket(clientSocket);
}
