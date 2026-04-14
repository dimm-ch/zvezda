#include "adc_cmd.hpp"
#include "tool_hub.hpp"
#include "server.hpp"

const std::unordered_map<std::string, AdcControl::cmd_handler> AdcControl::cmd_map {
    {"start", &AdcControl::start}
};

AdcControl::AdcControl() {
    baseName = "adc";
    printDebugName = "[AdcControl]";
    startTimeoutToolMs = -1;
    waitTimeoutToolS = 5;
}

std::string AdcControl::command() {
    return "adc";
}

size_t AdcControl::findTetrad() {
    return 4;
}

bool AdcControl::dispatch(const json &request, json &response) {
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

std::string AdcControl::get_ini(const std::string& ini) {
    return ini + " -b " + std::to_string(lid);
}
