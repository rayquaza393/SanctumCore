#pragma once
#include <string>
#include <unordered_map>

enum class LogChannel {
    AUTH,
    PACKET,
    CHARACTER,
    SCRIPT,
    CLI,
    ERROR_LOG,
    SESSION,
    SYSTEM
};

class Logger {
public:
    void Info(const std::string& msg);
    void Error(const std::string& msg);
    void Warning(const std::string& msg);
    void Success(const std::string& msg);
    void Debug(const std::string& msg);
    void Raw(const std::string& msg);

    void Log(LogChannel channel, const std::string& msg);

    static void SetChannelEnabled(LogChannel channel, bool enabled);

private:
    static std::unordered_map<LogChannel, bool> channelEnabled;
};

extern Logger GlobalLogger;
