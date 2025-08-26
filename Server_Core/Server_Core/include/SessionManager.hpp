#pragma once
#include <unordered_map>
#include <mutex>
#include <WinSock2.h>

struct ClientSession {
    int accountId = -1;
    std::string username;
    bool isAuthenticated = false;
};

class SessionManager {
public:
    void Set(SOCKET socket, const ClientSession& session);
    ClientSession* Get(SOCKET socket);
    void Remove(SOCKET socket);

private:
    std::unordered_map<SOCKET, ClientSession> sessions;
    std::mutex sessionMutex;
};

extern SessionManager GlobalSessionManager;

