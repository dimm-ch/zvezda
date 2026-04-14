#pragma once

#include <string>

#include "json.hpp"

using json = nlohmann::json;

class CommandExecutor {
public:
    using UniquePtr = std::unique_ptr<CommandExecutor>;

    json execute(const json& request);

protected:
    virtual bool check_lid(json& response);

    virtual bool dispatch(const json& request, json& response) = 0;
    virtual std::string get_ini(const std::string& ini);

    bool start(const json& request, json& response);

    int lid;
    std::string toolName;
    std::string baseName;
    std::string printDebugName;
    int startTimeoutToolMs;
    int waitTimeoutToolS;
};
