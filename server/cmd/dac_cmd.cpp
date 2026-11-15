#include "dac_cmd.hpp"
#include "tool_hub.hpp"
#include "server.hpp"

const std::unordered_map<std::string, DacControl::cmd_handler> DacControl::cmd_map {
    {"start", &DacControl::start},
    {"stop", &DacControl::stop},
    {"nco", &DacControl::nco},
    {"ping", &DacControl::ping}
};

DacControl::DacControl() {
    baseName = "dac";
    printDebugName = "[DacControl]";
    startTimeoutToolMs = 15000;
    waitTimeoutToolS = 15;
}

std::string DacControl::command() {
    return "dac";
}

size_t DacControl::findTetrad() {
    return 7;
}

bool DacControl::dispatch(const json &request, json &response) {
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

std::string DacControl::get_ini(const std::string& ini) {
    return ini + " -b " + std::to_string(lid);
}

bool DacControl::ping(const json& request, json& response) {
    if (!check_lid(response)) {
        return false;
    }

    auto respOpt = ToolHub::inst().call(toolName, "ping", json(), 1000);
    if (!respOpt) {
        response["error"] = "call error";
        printf("%s  ping(): %s\n", printDebugName.c_str(), response["error"].dump().c_str());
        return false;
    }

    const json& resp = *respOpt;
    if (!resp["result"]) {
        response["error"] = resp.value("error", "Server json parser error");
        return false;
    }

    response["result"] = resp.value("value", "Server json parser error");
    return true;
}

bool DacControl::nco(const json& request, json& response) {
    if (!check_lid(response)) {
        return false;
    }

    constexpr int64_t kMinFreq = 0;
    constexpr int64_t kMaxFreq = 6000000000LL;

    if (!request.contains("freq")) {
        response["error"] = "parametr error: freq is missing";
        printf("%s  nco(): %s\n", printDebugName.c_str(), response["error"].dump().c_str());
        return false;
    }

    int64_t freq = -1;

    try {
        if (request["freq"].is_number_integer() || request["freq"].is_number_unsigned()) {
            freq = request["freq"].get<int64_t>();
        } else {
            response["error"] = "parametr error: freq must be integer";
            printf("%s  nco(): %s\n", printDebugName.c_str(), response["error"].dump().c_str());
            return false;
        }
    }
    catch (const std::exception&) {
        response["error"] = "parametr error: freq out of range";
        printf("%s  nco(): %s\n", printDebugName.c_str(), response["error"].dump().c_str());
        return false;
    }

    if (freq < kMinFreq || freq > kMaxFreq) {
        response["error"] = "parametr error: freq must be in range 0..6000000000 Hz";
        printf("%s  nco(): %s\n", printDebugName.c_str(), response["error"].dump().c_str());
        return false;
    }

    json args;
    args["freq"] = freq;
    args["ch"] = request.value("ch", 0);

    auto& hub = ToolHub::inst();

    auto respOpt = ToolHub::inst().call(toolName, "nco", args, 5000);
    if (!respOpt) {
        response["error"] = "call error";
        printf("%s  nco(): %s\n", printDebugName.c_str(), response["error"].dump().c_str());
        return false;
    }

    const json& resp = *respOpt;
    if (!resp["result"]) {
        response["error"] = resp.value("error", "Server json parser error");
        return false;
    }

    response["result"] = resp.value("value", "Server json parser error");
    return true;
}
