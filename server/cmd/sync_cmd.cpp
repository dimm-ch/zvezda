#include "sync_cmd.hpp"
#include "tool_hub.hpp"
#include "server.hpp"

const std::unordered_map<std::string, FmcSync::cmd_handler> FmcSync::cmd_map {
    {"start", &FmcSync::start}
};

FmcSync::FmcSync() {
    baseName = "sync";
    printDebugName = "[FmcSync]";
    startTimeoutToolMs = -1;
    waitTimeoutToolS = 10;
}

std::string FmcSync::command() {
    return "sync";
}

bool FmcSync::check_lid(json& response) {
    return true;
}

bool FmcSync::dispatch(const json &request, json &response) {
    std::string cmd = request[baseName];

    const auto it = cmd_map.find(cmd);
    if (it == cmd_map.end()) {
        response["error"] = "Unrecognized command!!";
        printf("%s Unrecognized command %s\n", printDebugName.c_str(), cmd.c_str());
        return false;
    }
    printf("%s Recognized command \"%s\"\n", printDebugName.c_str(), cmd.c_str());

    return (this->*(it->second))(request, response);
}

std::string FmcSync::get_ini(const std::string& ini) {
    return "--exam-ini " + ini;
}
