// ScriptEngine.h
#pragma once
#include "json.hpp"

class ScriptEngine {
public:
    nlohmann::json Execute(const std::string& scriptCode, const nlohmann::json& input);
};

