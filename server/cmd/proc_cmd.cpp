#include "proc_cmd.hpp"
#include "tool_hub.hpp"
#include "server.hpp"
#include "bardy_session.hpp"
#include "../../log_level.hpp"

const std::unordered_map<std::string, CommandProcessor::cmd_handler> CommandProcessor::cmd_map {
    {"info", &CommandProcessor::info},
    {"version", &CommandProcessor::version},
    {"start", &CommandProcessor::start},
    {"stop", &CommandProcessor::stop},
    {"pause", &CommandProcessor::pause},
    {"reg", &CommandProcessor::reg}
};

static bool get_int(const json& request, const char* key, int& outValue)
{
    if (!request.contains(key))
        return false;

    const auto& v = request.at(key);

    try {
        if (v.is_number_integer()) {
            outValue = v.get<int>();
            return true;
        }

        if (v.is_string()) {
            std::string s = v.get<std::string>();
            if (s.empty())
                return false;

            char* end = nullptr;
            errno = 0;

            long value = std::strtol(s.c_str(), &end, 0);

            if (errno != 0 || end == s.c_str() || *end != '\0')
                return false;

            if (value < INT_MIN || value > INT_MAX)
                return false;

            outValue = static_cast<int>(value);
            return true;
        }
    }
    catch (...) {
        return false;
    }

    return false;
}

static bool get_uint(const json& request, const char* key, unsigned int& outValue) {
    if (!request.contains(key))
        return false;

    const auto& v = request.at(key);

    try {
        if (v.is_number_unsigned()) {
            outValue = v.get<unsigned int>();
            return true;
        }

        if (v.is_string()) {
            std::string s = v.get<std::string>();
            if (s.empty())
                return false;

            char* end = nullptr;
            errno = 0;

            long value = std::strtol(s.c_str(), &end, 0);

            if (errno != 0 || end == s.c_str() || *end != '\0')
                return false;

            if (value < 0 || value > UINT_MAX)
                return false;

            outValue = static_cast<unsigned int>(value);
            return true;
        }
    }
    catch (...) {
        return false;
    }

    return false;
}

static bool get_bool(const json& request, const char* key, bool& outValue)
{
    if (!request.contains(key))
        return false;

    const auto& v = request.at(key);

    try {
        if (v.is_boolean()) {
            outValue = v.get<bool>();
            return true;
        }

        if (v.is_number_integer()) {
            int value = v.get<int>();
            outValue = static_cast<bool>(value);
            return true;
        }

        if (v.is_string()) {
            std::string s = v.get<std::string>();
            if (s.empty())
                return false;

            if (s == "true" || s == "TRUE") {
                outValue = true;
                return true;
            }
            else if (s == "false" || s == "FALSE") {
                outValue = false;
                return true;
            }
            else {
                char* end = nullptr;
                errno = 0;

                long value = std::strtol(s.c_str(), &end, 0);

                if (errno != 0 || end == s.c_str() || *end != '\0')
                    return false;

                outValue = static_cast<bool>(value);
                return true;
            }
        }
    }
    catch (...) {
        return false;
    }

    return false;
}

CommandProcessor::CommandProcessor() {
    baseName = "srv";
    printDebugName = "[CommandProcessor]";
}

std::string CommandProcessor::command() {
    return "srv";
}

bool CommandProcessor::start(const json &request, json &response) {
    return BardySession::inst().open(response);
}

bool CommandProcessor::dispatch(const json &request, json &response) {
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

bool CommandProcessor::info(const json &request, json &response) {
    response["status"] = BardySession::inst().state_name();
    response["devices"] = x_DevNum;
    response["open_lids"] = json::array();
    printf("--------------------- INFO ------------------------- \n");
    printf("Server status: %s\n", BardySession::inst().state_name());
    for (size_t i = 0; i < MAX_LID_DEVICES; i++) {
        if (DevicesLid[i].device.isOpen()) {
            response["open_lids"].push_back(i);
            printf("* LID=%zu open in %s mode\n", i,
                getStrOpenModeDevice(DevicesLid[i].device.mode()).c_str());
        }
    }
    printf("---------------------------------------------------- \n");
    return true;
}

bool CommandProcessor::stop(const json &request, json &response) {
    return BardySession::inst().close(response);
}

bool CommandProcessor::version(const json &request, json &response) {
    std::string ver = std::to_string(version_srv_hi) + "." + std::to_string(version_srv_lo);
    printf("<SRV> Server version %s \n", ver.c_str());
    response["version"] = ver;
    return true;
}

bool CommandProcessor::pause(const json &request, json &response) {
    int time = request.value("time", 0);
    printf("<SRV> Pause  %d ms \n", time);
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
    return true;
}

bool CommandProcessor::reg(const json &request, json &response) {
    if (!BardySession::inst().require_open(response)) return false;
    if (!check_lid(response)) {
        return false;
    }

    commandLineParams paramReg{};

    if (!get_int(request, "t", paramReg.tetrad) || paramReg.tetrad < 0) {
        response["error"] = "parametr error: t";
        ZVEZDA_DEBUG_LOG(printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str()));
        return false;
    }

    if (!get_int(request, "r", paramReg.reg) || paramReg.reg < 0) {
        response["error"] = "parametr error: r";
        ZVEZDA_DEBUG_LOG(printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str()));
        return false;
    }

    if (request.contains("dir")){
        const auto& dir = request.at("dir");
        const std::string mode = dir.is_string() ? dir.get<std::string>() : "";
        if (mode == "D" || mode == "d") {
            paramReg.regType = RegisterType::Direct;
        }
        else if (mode == "I" || mode == "i") {
            paramReg.regType = RegisterType::Indirect;
        }
        else if (mode == "I2" || mode == "i2") {
            paramReg.regType = RegisterType::Indirect2;
        }
        else {
            bool indirect;
            if (!get_bool(request, "dir", indirect)) {
                response["error"] = "parametr error: dir";
                ZVEZDA_DEBUG_LOG(printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str()));
                return false;
            }
            paramReg.regType = indirect ? RegisterType::Indirect : RegisterType::Direct;
        }
    }

    if (request.contains("val")){
        if (!get_uint(request, "val", paramReg.value)) {
            response["error"] = "parametr error: val";
            ZVEZDA_DEBUG_LOG(printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str()));
            return false;
        }
        paramReg.write = true;
    }
    else
        paramReg.write = false;

    // Открываем устройство
    if(!DevicesLid[lid].device.isOpen()) {
        DevicesLid[lid].device.open(lid);
        if(!DevicesLid[lid].device.isOpen()) {
            response["error"] = DevicesLid[lid].device.error();
            ZVEZDA_DEBUG_LOG(printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str()));
            return false;
        }
    }

    // Захватываем сервис доступа к регистрам
    if(!DevicesLid[lid].servRegs.isCaptured()) {
        U32 mode = BRDcapt_SHARED;
        DevicesLid[lid].servRegs.setMode(mode);
        x_handleDevice = DevicesLid[lid].device.handle();
        DevicesLid[lid].servRegs.capture(x_handleDevice, "REG0", 10000);
        if(!DevicesLid[lid].servRegs.isCaptured()){
            response["error"] = DevicesLid[lid].device.error();
            ZVEZDA_DEBUG_LOG(printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str()));
            return false;
        }
    }

    // Пишем/читаем регистр
    paramReg.hService = DevicesLid[lid].servRegs.handle();
    S32 status = processReg(paramReg);
    const bool success = BRD_errcmp(status, BRDerr_OK);
    printf("%s: [%s] mode = %s, tetrada = %d, register = 0x%X, value = 0x%X\n",
            success ? "<SRV> OK" : "<ERR> ERROR",
            paramReg.write ? "Write" : "Read",
            paramReg.regType == RegisterType::Indirect2 ? "I2" :
                (paramReg.regType == RegisterType::Indirect ? "I" : "D"),
            paramReg.tetrad,
            paramReg.reg,
            paramReg.value);
    if (!success) {
        response["error"] = "error execution";
        ZVEZDA_DEBUG_LOG(printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str()));
        return false;
    }

    if (!paramReg.write) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << paramReg.value;
        response["value"] = "0x" + ss.str();
    }

    return true;
}
