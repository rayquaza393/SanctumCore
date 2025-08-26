#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mysql.h>
#include "Room.hpp" // assumes this now exists

struct Zone {
    std::string code;
    std::string name;
    std::string dbName;
    std::vector<Room> rooms;
};

class ZoneLoader {
public:
    static std::unordered_map<std::string, Zone> LoadZones(MYSQL* authConn);
};
