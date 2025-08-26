#include "DuktapeBindings.hpp"

static MYSQL* boundDB = nullptr;

static duk_ret_t DB_query(duk_context* ctx) {
    if (!duk_is_string(ctx, 0)) {
        return duk_error(ctx, DUK_ERR_TYPE_ERROR, "First argument must be SQL query string.");
    }

    std::string query = duk_get_string(ctx, 0);

    std::vector<std::string> args;
    if (duk_is_array(ctx, 1)) {
        duk_size_t len = duk_get_length(ctx, 1);
        for (duk_uarridx_t i = 0; i < len; ++i) {
            duk_get_prop_index(ctx, 1, i);
            if (duk_is_string(ctx, -1)) {
                args.push_back(duk_get_string(ctx, -1));
            }
            else {
                args.push_back(duk_json_encode(ctx, -1));
            }
            duk_pop(ctx);
        }
    }

    for (const auto& arg : args) {
        size_t pos = query.find("?");
        if (pos == std::string::npos) break;
        query.replace(pos, 1, "'" + arg + "'");
    }

    if (mysql_query(boundDB, query.c_str()) != 0) {
        GlobalLogger.Error("MySQL query failed: " + std::string(mysql_error(boundDB)));
        duk_push_null(ctx);
        return 1;
    }

    MYSQL_RES* res = mysql_store_result(boundDB);
    if (!res) {
        duk_push_array(ctx);
        return 1;
    }

    MYSQL_ROW row;
    int num_fields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    duk_idx_t arr_idx = duk_push_array(ctx);
    int row_idx = 0;

    while ((row = mysql_fetch_row(res))) {
        duk_idx_t obj_idx = duk_push_object(ctx);
        for (int i = 0; i < num_fields; ++i) {
            duk_push_string(ctx, fields[i].name);
            duk_push_string(ctx, row[i] ? row[i] : "null");
            duk_put_prop(ctx, obj_idx);
        }
        duk_put_prop_index(ctx, arr_idx, row_idx++);
    }

    mysql_free_result(res);
    return 1;
}

void DuktapeBindings::InjectDatabaseAPI(duk_context* ctx, MYSQL* db) {
    boundDB = db;

    duk_push_object(ctx);
    duk_push_c_function(ctx, DB_query, 2);
    duk_put_prop_string(ctx, -2, "query");
    duk_put_global_string(ctx, "DB");
}
