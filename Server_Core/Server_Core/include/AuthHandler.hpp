#pragma once

#include <mysql.h>
#include "json.hpp"

nlohmann::json HandlePlayerLogin(MYSQL* conn, const nlohmann::json& request);
