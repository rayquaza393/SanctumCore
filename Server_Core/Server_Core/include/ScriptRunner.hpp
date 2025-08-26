#pragma once
#include <string>
#include "json.hpp"

class ScriptRunner {
public:
    static nlohmann::json RunScript(const std::string& scriptName, const nlohmann::json& inputJson);
};

