const fs = require("fs");
const crypto = require("crypto");

const authDB = require("./DB_Connect").authDB;
const charactersDB = require("./DB_Connect").charactersDB;
const worldDB = require("./DB_Connect").worldDB;

(async () => {
    try {
        const raw = fs.readFileSync(0, "utf-8"); // read stdin
        const data = JSON.parse(raw);

        const { username, password } = data;

        if (!username || !password) {
            return console.log(JSON.stringify({
                type: "login.failed",
                success: false,
                reason: "Missing username or password."
            }));
        }

        // 1. Look up account
        const [accounts] = await authDB().execute(
            `SELECT id, pass_hash FROM accounts WHERE username = ? LIMIT 1`, [username]
        );

        if (accounts.length === 0) {
            return console.log(JSON.stringify({
                type: "login.failed",
                success: false,
                reason: "Invalid credentials."
            }));
        }

        const acct = accounts[0];
        const hash = crypto.createHash("sha256").update(password).digest("hex");
        const passOK = hash === acct.pass_hash;

        if (!passOK) {
            return console.log(JSON.stringify({
                type: "login.failed",
                success: false,
                reason: "Invalid credentials."
            }));
        }

        const accountId = acct.id;

        // 2. Get character scene
        const [chars] = await charactersDB().execute(
            `SELECT scene FROM playerchar WHERE account_id = ? LIMIT 1`, [accountId]
        );

        const scene = chars.length > 0 ? chars[0].scene : "World01";

        // 3. Get room from world DB
        const [roomRows] = await worldDB().execute(
            `SELECT name FROM rooms WHERE name = ? LIMIT 1`, [scene]
        );

        const room = roomRows.length > 0 ? roomRows[0].name : "World01";

        // 4. Get zone info from meta → auth
        const [metaRows] = await worldDB().execute(`SELECT zone_id FROM meta LIMIT 1`);
        const zoneId = metaRows.length > 0 ? metaRows[0].zone_id : 1;

        const [zoneRows] = await authDB().execute(
            `SELECT name FROM zones WHERE zone_id = ? LIMIT 1`, [zoneId]
        );

        const zone = zoneRows.length > 0 ? zoneRows[0].name : "Sanctum-US";

        // 5. Success response
        const response = {
            type: "login.success",
            success: true,
            accountId,
            username,
            zone,
            room
        };

        console.log(`[LOGIN SUCCESS] ${username} (ID: ${accountId}) → Zone: ${zone}, Room: ${room}`);
        console.log(JSON.stringify(response));
    } catch (err) {
        console.error("[PlayerLogin] Fatal error:", err);
        console.log(JSON.stringify({
            type: "login.failed",
            success: false,
            reason: "Internal server error."
        }));
    }
})();
