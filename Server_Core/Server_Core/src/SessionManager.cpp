#include "SessionManager.hpp"

SessionManager GlobalSessionManager;

void SessionManager::Set(SOCKET socket, const ClientSession& session) {
    std::lock_guard<std::mutex> lock(sessionMutex);
    sessions[socket] = session;
}

ClientSession* SessionManager::Get(SOCKET socket) {
    std::lock_guard<std::mutex> lock(sessionMutex);
    auto it = sessions.find(socket);
    return (it != sessions.end()) ? &it->second : nullptr;
}

void SessionManager::Remove(SOCKET socket) {
    std::lock_guard<std::mutex> lock(sessionMutex);
    sessions.erase(socket);
}
