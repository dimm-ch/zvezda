#include "proc_cmd.hpp"
#include "tool_hub.hpp"
#include "server.hpp"

const std::unordered_map<std::string, CommandProcessor::cmd_handler> CommandProcessor::cmd_map {
    {"info", &CommandProcessor::info},
    {"version", &CommandProcessor::version},
    {"close", &CommandProcessor::close},
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
    init();
}

std::string CommandProcessor::command() {
    return "srv";
}

bool  CommandProcessor::init() {
    if (initBrdLibFlag) {
        printf("%s Reinitialization attempt detected. Library BARDY is already initialized.\n", printDebugName.c_str());
        return true;
    }

    std::string fileini = (getExePath().parent_path().parent_path() / "brd/brd.ini").lexically_normal().string();

    // инициализировать библиотеку
    S32 status = BRD_init(fileini.c_str(), &x_DevNum);
    if (!BRD_errcmp(status, BRDerr_OK)) {
        printf(_BRDC("<ERR> BARDY Initialization = 0x%X \n"), status);
        return false;
    }
    else {
        printf("<SRV> Library BARDY - is initialize, device found : %d \n", x_DevNum);
        BRD_displayMode(BRDdm_VISIBLE | BRDdm_CONSOLE); // режим вывода информационных сообщений : отображать все уровни на консоле
        initBrdLibFlag = true;
    }
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
    BRD_Version ver;
    BRD_version(x_handleDevice, &ver);
    printf("--------------------- INFO ------------------------- \n");
    printf("-- Version: Shell v.%d.%d, Driver v.%d.%d --\n", ver.brdMajor, ver.brdMinor, ver.drvMajor, ver.drvMinor);
    for (size_t i = 0; i < MAX_LID_DEVICES; i++) {
        if (DevicesLid[i].device.isOpen()) {
            U32 v = 0;
            printf("* LID=%d open in %s mode (handle=%X), ver.%d \n", i,
                getStrOpenModeDevice(DevicesLid[i].device.mode()).c_str(), DevicesLid[i].device.handle(), v);
            
        }
    }
    printf("---------------------------------------------------- \n");
    return true;
}

bool CommandProcessor::close(const json &request, json &response) {
    return false;
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
    if (!check_lid(response)) {
        return false;
    }

    commandLineParams paramReg;

    if (!get_int(request, "t", paramReg.tetrad) || paramReg.tetrad < 0) {
        response["error"] = "parametr error: t";
        printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str());
        return false;
    }

    if (!get_int(request, "r", paramReg.reg) || paramReg.reg < 0) {
        response["error"] = "parametr error: r";
        printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str());
        return false;
    }

    if (request.contains("dir")){
        if (!get_bool(request, "dir", paramReg.indirect)) {
            response["error"] = "parametr error: dir";
            printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str());
            return false;
        }
    }
    else
        paramReg.indirect = true;

    if (request.contains("val")){
        if (!get_uint(request, "val", paramReg.value)) {
            response["error"] = "parametr error: dir";
            printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str());
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
            printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str());
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
            printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str());
            return false;
        }
    }

    // Пишем/читаем регистр
    paramReg.hService = DevicesLid[lid].servRegs.handle();
    U32 S = processReg(paramReg);
    printf("%s: [%s] tetrada = %d, register = 0x%X, value = 0x%X\n",
            (S > 0) ? "<SRV> OK" : "<ERR> ERROR",
            paramReg.write ? "Write" : "Read",
            paramReg.tetrad,
            paramReg.reg,
            paramReg.value);
    if (S <= 0) {
        response["error"] = "error execution";
        printf("%s  reg(): %s\n", printDebugName.c_str(), response["error"].dump().c_str());
        return false;
    }

    if (!paramReg.write) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << paramReg.value;
        response["value"] = "0x" + ss.str();
    }

    return true;
}