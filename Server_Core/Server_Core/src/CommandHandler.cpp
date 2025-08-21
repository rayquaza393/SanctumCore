#include "CommandHandler.hpp"
#include "HashUtils.hpp"
#include "Logger.hpp"
#include <sstream>
#include <iostream>
#include <mysql.h>
#include <string>
#include <vector>

void handleCommand(const std::vector<std::string>& tokens, MYSQL* conn)
{
    if (tokens.empty()) return;

    std::vector<std::string> args = tokens;

    // Strip optional '/' from first token
    if (!args[0].empty() && args[0][0] == '/')
        args[0] = args[0].substr(1);

    // Top-level help
    if (args[0] == "help") {
        if (args.size() == 1) {
            std::cout << "\n=== Help: Command Categories ===\n";
            std::cout << "/help account - View account-related commands\n";
            std::cout << "exit          - Shut down the server (CLI only)\n";
        }
        else if (args[1] == "account") {
            std::cout << "\n=== /account Commands ===\n";
            std::cout << "/account create <user> <pass> <email>\n";
            std::cout << "/account login <user> <pass>\n";
            std::cout << "/account logout <user>\n";
            std::cout << "/account ban <user>\n";
            std::cout << "/account unban <user>\n";
            std::cout << "/account kick <user>\n";
            std::cout << "/account list\n";
            std::cout << "/account onlinelist\n";
        }
        else {
            std::cout << "[ERROR] No help available for: " << args[1] << "\n";
        }
        return;
    }

    // Handle /account <command>
    if (args[0] == "account") {
        if (args.size() >= 4 && args[1] == "create") {
            std::string username = args[2];
            std::string passwordHash = HashUtils::SHA256(args[3]);
            std::string email = args.size() >= 5 ? args[4] : "";

            std::string query = "INSERT INTO accounts (username, pass_hash, email) VALUES ('" +
                username + "', '" + passwordHash + "', '" + email + "')";

            if (mysql_query(conn, query.c_str()) == 0) {
                std::cout << "[OK] Account created: " << username << "\n";
            }
            else {
                std::cerr << "[FAIL] Could not create account: " << mysql_error(conn) << "\n";
            }
        }

        else if (args.size() >= 4 && args[1] == "login") {
            const std::string& username = args[2];
            const std::string passwordHash = HashUtils::SHA256(args[3]);

            std::string query = "SELECT pass_hash, is_banned FROM accounts WHERE username='" + username + "'";
            if (mysql_query(conn, query.c_str()) != 0) {
                std::cerr << "[LOGIN ERROR] MySQL query failed: " << mysql_error(conn) << "\n";
                return;
            }

            MYSQL_RES* res = mysql_store_result(conn);
            if (!res) {
                std::cerr << "[LOGIN ERROR] Failed to retrieve result set.\n";
                return;
            }

            MYSQL_ROW row = mysql_fetch_row(res);
            if (!row) {
                std::cout << "[LOGIN FAILED] Invalid credentials.\n";
                mysql_free_result(res);
                return;
            }

            if (passwordHash == row[0]) {
                if (std::stoi(row[1]) == 1) {
                    std::cout << "[LOGIN FAILED] Account is banned.\n";
                }
                else {
                    std::string update = "UPDATE accounts SET is_online=1, last_login=NOW() WHERE username='" + username + "'";
                    mysql_query(conn, update.c_str());
                    std::cout << "[LOGIN SUCCESS] Welcome, " << username << "!\n";
                }
            }
            else {
                std::cout << "[LOGIN FAILED] Invalid credentials.\n";
            }

            mysql_free_result(res);
        }


        else if (args.size() >= 3 && args[1] == "logout") {
            const std::string& username = args[2];
            std::string query = "UPDATE accounts SET is_online=0 WHERE username='" + username + "'";
            if (mysql_query(conn, query.c_str()) == 0) {
                std::cout << "[LOGOUT SUCCESS] " << username << " has been logged out.\n";
            }
        }

        else if (args.size() >= 3 && args[1] == "ban") {
            const std::string& username = args[2];
            std::string query = "UPDATE accounts SET is_banned=1 WHERE username='" + username + "'";
            if (mysql_query(conn, query.c_str()) == 0) {
                std::cout << "[BAN] " << username << " has been banned.\n";
            }
        }

        else if (args.size() >= 3 && args[1] == "unban") {
            const std::string& username = args[2];
            std::string query = "UPDATE accounts SET is_banned=0 WHERE username='" + username + "'";
            if (mysql_query(conn, query.c_str()) == 0) {
                std::cout << "[UNBAN] " << username << " has been unbanned.\n";
            }
        }

        else if (args.size() >= 3 && args[1] == "kick") {
            const std::string& username = args[2];
            std::string query = "UPDATE accounts SET is_online=0 WHERE username='" + username + "'";
            if (mysql_query(conn, query.c_str()) == 0) {
                std::cout << "[KICK] " << username << " has been kicked (marked offline).\n";
            }
        }

        else if (args.size() >= 2 && args[1] == "list") {
            std::string query = "SELECT username, email, is_banned, is_online FROM accounts";
            if (mysql_query(conn, query.c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                MYSQL_ROW row;
                std::cout << "\n[ALL ACCOUNTS]\n";
                while ((row = mysql_fetch_row(res))) {
                    std::cout << row[0] << " | " << row[1] << " | Banned: " << row[2] << " | Online: " << row[3] << "\n";
                }
                mysql_free_result(res);
            }
        }

        else if (args.size() >= 2 && args[1] == "onlinelist") {
            std::string query = "SELECT username FROM accounts WHERE is_online=1";
            if (mysql_query(conn, query.c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                MYSQL_ROW row;
                std::cout << "\n[ONLINE ACCOUNTS]\n";
                while ((row = mysql_fetch_row(res))) {
                    std::cout << "- " << row[0] << "\n";
                }
                mysql_free_result(res);
            }
        }

        else {
            std::cout << "[ERR] Unknown /account command.\n";
        }

        return;
    }

    // Unknown top-level command
    std::cout << "[ERR] Unknown command. Try /help\n";
}

// Entry point from CLI and socket
void handleCommand(MYSQL* conn, const std::string& input)
{
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token)
        tokens.push_back(token);

    handleCommand(tokens, conn);
}
