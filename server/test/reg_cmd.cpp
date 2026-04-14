#include "reg_cmd.hpp"
#include "server.hpp"

std::string FastRegsAccess::command() {
    return "reg";
}

json FastRegsAccess::execute(const json& request) {
    json response = request;
    std::string message = "Parameter is missing: ";
    if (!request.contains("reg")) {
        response["error"] = message + "reg";
        return response;
    }

    std::string line = request["reg"];
    printf("Receive command reg > %s:  \n", line.c_str());

    if (!parsingLine(line, paramReg)) {
        printf("<ERR> .. the input >%s< is not recognized ..\n", line.c_str());
        response["error"] ="The input is not recognized (reg = ?)";
        return response;
    }

    // Определяем LID устройства
    std::string slid;
    x_lid = 1;
    if (request.contains("lid")) {
        slid = request["lid"];
    }
    if (!slid.empty()) {
        x_lid = atoi(slid.c_str());
        if (x_lid <= 0 || x_lid >= MAX_LID_DEVICES) {
            BRDC_printf("<ERR> Bad LID number!\n");
            response["error"] = "Bad LID!";
            return response;
        }
    }
    
    // Открываем устройство
    if(!DevicesLid[x_lid].device.isOpen()) {
        DevicesLid[x_lid].device.open(x_lid);
        if(!DevicesLid[x_lid].device.isOpen()) {
            BRDC_printf("<ERR> %s\n", DevicesLid[x_lid].device.error());
            response["error"] = DevicesLid[x_lid].device.error();
            return response;
        }
    }

    // Захватываем сервис доступа к регистрам
    if(!DevicesLid[x_lid].servRegs.isCaptured()) {
        //U32 mode = BRDcapt_EXCLUSIVE;
        //U32 mode = DevicesLid[x_lid].device.mode();
        U32 mode = BRDcapt_SHARED;
        DevicesLid[x_lid].servRegs.setMode(mode);
        x_handleDevice = DevicesLid[x_lid].device.handle();
        DevicesLid[x_lid].servRegs.capture(x_handleDevice, "REG0", 10000);
        if(!DevicesLid[x_lid].servRegs.isCaptured()){
            BRDC_printf("<ERR> %s\n", DevicesLid[x_lid].device.error());
            response["error"] = DevicesLid[x_lid].device.error();
            return response;
        }
    }

    // Пишем/читаем регистр
    paramReg.hService = DevicesLid[x_lid].servRegs.handle();
    U32 S = processReg(paramReg);
    printf("%s: [%s] tetrada = %d, register = 0x%X, value = 0x%X\n",
            (S > 0) ? "<SRV> OK" : "<ERR> ERROR",
            paramReg.write ? "Write" : "Read",
            paramReg.tetrad,
            paramReg.reg,
            paramReg.value);
    if (S <= 0) {
        response["error"] = "error execution ..";
        return response;
    }

    if (!paramReg.write) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << paramReg.value;
        response["value"] = "0x" + ss.str();
    }

    return response;
}

bool FastRegsAccess::parse_value(const std::string& s, U32& out) {
    try {
        size_t pos = 0;
        int base = 10;

        if (s.size() > 2 && (s[0] == '0') && (s[1] == 'x' || s[1] == 'X'))
            base = 16;

        out = std::stoul(s, &pos, base);
        return pos == s.size(); // весь токен должен быть числом
    } catch (...) {
        return false;
    }
}

bool FastRegsAccess::parsingLine(std::string& line, commandLineParams& params) {
    if (line.empty()) {
        return false;
    }
    
    bool write = false;
    bool indirect = true;
    U32 trd = 0;
    U32 reg = 0;
    U32 val = 0;

    auto p = line.find(':');
    if (p == std::string::npos) {
        return false;
    }

    // Тетрада
    if (!parse_value(line.substr(0, p), trd)) {
        return false;
    }
    
    //Тип регистра (косвенный или прямой)
    auto p_tmp = line.find(':', p + 1);
    if (p_tmp != std::string::npos) {
        std::string reg_type = line.substr(p + 1, p_tmp - p - 1);
        if (reg_type == "I" || reg_type == "i") {
            indirect = true;
        }
        else if (reg_type == "D" || reg_type == "d") {
            indirect = false;
        }
        else {
            return false;
        }

        p = p_tmp;
    }

    p_tmp = line.find('=', p + 1);

    // Регистр
    if (p_tmp == std::string::npos) {
        if (!parse_value(line.substr(p + 1), reg)) {
            return false;
        }
    }
    else {
        if (!parse_value(line.substr(p + 1, p_tmp - p - 1), reg)) {
            return false;
        }

        // Значение на запись
        if (!parse_value(line.substr(p_tmp + 1), val)) {
            return false;
        }

        write = true;
    }

    params.write = write;
    params.tetrad = trd;
    params.reg = reg;
    params.value = val;
    params.indirect = indirect;

    return true;
}
