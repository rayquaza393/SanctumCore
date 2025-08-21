#pragma once

#include <string>
#include <vector>
#include <mysql.h>
#include "json.hpp"

void handleCommand(MYSQL* conn, const std::string& input);
