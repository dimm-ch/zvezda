#pragma once

#include "brd.h"
#include "ctrlreg.h"
#include "ctrlstrm.h"
#include "gipcy.h"
#include "locale.h"
#include <iostream>
#include <string>

#ifdef __IPC_WIN__
#include "utypes.h"
#else
#include "utypes_linux.h"
#endif

typedef struct commandLineParams_ {
    BRD_Handle hService;
    bool write;
    S32 tetrad;
    S32 reg;
    U32 value;
    bool indirect;
} commandLineParams;

S32 regWriteIndir(BRD_Handle hSrv, S32 trdNo, S32 rgnum, U32 val);
U32 regReadIndir(BRD_Handle hSrv, S32 trdNo, S32 rgnum, S32& status);
S32 regWriteDir(BRD_Handle hSrv, S32 trdNo, S32 rgnum, U32 val);
U32 regReadDir(BRD_Handle hSrv, S32 trdNo, S32 rgnum, S32& status);
// SPD
U32 SpdRead(BRD_Handle hService, size_t tetr_num, size_t dev, size_t numb, size_t addr);
S32 SpdWrite(BRD_Handle hService, size_t tetr_num, size_t dev, size_t numb, size_t addr, size_t val);

std::string getStrOpenModeDevice(U32 mMode);
std::string getStrCaptureModeService(U32 nMode);

// Обёртка над RegXxx()
S32 processReg(commandLineParams& params);

// Обёртка над SPD доступом
void spdAccess(commandLineParams& params, BRD_Handle hDev);

//
void printInfo(commandLineParams params, BRD_Handle hDev);
U32 printConstRegsFromTetrads(BRD_Handle hService, BRD_Reg& reg);
