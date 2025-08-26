const { charDB } = require("./DB_Connect");

let input = "";
process.stdin.on("data", chunk => input += chunk);
process.stdin.on("end", async () => {
    try {
        const request = JSON.parse(input);
        const accountId = request.account_id;
        const charId = request.character_id;

        if (!accountId || !charId)
            return output({ type: "PlayerCharacterSelect", success: false, reason: "Missing fields" });

        const conn = await charDB();

        const [rows] = await conn.execute(
            `SELECT id, name, class_id, level, scene, pos_x, pos_y, pos_z, rot_y 
             FROM playerchar 
             WHERE id = ? AND account_id = ?`,
            [charId, accountId]
        );

        await conn.end();

        if (rows.length === 0)
            return output({ type: "PlayerCharacterSelect", success: false, reason: "Invalid character or ownership mismatch" });

        const char = rows[0];

        output({
            type: "PlayerCharacterSelect",
            success: true,
            id: char.id,
            name: char.name,
            class_id: char.class_id,
            level: char.level,
            scene: char.scene || "World01",
            pos_x: parseFloat(char.pos_x),
            pos_y: parseFloat(char.pos_y),
            pos_z: parseFloat(char.pos_z),
            rot_y: parseFloat(char.rot_y)
        });

    } catch (err) {
        output({ type: "PlayerCharacterSelect", success: false, reason: "Server error", details: err.message });
    }
});

function output(obj) {
    process.stdout.write(JSON.stringify(obj));
}
