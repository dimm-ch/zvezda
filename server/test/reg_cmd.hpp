#pragma once

#include <string>

#include "json.hpp"
#include "../work2/common/dev_util.h"
#include "exec_cmd.hpp"

using json = nlohmann::json;

class FastRegsAccess final : public CommandExecutor {
    commandLineParams paramReg;
    int indexDev;

public:
    static std::string command();
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final;
    bool parse_value(const std::string& s, U32& out);
    bool parsingLine(std::string& line, commandLineParams& params);
};