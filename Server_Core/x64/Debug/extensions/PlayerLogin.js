module.exports = function(conn, data, respond) {
    if (!data.username || !data.password) {
        return respond({
            type: "login.failed",
            success: false,
            reason: "Missing username or password",
            data: {}
        });
    }

    let users = DB.query("SELECT id, pass_hash FROM accounts WHERE username = ? LIMIT 1", [data.username]);
    if (!users || users.length === 0) {
        return respond({
            type: "login.failed",
            success: false,
            reason: "Invalid credentials",
            data: {}
        });
    }

    let user = users[0];

    // For now, we won't rehash the password — this is just structural
    let accountId = user.id;
    let zone = "Sanctum-US";
    let room = "CharacterLobby";

    respond({
        type: "login.success",
        success: true,
        reason: "",
        data: {
            accountId,
            username: data.username,
            zone,
            room
        }
    });
};
