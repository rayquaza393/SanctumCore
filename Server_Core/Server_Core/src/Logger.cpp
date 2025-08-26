#include "Logger.hpp"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <mutex>

// ANSI color codes
#define COLOR_RESET    "\033[0m"
#define COLOR_RED      "\033[31m"
#define COLOR_GREEN    "\033[32m"
#define COLOR_YELLOW   "\033[33m"
#define COLOR_BLUE     "\033[34m"
#define COLOR_MAGENTA  "\033[35m"
#define COLOR_CYAN     "\033[36m"
#define COLOR_WHITE    "\033[37m"
#define COLOR_GRAY     "\033[90m"
#define COLOR_BOLD     "\033[1m"

Logger GlobalLogger;

namespace {
    std::mutex logMutex;

    std::string currentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
        localtime_s(&tm, &t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    void printWithColor(const std::string& label, const std::string& msg, const char* color) {
        std::lock_guard<std::mutex> lock(logMutex);
        std::cout << "[" << currentTimestamp() << "] "
            << color << "[" << label << "] " << COLOR_RESET
            << msg << std::endl;
    }
}

// LogChannel toggles
std::unordered_map<LogChannel, bool> Logger::channelEnabled = {
    {LogChannel::AUTH, true},
    {LogChannel::PACKET, true},
    {LogChannel::CHARACTER, true},
    {LogChannel::SCRIPT, true},
    {LogChannel::CLI, true},
    {LogChannel::ERROR_LOG, true},
    {LogChannel::SESSION, true},
    {LogChannel::SYSTEM, true},
};

void Logger::SetChannelEnabled(LogChannel channel, bool enabled) {
    channelEnabled[channel] = enabled;
}

void Logger::Info(const std::string& msg) {
    printWithColor("INFO", msg, COLOR_CYAN);
}

void Logger::Error(const std::string& msg) {
    printWithColor("ERROR", msg, COLOR_RED);
}

void Logger::Warning(const std::string& msg) {
    printWithColor("WARN", msg, COLOR_YELLOW);
}

void Logger::Success(const std::string& msg) {
    printWithColor("SUCCESS", msg, COLOR_GREEN);
}

void Logger::Debug(const std::string& msg) {
    printWithColor("DEBUG", msg, COLOR_GRAY);
}

void Logger::Raw(const std::string& msg) {
    std::cout << msg << std::endl;
}

void Logger::Log(LogChannel channel, const std::string& msg) {
    if (!channelEnabled[channel]) return;

    const char* color = COLOR_WHITE;
    std::string tag;

    switch (channel) {
    case LogChannel::AUTH:
        tag = "🔐 AUTH"; color = COLOR_GREEN; break;
    case LogChannel::PACKET:
        tag = "📦 PACKET"; color = COLOR_CYAN; break;
    case LogChannel::CHARACTER:
        tag = "👤 CHAR"; color = COLOR_MAGENTA; break;
    case LogChannel::SCRIPT:
        tag = "📜 SCRIPT"; color = COLOR_YELLOW; break;
    case LogChannel::CLI:
        tag = "💻 CLI"; color = COLOR_BLUE; break;
    case LogChannel::ERROR_LOG:
        tag = "🔥 ERROR"; color = COLOR_RED; break;
    case LogChannel::SESSION:
        tag = "🔁 SESSION"; color = COLOR_WHITE; break;
    case LogChannel::SYSTEM:
        tag = "⚙️ SYSTEM"; color = COLOR_GRAY; break;
    }

    printWithColor(tag, msg, color);
}
