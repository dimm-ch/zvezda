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
#include <thread>
#ifndef WIN32
	#include <pthread.h>
	#include <sys/time.h>
	#include <unistd.h>
#endif

#define MAIN_REG_TST_IN 0x20E
#define MAIN_REG_TST_OUT 0xE

typedef union // REG_TST_IN 0x20E
{
    U16 asWhole;
    struct {
        U08 res : 1;
        U08 LVDS_VPX1 : 1;
        U08 LVDS_VPX2 : 1;
        U08 LVDS_VPX3 : 1;
        U08 LVDS_VPX4 : 1;
        U08 LVDS_VPX5 : 1;
        U08 LVDS_VPX6 : 1;
        U08 res2 : 1;
        U08 LVCMOS_VPX1 : 1;
        U08 LVCMOS_VPX2 : 1;
        U08 LVCMOS_VPX3 : 1;
        U08 LVCMOS_VPX4 : 1;
        U08 LVCMOS_FPGA0 : 1;
        U08 LVCMOS_FPGA1 : 1;
        U08 LVCMOS_FPGA2 : 1;
        U08 LVCMOS_FPGA3 : 1;
    }byBits;
} REG_TST_IN, *PREG_TST_IN;

typedef union // REG_TST_OUT 0xE
{
    U16 asWhole;
    struct {
        U08 res : 1;
        U08 LVDS_VPX1 : 1;
        U08 LVDS_VPX2 : 1;
        U08 LVDS_VPX3 : 1;
        U08 LVDS_VPX4 : 1;
        U08 LVDS_VPX5 : 1;
        U08 LVDS_VPX6 : 1;
        U08 res2 : 1;
        U08 LVCMOS_VPX1 : 1;
        U08 LVCMOS_VPX2 : 1;
        U08 LVCMOS_VPX3 : 1;
        U08 LVCMOS_VPX4 : 1;
        U08 LVCMOS_FPGA0 : 1;
        U08 LVCMOS_FPGA1 : 1;
        U08 LVCMOS_FPGA2 : 1;
        U08 LVCMOS_FPGA3 : 1;
    }byBits;
} REG_TST_OUT, * PREG_TST_OUT;