#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <mysql.h>
#include "json.hpp"
#include "Logger.hpp"
#include "CommandHandler.hpp"
#include "SocketServer.hpp"

using json = nlohmann::json;

int main()
{
    GlobalLogger.Info("=== Starting SanctumCore Server ===");

    // Load config.json
    GlobalLogger.Info("Loading config.json...");
    json config;
    std::ifstream configFile("config.json");
    if (!configFile.is_open())
    {
        GlobalLogger.Error("Could not open config.json. Make sure it's next to the executable or copied to the output directory.");
        return 1;
    }

    try {
        configFile >> config;
        GlobalLogger.Success("config.json loaded successfully.");
    }
    catch (const std::exception& e) {
        GlobalLogger.Error(std::string("Failed to parse config.json: ") + e.what());
        return 1;
    }

    if (config.empty()) {
        GlobalLogger.Error("config.json is empty or malformed.");
        return 1;
    }

    std::string dbHost = config["host"];
    int dbPort = config["port"];
    std::string dbUser = config["user"];
    std::string dbPass = config["password"];
    std::string dbName = config["database"];
    int socketPort = config["socket_port"];

    GlobalLogger.Info("Attempting database connection...");

    // Connect to MySQL
    MYSQL* conn = mysql_init(nullptr);
    if (!conn)
    {
        GlobalLogger.Error("mysql_init() failed.");
        return 1;
    }

    if (!mysql_real_connect(conn, dbHost.c_str(), dbUser.c_str(), dbPass.c_str(), dbName.c_str(), dbPort, nullptr, 0))
    {
        GlobalLogger.Error(std::string("mysql_real_connect() failed: ") + mysql_error(conn));
        mysql_close(conn);
        return 1;
    }

    GlobalLogger.Success("Connected to database: " + dbName);

    // Start socket server
    GlobalLogger.Info("Starting socket server on port " + std::to_string(socketPort) + "...");
    SocketServer server(socketPort, conn);
    if (!server.start())
    {
        GlobalLogger.Error("Failed to start SocketServer");
        mysql_close(conn);
        return 1;
    }

    // CLI loop
    GlobalLogger.Raw(R"(
\x1b[36m
========= SanctumCore CLI =========
***********************************
Type '/help' for a list of commands
Type 'exit' to shut down
\x1b[0m)");

    std::string input;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "exit")
        {
            GlobalLogger.Warning("Shutting down server...");
            break;
        }

        GlobalLogger.Info("Executing command: " + input);
        handleCommand(conn, input);
    }

    GlobalLogger.Info("Stopping socket server...");
    server.stop();

    GlobalLogger.Info("Closing MySQL connection...");
    mysql_close(conn);

    GlobalLogger.Success("SanctumCore server shut down cleanly.");
    return 0;
}
