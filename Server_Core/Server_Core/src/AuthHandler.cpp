#include "AuthHandler.hpp"
#include "HashUtils.hpp"
#include <mysql.h>
#include <string>
#include "json.hpp"

using json = nlohmann::json;

json HandlePlayerLogin(MYSQL* conn, const json& request)
{
    json response;
    response["type"] = "login.failed"; // Default to failed response

    if (!request.contains("username") || !request.contains("password")) {
        response["reason"] = "Missing fields";
        return response;
    }

    std::string username = request["username"];
    std::string password = request["password"];
    std::string passwordHash = HashUtils::SHA256(password);

    std::string query = "SELECT id, username, is_banned FROM accounts WHERE username='" + username + "' AND pass_hash='" + passwordHash + "'";

    if (mysql_query(conn, query.c_str()) != 0) {
        response["reason"] = std::string("MySQL error: ") + mysql_error(conn);
        return response;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);

    if (!row) {
        response["reason"] = "Invalid credentials";
        mysql_free_result(res);
        return response;
    }

    bool isBanned = std::stoi(row[2]) == 1;

    if (isBanned) {
        response["reason"] = "Account is banned";
        mysql_free_result(res);
        return response;
    }

    // Success
    response["type"] = "login.success";
    response["accountId"] = std::stoi(row[0]);
    response["username"] = row[1];

    std::string update = "UPDATE accounts SET is_online=1, last_login=NOW() WHERE username='" + username + "'";
    mysql_query(conn, update.c_str());

    mysql_free_result(res);
    return response;
}
