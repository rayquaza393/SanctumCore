#include "ScriptEngine.hpp"
#include "Logger.hpp"
#include "duktape.h"
#include <sstream>

nlohmann::json ScriptEngine::Execute(const std::string& scriptCode, const nlohmann::json& input) {
    duk_context* ctx = duk_create_heap_default();
    if (!ctx) {
        GlobalLogger.Error("Failed to create Duktape heap");
        return {
            {"type", "script.error"},
            {"success", false},
            {"reason", "Duktape heap creation failed"},
            {"data", nlohmann::json::object()}
        };
    }

    try {
        // Inject input JSON as global 'input'
        std::string inputJson = input.dump();
        duk_push_lstring(ctx, inputJson.c_str(), inputJson.length());
        duk_json_decode(ctx, -1);
        duk_put_global_string(ctx, "input");

        // Eval script
        if (duk_peval_string(ctx, scriptCode.c_str()) != 0) {
            std::string err = duk_safe_to_string(ctx, -1);
            duk_destroy_heap(ctx);
            return {
                {"type", "script.error"},
                {"success", false},
                {"reason", err},
                {"data", nlohmann::json::object()}
            };
        }

        // module.exports should now be on top of the stack
        duk_get_global_string(ctx, "module");
        duk_get_prop_string(ctx, -1, "exports");

        // Push args: conn (null), input, respond()
        duk_push_null(ctx); // conn stub
        duk_get_global_string(ctx, "input");

        // Push respond() shim
        duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t {
            nlohmann::json* out = reinterpret_cast<nlohmann::json*>(duk_get_heapptr(ctx, -1));
            *out = nlohmann::json::parse(duk_json_encode(ctx, 0));
            return 0;
            }, 1);

        // Create space for output
        nlohmann::json response;
        duk_push_pointer(ctx, &response);

        // Call exported function
        if (duk_pcall(ctx, 3) != 0) {
            std::string err = duk_safe_to_string(ctx, -1);
            duk_destroy_heap(ctx);
            return {
                {"type", "script.error"},
                {"success", false},
                {"reason", err},
                {"data", nlohmann::json::object()}
            };
        }

        duk_destroy_heap(ctx);
        return response;

    }
    catch (...) {
        duk_destroy_heap(ctx);
        return {
            {"type", "script.error"},
            {"success", false},
            {"reason", "Unhandled exception in ScriptEngine"},
            {"data", nlohmann::json::object()}
        };
    }
}
