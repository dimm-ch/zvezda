#pragma once

#include "json.hpp"

#include "server.hpp"

using json = nlohmann::json;

inline json getSysmonRequest(int device, int instance)
{
    json request {};

    request["device"] = device;
    request["instance"] = instance;
    request["command"] = GetSysmonExecutor::command();

    return request;
}

inline json writeRegIndRequest(int device, int instance, const BRD_Reg& reg)
{
    json request {};

    request["device"] = device;
    request["instance"] = instance;
    request["command"] = WriteRegIndExecutor::command();
    request["tetr"] = reg.tetr;
    request["reg"] = reg.reg;
    request["val"] = reg.val;

    return request;
}

inline json readRegIndRequest(int device, int instance, const BRD_Reg& reg)
{
    json request {};

    request["device"] = device;
    request["instance"] = instance;
    request["command"] = ReadRegIndExecutor::command();
    request["tetr"] = reg.tetr;
    request["reg"] = reg.reg;

    return request;
}

inline json writeRegDirRequest(int device, int instance, const BRD_Reg& reg)
{
    json request {};

    request["device"] = device;
    request["instance"] = instance;
    request["command"] = WriteRegDirExecutor::command();
    request["tetr"] = reg.tetr;
    request["reg"] = reg.reg;
    request["val"] = reg.val;

    return request;
}

inline json readRegDirRequest(int device, int instance, const BRD_Reg& reg)
{
    json request {};

    request["device"] = device;
    request["instance"] = instance;
    request["command"] = ReadRegDirExecutor::command();
    request["tetr"] = reg.tetr;
    request["reg"] = reg.reg;

    return request;
}

inline json writeSpdRequest(int device, int instance, const BRD_Spd& reg)
{
    json request {};

    request["device"] = device;
    request["instance"] = instance;
    request["command"] = WriteSpdExecutor::command();
    request["dev"] = reg.dev;
    request["is32bits"] = reg.mode;
    request["num"] = reg.num;
    request["synchr"] = reg.sync;
    request["tetr"] = reg.tetr;
    request["reg"] = reg.reg;
    request["val"] = reg.val;

    return request;
}

inline json readSpdRequest(int device, int instance, const BRD_Spd& reg)
{
    json request {};

    request["device"] = device;
    request["instance"] = instance;
    request["command"] = ReadSpdExecutor::command();
    request["dev"] = reg.dev;
    request["is32bits"] = reg.mode;
    request["num"] = reg.num;
    request["synchr"] = reg.sync;
    request["tetr"] = reg.tetr;
    request["reg"] = reg.reg;

    return request;
}

inline json interCommand(std::string comm, int std::strin param)
{
    json request {};

    request["icom"] = comm;
    request["param"] = param;

    return request;
}
