#pragma once

#include <string>

#include "../work2/common/dev_util.h"
#include "exec_cmd.hpp"

class CommandProcessor final : public CommandExecutor {
    int indexDev;
    bool initBrdLibFlag = false;

public:
    CommandProcessor();
    ~CommandProcessor() = default;

    static std::string command();  

private:
    bool dispatch(const json& request, json& response);

    bool info(const json& request, json& response);
    bool version(const json& request, json& response);
    bool close(const json& request, json& response);
    bool pause(const json& request, json& response);
    bool reg(const json& request, json& response);

    bool init();
    using cmd_handler = bool (CommandProcessor::*)(const json&, json&);
    static const std::unordered_map<std::string, cmd_handler> cmd_map;
};