#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <iostream>

class Logger
{
public:
    explicit Logger(const std::string& filename);
    ~Logger();

    void Info(const std::string& message);
    void Warn(const std::string& message);
    void Error(const std::string& message);
    void Raw(const std::string& message);

private:
    std::ofstream logFile;
    std::mutex logMutex;

    void Write(const std::string& level, const std::string& message);
    std::string GetTimestamp();
};

extern Logger GlobalLogger;
