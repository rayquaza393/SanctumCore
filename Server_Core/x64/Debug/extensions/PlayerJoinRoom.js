const RoomManager = require("./RoomManager");

module.exports = async function PlayerJoinRoom(conn, data, respond) {
    const roomName = data.room;
    if (!roomName) {
        return respond({
            type: "room.join.failed",
            reason: "Missing room name."
        });
    }

    const result = RoomManager.join(conn, roomName);

    if (!result.success) {
        return respond({
            type: "room.join.failed",
            reason: result.reason || "Unable to join room."
        });
    }

    respond({
        type: "room.join.success",
        success: true,
        room: roomName
    });

    console.log(`[JOIN] ${conn.sessionId} successfully joined ${roomName}`);
};
