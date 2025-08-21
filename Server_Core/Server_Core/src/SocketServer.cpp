#include "SocketServer.hpp"
#include "CommandHandler.hpp"  // <-- Add this so we can use handleCommand
#include <iostream>
#include <string>

SocketServer::SocketServer(int port, MYSQL* db)
    : port(port), conn(db), serverSocket(INVALID_SOCKET), running(false)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

SocketServer::~SocketServer()
{
    stop();
    WSACleanup();
}

bool SocketServer::start()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "[ERROR] Failed to create socket. Code: " << WSAGetLastError() << "\n";
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[ERROR] Bind failed. Code: " << WSAGetLastError() << "\n";
        return false;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[ERROR] Listen failed. Code: " << WSAGetLastError() << "\n";
        return false;
    }

    running = true;
    std::thread(&SocketServer::acceptLoop, this).detach();

    std::cout << "[OK] SocketServer started on port " << port << "\n";
    return true;
}

void SocketServer::stop()
{
    running = false;

    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }

    clientsMutex.lock();
    for (auto& t : clientThreads) {
        if (t.joinable()) t.join();
    }
    clientThreads.clear();
    clientsMutex.unlock();

    std::cout << "[OK] SocketServer stopped.\n";
}

void SocketServer::acceptLoop()
{
    while (running) {
        sockaddr_in clientAddr{};
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientLen);

        if (clientSocket != INVALID_SOCKET) {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientThreads.emplace_back(&SocketServer::handleClient, this, clientSocket);
        }
    }
}

void SocketServer::handleClient(SOCKET clientSocket)
{
    char buffer[1024];
    int bytesRead;

    std::cout << "[INFO] New client connected.\n";

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0 && running) {
        buffer[bytesRead] = '\0';
        std::string input(buffer);

        std::cout << "[CLIENT] " << input;

        // 🔁 Route to Command System
        handleCommand(conn, input);  // reuses existing CLI command logic

        std::string response = "[SERVER] Command processed\n";  // send generic response
        send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
    }

    closesocket(clientSocket);
    std::cout << "[INFO] Client disconnected.\n";
}
