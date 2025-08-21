#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <mutex>
#include <mysql.h>

class SocketServer {
public:
    SocketServer(int port, MYSQL* db);  // Pass MySQL pointer to the constructor
    ~SocketServer();

    bool start();
    void stop();

private:
    int port;
    SOCKET serverSocket;
    bool running;
    MYSQL* conn;

    std::vector<std::thread> clientThreads;
    std::mutex clientsMutex;

    void acceptLoop();
    void handleClient(SOCKET clientSocket);
};
