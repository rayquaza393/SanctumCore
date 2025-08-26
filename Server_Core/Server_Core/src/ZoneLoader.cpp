#include "ZoneLoader.hpp"
#include <sstream>
#include "Logger.hpp"

std::unordered_map<std::string, Zone> ZoneLoader::LoadZones(MYSQL* authConn) {
    std::unordered_map<std::string, Zone> loadedZones;

    const char* zoneQuery = "SELECT z.zone_id, z.code, z.name, r.world_schema_name FROM zones z \
                            JOIN zone_registry r ON z.zone_id = r.zone_id";

    if (mysql_query(authConn, zoneQuery) != 0) {
        GlobalLogger.Error("Failed to query zones: " + std::string(mysql_error(authConn)));
        return loadedZones;
    }

    MYSQL_RES* result = mysql_store_result(authConn);
    if (!result) return loadedZones;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        int zoneId = atoi(row[0]);
        std::string code = row[1];
        std::string name = row[2];
        std::string dbName = row[3];

        GlobalLogger.Info("[BOOT] Loading ZONE: " + code + "...");

        MYSQL* worldConn = mysql_init(nullptr);
        if (!mysql_real_connect(worldConn, "localhost", "root", "", dbName.c_str(), 0, nullptr, 0)) {
            GlobalLogger.Error("  Failed to connect to world DB: " + std::string(mysql_error(worldConn)));
            continue;
        }

        std::vector<Room> rooms;
        if (mysql_query(worldConn, "SELECT name, scene, room_type, max_users FROM rooms")) {
            GlobalLogger.Warning("  Failed to load rooms: " + std::string(mysql_error(worldConn)));
        }
        else {
            MYSQL_RES* rres = mysql_store_result(worldConn);
            MYSQL_ROW rrow;
            while ((rrow = mysql_fetch_row(rres))) {
                Room room;
                room.name = rrow[0];
                room.scene = rrow[1];
                room.roomType = rrow[2];
                room.maxUsers = atoi(rrow[3]);
                rooms.push_back(room);
                GlobalLogger.Info("  [ROOM] " + room.name + " | Scene: " + room.scene + " | Users: " + std::to_string(room.maxUsers));
            }
            mysql_free_result(rres);
        }

        int npcCount = 0, spawnCount = 0;
        if (mysql_query(worldConn, "SELECT COUNT(*) FROM npc_template") == 0) {
            MYSQL_RES* nres = mysql_store_result(worldConn);
            MYSQL_ROW nrow = mysql_fetch_row(nres);
            npcCount = nrow ? atoi(nrow[0]) : 0;
            mysql_free_result(nres);
        }

        if (mysql_query(worldConn, "SELECT COUNT(*) FROM npc_spawns") == 0) {
            MYSQL_RES* sres = mysql_store_result(worldConn);
            MYSQL_ROW srow = mysql_fetch_row(sres);
            spawnCount = srow ? atoi(srow[0]) : 0;
            mysql_free_result(sres);
        }

        GlobalLogger.Info("  Loaded " + std::to_string(npcCount) + " NPC templates");
        GlobalLogger.Info("  Loaded " + std::to_string(spawnCount) + " NPC spawns");

        Zone zone;
        zone.code = code;
        zone.name = name;
        zone.dbName = dbName;
        zone.rooms = rooms;

        loadedZones[code] = zone;
        GlobalLogger.Success("[ZONE: " + code + "] Started!");

        mysql_close(worldConn);
    }

    mysql_free_result(result);
    return loadedZones;
}
