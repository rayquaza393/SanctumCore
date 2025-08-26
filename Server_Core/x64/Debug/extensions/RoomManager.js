const worldDB = require("./DB_Connect").worldDB;

const RoomManager = {
    rooms: {},             // roomName → room object
    sessionToRoom: new Map(), // conn.sessionId → roomName

    async loadRooms() {
        try {
            const [rows] = await worldDB().execute(
                `SELECT * FROM rooms WHERE is_active = 1`
            );

            for (const row of rows) {
                this.rooms[row.name] = {
                    room_id: row.room_id,
                    name: row.name,
                    players: []
                };
            }

            console.log(`[RoomManager] Loaded ${rows.length} room(s) from DB.`);
        } catch (err) {
            console.error("[RoomManager] Failed to load rooms:", err);
        }
    },

    getRoomByName(name) {
        return this.rooms[name] || null;
    },

    getPlayersInRoom(name) {
        const room = this.getRoomByName(name);
        return room ? room.players : [];
    },

    join(conn, roomName) {
        const room = this.getRoomByName(roomName);
        if (!room) return { success: false, reason: "Room not found." };

        // Leave previous room
        this.leave(conn);

        room.players.push(conn);
        this.sessionToRoom.set(conn.sessionId, roomName);

        console.log(`[RoomManager] ${conn.sessionId} joined room ${roomName}`);
        return { success: true, room: roomName };
    },

    leave(conn) {
        const currentRoomName = this.sessionToRoom.get(conn.sessionId);
        if (!currentRoomName) return;

        const room = this.getRoomByName(currentRoomName);
        if (room) {
            room.players = room.players.filter(p => p.sessionId !== conn.sessionId);
        }

        this.sessionToRoom.delete(conn.sessionId);
        console.log(`[RoomManager] ${conn.sessionId} left room ${currentRoomName}`);
    }
};

module.exports = RoomManager;
