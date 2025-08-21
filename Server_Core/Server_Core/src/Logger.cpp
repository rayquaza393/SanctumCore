#include "Logger.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

Logger::Logger(const std::string& filename)
{
    logFile.open(filename, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "[ERROR] Failed to open log file: " << filename << "\n";
    }
}

Logger::~Logger()
{
    if (logFile.is_open())
        logFile.close();
}

void Logger::Info(const std::string& msg) { Write("INFO", msg); }
void Logger::Warn(const std::string& msg) { Write("WARN", msg); }
void Logger::Error(const std::string& msg) { Write("ERROR", msg); }

void Logger::Raw(const std::string& message)
{
    std::cout << message << std::endl;
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.flush();
    }
}

void Logger::Write(const std::string& level, const std::string& msg)
{
    std::lock_guard<std::mutex> lock(logMutex);

    std::string timestamp = GetTimestamp();
    std::string formatted = "[" + timestamp + "] [" + level + "] " + msg + "\n";

    std::cout << formatted;

    if (logFile.is_open()) {
        logFile << formatted;
        logFile.flush();
    }
}

std::string Logger::GetTimestamp()
{
    auto now = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// ✅ Global logger instance
Logger GlobalLogger("server.log");
