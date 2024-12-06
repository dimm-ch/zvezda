// brd_test.cpp : Defines the entry point for the console application.
//

// #include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
// #include <conio.h>
#include "gipcy.h"
#include <inttypes.h>
#include <time.h>

// TODO: reference additional headers your program requires here
#include "brd.h"
#include "ctrl.h"
#include "ctrladc.h"
#include "ctrldac.h"
#include "ctrlddc.h"
#include "ctrlstrm.h"
#include "ctrltest.h"
#include "icr.h"
// #include "ctrldspnode.h"
#include "ctrlgc5016.h"
#include "ctrlreg.h"
#include "ctrlsdram.h"

BRDCHAR iniFileName[MAX_PATH] = _BRDC("//brd_test.ini");
BRDCHAR sPldName[MAX_PATH];
BRDCHAR AdcSrvName[32];
BRDCHAR BrdIniName[MAX_PATH];
int InOutMode = 0;
int TestMode;
int TestOn;
int TestToMem;
int DebugOn = 0;
int SaveToFile;
int ErrorMsg;
int DaqMemMode;
int64_t ActiveZoneSize;
U32 all_error_num;
U32 error_num;
unsigned Seed;
ULONG MemoryAddress;
int NumCycles;
int iCycle = 0;
int MemRead;
int MemReadMode = 0;
ULONG AdcDrqFlag;
ULONG TstDrqFlag;
ULONG SdramDrqFlag;
int PldLoadForce;
int key_cycle;
int g_TstDma = 1;

unsigned long long pattern0;
unsigned long long pattern1;
BRDCHAR DspPldFile[MAX_PATH];

int DdcOn;
int IoDelay[8];
ULONG TestMask[4];

// S32 Testing(BRD_Handle handle, BRD_ServList* srv);
S32 AdcTesting(BRD_Handle hADC, BRD_Handle hTest, BRD_ServList* srv);
S32 DacTesting(BRD_Handle hADC, BRD_Handle hTest, BRD_ServList* srv);
S32 DdrInOutTesting(BRD_Handle hDdrIn, BRD_Handle hDdrOut);

void SetIoDelay(BRD_Handle handle);

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL /* constant vector a */
#define UMASK 0x80000000UL /* most significant w-r bits */
#define LMASK 0x7fffffffUL /* least significant r bits */
#define MIXBITS(u, v) (((u) & UMASK) | ((v) & LMASK))
#define TWIST(u, v) ((MIXBITS(u, v) >> 1) ^ ((v) & 1UL ? MATRIX_A : 0UL))

static unsigned long state[N]; /* the array for the state vector  */
static int left = 1;
static int initf = 0;
static unsigned long* next;

/* initializes state[N] with a seed */
void init_genrand(unsigned long s)
{
    int j;
    state[0] = s & 0xffffffffUL;
    for (j = 1; j < N; j++) {
        state[j] = (1812433253UL * (state[j - 1] ^ (state[j - 1] >> 30)) + j);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array state[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        state[j] &= 0xffffffffUL; /* for >32 bit machines */
    }
    left = 1;
    initf = 1;
}

static void next_state(void)
{
    unsigned long* p = state;
    int j;

    /* if init_genrand() has not been called, */
    /* a default initial seed is used         */
    if (initf == 0)
        init_genrand(5489UL);

    left = N;
    next = state;

    for (j = N - M + 1; --j; p++)
        *p = p[M] ^ TWIST(p[0], p[1]);

    for (j = M; --j; p++)
        *p = p[M - N] ^ TWIST(p[0], p[1]);

    *p = p[M - N] ^ TWIST(p[0], state[0]);
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
    unsigned long y;

    if (--left == 0)
        next_state();
    y = *next++;

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

void ReadIniFile(BRDCHAR* iniFileName)
{
    BRDCHAR iniFilePath[MAX_PATH];
    IPC_getCurrentDir(iniFilePath, sizeof(iniFilePath) / sizeof(BRDCHAR));
    // GetCurrentDirectory(sizeof(iniFilePath)/sizeof(BRDCHAR), iniFilePath);
    BRDC_strcat(iniFilePath, iniFileName);
    BRDCHAR Buffer[128];
    BRDCHAR* endptr;

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("PldFileName"), _BRDC("ambpcd_v10_adm212x200m.mcs"), sPldName, sizeof(sPldName), iniFilePath);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("PldLoadForce"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    PldLoadForce = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("BrdIniFileName"), _BRDC("brd.ini"), BrdIniName, sizeof(BrdIniName), iniFilePath);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("AdcServiceName"), _BRDC("ADC212X200M"), AdcSrvName, sizeof(AdcSrvName), iniFilePath);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("MemorySizeInMBytes"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath); // in MBytes
    ActiveZoneSize = BRDC_atoi64(Buffer) * 1024 * 1024;

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("MemoryAddressIn32BitWordsHex"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath); // in 32-bit words (hex)
    MemoryAddress = BRDC_strtoul(Buffer, &endptr, 16);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("Debug"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    DebugOn = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("InOutMode"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    InOutMode = BRDC_atoi(Buffer); // 0 - ввод, 1 - вывод

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("TestMode"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    TestMode = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("TestOn"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
    TestOn = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("TestMask0"), _BRDC("FFFFFFFF"), Buffer, sizeof(Buffer), iniFilePath);
    TestMask[0] = BRDC_strtoul(Buffer, &endptr, 16);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("TestMask1"), _BRDC("FFFFFFFF"), Buffer, sizeof(Buffer), iniFilePath);
    TestMask[1] = BRDC_strtoul(Buffer, &endptr, 16);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("TestMask2"), _BRDC("FFFFFFFF"), Buffer, sizeof(Buffer), iniFilePath);
    TestMask[2] = BRDC_strtoul(Buffer, &endptr, 16);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("TestMask3"), _BRDC("FFFFFFFF"), Buffer, sizeof(Buffer), iniFilePath);
    TestMask[3] = BRDC_strtoul(Buffer, &endptr, 16);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("Pattern0"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    pattern0 = BRDC_strtoui64(Buffer, &endptr, 16);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("Pattern1"), _BRDC("FFFFFFFFFFFFFFFF"), Buffer, sizeof(Buffer), iniFilePath);
    pattern1 = BRDC_strtoui64(Buffer, &endptr, 16);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("SaveToFile"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    SaveToFile = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("ErrorMsg"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
    ErrorMsg = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("DaqIntoMemory"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    DaqMemMode = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("TstDrqFlag"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
    TstDrqFlag = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("AdcDrqFlag"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    AdcDrqFlag = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("SdramDrqFlag"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    SdramDrqFlag = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("NumCycles"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
    NumCycles = BRDC_atoi(Buffer);
    if (!NumCycles)
        NumCycles = 1;

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("KeyCycle"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    key_cycle = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("MemRead"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
    MemRead = BRDC_atoi(Buffer);
    if (!MemRead)
        MemRead = 1;

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("MemReadMode"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    MemReadMode = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("TestToMem"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
    TestToMem = BRDC_atoi(Buffer);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("TstDmaEnable"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
    g_TstDma = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("DdcOn"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    DdcOn = BRDC_atoi(Buffer);

    BRDCHAR ParamName[128];
    for (int i = 0; i < 8; i++) {
        BRDC_sprintf(ParamName, _BRDC("IoDelay%d"), i);
        IPC_getPrivateProfileString(_BRDC("Option"), ParamName, _BRDC("128"), Buffer, sizeof(Buffer), iniFilePath);
        IoDelay[i] = BRDC_atoi(Buffer);
    }
}

int BRDC_main(int argc, BRDCHAR* argv[])
{
    S32 status;

    IPC_initKeyboard();

    if (argc > 1) {
        BRDC_sprintf(iniFileName, _BRDC("//%s"), argv[1]);
    }
    ReadIniFile(iniFileName);

    BRD_displayMode(BRDdm_VISIBLE | BRDdm_CONSOLE);

    //	BRD_Error* pErrInfo = new BRD_Error;
    //	status = BRD_error(&pErrInfo);
    S32 DevNum;
    //	status = BRD_initEx(BRDinit_FILE | BRDinit_AUTOINIT, "brd__.ini", NULL, &DevNum);
    //	status = BRD_init(_BRDC("brd.ini"), &DevNum);
    status = BRD_init(BrdIniName, &DevNum);

    //	status = BRD_error(&pErrInfo);
    BRD_LidList lidList;
    lidList.item = 10;
    lidList.pLID = new U32[10];
    status = BRD_shell(BRDshl_LID_LIST, &lidList);
    //	status = BRD_error(&pErrInfo);

    BRD_Info info;
    info.size = sizeof(info);
    BRD_Handle handle[10];

    for (ULONG i = 0; i < lidList.itemReal; i++) {
        BRD_getInfo(lidList.pLID[i], &info);
        BRDC_printf(_BRDC("Board: %s, DevID = 0x%x, RevID = 0x%x, Bus = %d, Dev = %d, Slot = %d.\n"),
            info.name, info.boardType >> 16, info.boardType & 0xff, info.bus, info.dev, info.slot);
        // ULONG DevId = info.boardType >> 16;
        // if(DevId == 0x5503)
        //	g_TstDma = 0;
        U32 open_mode;
        //		handle[i] = BRD_open(lidList.pLID[i], BRDopen_EXCLUSIVE, &open_mode);
        //		if(handle[i] > 0 && BRDopen_EXCLUSIVE == open_mode)
        handle[i] = BRD_open(lidList.pLID[i], BRDopen_SHARED, &open_mode);
        if (handle[i] > 0 && BRDopen_SHARED == open_mode) {
            BRD_PuList PuList[8];
            U32 ItemReal;
            status = BRD_puList(handle[i], PuList, 8, &ItemReal);
            if (ItemReal <= 8) {
                for (U32 j = 0; j < ItemReal; j++) {
                    BRDC_printf(_BRDC("PuList_%d: %s, Id = %d, Code = %x, Attr = %x.\n"),
                        j, PuList[j].puDescription, PuList[j].puId, PuList[j].puCode, PuList[j].puAttr);
                    if (PuList[j].puCode == PLD_CFG_TAG && PuList[j].puId == 0x100) {
                        U32 PldState;
                        status = BRD_puState(handle[i], PuList[j].puId, &PldState);
                        BRDC_printf(_BRDC("BRD_puState: ADM PLD State = %d\n"), PldState);
                        if (PldLoadForce || !PldState) {
                            BRDCHAR msg[255];
                            BRDC_sprintf(msg, _BRDC("PLD file name: %s\n"), sPldName);
                            BRDC_printf(_BRDC("%s"), msg);
                            BRDC_printf(_BRDC("BRD_puLoad: loading...\r"));
                            // LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
                            // int bHighRes = QueryPerformanceFrequency (&Frequency);
                            // QueryPerformanceCounter (&StartPerformCount);
                            IPC_TIMEVAL start, stop;
                            IPC_getTime(&start);
                            status = BRD_puLoad(handle[i], PuList[j].puId, sPldName, &PldState);
                            // QueryPerformanceCounter (&StopPerformCount);
                            // double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
                            IPC_getTime(&stop);
                            double msTime = IPC_getDiffTime(&start, &stop);
                            if (BRD_errcmp(status, BRDerr_OK))
                                BRDC_printf(_BRDC("BRD_puLoad: load is OK. Loading time is %.2f ms\n"), msTime);
                            else
                                BRDC_printf(_BRDC("BRD_puLoad: error loading. Loading time is %.2f ms\n"), msTime);
                        }
                    }
                }
            }

            if (BRD_errcmp(status, BRDerr_OK)) {
                // SetIoDelay(handle[i]); // программирование линий задержки

                BRD_ServList srvList[20];
                status = BRD_serviceList(handle[i], 0, srvList, 20, &ItemReal);
                if (ItemReal > 20)
                    ItemReal = 20;
                {
                    for (U32 j = 0; j < ItemReal; j++) {
                        BRDC_printf(_BRDC("srvList_%d: %s, Attr = %X.\n"),
                            j, srvList[j].name, srvList[j].attr);
                    }
                }

                if (InOutMode == 3) // тест через тетрады памяти
                {
                    BRD_Handle hDdrIn = NULL;
                    BRD_Handle hDdrOut = NULL;
                    int flag = 0;
                    for (U32 iSrv = 0; iSrv < ItemReal; iSrv++) {
                        if (BRDC_strstr(srvList[iSrv].name, _BRDC("BASESDRAM"))) {
                            if (srvList[iSrv].attr & BRDserv_ATTR_DIRECTION_IN) {
                                U32 mode = BRDcapt_EXCLUSIVE;
                                hDdrIn = BRD_capture(handle[i], 0, &mode, srvList[iSrv].name, 10000);
                                if (mode == BRDcapt_EXCLUSIVE) {
                                    BRDC_printf(_BRDC("%s Capture mode: EXCLUSIVE (%X)\n"), srvList[iSrv].name, hDdrIn);
                                    flag++;
                                }
                            }
                            if (srvList[iSrv].attr & BRDserv_ATTR_DIRECTION_OUT) {
                                U32 mode = BRDcapt_EXCLUSIVE;
                                hDdrOut = BRD_capture(handle[i], 0, &mode, srvList[iSrv].name, 10000);
                                if (mode == BRDcapt_EXCLUSIVE) {
                                    BRDC_printf(_BRDC("%s Capture mode: EXCLUSIVE (%X)\n"), srvList[iSrv].name, hDdrOut);
                                    flag++;
                                }
                            }
                        }
                        if (flag == 2)
                            break;
                    }
                    status = DdrInOutTesting(hDdrIn, hDdrOut);
                    status = BRD_release(hDdrIn, 0);
                    status = BRD_release(hDdrOut, 0);
                    status = BRD_close(handle[i]);
                    BRDC_printf(_BRDC("BRD_close: OK\n"));
                    break;
                }

                U32 iAdcSrv = 0;
                for (U32 iSrv = 0; iSrv < ItemReal; iSrv++) {
                    BRDCHAR srv_name[32];
                    BRDC_sprintf(srv_name, _BRDC("%s%d"), AdcSrvName, iAdcSrv);
                    if (!BRDC_strcmp(srvList[iSrv].name, srv_name)) {
                        if (SaveToFile) {
                            FILE* fstream = NULL;
                            fstream = BRDC_fopen(_BRDC("0.txt"), _BRDC("w+t"));
                            if (fstream)
                                fclose(fstream);
                        }
                        U32 mode = BRDcapt_EXCLUSIVE;
                        BRD_Handle hADC = BRD_capture(handle[i], 0, &mode, srv_name, 10000);
                        if (mode == BRDcapt_EXCLUSIVE)
                            BRDC_printf(_BRDC("%s Capture mode: EXCLUSIVE (%X)\n"), srv_name, hADC);
                        if (mode == BRDcapt_SPY)
                            BRDC_printf(_BRDC("%s Capture mode: SPY (%X)\n"), srv_name, hADC);
                        BRD_Handle hTest = BRD_capture(handle[i], 0, &mode, _BRDC("TEST0"), 10000);
                        if (mode == BRDcapt_EXCLUSIVE)
                            BRDC_printf(_BRDC("TEST Capture mode: EXCLUSIVE (%X)\n"), hTest);
                        if (mode == BRDcapt_SPY) {
                            BRDC_printf(_BRDC("TEST Capture mode: SPY (%X)\n"), hTest);
                            status = BRD_release(hTest, 0);
                            status = BRD_release(hADC, 0);
                            hADC = 0;
                            hTest = 0;
                        }
                        if (hADC > 0 && hTest > 0) {
                            for (iCycle = 0; iCycle < NumCycles; iCycle++) {
                                //							status = Testing(handle[i], &srvList[iSrv]);
                                if (InOutMode)
                                    status = DacTesting(hADC, hTest, &srvList[iSrv]);
                                else
                                    status = AdcTesting(hADC, hTest, &srvList[iSrv]);
                                if (key_cycle) {
                                    BRDC_printf(_BRDC("Press any key to continue...\n"));
                                    IPC_getch();
                                }
                            }
                            status = BRD_release(hTest, 0);
                            status = BRD_release(hADC, 0);
                        }
                        iAdcSrv++;
                    }
                }
            }
            status = BRD_close(handle[i]);
            BRDC_printf(_BRDC("BRD_close: OK\n"));
        }
        // else
        //{
        //	if(BRDopen_EXCLUSIVE != open_mode)
        //		BRDC_printf(_BRDC("BRD_open: Device open too!\n");
        //	else
        //		BRDC_printf(_BRDC("BRD_open: error by open!\n");
        // }
    }
    status = BRD_cleanup();
    BRDC_printf(_BRDC("BRD_cleanup: OK\n"));
    if (SaveToFile && !ErrorMsg)
        BRDC_printf(_BRDC("Goodbye!!!\n"));
    else {
        BRDC_printf(_BRDC("Press any key for leaving program...\n"));
        IPC_getch();
    }
    IPC_cleanupKeyboard();

    return 0;
}

void DisplayError(S32 status, const BRDCHAR* cmd_str)
{
    S32 real_status = BRD_errext(status);
    BRDCHAR msg[255];
    switch (real_status) {
    case BRDerr_OK:
        BRDC_sprintf(msg, _BRDC("%s: BRDerr_OK\n"), cmd_str);
        break;
    case BRDerr_BAD_MODE:
        BRDC_sprintf(msg, _BRDC("%s: BRDerr_BAD_MODE\n"), cmd_str);
        break;
    case BRDerr_INSUFFICIENT_SERVICES:
        BRDC_sprintf(msg, _BRDC("%s: BRDerr_INSUFFICIENT_SERVICES\n"), cmd_str);
        break;
    default:
        BRDC_sprintf(msg, _BRDC("%s: Unknown error\n"), cmd_str);
        break;
    }
    BRDC_printf(_BRDC("%s"), msg);
}

void SetIoDelay(BRD_Handle handle)
{
    S32 status;
    U32 mode = BRDcapt_SHARED;
    BRD_Handle hReg = BRD_capture(handle, 0, &mode, _BRDC("REG0"), 10000);
    if (hReg <= 0) {
        BRDC_printf(_BRDC("REG NOT capture (%X)\n"), hReg);
        return;
    }
    //	if(mode == BRDcapt_EXCLUSIVE)
    //		BRDC_printf(_BRDC("REG Capture mode: EXCLUSIVE (%X)\n", hReg);
    if (mode == BRDcapt_SHARED)
        BRDC_printf(_BRDC("REG Capture mode: SHARED (%X)\n"), hReg);
    else
        return;
    // if(mode == BRDcapt_SPY)	BRDC_printf(_BRDC("REG Capture mode: SPY (%X)\n", hReg);
    BRD_Reg regdata;
    // сначала находим тетраду памяти
    regdata.reg = 0x100;
    S32 iTetr = 0;
    for (iTetr = 0; iTetr < 8; iTetr++) {
        regdata.tetr = iTetr;
        status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
        if (!BRD_errcmp(status, BRDerr_OK))
            BRDC_printf(_BRDC("Error operation: Tetr = 0x%04X, Reg = 0x%04X\n"), regdata.tetr, regdata.reg);
        if (regdata.val == 0x6F || // SDRAM на PEX2
            regdata.val == 0x77 || // SDRAM на FMC105P
            regdata.val == 0x8F || // SDRAM на FMC106P
            regdata.val == 0xB3) // SDRAM на FMC106P (OUTPUT)
            break;
    }
    if (iTetr == 8) {
        BRDC_printf(_BRDC("SetIoDelay: SDRAM (0x6F | 0x77 | 0x8F | 0xB3) tetrad is NOT FOUND!!\n"));
        status = BRD_release(hReg, 0);
        return;
    }
    for (int iLine = 0; iLine < 8; iLine++) {
        if (IoDelay[iLine] != 128) {
            regdata.reg = 0x2F0 + iLine;
            regdata.val = 2; // сброс задержки
            status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);
            // BRDC_printf(_BRDC("IoDelay: reg=0x%X val=0x%X\n", regdata.reg, regdata.val);
            if (IoDelay[iLine]) {
                regdata.val = (IoDelay[iLine] > 0) ? 1 : 0;
                int num = abs(IoDelay[iLine]);
                for (int i = 0; i < num; i++) {
                    status = BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &regdata);
                    // BRDC_printf(_BRDC("IoDelay: reg=0x%X val=0x%X\n", regdata.reg, regdata.val);
                }
            }
        }
    }
    status = BRD_release(hReg, 0);
}

void ReadCalibrReg(BRD_Handle handle)
{
    S32 status;
    U32 mode = BRDcapt_SHARED;
    BRD_Handle hReg = BRD_capture(handle, 0, &mode, _BRDC("REG0"), 10000);
    if (hReg <= 0) {
        BRDC_printf(_BRDC("REG NOT capture (%X)\n"), hReg);
        return;
    }
    //	if(mode == BRDcapt_EXCLUSIVE)
    //		BRDC_printf(_BRDC("REG Capture mode: EXCLUSIVE (%X)\n", hReg);
    if (mode == BRDcapt_SHARED)
        BRDC_printf(_BRDC("REG Capture mode: SHARED (%X)\n"), hReg);
    else
        return;
    // if(mode == BRDcapt_SPY)	BRDC_printf(_BRDC("REG Capture mode: SPY (%X)\n", hReg);
    BRD_Reg regdata;
    // сначала находим тетраду памяти
    regdata.reg = 0x100;
    S32 iTetr = 0;
    for (iTetr = 0; iTetr < 8; iTetr++) {
        regdata.tetr = iTetr;
        status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
        if (!BRD_errcmp(status, BRDerr_OK))
            BRDC_printf(_BRDC("Error operation: Tetr = 0x%04X, Reg = 0x%04X\n"), regdata.tetr, regdata.reg);
        if (regdata.val == 0x6F || // SDRAM на PEX2
            regdata.val == 0x77 || // SDRAM на FMC105P
            regdata.val == 0x8F || // SDRAM на FMC106P
            regdata.val == 0xB3) // SDRAM на FMC106P (OUTPUT)
            break;
    }
    if (iTetr == 8) {
        BRDC_printf(_BRDC("ReadCalibrReg: SDRAM (0x6F | 0x77 | 0x8F | 0xB3) tetrad is NOT FOUND!!\n"));
        status = BRD_release(hReg, 0);
        return;
    }
    BRDC_printf(_BRDC("IODelay registers: "));
    for (int iLine = 0; iLine < 8; iLine++) {
        regdata.reg = 0x2F0 + iLine;
        // regdata.val = 2; // сброс задержки
        status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
        BRDC_printf(_BRDC(" %02X,"), regdata.val & 0xFF);
    }
    BRDC_printf(_BRDC("\b \n")); // '\b ' - backspace и пробел чтобы убрать последнюю запятую
    status = BRD_release(hReg, 0);
}

void SetTestBuffer(PVOID pBuf, U32 sizeBuf)
{
    switch (TestMode) {
    case 0: //
    {
        BRDC_printf(_BRDC("Counter test\n"));
        // short* buf_tst = (short*)pBuf;
        // for(U32 i = 0; i < sizeBuf / sizeof(short); i++)
        //	buf_tst[i] = (short)i;
        int* buf_tst = (int*)pBuf;
        for (U32 i = 0; i < sizeBuf / sizeof(int); i++)
            buf_tst[i] = (int)i;
    } break;
    case 5: //
    {
        BRDC_printf(_BRDC("64-bit Counter test\n"));
        int64_t* buf_tst = (int64_t*)pBuf;
        for (U32 i = 0; i < sizeBuf / sizeof(int64_t); i++)
            buf_tst[i] = (int64_t)i;
    } break;
    case 1: //
    {
        BRDC_printf(_BRDC("Running 0 test\n"));
        int64_t* buf_tst = (int64_t*)pBuf;
        U32 cnt = sizeBuf / sizeof(int64_t) / 64;
        U32 k = 0;
        for (U32 i = 0; i < cnt; i++) {
            int64_t mask = 0xfffffffffffffffe;
            for (U32 j = 0; j < 64; j++) {
                buf_tst[k++] = mask;
                mask = (mask << 1) + 1;
            }
        }
    } break;
    case 2: //
    {
        BRDC_printf(_BRDC("Running 1 test\n"));
        int64_t* buf_tst = (int64_t*)pBuf;
        U32 cnt = sizeBuf / sizeof(int64_t) / 64;
        U32 k = 0;
        for (U32 i = 0; i < cnt; i++) {
            int64_t mask = 1;
            for (U32 j = 0; j < 64; j++) {
                buf_tst[k++] = mask;
                mask = (mask << 1) + 1;
            }
        }
    } break;
    case 3: //
    {
        BRDC_printf(_BRDC("Random test\n"));
        // установка начального значения для функции генерации псевдослучайных чисел, которая формирует тестовые шаблоны
        Seed = (unsigned)time(NULL);
        srand(Seed);
        //		short* buf_tst = (short*)pBuf;
        //		for(U32 i = 0; i < sizeBuf / sizeof(short); i++)
        //			buf_tst[i] = (short)rand();
        UCHAR* buf_tst = (UCHAR*)pBuf;
        for (U32 i = 0; i < sizeBuf; i++)
            buf_tst[i] = (UCHAR)rand();
    } break;
    case 6: //
    {
        BRDC_printf(_BRDC("Mersenne Twister Random test\n"));
        // установка начального значения для функции генерации псевдослучайных чисел, которая формирует тестовые шаблоны
        Seed = (unsigned)time(NULL);
        init_genrand(Seed);
        USHORT* buf_tst = (USHORT*)pBuf;
        for (U32 i = 0; i < sizeBuf / sizeof(USHORT); i++)
            buf_tst[i] = (USHORT)genrand_int32();
    } break;
    case 4: //
    {
        BRDC_printf(_BRDC("Patterns test\n"));
        unsigned long long* buf_tst = (unsigned long long*)pBuf;
        U32 cnt = sizeBuf / sizeof(U64);
        for (U32 i = 0; i < cnt; i++) {
            buf_tst[i++] = pattern0;
            buf_tst[i] = pattern1;
        }
    }
    default:
        break;
    }
}

void ContinueTestBuffer(PVOID pBuf, U32 sizeBuf, short index)
{
    switch (TestMode) {
    case 0: //
    {
        int* buf_tst = (int*)pBuf;
        int counter = int((U32)index * (sizeBuf / sizeof(int)));
        for (U32 i = 0; i < sizeBuf / sizeof(int); i++)
            buf_tst[i] = counter++;
    } break;
    case 5: //
    {
        int64_t* buf_tst = (int64_t*)pBuf;
        int64_t counter = int64_t((U32)index * (sizeBuf / sizeof(int64_t)));
        for (U32 i = 0; i < sizeBuf / sizeof(int64_t); i++)
            buf_tst[i] = counter++;
    } break;
        // бегущими "0" и "1" продолжать заполнять не надо
    case 1: //
    case 2: //
        break;
    case 3: //
    {
        //		short* buf_tst = (short*)pBuf;
        //		for(U32 i = 0; i < sizeBuf / sizeof(short); i++)
        //			buf_tst[i] = (short)rand();
        UCHAR* buf_tst = (UCHAR*)pBuf;
        for (U32 i = 0; i < sizeBuf; i++)
            buf_tst[i] = (UCHAR)rand();
    } break;
    case 6: //
    {
        USHORT* buf_tst = (USHORT*)pBuf;
        for (U32 i = 0; i < sizeBuf / sizeof(USHORT); i++)
            buf_tst[i] = (USHORT)genrand_int32();
    } break;
    default:
        break;
    }
}

void TestBufferForComp(PVOID pBuf, U32 sizeBuf, short index)
{
    switch (TestMode) {
    case 0: //
    {
        int* buf_tst = (int*)pBuf;
        int counter = int((U32)index * (sizeBuf / sizeof(int)));
        for (U32 i = 0; i < sizeBuf / sizeof(int); i++)
            buf_tst[i] = counter++;
    } break;
    case 5: //
    {
        int64_t* buf_tst = (int64_t*)pBuf;
        int64_t counter = int64_t((U32)index * (sizeBuf / sizeof(int64_t)));
        for (U32 i = 0; i < sizeBuf / sizeof(int64_t); i++)
            buf_tst[i] = counter++;
    } break;
        // бегущими "0" и "1" продолжать заполнять не надо
    case 1: //
    case 2: //
        break;
    case 3: //
    {
        //		short* buf_tst = (short*)pBuf;
        //		for(U32 i = 0; i < sizeBuf / sizeof(short); i++)
        //			buf_tst[i] = (short)rand();
        UCHAR* buf_tst = (UCHAR*)pBuf;
        for (U32 i = 0; i < sizeBuf; i++)
            buf_tst[i] = (UCHAR)rand();
    } break;
    case 6: //
    {
        USHORT* buf_tst = (USHORT*)pBuf;
        for (U32 i = 0; i < sizeBuf / sizeof(USHORT); i++)
            buf_tst[i] = (USHORT)genrand_int32();
    } break;
    default:
        break;
    }
}

/*void WriteDataFile(int idx, PVOID pBuf, ULONG nNumberOfBytes)
{
        BRDCHAR fileName[64];
        BRDC_sprintf(fileName, _BRDC("test_%d.bin"), idx);
        //HANDLE hfile = CreateFile(	fileName,
        //							GENERIC_WRITE,
        //							FILE_SHARE_WRITE,
        //							NULL,
        //							CREATE_ALWAYS,
        //							FILE_ATTRIBUTE_NORMAL,
        //							NULL);
        //if(hfile == INVALID_HANDLE_VALUE)
        IPC_handle hfile = IPC_openFile(fileName, IPC_CREATE_FILE | IPC_FILE_WRONLY);
        if(!hfile)
        {
                BRDC_printf(_BRDC("Create file error\n"));
                return;
        }
        //ULONG nNumberOfWriteBytes;

        //BOOL bResult = WriteFile(hfile, pBuf, nNumberOfBytes, &nNumberOfWriteBytes, NULL);
        //CloseHandle(hfile);
        //if(bResult != TRUE)
        BOOL bResult = IPC_writeFile(hfile, pBuf, nNumberOfBytes);
        IPC_closeFile(hfile);
        if(bResult < 0)
                BRDC_printf(_BRDC("Write file error\n"));
}
*/
// установить параметры SDRAM
// S32 SdramSettings(BRD_Handle hSrv, ULONG& bBuf_size)
S32 SdramSettings(BRD_Handle hSrv, int64_t& bBuf_size)
{
    S32 status;

    // ULONG mode = 0; // можно применять только автоматический режим
    // status = BRD_ctrl(hSrv, 0, BRDctrl_SDRAM_SETREADMODE, &mode);
    //  режим чтения получаем из ini-файла: 0 = автоматический, 1 = произвольный
    status = BRD_ctrl(hSrv, 0, BRDctrl_SDRAM_SETREADMODE, &MemReadMode);
    if (MemReadMode)
        BRDC_printf(_BRDC("Random memory read mode\n"));
    else
        BRDC_printf(_BRDC("Automatic memory read mode\n"));

    status = BRD_ctrl(hSrv, 0, BRDctrl_SDRAM_SETSTARTADDR, &MemoryAddress); // установить адрес записи
    //	ULONG addr = 0;
    //	status = BRD_ctrl(hSrv, 0, BRDctrl_SDRAM_SETREADADDR, &addr); // установить адрес чтения
    status = BRD_ctrl(hSrv, 0, BRDctrl_SDRAM_SETREADADDR, &MemoryAddress); // установить адрес чтения

    //	ULONG mem_size = bBuf_size >> 2; // получить размер активной зоны в 32-разрядных словах
    uint64_t mem_size = uint64_t(bBuf_size >> 2); // получить размер активной зоны в 32-разрядных словах
    status = BRD_ctrl(hSrv, 0, BRDctrl_SDRAM_SETMEMSIZE, &mem_size);
    bBuf_size = (int64_t)mem_size << 2; // получить фактический размер активной зоны в байтах
    if (BRD_errcmp(status, BRDerr_OK))
        BRDC_printf(_BRDC("BRDctrl_SDRAM_SETMEMSIZE: SDRAM buffer size = %d Mbytes\n"), U32(bBuf_size / 1024 / 1024));
    else
        DisplayError(status, _BRDC("BRDctrl_SDRAM_SETMEMSIZE"));

    return status;
}

// тестовая тетрада выводит данные программным методом
// и сбор данных в FIFO с передачей в ПК через стрим
S32 FifoInput(BRD_Handle hADC, BRD_Handle hTest, void* pBuf)
{
    S32 status;
    ULONG Status = 0;

    BRD_TestCfg TstConfig;
    status = BRD_ctrl(hTest, 0, BRDctrl_TEST_GETCFG, &TstConfig);
    BRDC_printf(_BRDC("TEST FIFO size = %d bytes\n"), TstConfig.FifoSize);

    ULONG tetrad;

    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETSRCSTREAM, &tetrad);
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_GETSRCSTREAM, &tetrad);
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_GETSRCSTREAM, &tetrad);
        break;
    }

    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_SETSRC, &tetrad);

    BRD_AdcCfg AdcConfig;
    if (!DdcOn) {
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETCFG, &AdcConfig);
    }

    // установить флаг для формирования запроса ПДП
    //	ULONG flag = BRDstrm_DRQ_ALMOST;
    //	ULONG flag = BRDstrm_DRQ_READY;
    //	ULONG flag = BRDstrm_DRQ_HALF; // рекомендуется флаг - FIFO наполовину заполнено
    ULONG flag = AdcDrqFlag;
    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_SETDRQ, &flag);

    ULONG tst_mode = 1;
    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_SETTESTMODE, &tst_mode);
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_SETTESTMODE, &tst_mode);
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_SETTESTMODE, &tst_mode);
        break;
    }

    IPC_delay(1);

    status = BRD_ctrl(hTest, 0, BRDctrl_TEST_FIFORESET, NULL);
    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_FIFORESET, NULL);
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_FIFORESET, NULL);
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_FIFORESET, NULL);
        break;
    }

    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_RESETFIFO, NULL);

    BRDctrl_StreamCBufStart start_pars;
    start_pars.isCycle = 0;
    ULONG msTimeout = 1; //-1;

    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_START, &start_pars);

    ULONG enable = 1;
    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_ENABLE, &enable);
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_START, &enable);
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_ENABLE, &enable);
        break;
    }

    ULONG bufSize = TstConfig.FifoSize >> 2; // half-FIFO
    int cnt = 4096 * 256 / bufSize;
    BRD_DataBuf tst_data_buf;
    UCHAR* pCurBuf = (UCHAR*)pBuf;
    int j = 0;
    int i = 0;
    do {
        tst_data_buf.pData = pCurBuf;
        tst_data_buf.size = bufSize;
        status = BRD_ctrl(hTest, 0, BRDctrl_TEST_SETDATA, &tst_data_buf);
        for (i = 0; i < 1000; i++) {
            IPC_delay(1);
            status = BRD_ctrl(hTest, 0, BRDctrl_TEST_FIFOSTATUS, &Status);
            if ((Status & 0x8) == 0) // Almost Empty FIFO flag
                break;
        }
        if (i >= 1000) {
            BRDC_printf(_BRDC("\nERROR by BRDctrl_TEST_SETDATA, Status = %X\n"), Status);
            break;
        }
        status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout);
        pCurBuf += bufSize;
        BRDC_printf(_BRDC("%d\r"), ++j);
        if (j >= cnt)
            pCurBuf = (UCHAR*)pBuf;
    } while (BRD_errcmp(status, BRDerr_WAIT_TIMEOUT));

    // BRDC_printf(_BRDC("\nPress any key to continue...\n");
    // IPC_getch();
    enable = 0;

    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_ENABLE, &enable);
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_FIFOSTATUS, &Status);
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_STOP, &enable);
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_FIFOSTATUS, &Status);
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_ENABLE, &enable);
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_FIFOSTATUS, &Status);
        break;
    }

    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_STOP, NULL);

    return status;
}

// тестовая тетрада выводит данные через стрим
// и сбор данных в FIFO с передачей в ПК через стрим
S32 FifoInputDMA(BRD_Handle hADC, BRD_Handle hTest)
{
    S32 status;
    ULONG Status = 0;

    BRD_TestCfg TstConfig;
    status = BRD_ctrl(hTest, 0, BRDctrl_TEST_GETCFG, &TstConfig);

    ULONG tetrad;
    status = BRD_ctrl(hTest, 0, BRDctrl_TEST_GETSRCSTREAM, &tetrad);
    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_SETSRC, &tetrad);

    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETSRCSTREAM, &tetrad);
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_GETSRCSTREAM, &tetrad);
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_GETSRCSTREAM, &tetrad);
        break;
    }

    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_SETSRC, &tetrad);

    BRD_AdcCfg AdcConfig;
    if (!DdcOn) {
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETCFG, &AdcConfig);
    }

    // установить флаг для формирования запроса ПДП
    //	ULONG flag = BRDstrm_DRQ_ALMOST;
    //	ULONG flag = BRDstrm_DRQ_READY;
    //	ULONG flag = BRDstrm_DRQ_HALF; // рекомендуется флаг - FIFO наполовину заполнено
    ULONG flag = TstDrqFlag;
    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_SETDRQ, &flag);
    flag = AdcDrqFlag;
    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_SETDRQ, &flag);

    ULONG tst_mode = 1;
    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_SETTESTMODE, &tst_mode);
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_SETTESTMODE, &tst_mode);
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_SETTESTMODE, &tst_mode);
        break;
    }

    IPC_delay(1);

    status = BRD_ctrl(hTest, 0, BRDctrl_TEST_FIFORESET, NULL);
    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_FIFORESET, NULL);
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_FIFORESET, NULL);
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_FIFORESET, NULL);
        break;
    }

    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_RESETFIFO, NULL);
    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_RESETFIFO, NULL);

    BRDctrl_StreamCBufStart start_pars;
    start_pars.isCycle = 0;

    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_START, &start_pars);

    ULONG enable = 1;
    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_ENABLE, &enable);
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_START, &enable);
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_ENABLE, &enable);
        break;
    }

    start_pars.isCycle = 1;
    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_START, &start_pars);

    ULONG msTimeout = 3000; //-1;
    // int icnt = 0;
    // do {
    // status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout);
    // if(!BRD_errcmp(status, BRDerr_WAIT_TIMEOUT))
    //{	// если вышли не по тайм-ауту
    //	if(DebugOn)
    //		BRDC_printf(_BRDC("Output testing buffer %d\n", icnt++);
    //	status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_START, &start_pars);
    // }
    // else
    //{
    //	status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout);
    //	if(BRD_errcmp(status, BRDerr_WAIT_TIMEOUT))
    //	{
    //		BRDC_printf(_BRDC("TIME-OUT by test output!\n");
    //		BRDC_printf(_BRDC("Press any key for continue program...\n");
    //		IPC_getch();
    //	}
    //	status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
    //	break;
    // }
    //		status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_WAITBUF, &msTimeout);
    //	status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout);
    //} while(BRD_errcmp(status, BRDerr_WAIT_TIMEOUT));

    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout);
    if (BRD_errcmp(status, BRDerr_WAIT_TIMEOUT))
        BRDC_printf(_BRDC("TIME-OUT by input!\n"));

    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
    enable = 0;

    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_ENABLE, &enable);
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_FIFOSTATUS, &Status);
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_STOP, &enable);
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_FIFOSTATUS, &Status);
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_ENABLE, &enable);
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_FIFOSTATUS, &Status);
        break;
    }

    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_STOP, NULL);

    return status;
}

// выполнить сбор данных в SDRAM с ПДП-методом передачи в ПК
S32 SdramInputDMA(BRD_Handle hADC, BRD_Handle hTest, PVOID pBuf, U32 sizeBuf)
{
    S32 status;
    ULONG Status = 0;

    ULONG tetrad;
    status = BRD_ctrl(hTest, 0, BRDctrl_TEST_GETSRCSTREAM, &tetrad);
    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_SETSRC, &tetrad);

    // установить флаг для формирования запроса ПДП
    //	ULONG flag = BRDstrm_DRQ_ALMOST;
    //	ULONG flag = BRDstrm_DRQ_READY;
    //	ULONG flag = BRDstrm_DRQ_HALF; // рекомендуется флаг - FIFO наполовину заполнено

    ULONG flag = TstDrqFlag;
    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_SETDRQ, &flag);
    // flag = AdcDrqFlag;
    // status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_SETDRQ, &flag);

    BRDctrl_StreamCBufStart start_pars;
    start_pars.isCycle = 0;
    // start_pars.isCycle = 1;
    ULONG msTimeout = 2000; //-1;

    ULONG tst_mode = 1;
    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_SETTESTMODE, &tst_mode);
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_SETTESTMODE, &tst_mode);
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_SETTESTMODE, &tst_mode);
        break;
    }

    status = BRD_ctrl(hTest, 0, BRDctrl_TEST_FIFORESET, NULL);
    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_FIFORESET, NULL); // сброс FIFO АЦП
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_FIFORESET, NULL); // сброс FIFO DDC GC5016
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_FIFORESET, NULL); // сброс FIFO DDC
        break;
    }

    status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_FIFORESET, NULL); // сборс FIFO SDRAM

    ULONG enable = 1;
    status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_ENABLE, &enable); // разрешение записи в SDRAM
    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_ENABLE, &enable); // разрешение работы АЦП
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_START, &enable); // разрешение работы DDC GC5016
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_ENABLE, &enable); // разрешение работы DDC
        break;
    }

    // закомментированное можно использовать для тестов на скорость
    // status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_START, &start_pars); // стартуем вывод тестовой последовательности
    // do{
    // status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_ISACQCOMPLETE, &Status); // проверяем заполнилась ли активная зона памяти
    //} while(!Status);

    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_RESETFIFO, NULL);

    short index = 1;
    // дожидаемся окончания сбора
    do {
        status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_START, &start_pars); // стартуем вывод тестовой последовательности
        status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout); // дожидаемся окончания вывода тестовой последовательности
        BRD_ctrl(hADC, 0, BRDctrl_SDRAM_ISACQCOMPLETE, &Status); // проверяем заполнилась ли активная зона памяти
        if (BRD_errcmp(status, BRDerr_WAIT_TIMEOUT)) { // если вышли по тайм-ауту
            if (!Status) {
                DisplayError(status, _BRDC("TIME-OUT"));
                BRDC_printf(_BRDC("Press any key for continue program...\n"));
                IPC_getch();
                status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
            }
        }
        // status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_ISACQCOMPLETE, &Status); // проверяем заполнилась ли активная зона памяти
        if (!Status) {
            BRDCHAR Msg[128];
            ULONG address = MemoryAddress + index * (sizeBuf / sizeof(int));
            BRDC_sprintf(Msg, _BRDC("Writing to SDRAM: %0X\r"), address);
            BRDC_printf(_BRDC("%s"), Msg);
            //		BRDC_printf(_BRDC(".");
            ContinueTestBuffer(pBuf, sizeBuf, index);
            index++;
        }
    } while (!Status);
    BRDC_printf(_BRDC("                                                  \r"));
    //	BRDC_printf(_BRDC("\n");

    enable = 0;
    switch (DdcOn) {
    case 0:
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_ENABLE, &enable); // запрет работы АЦП
        break;
    case 1:
        status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_STOP, &enable); // запрет работы DDC GC5016
        break;
    case 2:
        status = BRD_ctrl(hADC, 0, BRDctrl_DDC_ENABLE, &enable); // запрет работы DDC
        break;
    }
    status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_ENABLE, &enable); // запрет записи в SDRAM

    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_STOP, NULL); // останавливаем вывод тестовой последовательности

    // установить, что стрим работает с памятью
    // ULONG tetrad;
    /*	status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_GETSRCSTREAM, &tetrad);
            status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_SETSRC, &tetrad);

            flag = SdramDrqFlag;
            status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_SETDRQ, &flag);
    */
    return status;
}

void WriteDataFile(PVOID pBuf, ULONG nNumberOfBytes)
{
    BRDCHAR fileName[64];
    BRDC_sprintf(fileName, _BRDC("adc_data.bin"));
    // HANDLE hfile = CreateFile(	fileName,
    //							GENERIC_WRITE,
    //							FILE_SHARE_WRITE | FILE_SHARE_READ,
    //							NULL,
    //							CREATE_ALWAYS,
    //							FILE_ATTRIBUTE_NORMAL,
    //							NULL);
    // if(hfile == INVALID_HANDLE_VALUE)
    IPC_handle hfile = IPC_openFile(fileName, IPC_CREATE_FILE | IPC_FILE_WRONLY);
    if (!hfile) {
        BRDC_printf(_BRDC("Create file %s error.\n"), fileName);
        return;
    }
    // ULONG nNumberOfWriteBytes;

    // BOOL bResult = WriteFile(hfile, pBuf, nNumberOfBytes, &nNumberOfWriteBytes, NULL);
    // CloseHandle(hfile);
    // if(bResult != TRUE)
    int bResult = IPC_writeFile(hfile, pBuf, nNumberOfBytes);
    IPC_closeFile(hfile);
    if (bResult < 0)
        BRDC_printf(_BRDC("Write file %s error.\n"), fileName);
}

// #define __SIZE_4M

// S32 Testing(BRD_Handle handle, BRD_ServList* srv)
S32 AdcTesting(BRD_Handle hADC, BRD_Handle hTest, BRD_ServList* srv)
{
    //	ULONG bBufSize = 4096 * 64;//1024; // получить размер активной зоны в байтах
    //	ActiveZoneSize = 4096 * 128;
    int flOK = 1;
    all_error_num = 0;
    S32 status;
    int firstTime = 1;
    // U32 mode = BRDcapt_EXCLUSIVE;
    // BRD_Handle hADC = BRD_capture(handle, 0, &mode, srv->name, 10000);
    // BRD_Handle hTest = BRD_capture(handle, 0, &mode, "TEST0", 10000);
    // if(mode == BRDcapt_EXCLUSIVE) BRDC_printf(_BRDC("ADC Capture mode: EXCLUSIVE\n");
    // if(mode == BRDcapt_SPY)	BRDC_printf(_BRDC("ADC Capture mode: SPY\n");
    // if(hADC > 0 && hTest > 0)
    {
        void* pBuf_tst = NULL;
#ifdef __SIZE_4M
        ULONG size_tst = 4096 * 1024;
#else
        ULONG size_tst = 4096 * 256;
#endif
        BRDctrl_StreamCBufAlloc buf_dscr_tst;
        if (g_TstDma) {
            buf_dscr_tst.dir = BRDstrm_DIR_OUT;
            buf_dscr_tst.isCont = 0;
            // buf_dscr_tst.isCont = 0;//1;
            buf_dscr_tst.blkNum = 1;
            buf_dscr_tst.blkSize = size_tst; // 4096 * 264; // 1 MB
            //		buf_dscr_tst.blkSize = 4096 * 1024; // 4 MB
            buf_dscr_tst.ppBlk = new PVOID[buf_dscr_tst.blkNum];
            status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_ALLOC, &buf_dscr_tst);

            pBuf_tst = (int*)buf_dscr_tst.ppBlk[0];
        } else {
            pBuf_tst = new char[size_tst];
        }
        BRDctrl_StreamCBufAlloc buf_dscr_adc;
        buf_dscr_adc.dir = BRDstrm_DIR_IN;
        buf_dscr_adc.isCont = 0;
        // buf_dscr_adc.isCont = 0;//1;
        buf_dscr_adc.blkNum = 1;
#ifdef __SIZE_4M
        buf_dscr_adc.blkSize = 4096 * 1024; // 4 MB
#else
        buf_dscr_adc.blkSize = 4096 * 256; // 1 MB
#endif
        buf_dscr_adc.ppBlk = new PVOID[buf_dscr_adc.blkNum];
        status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_ALLOC, &buf_dscr_adc);

        int* buf_adc = (int*)buf_dscr_adc.ppBlk[0];
        for (U32 i = 0; i < buf_dscr_adc.blkSize / sizeof(int); i++)
            buf_adc[i] = 0;

        // заполняем буфер тестовой последовательностью
        // SetTestBuffer(buf_dscr_tst.ppBlk[0], buf_dscr_tst.blkSize);
        SetTestBuffer(pBuf_tst, size_tst);

        if (DaqMemMode) {
            // проверяем наличие динамической памяти
            BRD_SdramCfgEx SdramConfig;
            SdramConfig.Size = sizeof(BRD_SdramCfgEx);
            uint64_t PhysMemSize;
            status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_GETCFGEX, &SdramConfig);
            if (status < 0)
                PhysMemSize = 0;
            else {
                if (SdramConfig.MemType == 11 || // DDR3
                    SdramConfig.MemType == 12) // DDR4
                    PhysMemSize = (uint64_t)((
                                                 (((int64_t)SdramConfig.CapacityMbits * 1024 * 1024) >> 3) * (int64_t)SdramConfig.PrimWidth / SdramConfig.ChipWidth * SdramConfig.ModuleBanks * SdramConfig.ModuleCnt)
                        >> 2); // в 32-битных словах
                else
                    PhysMemSize = (1 << SdramConfig.RowAddrBits) * (1 << SdramConfig.ColAddrBits) * SdramConfig.ModuleBanks * SdramConfig.ChipBanks * SdramConfig.ModuleCnt * 2; // в 32-битных словах
            }
            if (!iCycle) {
                SetIoDelay(hADC & 0xFFFF); // программирование линий задержки
                ReadCalibrReg(hADC & 0xFFFF);
            }

            if (PhysMemSize) { // динамическая память присутствует на модуле
                BRDC_printf(_BRDC("SDRAM Config: Memory size = %d MBytes\n"), (PhysMemSize / (1024 * 1024)) * 4);
                ULONG target = 2; // будем осуществлять сбор данных в память

                switch (DdcOn) {
                case 0:
                    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_SETTARGET, &target);
                    break;
                case 1:
                    status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_SETTARGET, &target);
                    break;
                case 2:
                    status = BRD_ctrl(hADC, 0, BRDctrl_DDC_SETTARGET, &target);
                    break;
                }

                //				status = SdramSettings(hADC, bBufSize); // установить параметры SDRAM
                status = SdramSettings(hADC, ActiveZoneSize); // установить параметры SDRAM

                // LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
                // int bHighRes = QueryPerformanceFrequency (&Frequency);
                IPC_TIMEVAL start, stop;

                //				if(TestOn != 2 || (TestOn == 2 && !iCycle))
                if (TestToMem) {
                    // QueryPerformanceCounter (&StartPerformCount);
                    IPC_getTime(&start);
                    SdramInputDMA(hADC, hTest, buf_dscr_tst.ppBlk[0], buf_dscr_tst.blkSize);
                    // SdramInput(hADC, hTest, pBuf_tst, size_tst);
                    // QueryPerformanceCounter (&StopPerformCount);
                    // double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
                    IPC_getTime(&stop);
                    double msTime = IPC_getDiffTime(&start, &stop);
                    BRDC_printf(_BRDC("Write is complete. Writing time is %.2f ms\n"), msTime);
                }
                ULONG tetrad;
                status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_GETSRCSTREAM, &tetrad);
                status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_SETSRC, &tetrad);
                ULONG flag = SdramDrqFlag;
                status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_SETDRQ, &flag);
                BRDC_printf(_BRDC("SDRAM DRQ flag = %0X\n"), flag);

                BRDctrl_StreamCBufStart start_pars;
                start_pars.isCycle = 0;
                ULONG msTimeout = -1;
                status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_RESETFIFO, NULL);
                for (int iRead = 0; iRead < MemRead; iRead++) {
                    // LONGLONG startStopTime = 0;
                    double startStopTime = 0.0;

                    if (MemReadMode) {
                        status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_RESETFIFO, NULL);
                        status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_SETREADADDR, &MemoryAddress); // установить адрес чтения
                    }
                    int key_char = 0;
                    error_num = 0;
                    // int* buf_tst = (int*)buf_dscr_tst.ppBlk[0];
                    int* buf_tst = (int*)pBuf_tst;
                    if (TestMode == 3)
                        srand(Seed);
                    if (TestMode == 6)
                        init_genrand(Seed);
                    //				for(int idx = 0; idx < int(bBufSize / buf_dscr_adc.blkSize); idx++)
                    for (int idx = 0; idx < int(ActiveZoneSize / buf_dscr_adc.blkSize); idx++) {
                        ULONG rden = 1;
                        if (MemReadMode) // разрешение чтения в произвольном режиме
                        {
                            status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_FIFORESET, NULL);
                            status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_READENABLE, &rden);
                        }
                        // QueryPerformanceCounter (&StartPerformCount);
                        IPC_getTime(&start);
                        status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_START, &start_pars);
                        status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout);
                        // QueryPerformanceCounter (&StopPerformCount);
                        // startStopTime += StopPerformCount.QuadPart - StartPerformCount.QuadPart;
                        IPC_getTime(&stop);
                        startStopTime += IPC_getDiffTime(&start, &stop);
                        ULONG address = MemoryAddress + idx * (buf_dscr_adc.blkSize / sizeof(int));
                        rden = 0;
                        if (MemReadMode) // запрещение чтения в произвольном режиме
                        {
                            status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_READENABLE, &rden);
                            status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_RESETFIFO, NULL);
                            ULONG addr_mem = MemoryAddress + (idx + 1) * (buf_dscr_adc.blkSize / sizeof(int));
                            status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_SETREADADDR, &addr_mem); // установить адрес чтения
                        }
                        //					BRDCHAR Msg[128];
                        //					sprintf(Msg, "Testing to SDRAM: %0X\r", address);
                        if (TestOn) {
                            BRDC_printf(_BRDC("Testing of SDRAM: %0X (reading time is %.2f ms)\r"), address, startStopTime);
                            if (firstTime) {
                                // WriteDataFile(buf_adc, buf_dscr_adc.blkSize);
                                firstTime = 0;
                            }
                            // TestBufferForComp(buf_dscr_tst.ppBlk[0], buf_dscr_adc.blkSize, (short)idx);
                            TestBufferForComp(pBuf_tst, buf_dscr_adc.blkSize, (short)idx);
                            U32 cnt = buf_dscr_adc.blkSize / sizeof(int);
                            for (U32 i = 0; i < cnt; i++) {
                                U32 imask = i % 4;
                                U32 val_adc = buf_adc[i] & TestMask[imask];
                                U32 val_tst = buf_tst[i] & TestMask[imask];
                                // if(buf_adc[i] != buf_tst[i])
                                if (val_adc != val_tst) {
                                    error_num++;
                                    all_error_num++;
                                    BRDCHAR Message[128];
                                    address = MemoryAddress + cnt * idx + i;
                                    //							BRDC_printf(Message, //"Начальное значение функции генерации псевдослучайных чисел = %04X\n"
                                    //											"Error to offset: %08X, write: %08X, read: %08X\n",
                                    //											"Начать чтение сначала (Да), продолжить дальше (Нет) или прервать (Отмена)?",
                                    // Seed, dwMemBufAddr + i, pTestMask[i], pBuf[i]);
                                    //											address, buf_tst[i+2], buf_adc[i]);
                                    BRDC_sprintf(Message, _BRDC("Error to offset: %08X, write: %08X, read: %08X, XOR: %08X; reading = %d\n"),
                                        // address, buf_tst[i], buf_adc[i], buf_tst[i] ^ buf_adc[i], iRead);
                                        address, val_tst, val_adc, val_tst ^ val_adc, iRead);
                                    if (ErrorMsg)
                                        BRDC_printf(_BRDC("%s"), Message);
                                    if (SaveToFile) {
                                        if (ErrorMsg) {
                                            FILE* fstream = NULL;
                                            fstream = BRDC_fopen(_BRDC("0.txt"), _BRDC("a+t, ccs=UTF-8"));
                                            if (fstream) {
                                                //										int numwritten = fwrite(Message, sizeof(BRDCHAR), lstrlen(Message), fstream);
                                                fwrite(Message, sizeof(BRDCHAR), BRDC_strlen(Message), fstream);
                                                fclose(fstream);
                                            }
                                        }
                                    } else {
                                        BRDC_printf(_BRDC("Press Esc key for breaking program...\n"));
                                        BRDC_printf(_BRDC("Press Enter key for breaking read cycle...\n"));
                                        key_char = IPC_getch();
                                        if (key_char == 0x1B || key_char == 0x0D) // Esc или Enter
                                            break;
                                        BRDC_printf(_BRDC("                                        \r"));
                                    }
                                }
                            }
                            if (key_char == 0x1B || key_char == 0x0D) // Esc или Enter
                                break;
                        } else
                            BRDC_printf(_BRDC("Reading of SDRAM: %0X\r"), address);
                        if (key_char == 0x1B || key_char == 0x0D) // Esc или Enter
                            break;
                    }

                    // double sTime = (double)startStopTime / (double)Frequency.QuadPart;
                    double sTime = startStopTime / 1.E3;
                    BRDC_printf(_BRDC("Reading time is %.2f ms, size = %d MByte, speed is %.2f MBps\n"),
                        sTime * 1.E3, (int)(ActiveZoneSize / 1024 / 1024), (double)(ActiveZoneSize / 1024 / 1024) / sTime);

                    if (key_char == 0x1B) // Esc
                        break; // выход совсем
                    BRDCHAR errMsg[80];
                    BRDC_sprintf(errMsg, _BRDC("%d reading is complete! Error numbers = %d.         \n"), iRead, error_num);
                    BRDC_printf(_BRDC("%s"), errMsg);

                    if (SaveToFile) {
                        FILE* fstream = NULL;
                        fstream = BRDC_fopen(_BRDC("0.txt"), _BRDC("a+t, ccs=UTF-8"));
                        if (fstream) {
                            //					int numwritten = fwrite(Message, sizeof(BRDCHAR), lstrlen(Message), fstream);
                            fwrite(errMsg, sizeof(BRDCHAR), BRDC_strlen(errMsg), fstream);
                            fclose(fstream);
                        }
                    }
                }
                if (!TestOn)
                    BRDC_printf(_BRDC("Read is complete!                                        \n"));
            } else {
                flOK = 0;
                BRDC_printf(_BRDC("No memory. Test don't start.\n"));
            }
        } else {
            if (g_TstDma)
                FifoInputDMA(hADC, hTest);
            else
                FifoInput(hADC, hTest, pBuf_tst);
            BRDC_printf(_BRDC("Write and read test data is completed.\n"));
            //			short* buf_tst = (short*)buf_dscr_tst.ppBlk[0];
            //		    for(U32 i = 0; i < buf_dscr_adc.blkSize / sizeof(short); i++)
            if (TestOn) {
                // int* buf_tst = (int*)buf_dscr_tst.ppBlk[0];
                int* buf_tst = (int*)pBuf_tst;
                for (U32 i = 0; i < buf_dscr_adc.blkSize / sizeof(int); i++) {
                    if (buf_adc[i] != buf_tst[i]) {
                        error_num++;
                        all_error_num++;
                        BRDCHAR Message[128];
                        //					sprintf(Message, //"Начальное значение функции генерации псевдослучайных чисел = %04X\n"
                        BRDC_sprintf(Message, _BRDC("Error to offset: %08X, write: %08X, read: %08X, XOR: %08X\n"), i, buf_tst[i], buf_adc[i], buf_tst[i] ^ buf_adc[i]);
                        if (ErrorMsg)
                            BRDC_printf(_BRDC("%s"), Message);
                        if (SaveToFile) {
                            if (ErrorMsg) {
                                FILE* fstream = NULL;
                                fstream = BRDC_fopen(_BRDC("0.txt"), _BRDC("a+t, ccs=UTF-8"));
                                if (fstream) {
                                    // #ifdef _WIN64
                                    //									char errMsg[128];
                                    //									wcstombs(errMsg, Message, 128);
                                    //									fwrite(errMsg, sizeof(char), strlen(errMsg), fstream);
                                    // #else
                                    fwrite(Message, sizeof(BRDCHAR), BRDC_strlen(Message), fstream);
                                    // #endif
                                    fclose(fstream);
                                }
                            }
                        } else {
                            BRDC_printf(_BRDC("Press Esc key for breaking program...\r"));
                            int key_char = IPC_getch();
                            if (key_char == 0x1B) // Esc
                                break;
                            BRDC_printf(_BRDC("                                        \r"));
                        }
                    }
                }
            }
        }
        if (flOK && TestOn) {
            BRDCHAR errMsg[80];
            BRDC_sprintf(errMsg, _BRDC("Test is complete! Total errors = %d.\n"), all_error_num);
            //			BRDC_printf(_BRDC("Test is complete! Error numbers = %d.\n", error_num);
            BRDC_printf(_BRDC("%s"), errMsg);
            if (SaveToFile) {
                FILE* fstream = NULL;
                fstream = BRDC_fopen(_BRDC("0.txt"), _BRDC("a+t, ccs=UTF-8"));
                if (fstream) {
                    // #ifdef _WIN64
                    //					wcstombs(errMsg, errMsg, 80);
                    // #endif
                    fwrite(errMsg, sizeof(BRDCHAR), BRDC_strlen(errMsg), fstream);
                    fclose(fstream);
                }
            }
        }
        //		status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_FREEBUF, &buf_dscr_tst);
        if (g_TstDma) {
            status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_FREE, &buf_dscr_tst);
            delete[] buf_dscr_tst.ppBlk;
        } else
            delete (char*)pBuf_tst;
        //		WriteDataFile(0, buf_dscr_adc.ppBlk[0], buf_dscr_adc.blkSize);

        //		status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_FREEBUF, &buf_dscr_adc);
        status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_FREE, &buf_dscr_adc);
        delete[] buf_dscr_adc.ppBlk;

        ULONG tst_mode = 0;

        switch (DdcOn) {
        case 0:
            status = BRD_ctrl(hADC, 0, BRDctrl_ADC_SETTESTMODE, &tst_mode);
            break;
        case 1:
            status = BRD_ctrl(hADC, 0, BRDctrl_GC5016_SETTESTMODE, &tst_mode);
            break;
        case 2:
            status = BRD_ctrl(hADC, 0, BRDctrl_DDC_SETTESTMODE, &tst_mode);
            break;
        }

        // status = BRD_release(hTest, 0);
        // status = BRD_release(hADC, 0);
    }
    return status;
}

// выполнить вывод данных в FIFO с ПДП-методом передачи из ПК
S32 FifoOutputDMA(BRD_Handle hDAC, BRD_Handle hTest)
{
    S32 status;
    ULONG Status = 0;

    ULONG tetrad;
    status = BRD_ctrl(hTest, 0, BRDctrl_TEST_GETSRCSTREAM, &tetrad);
    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_SETSRC, &tetrad);

    status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_GETSRCSTREAM, &tetrad);
    status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_SETSRC, &tetrad);

    //	BRD_DacCfg DacConfig;
    //	status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_GETCFG, &DacConfig);

    // установить флаг для формирования запроса ПДП
    //	ULONG flag = BRDstrm_DRQ_ALMOST;
    //	ULONG flag = BRDstrm_DRQ_READY;
    //	ULONG flag = BRDstrm_DRQ_HALF; // рекомендуется флаг - FIFO наполовину заполнено
    ULONG flag = TstDrqFlag;
    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_SETDRQ, &flag);
    flag = AdcDrqFlag;
    status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_SETDRQ, &flag);

    ULONG tst_mode = 1;
    status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_SETTESTMODE, &tst_mode);
    // Sleep(1);

    status = BRD_ctrl(hTest, 0, BRDctrl_TEST_FIFORESET, NULL);
    status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_FIFORESET, NULL);

    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_RESETFIFO, NULL);
    status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_RESETFIFO, NULL);

    BRDctrl_StreamCBufStart start_pars;
    start_pars.isCycle = 0;
    ULONG msTimeout = 3000; //-1;
    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_START, &start_pars);

    ULONG enable = 1;
    status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_ENABLE, &enable);

    status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_CBUF_START, &start_pars);

    int icnt = 0;
    do {
        status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout);
        if (!BRD_errcmp(status, BRDerr_WAIT_TIMEOUT)) { // если вышли не по тайм-ауту
            if (DebugOn)
                BRDC_printf(_BRDC("Output testing buffer %d\n"), icnt++);
            status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_CBUF_START, &start_pars);
        } else {
            DisplayError(status, _BRDC("TIME-OUT"));
            BRDC_printf(_BRDC("Press any key for continue program...\n"));
            IPC_getch();
            status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
            break;
        }
        status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout);
    } while (BRD_errcmp(status, BRDerr_WAIT_TIMEOUT));

    enable = 0;
    status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_ENABLE, &enable);
    status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_FIFOSTATUS, &Status);
    status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_CBUF_STOP, NULL);

    return status;
}

// выполнить вывод данных в SDRAM с ПДП-методом передачи из ПК
S32 SdramOutputDMA(BRD_Handle hDAC, BRD_Handle hTest, PVOID pBuf, U32 sizeBuf)
{
    S32 status;
    ULONG Status = 0;

    ULONG tetrad;
    // установить, что стрим работает с памятью
    status = BRD_ctrl(hDAC, 0, BRDctrl_SDRAM_GETSRCSTREAM, &tetrad);
    status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_SETSRC, &tetrad);

    BRDctrl_StreamCBufStart start_pars;
    start_pars.isCycle = 0;
    ULONG msTimeout = 2000; //-1;

    status = BRD_ctrl(hDAC, 0, BRDctrl_SDRAM_FIFORESET, NULL); // сброс FIFO SDRAM

    status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_RESETFIFO, NULL);
    // short index = 1;
    //  дожидаемся окончания записи
    int cnt = int(ActiveZoneSize / sizeBuf) + (int(ActiveZoneSize % sizeBuf) ? 1 : 0);
    for (int i = 0; i < cnt; i++)
    // do {
    {
        status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_CBUF_START, &start_pars); // стартуем вывод тестовой последовательности
        status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout); // дожидаемся окончания вывода тестовой последовательности
        if (BRD_errcmp(status, BRDerr_WAIT_TIMEOUT)) { // если вышли по тайм-ауту
            status = BRD_ctrl(hDAC, 0, BRDctrl_SDRAM_ISACQCOMPLETE, &Status); // проверяем заполнилась ли активная зона памяти
            if (!Status) {
                DisplayError(status, _BRDC("TIME-OUT"));
                BRDC_printf(_BRDC("Press any key for continue program...\n"));
                IPC_getch();
                status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
            }
        }
        // status = BRD_ctrl(hDAC, 0, BRDctrl_SDRAM_ISACQCOMPLETE, &Status); // проверяем заполнилась ли активная зона памяти
        BRDCHAR Msg[128];
        // ULONG address = MemoryAddress + index * (sizeBuf / sizeof(int));
        uint64_t address = MemoryAddress + i * (sizeBuf / sizeof(int));
        BRDC_sprintf(Msg, _BRDC("Writing to SDRAM: %0X\r"), address);
        BRDC_printf(_BRDC("%s"), Msg);
        //		BRDC_printf(_BRDC(".");
        // ContinueTestBuffer(pBuf, sizeBuf, index);
        ContinueTestBuffer(pBuf, sizeBuf, i + 1);
    }
    //	index++;
    //} while(!Status);
    BRDC_printf(_BRDC("                                                  \r"));
    //	BRDC_printf(_BRDC("\n");

    status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_CBUF_STOP, NULL); // останавливаем вывод тестовой последовательности

    // ULONG tetrad;
    //  установить, что стрим работает с тестовой службой
    status = BRD_ctrl(hTest, 0, BRDctrl_TEST_GETSRCSTREAM, &tetrad);
    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_SETSRC, &tetrad);

    //	ULONG tst_mode = 1;
    //	status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_SETTESTMODE, &tst_mode);

    return status;
}

S32 DacTesting(BRD_Handle hDAC, BRD_Handle hTest, BRD_ServList* srv)
{
    //	ULONG bBufSize = 4096 * 64;//1024; // получить размер активной зоны в байтах
    //	ActiveZoneSize = 4096 * 128;
    int flOK = 1;
    all_error_num = 0;
    S32 status;
    int firstTime = 1;
    // U32 mode = BRDcapt_EXCLUSIVE;
    // BRD_Handle hADC = BRD_capture(handle, 0, &mode, srv->name, 10000);
    // BRD_Handle hTest = BRD_capture(handle, 0, &mode, "TEST0", 10000);
    // if(mode == BRDcapt_EXCLUSIVE) BRDC_printf(_BRDC("ADC Capture mode: EXCLUSIVE\n");
    // if(mode == BRDcapt_SPY)	BRDC_printf(_BRDC("ADC Capture mode: SPY\n");
    // if(hADC > 0 && hTest > 0)
    {
        BRDctrl_StreamCBufAlloc buf_dscr_tst;
        buf_dscr_tst.dir = BRDstrm_DIR_IN;
        buf_dscr_tst.isCont = 1;
        buf_dscr_tst.blkNum = 1;
#ifdef __SIZE_4M
        buf_dscr_tst.blkSize = 4096 * 1024; // 4 MB
#else
        buf_dscr_tst.blkSize = 4096 * 256; // 1 MB
#endif
        buf_dscr_tst.ppBlk = new PVOID[buf_dscr_tst.blkNum];
        status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_ALLOC, &buf_dscr_tst);

        BRDctrl_StreamCBufAlloc buf_dscr_dac;
        buf_dscr_dac.dir = BRDstrm_DIR_OUT;
        buf_dscr_dac.isCont = 1;
        buf_dscr_dac.blkNum = 1;
#ifdef __SIZE_4M
        buf_dscr_dac.blkSize = 4096 * 1024; // 4 MB
#else
        buf_dscr_dac.blkSize = 4096 * 256; // 1 MB
#endif
        buf_dscr_dac.ppBlk = new PVOID[buf_dscr_dac.blkNum];
        status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_CBUF_ALLOC, &buf_dscr_dac);

        int* buf_tst = (int*)buf_dscr_tst.ppBlk[0];
        for (U32 i = 0; i < buf_dscr_tst.blkSize / sizeof(int); i++)
            buf_tst[i] = 0;

        // заполняем буфер тестовой последовательностью
        SetTestBuffer(buf_dscr_dac.ppBlk[0], buf_dscr_dac.blkSize);

        if (DaqMemMode) {
            // проверяем наличие динамической памяти
            BRD_SdramCfgEx SdramConfig;
            SdramConfig.Size = sizeof(BRD_SdramCfgEx);
            ULONG PhysMemSize;
            status = BRD_ctrl(hDAC, 0, BRDctrl_SDRAM_GETCFGEX, &SdramConfig);
            if (status < 0)
                PhysMemSize = 0;
            else {
                if (SdramConfig.MemType == 11) // DDR3
                    PhysMemSize = (ULONG)((
                                              (((int64_t)SdramConfig.CapacityMbits * 1024 * 1024) >> 3) * (int64_t)SdramConfig.PrimWidth / SdramConfig.ChipWidth * SdramConfig.ModuleBanks * SdramConfig.ModuleCnt)
                        >> 2); // в 32-битных словах
                else
                    PhysMemSize = (1 << SdramConfig.RowAddrBits) * (1 << SdramConfig.ColAddrBits) * SdramConfig.ModuleBanks * SdramConfig.ChipBanks * SdramConfig.ModuleCnt * 2; // в 32-битных словах
            }
            if (!iCycle) {
                SetIoDelay(hDAC & 0xFFFF); // программирование линий задержки
                ReadCalibrReg(hDAC & 0xFFFF);
            }

            if (PhysMemSize) { // динамическая память присутствует на модуле
                ULONG tst_mode = 1;
                status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_SETTESTMODE, &tst_mode);
                ULONG source = 2; // будем осуществлять вывод данных из памяти
                status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_SETSOURCE, &source);
                status = SdramSettings(hDAC, ActiveZoneSize); // установить параметры SDRAM

                //				if(TestOn != 2 || (TestOn == 2 && !iCycle))
                {
                    // LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
                    // int bHighRes = QueryPerformanceFrequency (&Frequency);
                    // QueryPerformanceCounter (&StartPerformCount);
                    IPC_TIMEVAL start, stop;
                    IPC_getTime(&start);
                    SdramOutputDMA(hDAC, hTest, buf_dscr_dac.ppBlk[0], buf_dscr_dac.blkSize);
                    // QueryPerformanceCounter (&StopPerformCount);
                    // double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
                    IPC_getTime(&stop);
                    double msTime = IPC_getDiffTime(&start, &stop);
                    BRDC_printf(_BRDC("Write is complete. Writing time is %.2f ms\n"), msTime);
                }
                BRDctrl_StreamCBufStart start_pars;
                start_pars.isCycle = 0;
                ULONG msTimeout = -1;
                for (int iRead = 0; iRead < MemRead; iRead++) {
                    status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_FIFORESET, NULL); // сброс FIFO ЦАП
                    status = BRD_ctrl(hTest, 0, BRDctrl_TEST_FIFORESET, NULL);
                    status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_RESETFIFO, NULL);
                    error_num = 0;
                    int* buf_dac = (int*)buf_dscr_dac.ppBlk[0];
                    if (TestMode == 3)
                        srand(Seed);
                    if (TestMode == 6)
                        init_genrand(Seed);
                    //				for(int idx = 0; idx < int(bBufSize / buf_dscr_adc.blkSize); idx++)
                    ULONG enable = 1;
                    for (int idx = 0; idx < int(ActiveZoneSize / buf_dscr_tst.blkSize); idx++) {
                        int key_char = 0;
                        status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_START, &start_pars);
                        status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_ENABLE, &enable); // разрешение работы ЦАП
                        status = BRD_ctrl(hDAC, 0, BRDctrl_SDRAM_ENABLE, &enable); // разрешение чтения из SDRAM

                        status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout);

                        uint64_t address = MemoryAddress + idx * buf_dscr_tst.blkSize / sizeof(int);
                        //					BRDCHAR Msg[128];
                        //					sprintf(Msg, "Testing to SDRAM: %0X\r", address);
                        if (TestOn) {
                            BRDC_printf(_BRDC("Testing of SDRAM: %0X\r"), address);
                            if (firstTime) {
                                // WriteDataFile(buf_tst, buf_dscr_tst.blkSize);
                                firstTime = 0;
                            }
                            TestBufferForComp(buf_dscr_dac.ppBlk[0], buf_dscr_tst.blkSize, (short)idx);
                            U32 cnt = buf_dscr_tst.blkSize / sizeof(int);
                            for (U32 i = 0; i < cnt; i++) {
                                U32 imask = i % 4;
                                U32 val_dac = buf_dac[i] & TestMask[imask];
                                U32 val_tst = buf_tst[i] & TestMask[imask];
                                // if(buf_dac[i] != buf_tst[i])
                                if (val_dac != val_tst) {
                                    error_num++;
                                    all_error_num++;
                                    BRDCHAR Message[128];
                                    address = MemoryAddress + cnt * idx + i;
                                    //							BRDC_printf(Message, //"Начальное значение функции генерации псевдослучайных чисел = %04X\n"
                                    //											"Error to offset: %08X, write: %08X, read: %08X\n",
                                    //											"Начать чтение сначала (Да), продолжить дальше (Нет) или прервать (Отмена)?",
                                    // Seed, dwMemBufAddr + i, pTestMask[i], pBuf[i]);
                                    //											address, buf_tst[i+2], buf_adc[i]);
                                    BRDC_sprintf(Message, _BRDC("Error to offset: %08X, write: %08X, read: %08X, XOR: %08X; reading = %d\n"),
                                        // address, buf_dac[i], buf_tst[i], buf_tst[i] ^ buf_dac[i], iRead);
                                        address, val_dac, val_tst, val_tst ^ val_dac, iRead);
                                    if (ErrorMsg)
                                        BRDC_printf(_BRDC("%s"), Message);
                                    if (SaveToFile) {
                                        if (ErrorMsg) {
                                            FILE* fstream = NULL;
                                            fstream = BRDC_fopen(_BRDC("0.txt"), _BRDC("a+t, ccs=UTF-8"));
                                            if (fstream) {
                                                //										int numwritten = fwrite(Message, sizeof(BRDCHAR), lstrlen(Message), fstream);
                                                fwrite(Message, sizeof(BRDCHAR), BRDC_strlen(Message), fstream);
                                                fclose(fstream);
                                            }
                                        }
                                    } else {
                                        BRDC_printf(_BRDC("Press Esc key for breaking program...\n"));
                                        key_char = IPC_getch();
                                        if (key_char == 0x1B) // Esc
                                            break;
                                        BRDC_printf(_BRDC("                                        \r"));
                                    }
                                }
                            }
                        } else
                            BRDC_printf(_BRDC("Reading of SDRAM: %0X\r"), address);
                        if (key_char == 0x1B) // Esc
                            break;
                    }
                    enable = 0;
                    status = BRD_ctrl(hDAC, 0, BRDctrl_SDRAM_ENABLE, &enable); // запрет чтения из SDRAM
                    status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_ENABLE, &enable); // разрешение работы ЦАП

                    BRDCHAR errMsg[80];
                    BRDC_sprintf(errMsg, _BRDC("%d reading is complete! Error numbers = %d.         \n"), iRead, error_num);
                    BRDC_printf(_BRDC("%s"), errMsg);
                    if (SaveToFile) {
                        FILE* fstream = NULL;
                        fstream = BRDC_fopen(_BRDC("0.txt"), _BRDC("a+t, ccs=UTF-8"));
                        if (fstream) {
                            //					int numwritten = fwrite(Message, sizeof(BRDCHAR), lstrlen(Message), fstream);
                            fwrite(errMsg, sizeof(BRDCHAR), BRDC_strlen(errMsg), fstream);
                            fclose(fstream);
                        }
                    }
                }
                if (!TestOn)
                    BRDC_printf(_BRDC("Read is complete!                                        \n"));
            } else {
                flOK = 0;
                BRDC_printf(_BRDC("No memory. Test don't start.\n"));
            }
        } else {
            FifoOutputDMA(hDAC, hTest);
            BRDC_printf(_BRDC("Write and read test data is completed.\n"));
            //			short* buf_tst = (short*)buf_dscr_tst.ppBlk[0];
            //		    for(U32 i = 0; i < buf_dscr_adc.blkSize / sizeof(short); i++)
            if (TestOn) {
                int* buf_dac = (int*)buf_dscr_dac.ppBlk[0];
                for (U32 i = 0; i < buf_dscr_tst.blkSize / sizeof(int); i++) {
                    if (buf_dac[i] != buf_tst[i]) {
                        error_num++;
                        all_error_num++;
                        BRDCHAR Message[128];
                        //					sprintf(Message, //"Начальное значение функции генерации псевдослучайных чисел = %04X\n"
                        BRDC_sprintf(Message, _BRDC("Error to offset: %08X, write: %08X, read: %08X, XOR: %08X\n"), i, buf_dac[i], buf_tst[i], buf_tst[i] ^ buf_dac[i]);
                        if (ErrorMsg)
                            BRDC_printf(_BRDC("%s"), Message);
                        if (SaveToFile && ErrorMsg) {
                            if (ErrorMsg) {
                                FILE* fstream = NULL;
                                fstream = BRDC_fopen(_BRDC("0.txt"), _BRDC("a+t, ccs=UTF-8"));
                                if (fstream) {
                                    //								int numwritten = fwrite(Message, sizeof(BRDCHAR), lstrlen(Message), fstream);
                                    fwrite(Message, sizeof(BRDCHAR), BRDC_strlen(Message), fstream);
                                    fclose(fstream);
                                }
                            }
                        } else {
                            BRDC_printf(_BRDC("Press Esc key for breaking program...\r"));
                            int key_char = IPC_getch();
                            if (key_char == 0x1B) // Esc
                                break;
                            BRDC_printf(_BRDC("                                        \r"));
                        }
                    }
                }
            }
        }
        if (flOK && TestOn) {
            BRDCHAR errMsg[80];
            BRDC_sprintf(errMsg, _BRDC("Test is complete! Total errors = %d.\n"), all_error_num);
            //			BRDC_printf(_BRDC("Test is complete! Error numbers = %d.\n", error_num);
            BRDC_printf(_BRDC("%s"), errMsg);
            if (SaveToFile) {
                FILE* fstream = NULL;
                fstream = BRDC_fopen(_BRDC("0.txt"), _BRDC("a+t, ccs=UTF-8"));
                if (fstream) {
                    //					int numwritten = fwrite(Message, sizeof(BRDCHAR), lstrlen(Message), fstream);
                    fwrite(errMsg, sizeof(BRDCHAR), BRDC_strlen(errMsg), fstream);
                    fclose(fstream);
                }
            }
        }
        //		status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_FREEBUF, &buf_dscr_tst);
        status = BRD_ctrl(hTest, 0, BRDctrl_STREAM_CBUF_FREE, &buf_dscr_tst);
        delete[] buf_dscr_tst.ppBlk;

        //		WriteDataFile(0, buf_dscr_adc.ppBlk[0], buf_dscr_adc.blkSize);

        status = BRD_ctrl(hDAC, 0, BRDctrl_STREAM_CBUF_FREE, &buf_dscr_dac);
        delete[] buf_dscr_dac.ppBlk;

        ULONG tst_mode = 0;
        status = BRD_ctrl(hDAC, 0, BRDctrl_DAC_SETTESTMODE, &tst_mode);
    }
    return status;
}

// выполнить вывод данных в SDRAM с ПДП-методом передачи из ПК
S32 DdrOut(BRD_Handle hDdrOut, PVOID pBuf, U32 sizeBuf)
{
    S32 status;
    ULONG Status = 0;

    ULONG tetrad;
    // установить, что стрим работает с памятью
    status = BRD_ctrl(hDdrOut, 0, BRDctrl_SDRAM_GETSRCSTREAM, &tetrad);
    status = BRD_ctrl(hDdrOut, 0, BRDctrl_STREAM_SETSRC, &tetrad);

    BRDctrl_StreamCBufStart start_pars;
    start_pars.isCycle = 0;
    ULONG msTimeout = 2000; //-1;

    status = BRD_ctrl(hDdrOut, 0, BRDctrl_SDRAM_FIFORESET, NULL); // сброс FIFO SDRAM

    status = BRD_ctrl(hDdrOut, 0, BRDctrl_STREAM_RESETFIFO, NULL);

    // дожидаемся окончания записи
    int cnt = int(ActiveZoneSize / sizeBuf) + (int(ActiveZoneSize % sizeBuf) ? 1 : 0);
    for (int i = 0; i < cnt; i++) {
        status = BRD_ctrl(hDdrOut, 0, BRDctrl_STREAM_CBUF_START, &start_pars); // стартуем вывод тестовой последовательности
        status = BRD_ctrl(hDdrOut, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout); // дожидаемся окончания вывода тестовой последовательности
        if (BRD_errcmp(status, BRDerr_WAIT_TIMEOUT)) { // если вышли по тайм-ауту
            status = BRD_ctrl(hDdrOut, 0, BRDctrl_SDRAM_ISACQCOMPLETE, &Status); // проверяем заполнилась ли активная зона памяти
            if (!Status) {
                DisplayError(status, _BRDC("TIME-OUT"));
                BRDC_printf(_BRDC("Press any key for continue program...\n"));
                IPC_getch();
                status = BRD_ctrl(hDdrOut, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
            }
        }
        // status = BRD_ctrl(hDAC, 0, BRDctrl_SDRAM_ISACQCOMPLETE, &Status); // проверяем заполнилась ли активная зона памяти
        BRDCHAR Msg[128];
        // ULONG address = MemoryAddress + index * (sizeBuf / sizeof(int));
        ULONG address = MemoryAddress + i * (sizeBuf / sizeof(int));
        BRDC_sprintf(Msg, _BRDC("Writing to SDRAM: %0X\r"), address);
        BRDC_printf(_BRDC("%s"), Msg);
        //		BRDC_printf(_BRDC(".");
        // ContinueTestBuffer(pBuf, sizeBuf, index);
        ContinueTestBuffer(pBuf, sizeBuf, i + 1);
    }
    BRDC_printf(_BRDC("                                                  \r"));
    //	BRDC_printf(_BRDC("\n");

    status = BRD_ctrl(hDdrOut, 0, BRDctrl_STREAM_CBUF_STOP, NULL); // останавливаем вывод тестовой последовательности

    return status;
}

S32 DdrInOutTesting(BRD_Handle hDdrIn, BRD_Handle hDdrOut)
{
    S32 status;

    BRD_SdramCfgEx SdramConfig;
    SdramConfig.Size = sizeof(BRD_SdramCfgEx);
    ULONG PhysMemSize;
    status = BRD_ctrl(hDdrOut, 0, BRDctrl_SDRAM_GETCFGEX, &SdramConfig);
    if (status < 0)
        PhysMemSize = 0;
    else {
        if (SdramConfig.MemType == 11 || // DDR3
            SdramConfig.MemType == 12) // DDR4
            PhysMemSize = (ULONG)((
                                      (((int64_t)SdramConfig.CapacityMbits * 1024 * 1024) >> 3) * (int64_t)SdramConfig.PrimWidth / SdramConfig.ChipWidth * SdramConfig.ModuleBanks * SdramConfig.ModuleCnt)
                >> 2); // в 32-битных словах
        else
            PhysMemSize = (1 << SdramConfig.RowAddrBits) * (1 << SdramConfig.ColAddrBits) * SdramConfig.ModuleBanks * SdramConfig.ChipBanks * SdramConfig.ModuleCnt * 2; // в 32-битных словах
    }
    if (!PhysMemSize)
        return -1;

    BRDC_printf(_BRDC("SDRAM Config: Memory size = %d MBytes\n"), (PhysMemSize / (1024 * 1024)) * 4);

    BRDctrl_StreamCBufAlloc buf_dscr_out;
    buf_dscr_out.dir = BRDstrm_DIR_OUT;
    buf_dscr_out.isCont = 1;
    buf_dscr_out.blkNum = 1;
    buf_dscr_out.blkSize = 4096 * 256; // 1 MB
    //	buf_dscr_dac.blkSize = 4096 * 1024; // 4 MB

    buf_dscr_out.ppBlk = new PVOID[buf_dscr_out.blkNum];
    status = BRD_ctrl(hDdrOut, 0, BRDctrl_STREAM_CBUF_ALLOC, &buf_dscr_out);

    void* pBuf_out = (int*)buf_dscr_out.ppBlk[0];

    status = SdramSettings(hDdrOut, ActiveZoneSize); // установить параметры SDRAM

    IPC_TIMEVAL start, stop;
    IPC_getTime(&start);
    DdrOut(hDdrOut, buf_dscr_out.ppBlk[0], buf_dscr_out.blkSize);
    IPC_getTime(&stop);
    double msTime = IPC_getDiffTime(&start, &stop);
    BRDC_printf(_BRDC("Write is complete. Writing time is %.2f ms\n"), msTime);

    BRDctrl_StreamCBufAlloc buf_dscr_in;
    buf_dscr_in.dir = BRDstrm_DIR_IN;
    buf_dscr_in.isCont = 1;
    buf_dscr_in.blkNum = 1;
    buf_dscr_in.blkSize = 4096 * 256; // 1 MB
    //	buf_dscr_dac.blkSize = 4096 * 1024; // 4 MB

    buf_dscr_in.ppBlk = new PVOID[buf_dscr_in.blkNum];
    status = BRD_ctrl(hDdrIn, 0, BRDctrl_STREAM_CBUF_ALLOC, &buf_dscr_in);

    int* buf_in = (int*)buf_dscr_in.ppBlk[0];
    for (U32 i = 0; i < buf_dscr_in.blkSize / sizeof(int); i++)
        buf_in[i] = 0;

    status = SdramSettings(hDdrIn, ActiveZoneSize); // установить параметры SDRAM

    ULONG tetrad;
    status = BRD_ctrl(hDdrIn, 0, BRDctrl_SDRAM_GETSRCSTREAM, &tetrad);
    status = BRD_ctrl(hDdrIn, 0, BRDctrl_STREAM_SETSRC, &tetrad);

    ULONG start_rd = 1;
    status = BRD_ctrl(hDdrIn, 0, BRDctrl_SDRAM_READENABLE, &start_rd);

    BRDctrl_StreamCBufStart start_pars;
    start_pars.isCycle = 0;
    ULONG msTimeout = 10000;
    status = BRD_ctrl(hDdrIn, 0, BRDctrl_STREAM_RESETFIFO, NULL);
    for (int iRead = 0; iRead < MemRead; iRead++) {
        double startStopTime = 0.0;

        // if (MemReadMode)
        //{
        //	status = BRD_ctrl(hDdrIn, 0, BRDctrl_STREAM_RESETFIFO, NULL);
        //	status = BRD_ctrl(hDdrIn, 0, BRDctrl_SDRAM_SETREADADDR, &MemoryAddress); // установить адрес чтения
        // }
        int key_char = 0;
        error_num = 0;
        int* buf_out = (int*)pBuf_out;
        if (TestMode == 3)
            srand(Seed);
        if (TestMode == 6)
            init_genrand(Seed);
        for (int idx = 0; idx < int(ActiveZoneSize / buf_dscr_in.blkSize); idx++) {
            ULONG rden = 1;
            // if (MemReadMode) // разрешение чтения в произвольном режиме
            //{
            //	status = BRD_ctrl(hDdrIn, 0, BRDctrl_SDRAM_FIFORESET, NULL);
            //	status = BRD_ctrl(hDdrIn, 0, BRDctrl_SDRAM_READENABLE, &rden);
            // }
            // QueryPerformanceCounter (&StartPerformCount);
            IPC_getTime(&start);
            status = BRD_ctrl(hDdrIn, 0, BRDctrl_STREAM_CBUF_START, &start_pars);
            status = BRD_ctrl(hDdrIn, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout);
            // QueryPerformanceCounter (&StopPerformCount);
            // startStopTime += StopPerformCount.QuadPart - StartPerformCount.QuadPart;
            IPC_getTime(&stop);
            startStopTime += IPC_getDiffTime(&start, &stop);
            ULONG address = MemoryAddress + idx * (buf_dscr_in.blkSize / sizeof(int));
            rden = 0;
            // if (MemReadMode) // запрещение чтения в произвольном режиме
            //{
            //	status = BRD_ctrl(hDdrIn, 0, BRDctrl_SDRAM_READENABLE, &rden);
            //	status = BRD_ctrl(hDdrIn, 0, BRDctrl_STREAM_RESETFIFO, NULL);
            //	ULONG addr_mem = MemoryAddress + (idx + 1) * (buf_dscr_in.blkSize / sizeof(int));
            //	status = BRD_ctrl(hDdrIn, 0, BRDctrl_SDRAM_SETREADADDR, &addr_mem); // установить адрес чтения
            // }
            //					BRDCHAR Msg[128];
            //					sprintf(Msg, "Testing to SDRAM: %0X\r", address);
            if (TestOn) {
                BRDC_printf(_BRDC("Testing of SDRAM: %0X (reading time is %.2f ms)\r"), address, startStopTime);
                // if (firstTime)
                //{
                //	//WriteDataFile(buf_adc, buf_dscr_adc.blkSize);
                //	firstTime = 0;
                // }
                // TestBufferForComp(buf_dscr_tst.ppBlk[0], buf_dscr_adc.blkSize, (short)idx);
                TestBufferForComp(pBuf_out, buf_dscr_in.blkSize, (short)idx);
                U32 cnt = buf_dscr_in.blkSize / sizeof(int);
                for (U32 i = 0; i < cnt; i++) {
                    U32 imask = i % 4;
                    U32 val_adc = buf_in[i] & TestMask[imask];
                    U32 val_tst = buf_out[i] & TestMask[imask];
                    // if(buf_adc[i] != buf_tst[i])
                    if (val_adc != val_tst) {
                        error_num++;
                        all_error_num++;
                        BRDCHAR Message[128];
                        address = MemoryAddress + cnt * idx + i;
                        //							BRDC_printf(Message, //"Начальное значение функции генерации псевдослучайных чисел = %04X\n"
                        //											"Error to offset: %08X, write: %08X, read: %08X\n",
                        //											"Начать чтение сначала (Да), продолжить дальше (Нет) или прервать (Отмена)?",
                        // Seed, dwMemBufAddr + i, pTestMask[i], pBuf[i]);
                        //											address, buf_tst[i+2], buf_adc[i]);
                        BRDC_sprintf(Message, _BRDC("Error to offset: %08X, write: %08X, read: %08X, XOR: %08X; reading = %d\n"),
                            // address, buf_tst[i], buf_adc[i], buf_tst[i] ^ buf_adc[i], iRead);
                            address, val_tst, val_adc, val_tst ^ val_adc, iRead);
                        if (ErrorMsg)
                            BRDC_printf(_BRDC("%s"), Message);
                        if (SaveToFile) {
                            if (ErrorMsg) {
                                FILE* fstream = NULL;
                                fstream = BRDC_fopen(_BRDC("0.txt"), _BRDC("a+t, ccs=UTF-8"));
                                if (fstream) {
                                    //										int numwritten = fwrite(Message, sizeof(BRDCHAR), lstrlen(Message), fstream);
                                    fwrite(Message, sizeof(BRDCHAR), BRDC_strlen(Message), fstream);
                                    fclose(fstream);
                                }
                            }
                        } else {
                            BRDC_printf(_BRDC("Press Esc key for breaking program...\n"));
                            BRDC_printf(_BRDC("Press Enter key for breaking read cycle...\n"));
                            key_char = IPC_getch();
                            if (key_char == 0x1B || key_char == 0x0D) // Esc или Enter
                                break;
                            BRDC_printf(_BRDC("                                        \r"));
                        }
                    }
                }
                if (key_char == 0x1B || key_char == 0x0D) // Esc или Enter
                    break;
            } else
                BRDC_printf(_BRDC("Reading of SDRAM: %0X\r"), address);
            if (key_char == 0x1B || key_char == 0x0D) // Esc или Enter
                break;
        }

        // double sTime = (double)startStopTime / (double)Frequency.QuadPart;
        double sTime = startStopTime / 1.E3;
        BRDC_printf(_BRDC("Reading time is %.2f ms, size = %d MByte, speed is %.2f MBps\n"),
            sTime * 1.E3, (int)(ActiveZoneSize / 1024 / 1024), (double)(ActiveZoneSize / 1024 / 1024) / sTime);

        if (key_char == 0x1B) // Esc
            break; // выход совсем
        BRDCHAR errMsg[80];
        BRDC_sprintf(errMsg, _BRDC("%d reading is complete! Error numbers = %d.         \n"), iRead, error_num);
        BRDC_printf(_BRDC("%s"), errMsg);

        if (SaveToFile) {
            FILE* fstream = NULL;
            fstream = BRDC_fopen(_BRDC("0.txt"), _BRDC("a+t, ccs=UTF-8"));
            if (fstream) {
                //					int numwritten = fwrite(Message, sizeof(BRDCHAR), lstrlen(Message), fstream);
                fwrite(errMsg, sizeof(BRDCHAR), BRDC_strlen(errMsg), fstream);
                fclose(fstream);
            }
        }
    }
    if (!TestOn)
        BRDC_printf(_BRDC("Read is complete!                                        \n"));

    status = BRD_ctrl(hDdrOut, 0, BRDctrl_STREAM_CBUF_FREE, &buf_dscr_out);
    delete[] buf_dscr_out.ppBlk;

    status = BRD_ctrl(hDdrIn, 0, BRDctrl_STREAM_CBUF_FREE, &buf_dscr_in);
    delete[] buf_dscr_in.ppBlk;

    return status;
}
