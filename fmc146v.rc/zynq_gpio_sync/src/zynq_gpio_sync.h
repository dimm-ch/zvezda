#pragma once
#include "bardy.h"
#include "brd_dev.h"
#include "exceptinfo.h"
#include "strconv.h"
#include "time_ipc.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <math.h>
#include <memory>
#include <mutex>
#include <string>
#include <map>
#include <thread>
#ifndef WIN32
	#include <pthread.h>
	#include <sys/time.h>
	#include <unistd.h>
#endif

#define TETR_ID 0x134
#define GPIO_REG 0x208 //8
#define VPX_GA 0x209 //geographics
#define VPX_DISC 0x20A //discrete signal

typedef union // REG_TST_IN 0x20E
{
    U16 asWhole;
    struct {
        U08 GPIO1_T : 1;
        U08 GPIO2_T : 1;
        U08 GPIO3_T : 1;
        U08 GPIO4_T : 1;
        U08 GPIO1_O : 1;
        U08 GPIO2_O : 1;
        U08 GPIO3_O : 1;
        U08 GPIO4_O : 1;
        U08 GPIO1_I : 1;
        U08 GPIO2_I : 1;
        U08 GPIO3_I : 1;
        U08 GPIO4_I : 1;
        U08 res : 4;
    }byBits;
} REG_GPIO, *PREG_GPIO;

enum class Mode {
    Set,
    Blink,
    Test,
    Help
};

struct Arguments {
    Mode mode = Mode::Help;
    int value = -1;
};

std::map<std::string, Arguments> arg_map = {
    { "--set", {Mode::Set, -1} },
    { "-s", {Mode::Set, -1} },
    { "--blink", {Mode::Blink, -1} },
    { "--test", {Mode::Test, -1} },
    { "--help", {Mode::Help, -1} },
    { "-h", {Mode::Help, -1} }
};

Arguments arg;
bool do_blink = false;
int value_to_set = -1;
S32 ulid = -1;
bool is_help = false;
S32 brd_count = 0;
brd_dev_t fpga = nullptr;
std::vector<U32> lids;
S32 fmc146_trd = -1;
REG_GPIO reg = {0};
U32 raw = 0;