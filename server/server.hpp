#pragma once

#include <algorithm>
#include <arpa/inet.h>
#include <array>
#include <cassert>
#include <csignal>
#include <cstring>
#include <errno.h>
#include <future>
#include <ifaddrs.h>
#include <iomanip>
#include <map>
#include <memory>
#include <net/if.h>
#include <net/route.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>

#include "json.hpp"

// #include "adm2if.h"
#include "brd_string.h"
#include "brdapi.h"
#include "gipcy.h"

#include "ctrladc.h"
#include "ctrldac.h"
#include "ctrlreg.h"
#include "ctrlsysmon.h"

#include "../work2/common/dev_util.h"
#include "adc/adc_ctrl.h"
#include "dac/exam_edac.h"
#include "sync/fmc146v_sync.h"
#include "cmd/exec_cmd.hpp"
#include "get_json.hpp"

using namespace std::chrono_literals;
using namespace std::string_literals;

using namespace InSys;

using json = nlohmann::json;

extern S32 x_DevNum; // кол-во у-в
extern FuDevs DevicesLid[MAX_LID_DEVICES];
extern int x_lid;
extern BRD_Handle x_handleDevice;

static bool is_cancel { false };
static bool PrintDebugFlag { true };

enum CmdExcCtrl {
    UNDEF,
    DESC,
    END
};

struct ExcCtrl {
    ExcCtrl() : ctrl(CmdExcCtrl::UNDEF) {}
    CmdExcCtrl ctrl;
    std::string desc;
};

fs::path getExePath();

// Получить имя интерфейса, с которым можно работать
std::string get_interface();