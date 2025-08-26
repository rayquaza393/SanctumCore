const mysql = require("mysql2/promise");

const authConfig = {
    host: "localhost",
    user: "root",
    password: "K2rie6no0?",
    database: "auth"
};

const charConfig = {
    host: "localhost",
    user: "root",
    password: "K2rie6no0?",
    database: "characters"
};

const worldConfig = {
    host: "localhost",
    user: "root",
    password: "K2rie6no0?",
    database: "world-us01"
};

async function authDB() {
    return await mysql.createConnection(authConfig);
}

async function charDB() {
    return await mysql.createConnection(charConfig);
}

async function worldDB() {
    return await mysql.createConnection(worldConfig);
}

module.exports = {
    authDB,
    charDB,
    worldDB
};

