#include "ScriptRunner.hpp"
#include "ScriptEngine.hpp"
#include "Logger.hpp"

#include <fstream>       // ✅ std::ifstream
#include <sstream>       // ✅ std::stringstream
#include <string>        // ✅ std::string
#include "json.hpp"  // ✅ JSON lib
nlohmann::json ScriptRunner::RunScript(const std::string& type, const nlohmann::json& input) {
    std::string scriptPath = "./extensions/" + type + ".js";
    std::ifstream file(scriptPath);

    if (!file.is_open()) {
        GlobalLogger.Error("[SCRIPT] Not found: " + scriptPath);
        return {
            {"type", type + ".error"},
            {"success", false},
            {"reason", "Script file not found: " + scriptPath},
            {"data", nlohmann::json::object()}
        };
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string scriptCode = buffer.str();
    ScriptEngine engine;
    nlohmann::json result = engine.Execute(scriptCode, input);

    if (!result.contains("type") || !result.contains("success") ||
        !result.contains("reason") || !result.contains("data")) {
        GlobalLogger.Warning("[SCRIPT] Malformed response from: " + type);
        return {
            {"type", type + ".error"},
            {"success", false},
            {"reason", "Script returned malformed packet"},
            {"data", nlohmann::json::object()}
        };
    }

    return result;
}
