#pragma once

#include <duktape.h>
#include <mysql/mysql.h>

class DuktapeBindings {
public:
    static void InjectDatabaseAPI(duk_context* ctx, MYSQL* db);
};
