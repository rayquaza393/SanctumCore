const { charDB } = require("./DB_Connect");

let input = "";
process.stdin.on("data", chunk => input += chunk);
process.stdin.on("end", async () => {
    try {
        const request = JSON.parse(input);
        const accountId = request.account_id;

        if (!accountId)
            return output({ type: "PlayerCharacterList", success: false, reason: "Missing account_id" });

        const conn = await charDB();

        const [rows] = await conn.execute(
            "SELECT id, name, class_id, level FROM playerchar WHERE account_id = ?",
            [accountId]
        );

        await conn.end();

        output({
            type: "PlayerCharacterList",
            success: true,
            characters: rows
        });

    } catch (err) {
        output({ type: "PlayerCharacterList", success: false, reason: "Server error", details: err.message });
    }
});

function output(obj) {
    process.stdout.write(JSON.stringify(obj));
}
