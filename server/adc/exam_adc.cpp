//********************************************************
//
// Example Application for ADC service
//
// (C) InSys, 2007-2017
//
// https://github.com/andorok/DaqTools_Tutorial - Пошаговое пособие,
// иллюстрирующее пакет DaqTools и приемы работы с АЦП в том числе
//
////********************************************************

#include <chrono>
#include <fcntl.h>
#include <math.h>
#include <stdexcept>
#include <stdio.h>

#include "../total.h"
#include "adc_ctrl.h"

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
#include "gipcy.h"
#include <ctype.h>
#include <signal.h>
#include <string.h>
void WriteIsviParams(IPC_handle hfile, int lid, unsigned long long nNumberOfBytes);
#else
#include <conio.h>
#include <io.h>
#include <share.h>
#include <sys\stat.h>
#include <windows.h>

void WriteIsviParams(HANDLE hfile, int lid, unsigned long long nNumberOfBytes);
#endif

S32 SetParamSrv(int lid /*, BRD_ServList* srv, int idx*/);
S32 GetAdcData(int lid, unsigned long long bBufSize, unsigned long long bMemBufSize);

S32 DataFromMemWriteFile(int lid /*BRD_Handle hADC*/, PVOID* pBuf, unsigned long long bBufSize, unsigned long long bMemBufSize, ULONG DmaOn);
void WriteDataFile(int lid /*BRD_Handle hADC*/, PVOID* pBuf, unsigned long long nNumberOfBytes);
void WriteFlagSinc(int flg, int isNewParam, int lid);
int ReadFlagSinc(int lid);

// Options
/*
BRDCHAR g_AdcSrvName[64]; // с номером службы
BRDCHAR g_SrvName[64]; // без номера службы
int g_AdcSrvNum; // номер службы
BRDCHAR g_pldFileName[MAX_PATH];
int g_isPldLoadAlways = 0;
int g_OnlySetParams = 0; // 1 - только установка параметров АЦП

// int g_lid_adc = -1;
int g_subNo = 0; // номер службы АЦП из командной строки
ULONG g_times = 0;

int g_fileMap = 0;
ULONG g_DmaOn = 1;
ULONG g_Cycle = 0;
ULONG g_Pause = 0;
ULONG g_MemOn = 1;
ULONG g_IsWriteFile = 1;
ULONG g_IsSysMem = 0;
unsigned long long g_samplesOfChannel = 16384;
unsigned long long g_memorySamplesOfChannel = 16384;

ULONG DspMode = 0; // использовать ли ПЛИС ЦОС
ULONG g_MemAsFifo = 0;

ULONG g_AdcDrqFlag;
ULONG g_MemDrqFlag;

int g_IoDelay;
ULONG g_SwitchOutMask; // маска включенных выходов коммутатора для FMC-модулей

uint64_t g_bBufSize;
uint64_t g_bMemBufSize;
ULONG g_bBlkNum;
ULONG g_FileBlkNum;
ULONG g_MsTimeout;
*/

void DirectFile(int lid, ULONG bufType, ULONG FileBufSize, ULONG FileBufNum, ULONG FileBlkNum);
void WriteIsviParamDirFile(int lid);

/*
int g_DirWriteFile;
ULONG g_FileBufSize;
BRDCHAR g_dirFileName[MAX_PATH] = _BRDC("adcdir.bin");
ULONG g_adjust_mode = 0;
ULONG g_quick_quit = 0;

int g_PretrigMode = 0;
// long long g_nPreTrigSamples = 16;
long long g_nPostTrigSamples = 16384;
long long g_bPostTrigSize = 32768;
long long g_bStartEvent;
long long g_bRealSize;
ULONG g_isPassMemEnd;

int g_numChan;

BRDCHAR g_iniFileNameAdc[FILENAME_MAX] = _BRDC("//exam_adc.ini");

BRD_Info g_info;
double g_samplRate;

int g_transRate;

void* g_pMapBuf;
int g_regdbg;

// U32 modeAdcServiceCapture = BRDcapt_EXCLUSIVE;
*/

S32 RegProg(BRD_Handle hAdc, int idx, int isx);

#if defined(__IPC_LINUX__)
/*
static int g_StopFlag = 0;
*/

void stop_exam(int sig)
{
    fprintf(stderr, "\n");
    fprintf(stderr, "SIGNAL = %d\n", sig);
    for (auto& dp : DevicesLid)
        dp.paramsAdc.g_StopFlag = 1;
}
#endif

/*
// Получить параметр типа строка из ini-файла:
//    FileName - имя ini-файла (с путем, если нужно)
//    SectionName - название секции
//    ParamName - название параметра (ключа)
//    defValue - значение параметра по-умолчанию (если параметра в файле нет)
//    strValue - значение параметра
//    strSize - максимальная длина параметра
static void GetInifileString(const BRDCHAR* FileName, const BRDCHAR* SectionName, const BRDCHAR* ParamName, const BRDCHAR* defValue, BRDCHAR* strValue, int strSize)
{
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
    IPC_getPrivateProfileString(SectionName, ParamName, defValue, strValue, strSize, FileName);
#else
    GetPrivateProfileString(SectionName, ParamName, defValue, strValue, strSize, FileName);
#endif
    // удалить комментарий из строки
    BRDCHAR* pChar = BRDC_strchr(strValue, ';'); // признак комментария или ;
    if (pChar)
        *pChar = 0;
    pChar = BRDC_strchr(strValue, '/'); // или //
    if (pChar)
        if (*(pChar + 1) == '/')
            *pChar = 0;

    // Удалить пробелы в конце строки
    int str_size = (int)BRDC_strlen(strValue);
    for (int i = str_size - 1; i > 1; i--)
        if (strValue[i] != ' ' && strValue[i] != '\t') {
            strValue[i + 1] = 0;
            break;
        }
}
*/

// получить параметры из ini-файла
void adcGetOptions(int lid, const std::string inifile)
{
    BRDCHAR Buffer[4096];
    BRDCHAR iniFilePath[4096];

#if defined(__IPC_LINUX__)
    signal(SIGINT, stop_exam);
#endif
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    strcpy(p.g_iniFileNameAdc, inifile.c_str());

    //    IPC_getCurrentDir(iniFilePath, sizeof(iniFilePath) / sizeof(BRDCHAR));
    //    BRDC_strcat(iniFilePath, "/");
    //    BRDC_strcat(iniFilePath, g_iniFileNameAdc);
    S32 err = IPC_getFullPath(inifile.c_str(), iniFilePath);
    if (0 > err) {
        BRDC_printf(_BRDC("ERROR: Can't find ini-file '%s'\n\n"), inifile.c_str());
        return;
    }
    strcpy(p.g_iniFileNameAdc, iniFilePath);

    printf("<SRV> Filename: g_iniFileNameAdc = %s \n", p.g_iniFileNameAdc);

    BRDCHAR* endptr;
    // GetPrivateProfileString("Option", "AdcServiceName", "ADC212X200M", g_SrvName, sizeof(g_SrvName), iniFilePath);
    // GetInifileString(iniFilePath, _BRDC("Option"), _BRDC("AdcServiceName"), _BRDC("ADC214x3GDA"), p.g_SrvName, sizeof(p.g_SrvName));
    GetInifileString(_BRDC("Option"), _BRDC("AdcServiceName"), _BRDC("ADC214x3GDA"), p.g_SrvName, sizeof(p.g_SrvName), iniFilePath);
    // BRDC_strncpy(g_SrvName, Buffer, 64);
    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("AdcServiceNum"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    GetInifileInt(_BRDC("Option"), _BRDC("AdcServiceNum"), _BRDC("0"), p.g_AdcSrvNum, iniFilePath);
    // p.g_AdcSrvNum = BRDC_atoi(Buffer);
    if (p.g_subNo >= 0)
        p.g_AdcSrvNum = p.g_subNo;
    BRDC_sprintf(p.g_AdcSrvName, _BRDC("%s%d"), p.g_SrvName, p.g_AdcSrvNum);

    GetInifileString(_BRDC("Option"), _BRDC("PldFileName"), _BRDC("ambpcd_v10_adm212x200m.mcs"), p.g_pldFileName, sizeof(p.g_pldFileName), iniFilePath);

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("isPldLoadAlways"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_isPldLoadAlways = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("isPldLoadAlways"), _BRDC("0"), p.g_isPldLoadAlways, iniFilePath);

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("BusMasterEnable"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_DmaOn = BRDC_atoi(Buffer);
    int temp;
    GetInifileInt(_BRDC("Option"), _BRDC("BusMasterEnable"), _BRDC("1"), temp, iniFilePath);
    p.g_DmaOn = temp;

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("Cycle"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_Cycle = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("Cycle"), _BRDC("0"), temp, iniFilePath);
    p.g_Cycle = temp;

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("Pause"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_Pause = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("Pause"), _BRDC("0"), temp, iniFilePath);
    p.g_Pause = temp;

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("DaqIntoMemory"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_MemOn = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("DaqIntoMemory"), _BRDC("0"), temp, iniFilePath);
    p.g_MemOn = temp;

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("IsWriteFile"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_IsWriteFile = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("IsWriteFile"), _BRDC("0"), temp, iniFilePath);
    p.g_IsWriteFile = temp;

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("SamplesPerChannel"), _BRDC("16384"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_samplesOfChannel = BRDC_atoi64(Buffer);
    long long bv;
    GetInifileBig(_BRDC("Option"), _BRDC("SamplesPerChannel"), _BRDC("16384"), bv, iniFilePath);
    p.g_samplesOfChannel = bv;

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("MemSamplesPerChan"), _BRDC("16384"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_memorySamplesOfChannel = BRDC_atoi64(Buffer);
    GetInifileBig(_BRDC("Option"), _BRDC("MemSamplesPerChan"), _BRDC("16384"), bv, iniFilePath);
    p.g_memorySamplesOfChannel = bv;

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("IsSystemMemory"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_IsSysMem = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("IsSystemMemory"), _BRDC("0"), temp, iniFilePath);
    p.g_IsSysMem = temp;

    if (p.g_IsSysMem == 1 && p.g_info.busType == BRDbus_ETHERNET) {
        BRDC_printf(_BRDC("Ethernet: IsSystemMemory=0 or IsSystemMemory=2 (NOT EQUAL 1)!!!\n"));
        p.g_IsSysMem = 0;
    }

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("DirFileBufSize"), _BRDC("64"), Buffer, sizeof(Buffer), iniFilePath); // KBytes
    // p.g_FileBufSize = BRDC_atoi(Buffer) * 1024;
    GetInifileInt(_BRDC("Option"), _BRDC("DirFileBufSize"), _BRDC("64"), temp, iniFilePath);
    p.g_FileBufSize = temp;
    p.g_FileBufSize *= 1024;

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("DirNumBufWrite"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath); // KBytes
    // p.g_DirWriteFile = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("DirNumBufWrite"), _BRDC("0"), p.g_DirWriteFile, iniFilePath);

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("DirNumBlock"), _BRDC("2"), Buffer, sizeof(Buffer), iniFilePath); // KBytes
    // p.g_FileBlkNum = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("DirNumBlock"), _BRDC("2"), temp, iniFilePath);
    p.g_FileBlkNum = temp;
    if (p.g_FileBlkNum < 2)
        p.g_FileBlkNum = 2;

    GetInifileString(_BRDC("Option"), _BRDC("DirFileName"), _BRDC("adcdir.bin"), p.g_dirFileName, sizeof(p.g_dirFileName), iniFilePath);

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("AdcDrqFlag"), _BRDC("2"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_AdcDrqFlag = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("AdcDrqFlag"), _BRDC("2"), temp, iniFilePath);
    p.g_AdcDrqFlag = temp;

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("MemDrqFlag"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_MemDrqFlag = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("MemDrqFlag"), _BRDC("0"), temp, iniFilePath);
    p.g_MemDrqFlag = temp;

    // IPC_getPrivateProfileString(_BRDC("Debug"), _BRDC("IoDelay"), _BRDC("128"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_IoDelay = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("IoDelay"), _BRDC("128"), p.g_IoDelay, iniFilePath);

    if (p.g_IsWriteFile == 4) {
        BRDC_printf(_BRDC("File mapping mode: IsWriteFile=0, IsSystemMemory=2 !!!\n"));
        p.g_IsWriteFile = 0;
        p.g_fileMap = 1;
        p.g_IsSysMem = 2;
    }

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("RateRate"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_transRate = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("RateRate"), _BRDC("0"), p.g_transRate, iniFilePath);

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("TimeoutSec"), _BRDC("5"), Buffer, sizeof(Buffer), iniFilePath); // sec
    // p.g_MsTimeout = BRDC_atoi(Buffer) * 1000;
    GetInifileInt(_BRDC("Option"), _BRDC("TimeoutSec"), _BRDC("5"), temp, iniFilePath);
    p.g_MsTimeout = temp;
    p.g_MsTimeout *= 1000;

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("SwitchOutMask"), _BRDC("0x18"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_SwitchOutMask = BRDC_strtol(Buffer, &endptr, 0);
    GetInifileInt(_BRDC("Option"), _BRDC("SwitchOutMask"), _BRDC("0x18"), temp, iniFilePath);
    p.g_SwitchOutMask = temp;

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("RegDbg"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_regdbg = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("RegDbg"), _BRDC("0"), p.g_regdbg, iniFilePath);

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("AdjustMode"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_adjust_mode = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("AdjustMode"), _BRDC("0"), temp, iniFilePath);
    p.g_adjust_mode = temp;

    // IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("QuickQuit"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    // p.g_quick_quit = BRDC_atoi(Buffer);
    GetInifileInt(_BRDC("Option"), _BRDC("QuickQuit"), _BRDC("0"), temp, iniFilePath);
    p.g_quick_quit = temp;
}

#include "ctrlbasef.h"
//=******************* SetSwitchAdc *****************************
//=************************************************************
void SetSwitchAdc(int lid /*BRD_Handle handle*/)
{
    S32 status;
    U32 mode = BRDcapt_SHARED;
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    BRD_Handle handle = DevicesLid[lid].device.handle();
    BRD_Handle hSrv = BRD_capture(handle, 0, &mode, _BRDC("BASEFMC0"), 10000);
    if (hSrv <= 0) {
        BRDC_printf(_BRDC("BASEFMC0 NOT capture (0x%X)\n"), hSrv);
        return;
    }
    if (mode == BRDcapt_EXCLUSIVE)
        BRDC_printf(_BRDC("BASEFMC Capture mode: EXCLUSIVE (%X)\n"), hSrv);
    if (mode == BRDcapt_SHARED)
        BRDC_printf(_BRDC("BASEFMC Capture mode: SHARED (%X)\n"), hSrv);
    if (mode == BRDcapt_SPY)
        BRDC_printf(_BRDC("BASEFMC Capture mode: SPY (%X)\n"), hSrv);

    // после подачи питания все выходы коммутатора включены
    BRD_Switch sw;
    for (int i = 0; i < 8; i++) {
        sw.out = i;
        sw.val = (p.g_SwitchOutMask >> i) & 0x1; // 1 = on
        // if(!sw.val)
        //  отключаем ненужные и включаем нужные выходы коммутатора
        status = BRD_ctrl(hSrv, 0, BRDctrl_BASEF_SWITCHONOFF, &sw);
    }

    status = BRD_release(hSrv, 0);
}

// захватываем службу АЦП,
// устанавливаем параметры АЦП,
// получаем размер собираемых данных в байтах,
// проверяем наличие динамической памяти на модуле и, если надо, устанавливаем ее параметры, включая размер активной зоны
S32 SetParamSrv(int lid /*, BRD_ServList* srv, int idx*/)
{
    S32 status = BRDerr_OK;

    if (!DevicesLid[lid].device.isOpen()) {
        printf("<ERR> SetParamSrv: device d'nt open .. \n");
        return -1;
    }
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    BRD_Handle handle = DevicesLid[lid].device.handle();
    DevicesLid[lid].adc.capture(handle, p.g_AdcSrvName, 10000);
    if (!DevicesLid[lid].adc.isCaptured()) {
        printf("<ERR> SetParamSrv: service %s not captured ..\n", p.g_AdcSrvName);
        return -1;
    }

    U32 md = DevicesLid[lid].adc.mode();
    BRD_Handle hADC = DevicesLid[lid].adc.handle();

    BRDC_printf(_BRDC("%s"), DevicesLid[lid].adc.strMode());

    if (hADC > 0) {
        if (p.g_SwitchOutMask <= 0xFF)
            SetSwitchAdc(lid /*handle*/);

        p.g_numChan = AdcSettings(lid); // установить параметры АЦП

        ULONG format = 0;
        BRD_ctrl(hADC, 0, BRDctrl_ADC_GETFORMAT, &format);
        ULONG sample_size = format ? format : sizeof(short);
        if (format == 0x80) // упакованные 12-разрядные данные
            sample_size = 2;

        p.g_bBufSize = (p.g_samplesOfChannel * p.g_numChan) * sample_size; // получить размер собираемых данных в байтах

        if (!p.g_DirWriteFile)
            BRDC_printf(_BRDC("<SRV> SetParamSrv: SamplesPerChannel = %lld (%lld kBytes)\n"), p.g_samplesOfChannel, p.g_bBufSize / 1024);

        p.g_bMemBufSize = (p.g_memorySamplesOfChannel * p.g_numChan) * sample_size; // получить размер собираемых в память данных в байтах

        p.g_bPostTrigSize = (p.g_nPostTrigSamples * p.g_numChan) * sample_size; // получить размер пост-триггерных данных (в байтах), собираемых в память

        // проверяем наличие динамической памяти
        BRD_SdramCfgEx SdramConfig;
        SdramConfig.Size = sizeof(BRD_SdramCfgEx);
        uint64_t PhysMemSize;
        status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_GETCFGEX, &SdramConfig);
        if (status < 0) {
            if (p.g_MemOn) {
                BRDC_printf(_BRDC("<ERR> SetParamSrv: Get SDRAM Config: status = 0x%X!!!\n"), status);
                p.g_MemOn = 0;
            }
            PhysMemSize = 0;
            return BRDerr_OK;
        } else {
            if (SdramConfig.MemType == 11 || // DDR3
                SdramConfig.MemType == 12) // DDR4
                PhysMemSize = (uint64_t)((
                                             (((__int64)SdramConfig.CapacityMbits * 1024 * 1024) >> 3) * (__int64)SdramConfig.PrimWidth / SdramConfig.ChipWidth * SdramConfig.ModuleBanks * SdramConfig.ModuleCnt)
                    >> 2); // в 32-битных словах
            else
                PhysMemSize = (1 << SdramConfig.RowAddrBits) * (1 << SdramConfig.ColAddrBits) * SdramConfig.ModuleBanks * SdramConfig.ChipBanks * SdramConfig.ModuleCnt * 2; // в 32-битных словах
        }
        if (PhysMemSize && p.g_MemOn) { // динамическая память присутствует на модуле
            BRDC_printf(_BRDC("<SRV> SetParamSrv: SDRAM Config: Memory size = %d MBytes\n"), (PhysMemSize / (1024 * 1024)) * 4);

            status = SdramSettings(p.g_MemOn, lid, p.g_bMemBufSize); // установить параметры SDRAM
            p.g_memorySamplesOfChannel = (p.g_bMemBufSize / sample_size) / p.g_numChan;
            if (p.g_MemOn == 2) {
                BRDC_printf(_BRDC("<SRV> SetParamSrv: SDRAM as a FIFO mode!!!\n"));
                p.g_MemOn = 0;
                p.g_MemAsFifo = 1;
            } else
                BRDC_printf(_BRDC("<SRV> SetParamSrv: Samples of channel = %lld\n"), p.g_memorySamplesOfChannel);

        } else {
            // освободить службу SDRAM (она могла быть захвачена командой BRDctrl_SDRAM_GETCFG, если та отработала без ошибки)
            ULONG mem_size = 0;
            status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_SETMEMSIZE, &mem_size);
            if (p.g_MemOn) {
                BRDC_printf(_BRDC("<SRV> SetParamSrv: No SDRAM on board!!!\n"));
                p.g_MemOn = 0;
            }

            // установка узла ЦОС в режим получения данных от ПЛИС ЦОС
            // ULONG p.DspMode = 1;
            // status = BRD_ctrl(hADC, 0, BRDctrl_DSPNODE_SETMODE, &p.DspMode);
            // if(status < 0) // с узлом ЦОС работать нельзя
            //	p.DspMode = 0;
        }
    }

    return status;
}

// получить параметры, актуальные в режиме претриггера при вводе через FIFO
S32 GetPretrigData(int lid)
{
    S32 status;
    BRD_Handle hADC = DevicesLid[lid].adc.handle();
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    // получить момент срабатывания на событие старта в режиме претриггера
    ULONG start_event;
    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETPRETRIGEVENT, &start_event);
    p.g_bStartEvent = start_event << 2; // запомнить в байтах
    // уточнить момент срабатывания
    ULONG prec_event;
    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETPREVENTPREC, &prec_event);
    ULONG bPrecEvent = prec_event << 1; // запомнить в байтах
    p.g_bStartEvent += bPrecEvent * p.g_numChan;

    return status;
}

// получить параметры, актуальные в режиме претриггера (пост-триггера) при вводе через SDRAM
S32 GetPostrigData(int lid)
{
    S32 status;
    BRD_Handle hADC = DevicesLid[lid].adc.handle();
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    // получить момент срабатывания на событие старта в режиме претриггера
    ULONG start_event;
    status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_GETPRETRIGEVENT, &start_event);
    p.g_bStartEvent = start_event << 2; // запомнить в байтах
    // уточнить момент срабатывания
    ULONG prec_event;
    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETPREVENTPREC, &prec_event);
    ULONG bPrecEvent = prec_event << 1; // запомнить в байтах
    p.g_bStartEvent += bPrecEvent * p.g_numChan;

    // выяснить был ли переход через конец активной зоны
    status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_ISPASSMEMEND, &p.g_isPassMemEnd);
    // получить реальное число собранных данных
    ULONG acq_size;
    status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_GETACQSIZE, &acq_size);
    p.g_bRealSize = acq_size << 2; // запомнить в байтах

    return status;
}

S32 GetAdcData(int lid, unsigned long long bBufSize, unsigned long long bMemBufSize)
{
    S32 status;
    int idx = lid;

    printf("<DBG>  GetAdcData: ADC-%d is %s (hdl= %X)\n", lid, DevicesLid[lid].device.isOpen() ? "open" : "close",
        DevicesLid[lid].device.handle());

    if (!DevicesLid[lid].adc.isCaptured()) {
        printf("<ERR> GetAdcData: service ADC-%d d'not captured", lid);
        return -1;
    }

    BRD_Handle hADC = DevicesLid[lid].adc.handle();
    ParamsAdc& p = DevicesLid[lid].paramsAdc;

    printf("<DBG>  GetAdcData: service>%s< is %s (hdl=%X)\n", DevicesLid[lid].adc.name().c_str(),
        DevicesLid[lid].adc.isCaptured() ? "capture" : "not capture",
        hADC);

    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_PREPARESTART, NULL);
    if (status < 0)
        if (!(BRD_errcmp(status, BRDerr_CMD_UNSUPPORTED)
                || BRD_errcmp(status, BRDerr_INSUFFICIENT_SERVICES))) {
            BRDC_printf(_BRDC("<ERR> GetAdcData: Prepare Start Error = 0x%X !\n"), status);
            return status;
        }

    // RegProg(hADC, 0, p.g_AdcSrvNum);
    RegProg(hADC, 0, p.g_AdcSrvNum);

    p.g_bBlkNum = 1;
    // PVOID pSig = NULL;
    PVOID* pSig = NULL; // указатель на массив указателей на блоки памяти с сигналом
    PVOID pSigNonDMA = NULL; // указатель на память с сигналом при программном (не ПДП) вводе
    if (p.g_DmaOn) {
        if (!p.g_Cycle)
            BRDC_printf(_BRDC("AllocDaqBuf (memory type = %d)\n"), p.g_IsSysMem);
        // status = AllocDaqBuf(hADC, &pSig, &bBufSize, p.g_IsSysMem);
        status = AllocDaqBuf(lid, pSig, &bBufSize, p.g_IsSysMem, &p.g_bBlkNum);
        if (!BRD_errcmp(status, BRDerr_OK)) {
            printf("<ERR> AllocDaqBuf: status=%X\n ", status);
            return status;
        }
        if (!pSig) {
            BRDC_printf(_BRDC("<ERR> AllocDaqBuf: Buffer NOT allocate!!!\n"));
            return status;
        }
    } else
    // pSig = new char[bBufSize];
    {
        pSigNonDMA = new char[bBufSize];
        pSig = &pSigNonDMA;

        BRD_AdcCfg adc_cfg;
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETCFG, &adc_cfg);
        if (adc_cfg.FifoSize < bBufSize) {
            ULONG format = 0;
            BRD_ctrl(hADC, 0, BRDctrl_ADC_GETFORMAT, &format);
            ULONG sample_size = format ? format : sizeof(short);
            if (format == 0x80) // упакованные 12-разрядные данные
                sample_size = 2;
            // ULONG daq_sample_num = bBufSize / g_numChan / sample_size;
            ULONG fifo_sample_num = adc_cfg.FifoSize / p.g_numChan / sample_size;
            BRDC_printf(_BRDC("Real acquisition data = %d samples per channel (It's FIFO size)\n"), fifo_sample_num);
        }
    }

    // BRDC_printf(_BRDC("Press any key for continue\n"));
    // IPC_getch();

    if (p.g_IsWriteFile || p.g_fileMap)
        WriteFlagSinc(0, 0, lid);
    int newParam_fl = 0xffffffff;
    if (!p.g_Cycle) { // однократный сбор данных
        if (!p.g_DmaOn) { // программная передача данных
            if (p.g_MemOn) { // сбор в память и программная передача данных
                // status = DaqIntoSdram(hADC, pSig, bBufSize, numChan); // выполнить сбор данных
                status = DaqIntoSdram(hADC); // выполнить сбор данных в память
                status = DataFromMemWriteFile(lid, pSig, bBufSize, bMemBufSize, p.g_DmaOn); // передать данные из памяти в ОЗУ ПК, а затем в файл
            } else { // сбор в FIFO и программная передача данных
                status = DaqIntoFifo(lid, pSigNonDMA, (ULONG)bBufSize, p.DspMode); // выполнить сбор данных
                WriteDataFile(lid, pSig, bBufSize);
            }
            //				WriteDataFile(lid, pSig, bBufSize);
        } else { // ПДП-передача данных
            if (p.g_MemOn) { // сбор в память и ПДП-передача данных
                BRDC_printf(_BRDC("DAQ into SDRAM\n"));
                status = DaqIntoSdramDMA(lid); // выполнить сбор данных
                if (status != -1)
                    status = DataFromMemWriteFile(lid, pSig, bBufSize, bMemBufSize, p.g_DmaOn); // передать данные из памяти в ОЗУ ПК, а затем в файл
            } else { // сбор в FIFO и ПДП-передача данных
                BRDC_printf(_BRDC("DAQ into FIFO\n"));
                status = DaqIntoFifoDMA(lid); // выполнить сбор данных
                WriteDataFile(lid, pSig, bBufSize);
            }
        }

        if (p.g_IsWriteFile || p.g_fileMap) {
            int rd_fl = ReadFlagSinc(lid);
            if (rd_fl == 1)
                WriteFlagSinc(1, newParam_fl, lid);
            else
                WriteFlagSinc(0xffffffff, newParam_fl, lid);
        }

    } else { // циклический (многократный) сбор данных
        // if(p.g_IsWriteFile || p.g_fileMap)
        //	WriteFlagSinc(0, 0, 0);
        //				WriteFlagSinc(0, 0, idx);
        BRDC_printf(_BRDC("\n\n"));
        // BRDC_printf(_BRDC("Debugging pause. Press any key....\r"));
        //_getch();
        int loop = 0;
        // int newParam_fl = 0xffffffff;
        do {
            // printf("Send Loop: [0x%08lX]\r", loop );
            // BRDC_printf(_BRDC("Send Loop: [%08d]\r"), loop );

            BRDCHAR lin[500] = _BRDC("... info ...");
            auto rTraceText = BRD_TraceText {
                /*.traceId = */ 1,
                /*.sizeb = */ sizeof(lin),
                /*.pText = */ lin
            };

            auto err = BRD_ctrl(hADC, 0, BRDctrl_GETTRACETEXT, &rTraceText);
            if (err < 0)
                BRDC_printf(_BRDC("Send Loop: [%08d]\r"), loop);
            else
                BRDC_printf(_BRDC("Send Loop: [%08d] %s\r"), loop, lin);

            if (!p.g_DmaOn) { // программная передача данных
                if (p.g_MemOn) { // сбор в память и программная передача данных
                    // status = DaqIntoSdram(hADC, pSig, bBufSize, numChan); // выполнить сбор данных
                    status = DaqIntoSdram(hADC); // выполнить сбор данных в память
                    status = DataFromMemWriteFile(lid, pSig, bBufSize, bMemBufSize, p.g_DmaOn); // передать данные из памяти в ОЗУ ПК, а затем в файл
                } else { // сбор в FIFO и программная передача данных
                    status = DaqIntoFifo(lid, pSigNonDMA, (ULONG)bBufSize, p.DspMode); // выполнить сбор данных
                    WriteDataFile(lid, pSig, bBufSize);
                }
                //					WriteDataFile(lid, pSig, bBufSize);
            } else { // сбор в память и ПДП-передача данных
                if (p.g_MemOn) { // сбор в память и ПДП-передача данных
#if defined(__IPC_LINUX__)
                    status = DaqIntoSdramDMA(lid); // выполнить сбор данных в память без использования прерывания по окончанию сбора
#else
                    if (p.g_info.busType == BRDbus_USB)
                        status = DaqIntoSdramDMA(lid); // если устройство на шине USB, то выполнить сбор данных в память без использования прерывания по окончанию сбора
                    else {
                        status = StartDaqIntoSdramDMA(hADC, idx); // стартует сбор данных в память с использованием прерывания по окончанию сбора
                        while (!CheckDaqIntoSdramDMA(lid)) // проверяет завершение сбора данных
                        {
                            if (DevicesLid[lid].adcCtrlThr.stop.load()) {
                                BreakDaqIntoSdramDMA(lid); // // прерывает сбор данных
                                loop = -1;
                            }
                        }
                        status = EndDaqIntoSdramDMA(lid); // закрывает открытые описатели (Handles)
                    }
#endif
                    // if(!BRD_errcmp(status, BRDerr_OK))
                    //	loop = -1;
                    // if(loop != -1)
                    if (loop != -1 && BRD_errcmp(status, BRDerr_OK))
                        status = DataFromMemWriteFile(lid, pSig, bBufSize, bMemBufSize, p.g_DmaOn); // передать данные из памяти в ОЗУ ПК, а затем в файл
                } else { // сбор в FIFO и ПДП-передача данных
                    status = DaqIntoFifoDMA(lid); // выполнить сбор данных
                    if (loop != -1)
                        WriteDataFile(lid, pSig, bBufSize);
                }
                // WriteDataFile(lid, p.g_buf_dscr.ppBlk[0], bBufSize);
                // WriteDataFile0(idx, buf_dscr.ppBlk[0], bBufSize);
            }
            if (p.g_Pause) {
                BRDC_printf(_BRDC("\nPress any key for next start...\n"));
                IPC_getch();
            }
            if (loop == -1)
                break;
            if (p.g_IsWriteFile || p.g_fileMap) {
                int rd_fl = ReadFlagSinc(lid);
                // BRDC_printf(_BRDC("rd_fl = %d\n"), rd_fl);
                if (rd_fl == 1)
                    WriteFlagSinc(1, newParam_fl, lid);
                else {
                    if (p.g_Cycle <= 1)
                        WriteFlagSinc(0xffffffff, newParam_fl, lid);
                    if (p.g_Cycle > 1 && p.g_times == p.g_Cycle)
                        WriteFlagSinc(0xffffffff, 0xffffffff, lid);
                }
                // WriteFlagSinc(0xffffffff, newParam_fl, lid);
                loop++;
                int rd_idx = 0;
                newParam_fl = 0;
                do {
                    rd_fl = ReadFlagSinc(lid);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
                    if (IPC_kbhit()) {
                        int ch = IPC_getch();
#else
                    if (_kbhit()) {
                        int ch = _getch();
#endif
                        if (0x1B == ch) // Esc
                            loop = 0;
                        if (0x20 == ch) { // Space
                            // my//status = AdcSettings(hADC, idx, p.g_AdcSrvNum, p.g_SrvName, p.g_iniFileNameAdc); // установить параметры АЦП
                            status = AdcSettings(lid); // установить параметры АЦП
                            status = BRD_ctrl(hADC, 0, BRDctrl_ADC_PREPARESTART, NULL);
                            newParam_fl = 0xffffffff;
                        }
                    }
#if defined(__IPC_LINUX__)
                    if ((rd_fl == 0) || p.g_StopFlag)
#else
                    if (rd_fl == 0)
#endif
                    {
                        break;
                    }
                    // if((rd_fl == 0xffffffff) && (rd_idx>50))
                    if ((rd_fl == -1) && (rd_idx > 20))
                        break;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
                    IPC_delay(30);
#else
                    Sleep(30);
#endif
                    rd_idx++;
                } while (loop);
            } else {
                loop++;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
                if (IPC_kbhit())
                    if (0x1B == IPC_getch())
#else
                if (_kbhit())
                    if (0x1B == _getch())
#endif
                        loop = 0;
            }
            if (p.g_Cycle > 1 && (ULONG)loop == p.g_Cycle)
                break;
#if defined(__IPC_LINUX__)
            if (p.g_StopFlag)
                break;
#endif
        } while (loop);
    }
    if (p.g_DmaOn)
        status = FreeDaqBuf(hADC, p.g_bBlkNum);
    else
        delete (char*)pSigNonDMA;

    return status;
}

// S32	DataFromMemWriteFile(BRD_Handle hADC, PVOID pBuf, unsigned long long bBufSize, unsigned long long bMemBufSize, ULONG DmaOn)
S32 DataFromMemWriteFile(int lid /*BRD_Handle hADC*/, PVOID* pBuf, unsigned long long bBufSize, unsigned long long bMemBufSize, ULONG DmaOn)
{
    if (!DevicesLid[lid].adc.isCaptured()) {
        printf("<ERR> DataFromMemWriteFile: service ADC-%d d'not captured", lid);
        return -1;
    }

    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    BRD_Handle hADC = DevicesLid[lid].adc.handle();

#ifdef __linux__
    if (p.g_fileMap)
        return MapDataFromMemWriteData(lid, pBuf, bBufSize, bMemBufSize, DmaOn);
#endif

    S32 status = BRDerr_OK;
    if (!p.g_IsWriteFile)
        return 1;
    if (!p.g_Cycle)
        BRDC_printf(_BRDC("<SRV> Reading file...\n"));

    if (p.g_PretrigMode == 3) {
        // получить параметры, актуальные в режиме претриггера
        status = GetPostrigData(lid);
    }

    BRDCHAR fileName[64];
    BRDC_sprintf(fileName, _BRDC("data_%d.bin"), lid);

    IPC_handle hfile = IPC_openFile(fileName, IPC_CREATE_FILE | IPC_FILE_WRONLY);
    if (!hfile) {
        BRDC_printf(_BRDC("Create file %s error.\n"), fileName);
        return 1;
    }
    int bResult = TRUE;

    int nCnt = int(bMemBufSize / bBufSize);
    for (int iCnt = 0; iCnt < nCnt; iCnt++) {
        status = DataFromMem(hADC, *pBuf, (ULONG)bBufSize, DmaOn);
        if (!BRD_errcmp(status, BRDerr_OK))
            return status;
        bResult = IPC_writeFile(hfile, *pBuf, (ULONG)bBufSize);
    }
    ULONG ostSize = ULONG(bMemBufSize % bBufSize);
    if (ostSize) {
        status = DataFromMem(hADC, *pBuf, (ULONG)bBufSize, DmaOn);
        bResult = IPC_writeFile(hfile, *pBuf, ostSize);
    }

    if (p.g_IsWriteFile == 1)
        WriteIsviParams(hfile, hADC, bMemBufSize);

    IPC_closeFile(hfile);
    if (bResult < 0)
        BRDC_printf(_BRDC("<ERR> Write file %s error.\n"), fileName);
    else if (!p.g_Cycle)
        BRDC_printf(_BRDC("<SRV> Write file %s is OK.\n"), fileName);
    return status;
}

void WriteDataFile(int lid, PVOID* pBuf, unsigned long long nNumberOfBytes)
{
    S32 status = BRDerr_OK;
    if (!DevicesLid[lid].adc.isCaptured()) {
        printf("<ERR> WriteDataFile: service ADC-%d d'not captured", lid);
        return;
    }

    BRD_Handle hADC = DevicesLid[lid].adc.handle();
    ParamsAdc& p = DevicesLid[lid].paramsAdc;

#ifdef __linux__
    if (p.g_fileMap) {
        MapWriteData(lid, pBuf, nNumberOfBytes);
        return;
    }
#endif

    if (!p.g_IsWriteFile)
        return;
    if (!p.g_Cycle)
        BRDC_printf(_BRDC("<SRV> Writing file...\n"));

    if (p.g_PretrigMode) { // получить параметры, актуальные в режиме претриггера
        status = GetPretrigData(lid);
    }

    BRDCHAR fileName[64];
    BRDC_sprintf(fileName, _BRDC("data_%d.bin"), lid);
    IPC_handle hfile = 0;
    if (p.g_Cycle > 1 && p.g_times) {
        hfile = IPC_openFile(fileName, IPC_OPEN_FILE | IPC_FILE_WRONLY);
        IPC_setPosFile(hfile, 0, IPC_FILE_END);
    } else {
        hfile = IPC_openFile(fileName, IPC_CREATE_FILE | IPC_FILE_WRONLY);
    }
    p.g_times++;
    if (!hfile) {
        BRDC_printf(_BRDC("Create file %s error.\n"), fileName);
        return;
    }

    int bResult;
    DWORD blockSize = DWORD(nNumberOfBytes / p.g_bBlkNum);
    for (ULONG i = 0; i < p.g_bBlkNum; i++) {
        if (BRDC_stricmp(p.g_SrvName, _BRDC("FFT")) == 0) {
            // конвертируем, если собрали FFT
            auto buf_dbl = new double[blockSize]; // double для FFT
            auto buf_src = reinterpret_cast<int*>(pBuf[i]);
            for (auto j = 0U; j < blockSize / sizeof(int); j++) {
                buf_dbl[j] = static_cast<double>(buf_src[j]);
            }
            bResult = IPC_writeFile(hfile, buf_dbl, blockSize / sizeof(int) * sizeof(double));
            delete[] buf_dbl;

        } else
            bResult = IPC_writeFile(hfile, pBuf[i], blockSize);
    }
    //	BOOL bResult = WriteFile(hfile, pBuf, nNumberOfBytes, &nNumberOfWriteBytes, NULL);

    if (p.g_IsWriteFile == 1)
        if ((p.g_Cycle > 1 && p.g_times == p.g_Cycle) || p.g_Cycle <= 1)
            WriteIsviParams(hfile, hADC, nNumberOfBytes);

    IPC_closeFile(hfile);
    if (bResult < 0)
        BRDC_printf(_BRDC("Write file %s error.\n"), fileName);
    else if (!p.g_Cycle)
        BRDC_printf(_BRDC("Write file %s is OK.\n"), fileName);
}

void WriteIsviParamDirFile(int lid)
{
    printf("<DBG> WriteIsviParamDirFile: ADC-%d \n", lid);

    BRDCHAR fileName[MAX_PATH];
    ParamsAdc& p = DevicesLid[lid].paramsAdc;

    BRDC_strcpy(fileName, p.g_dirFileName);

    printf("<DBG> WriteIsviParamDirFile: fileName>%s< \n", fileName);

    IPC_handle hfile = IPC_openFile(fileName, IPC_OPEN_FILE | IPC_FILE_RDWR);
    if (!hfile) {
        BRDC_printf(_BRDC("Create file %s error.\n"), fileName);
        return;
    }
    int BytesNum = IPC_setPosFile(hfile, 0, IPC_FILE_END);

    if (BytesNum == -1)
        BRDC_printf(_BRDC("File %s is too big for ISVI parameters writing.\n"), fileName);
    else
        WriteIsviParams(hfile, lid, BytesNum);
    IPC_closeFile(hfile);
}

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
void WriteIsviParams(IPC_handle hfile, int lid, unsigned long long nNumberOfBytes)
#else
void WriteIsviParams(HANDLE hfile, int lid, unsigned long long nNumberOfBytes)
#endif
{
    char fileInfo[200 * 1024]; // 200Кб
    char str_buf[10 * 1024]; // 10Кб

    BRD_Handle hADC = DevicesLid[lid].adc.handle();
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    // для служб FFT пишем специальные параметры
    // FFT_SIZE____________ 65536
    // NUMBER_OF_CHANNELS__ 2
    // NUMBERS_OF_CHANNELS_ 0,1
    // SAMPLING_RATE_______ 64000000
    // BRDC_printf(_BRDC("NAME: %s\n"), p.g_SrvName);
    if (BRDC_stricmp(p.g_SrvName, _BRDC("FFT")) == 0) {
        // str_buf[0] = 0;
        // BRD_ctrl(hADC, 0, BRDctrl_ADC_GETCHANORDER, &fft_size);
        auto fft_size = 65536;
        sprintf(str_buf, "\r\nFFT_SIZE___________ %d", fft_size);
        lstrcatA(fileInfo, str_buf);
        sprintf(str_buf, "\r\nNUMBER_OF_CHANNELS__ 2");
        lstrcatA(fileInfo, str_buf);
        sprintf(str_buf, "\r\nNUMBERS_OF_CHANNELS_ 0,1");
        lstrcatA(fileInfo, str_buf);
        sprintf(str_buf, "\r\nSAMPLING_RATE_______ %f\r\n", p.g_samplRate);
        lstrcatA(fileInfo, str_buf);

        int len = lstrlenA(fileInfo);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
        int bResult = IPC_writeFile(hfile, fileInfo, len);
        if (bResult < 0)
#else
        ULONG nNumberOfWriteBytes;
        BOOL bResult = WriteFile(hfile, fileInfo, len, &nNumberOfWriteBytes, NULL);
        if (bResult != TRUE)
#endif
            BRDC_printf(_BRDC("Write ISVI info error\n"));
        return;
    }

    sprintf(fileInfo, "\r\nDEVICE_NAME_________ ");
#ifdef _WIN64
    char srvName[32];
    wcstombs(srvName, p.g_AdcSrvName, 32);
    lstrcatA(fileInfo, srvName);
#else
    lstrcatA(fileInfo, p.g_AdcSrvName);
#endif

    BRD_AdcCfg adc_cfg;
    BRD_ctrl(hADC, 0, BRDctrl_ADC_GETCFG, &adc_cfg);
    // BRD_ctrl(hADC, 0, BRDctrl_ADC_GETRATE, &p.g_samplRate);
    ULONG chanMask;
    BRD_ctrl(hADC, 0, BRDctrl_ADC_GETCHANMASK, &chanMask);
    ULONG format = 0;
    BRD_ctrl(hADC, 0, BRDctrl_ADC_GETFORMAT, &format);

    ULONG is_complex = 0;
    BRD_ctrl(hADC, 0, BRDctrl_ADC_ISCOMPLEX, &is_complex);

    int num_chan = 0;
    int chans[2 * MAX_CHAN];
    double gains[2 * MAX_CHAN];
    double volt_offset[2 * MAX_CHAN];
    ULONG mask = 1;
    BRD_ValChan val_chan;
    for (ULONG iChan = 0; iChan < MAX_CHAN; iChan++) {
        if (is_complex) {
            if (chanMask & mask) {
                chans[num_chan++] = iChan * 2;
                chans[num_chan++] = iChan * 2 + 1;
            }
            val_chan.chan = iChan;
            BRD_ctrl(hADC, 0, BRDctrl_ADC_GETGAIN, &val_chan);
            if (adc_cfg.ChanType == 1)
                val_chan.value = pow(10., val_chan.value / 20); // dB -> разы
            gains[iChan * 2] = val_chan.value;
            volt_offset[iChan * 2] = 0.0;
            gains[iChan * 2 + 1] = val_chan.value;
            volt_offset[iChan * 2 + 1] = 0.0;
        } else {
            if (chanMask & mask)
                chans[num_chan++] = iChan;
            val_chan.chan = iChan;
            BRD_ctrl(hADC, 0, BRDctrl_ADC_GETGAIN, &val_chan);
            if (adc_cfg.ChanType == 1)
                val_chan.value = pow(10., val_chan.value / 20); // dB -> разы
            gains[iChan] = val_chan.value;
            volt_offset[iChan] = 0.0;
        }
        mask <<= 1;
    }

    if (BRDC_strstr(p.g_AdcSrvName, _BRDC("ADC1624X192")) || BRDC_strstr(p.g_AdcSrvName, _BRDC("ADC1624X128"))) {
        int i = 0, j = 0;
        num_chan = 0;
        for (int i = 0; i < 16; i++)
            num_chan += (chanMask >> i) & 0x1;
        for (i = 0; i < 16; i += 2) {
            if (chanMask & (0x1 << i))
                chans[j++] = i;
        }
        for (i = 1; i < 16; i += 2) {
            if (chanMask & (0x1 << i))
                chans[j++] = i;
        }
    }

    if (!num_chan) {
        BRDC_printf(_BRDC("Number of channels for Isvi Parameters is 0 - Error!!!\n"));
        return;
    }
    BRD_ItemArray chan_order;
    chan_order.item = num_chan;
    chan_order.itemReal = 0;
    chan_order.pItem = &chans;
    BRD_ctrl(hADC, 0, BRDctrl_ADC_GETCHANORDER, &chan_order);

    sprintf(str_buf, "\r\nNUMBER_OF_CHANNELS__ %d", num_chan);
    lstrcatA(fileInfo, str_buf);

    sprintf(str_buf, "\r\nNUMBERS_OF_CHANNELS_ ");
    lstrcatA(fileInfo, str_buf);
    str_buf[0] = 0;
    for (int iChan = 0; iChan < num_chan; iChan++) {
        char buf[16];
        sprintf(buf, "%d,", chans[iChan]);
        lstrcatA(str_buf, buf);
    }
    lstrcatA(fileInfo, str_buf);

    ULONG sample_size = format ? format : sizeof(short);
    if (format == 0x80) // упакованные 12-разрядные данные
        sample_size = 2;
    unsigned long long samples = nNumberOfBytes / sample_size / num_chan;
    if (p.g_Cycle > 1)
        samples *= p.g_Cycle;
    sprintf(str_buf, "\r\nNUMBER_OF_SAMPLES___ %lld", samples);
    lstrcatA(fileInfo, str_buf);

    sprintf(str_buf, "\r\nSAMPLING_RATE_______ %f", p.g_samplRate);
    lstrcatA(fileInfo, str_buf);

    sprintf(str_buf, "\r\nBYTES_PER_SAMPLES___ %lu", sample_size);
    lstrcatA(fileInfo, str_buf);

    if (format == 0x80) // два 12-и битных отсчета упаковано в 3 байта
        lstrcatA(fileInfo, "\r\nSAMPLES_PER_BYTES___ 3");
    else
        lstrcatA(fileInfo, "\r\nSAMPLES_PER_BYTES___ 1");

    if (is_complex)
        sprintf(str_buf, "\r\nIS_COMPLEX_SIGNAL?__ YES");
    else
        sprintf(str_buf, "\r\nIS_COMPLEX_SIGNAL?__ NO");
    lstrcatA(fileInfo, str_buf);

    double fc[MAX_CHAN];
    sprintf(str_buf, "\r\nSHIFT_FREQUENCY_____ ");
    lstrcatA(fileInfo, str_buf);
    str_buf[0] = 0;
    for (int iChan = 0; iChan < num_chan; iChan++) {
        char buf[16];
        fc[iChan] = 0.0;
        if (is_complex) {
            BRD_ValChan val_chan;
            val_chan.chan = iChan / 2;
            BRD_ctrl(hADC, 0, BRDctrl_ADC_GETFC, &val_chan);
            fc[iChan] = val_chan.value;
        }
        sprintf(buf, "%.2f,", fc[iChan]);
        lstrcatA(str_buf, buf);
    }
    lstrcatA(fileInfo, str_buf);

    sprintf(str_buf, "\r\nGAINS_______________ ");
    lstrcatA(fileInfo, str_buf);
    str_buf[0] = 0;
    for (int iChan = 0; iChan < num_chan; iChan++) {
        char buf[16];
        sprintf(buf, "%f,", gains[chans[iChan]]);
        lstrcatA(str_buf, buf);
    }
    lstrcatA(fileInfo, str_buf);

    sprintf(str_buf, "\r\nVOLTAGE_OFFSETS_____ ");
    lstrcatA(fileInfo, str_buf);
    str_buf[0] = 0;
    for (int iChan = 0; iChan < num_chan; iChan++) {
        char buf[16];
        sprintf(buf, "%f,", volt_offset[iChan]);
        lstrcatA(str_buf, buf);
    }
    lstrcatA(fileInfo, str_buf);

    sprintf(str_buf, "\r\nVOLTAGE_RANGE_______ %f", adc_cfg.InpRange / 1000.);
    lstrcatA(fileInfo, str_buf);

    // int BitsPerSample = (format == 1) ? 8 : adc_cfg.Bits;
    // if(BRDC_strstr(p.g_AdcSrvName, _BRDC("ADC1624X192")) || BRDC_strstr(p.g_AdcSrvName, _BRDC("ADC818X800")))
    //	if(format == 0 || format == 2)
    //		BitsPerSample = 16;
    int BitsPerSample = adc_cfg.Bits;
    if (format == 1)
        if (BitsPerSample > 8)
            BitsPerSample = 8;
    if ((format == 0) || (format == 2))
        if (BitsPerSample > 16)
            BitsPerSample = 16;
    sprintf(str_buf, "\r\nBIT_RANGE___________ %d", BitsPerSample);
    lstrcatA(fileInfo, str_buf);
    lstrcatA(fileInfo, "\r\n");

    if (p.g_PretrigMode) {
        ULONG event_mark = ULONG(p.g_bStartEvent / sample_size / num_chan);
        sprintf(str_buf, "\r\nPRETRIGGER_SAMPLE___ %lu", event_mark);
        lstrcatA(fileInfo, str_buf);
    }

    int len = lstrlenA(fileInfo);

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
    int bResult = IPC_writeFile(hfile, fileInfo, len);
    if (bResult < 0)
#else
    ULONG nNumberOfWriteBytes;
    BOOL bResult = WriteFile(hfile, fileInfo, len, &nNumberOfWriteBytes, NULL);
    if (bResult != TRUE)
#endif
        BRDC_printf(_BRDC("Write ISVI info error\n"));
}

//********************************************************
void WriteFlagSinc(int flg, int isNewParam, int lid)
{
    ParamsAdc& p = DevicesLid[lid].paramsAdc;

    if (p.g_fileMap)
        MapWrFlagSinc(lid, flg, isNewParam);
    else {
        BRDCHAR fileName[64];
        BRDC_sprintf(fileName, _BRDC("data_%d.flg"), lid);
        int val[2];
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
        IPC_handle handle = NULL;
        while (!handle)
            handle = IPC_openFile(fileName, IPC_CREATE_FILE | IPC_FILE_WRONLY);
        val[0] = flg;
        val[1] = isNewParam;
        IPC_writeFile(handle, val, 8);
        IPC_closeFile(handle);
#else
        int fs = -1;
        while (fs == -1)
            fs = BRDC_sopen(fileName, _O_WRONLY | _O_BINARY | _O_CREAT | _O_TRUNC, _SH_DENYNO, _S_IWRITE);
        //    errno_t err = _sopen_s( &fs, fileName, _O_WRONLY|_O_BINARY|_O_CREAT, _SH_DENYNO, S_IWRITE );
        val[0] = flg;
        val[1] = isNewParam;
        _write(fs, val, 8);
        _close(fs);
#endif
    }
}

//********************************************************
int ReadFlagSinc(int lid)
{
    int flg;
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    if (p.g_fileMap)
        flg = MapRdFlagSinc(lid);
    else {
        BRDCHAR fileName[64];
        BRDC_sprintf(fileName, _BRDC("data_%d.flg"), lid);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
        IPC_handle handle = NULL;
        while (!handle)
            handle = IPC_openFile(fileName, IPC_OPEN_FILE | IPC_FILE_RDONLY);
        IPC_readFile(handle, &flg, 4);
        IPC_closeFile(handle);
#else
        int fs = -1;
        while (fs == -1)
            fs = BRDC_sopen(fileName, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD);
        //    errno_t err = _sopen_s( &fs, fileName, _O_RDONLY|_O_BINARY|_O_CREAT, _SH_DENYNO, S_IREAD );
        _read(fs, &flg, 4);
        _close(fs);
#endif
    }
    return flg;
}

#include "ctrlreg.h"
S32 RegRwSpd(BRD_Handle hDev, BRDCHAR* fname);

S32 RegProg(BRD_Handle hAdc, int idx, int isx)
{
    S32 status = BRDerr_OK;
    ParamsAdc& p = DevicesLid[idx].paramsAdc;
    BRDCHAR iniFilePath[MAX_PATH];
    BRDCHAR iniSectionName[MAX_PATH];
    IPC_getCurrentDir(iniFilePath, sizeof(iniFilePath) / sizeof(BRDCHAR));
    BRDC_strcat(iniFilePath, p.g_iniFileNameAdc);
    BRDC_sprintf(iniSectionName, _BRDC("device%d_%s%d"), idx, p.g_SrvName, (p.g_subNo < 0) ? isx : 0);

    BRDCHAR regfname[128];
    // GetInifileString(iniFilePath, iniSectionName, _BRDC("RegFileName"), _BRDC("spd_dev.ini"), regfname, sizeof(regfname));
    GetInifileString(iniSectionName, _BRDC("RegFileName"), _BRDC(""), regfname, sizeof(regfname), iniFilePath);

    // Если указан файл, то подгрузить регистры из файла
    if (regfname[0])
        status = RegRwSpd(hAdc & 0xFFFF, regfname); // hAdc & 0xFFFF - из дескриптора службы делаем дескриптор устройства

    return status;
}

//
// End of file
//
void printLids(void)
{
    // получить список LID (каждая запись соответствует устройству)
    BRD_LidList lidList;
    lidList.item = MAX_DEV; // считаем, что устройств может быть не больше MAX_DEV
    lidList.pLID = new U32[MAX_DEV];
    auto status = BRD_lidList(lidList.pLID, lidList.item, &lidList.itemReal);

    // BRD_Handle handle[MAX_DEV];
    int iDev = 0;
    // ULONG iDev = 0;

    // отображаем информацию об всех устройствах, указанных в ini-файле
    for (iDev = 0; iDev < lidList.itemReal; iDev++) {
        BRD_Info& info = DevicesLid[iDev].paramsAdc.g_info;
        info.size = sizeof(info);
        BRD_getInfo(lidList.pLID[iDev], &info); // получить информацию об устройстве
        if (info.busType == BRDbus_ETHERNET)
            BRDC_printf(_BRDC("%s, DevID = 0x%x, RevID = 0x%x, IP %u.%u.%u.%u, Port %u, PID = %d.\n"),
                info.name, info.boardType >> 16, info.boardType & 0xff,
                (UCHAR)info.bus, (UCHAR)(info.bus >> 8), (UCHAR)(info.bus >> 16), (UCHAR)(info.bus >> 24),
                info.dev, info.pid);
        else {
            ULONG dev_id = info.boardType >> 16;
            if (dev_id == 0x53B1 || dev_id == 0x53B3) // FMC115cP or FMC117cP
                BRDC_printf(_BRDC("%s, DevID = 0x%x, RevID = 0x%x, Bus = %d, Dev = %d, G.adr = %d, Order = %d, PID = %d.\n"),
                    info.name, info.boardType >> 16, info.boardType & 0xff, info.bus, info.dev,
                    info.slot & 0xffff, info.pid >> 28, info.pid & 0xfffffff);
            else
                BRDC_printf(_BRDC("%s, DevID = 0x%x, RevID = 0x%x, Bus = %d, Dev = %d, Slot = %d, PID = %d.\n"),
                    info.name, info.boardType >> 16, info.boardType & 0xff, info.bus, info.dev, info.slot, info.pid);
        }
    }
}

void puListLoad(int lid)
{
    if (!DevicesLid[lid].device.isOpen()) {
        BRDC_printf(_BRDC("<ERR> puListLoad: lid #%d not opened!\n"), lid);
        return;
    }
    BRD_PuList PuList[MAX_PU];
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    U32 ItemReal;
    BRD_Handle hdev = DevicesLid[lid].device.handle();
    auto status = BRD_puList(hdev, PuList, MAX_PU, &ItemReal); // получить список программируемых устройств
    printf("<SRV> puListLoad: %d programmable units found/n", ItemReal);
    if (ItemReal <= MAX_PU) {
        for (U32 j = 0; j < ItemReal; j++) {
            if (PuList[j].puCode == PLD_CFG_TAG) { // Тип программируемого устройства - ПЛИС
                U32 PldState;
                status = BRD_puState(hdev, PuList[j].puId, &PldState); // получить состояние ПЛИС
                BRDC_printf(_BRDC("PU%d: %s, Id = %d, Code = %x, Attr = %x, PldState = %d\n"),
                    j, PuList[j].puDescription, PuList[j].puId, PuList[j].puCode, PuList[j].puAttr, PldState);
                if (PuList[j].puId == 0x100) {
                    if (p.g_isPldLoadAlways || !PldState) { // загрузить ПЛИС ADM
                        BRDC_printf(_BRDC("BRD_puLoad: loading...\r"));
                        status = BRD_puLoad(hdev, PuList[j].puId, p.g_pldFileName, &PldState);
                        if (BRD_errcmp(status, BRDerr_OK))
                            BRDC_printf(_BRDC("BRD_puLoad: load is OK\n"));
                        else if (BRD_errcmp(status, BRDerr_PLD_TEST_DATA_ERROR))
                            BRDC_printf(_BRDC("BRD_puLoad: data error starting test\n"));
                        else if (BRD_errcmp(status, BRDerr_PLD_TEST_ADDRESS_ERROR))
                            BRDC_printf(_BRDC("BRD_puLoad: address error starting test\n"));
                        else
                            BRDC_printf(_BRDC("BRD_puLoad: error loading\n"));
                    }
                    if (PldState) { // если ПЛИС ADM загружена
                        BRDextn_PLDINFO pld_info;
                        pld_info.pldId = 0x100;
                        status = BRD_extension(hdev, 0, BRDextn_GET_PLDINFO, &pld_info);
                        if (BRD_errcmp(status, BRDerr_OK))
                            BRDC_printf(_BRDC("ADM PLD: Version %d.%d, Modification %d, Build 0x%X\n"), pld_info.version >> 8, pld_info.version & 0xff, pld_info.modification, pld_info.build);
                    }
                } else { // загрузить ПЛИС ЦОС
                         // status = BRD_puLoad(hdev, PuList[j].puId, _BRDC("fmc106p_exp_filter.bit"), &PldState);
                         // status = BRD_puLoad(hdev, PuList[j].puId, _BRDC("fmc106p_dsp_base_lx240t.bit"), &PldState);
                         // status = BRD_puLoad(hdev, PuList[j].puId, _BRDC("fmc106p_dsp_base_sx315t_full.bit"), &PldState);
                         // status = BRD_puLoad(hdev, PuList[j].puId, _BRDC("ambpcd_v10_dsp_test1.mcs"), &PldState);
                         // if(BRD_errcmp(status, BRDerr_OK))
                         //	BRDC_printf(_BRDC("BRD_puLoad: load is OK\n"));
                         // else
                         //	BRDC_printf(_BRDC("BRD_puLoad: error loading\n"));
                }
            }
        }
    }
}

bool checkPower(int lid)
{
    bool retVal = false;

    BRD_Handle hdev = DevicesLid[lid].device.handle();
    if (hdev > 0) {
        // получаем состояние FMC-питания (если не FMC-модуль, то ошибка)
        BRDextn_FMCPOWER power;
        power.slot = 0;
        S32 status = BRD_extension(hdev, 0, BRDextn_GET_FMCPOWER, &power);
        if (BRD_errcmp(status, BRDerr_OK)) {
            if (power.onOff) {
                BRDC_printf(_BRDC("<SRV> FMC Power: ON %.2f Volt\n"), power.value / 100.);
            } else {
                BRDC_printf(_BRDC("<SRV> FMC Power is turned off, enabling now...\n"));
                power.onOff = 1;
                status = BRD_extension(hdev, 0, BRDextn_SET_FMCPOWER, &power);
                status = BRD_extension(hdev, 0, BRDextn_GET_FMCPOWER, &power);
                if (power.onOff)
                    BRDC_printf(_BRDC("<SRV> FMC Power: ON %.2f Volt\n"), power.value / 100.);
                else
                    BRDC_printf(_BRDC("<SRV> FMC Power: OFF %.2f Volt\n"), power.value / 100.);
            }
            return true;
        } else {
            BRDC_printf(_BRDC("<ERR> ADC-%d - error of check power parameters\n"), lid);
            return false;
        }
    }
    BRDC_printf(_BRDC("<ERR> ADC-%d - bad handle device\n"), lid);
    return false;
}

bool captureServiceAndSetParams(int lid, int mode)
{
    if (!DevicesLid[lid].device.isOpen()) {
        printf("<ERR> device lid=%d d'nt opened!\n", lid);
        return false;
    }
    BRD_Handle handleDevice = DevicesLid[lid].device.handle();
    DevicesLid[lid].adc.setMode(mode);
    ParamsAdc& p = DevicesLid[lid].paramsAdc;

    U32 ItemReal = 0;
    if (handleDevice > 0) {
        BRD_ServList srvList[MAX_SRV];
        S32 status = BRD_serviceList(handleDevice, 0, srvList, MAX_SRV, &ItemReal);
        if (ItemReal <= MAX_SRV) {
            for (U32 j = 0; j < ItemReal; j++) {
                BRDC_printf(_BRDC("Service %d: %s, Attr = 0x%X\n"), j, srvList[j].name, srvList[j].attr);
                if (strcasecmp(srvList[j].name, p.g_AdcSrvName) == 0)
                    SetParamSrv(lid);
            }
        } else {
            BRDC_printf(_BRDC("<ERR> captureServiceAndSetParams: RealItems = %d (> %d)!\n"), ItemReal, MAX_SRV);
            return false;
        }
        if (!DevicesLid[lid].adc.isCaptured()) {
            BRDC_printf(_BRDC("<ERR> service %s is not found!\n"), p.g_AdcSrvName);
            mode = BRDcapt_SPY; // это для того, чтобы обойти код сбора данных
            DevicesLid[lid].adc.setMode(mode);
            return false;
        }
    }

    return true;
}

void workFlow(int lid)
{

    U32 mode = DevicesLid[lid].adc.mode();
    printf("<debug> Start work ADC-%d .. with mode = %d \n", lid, mode);
    if (mode == BRDcapt_SPY)
        return;
    ParamsAdc& p = DevicesLid[lid].paramsAdc;

    if (p.g_DirWriteFile) {
        if (p.g_DirWriteFile == -1)
            ContinueDaq(lid, p.g_FileBufSize, p.g_FileBlkNum);
        else {
            DirectFile(lid, p.g_IsSysMem, p.g_FileBufSize, p.g_DirWriteFile, p.g_FileBlkNum);
            if (p.g_IsWriteFile == 1)
                WriteIsviParamDirFile(lid);
        }
    } else {
        GetAdcData(lid, p.g_bBufSize, p.g_bMemBufSize);
    }
}

void ListParametersAdc(int lid)
{
    ParamsAdc& p = DevicesLid[lid].paramsAdc;

    printf("/n List ADC-%d parameters : \n", lid);
    printf("g_AdcDrqFlag = %d\n", p.g_AdcDrqFlag);
    printf("g_AdcSrvName =>%s<\n", p.g_AdcSrvName);
    printf("g_AdcSrvNum = %d\n", p.g_AdcSrvNum);
    printf("g_adjust_mode = %d\n", p.g_adjust_mode);
    printf("g_bBufSize = %d\n", p.g_bBufSize);
    printf("g_bMemBufSize = %lu\n", p.g_bMemBufSize);
    printf("g_bPostTrigSize = %lld\n", p.g_bPostTrigSize);
    printf("g_Cycle = %d\n", p.g_Cycle);
    printf("g_dirFileName = %s\n", p.g_dirFileName);
    printf("g_DirWriteFile = %d\n", p.g_DirWriteFile);
    printf("g_DmaOn = %d\n", p.g_DmaOn);
    printf("g_FileBlkNum = %d\n", p.g_FileBlkNum);
    printf("g_FileBufSize = %d\n", p.g_FileBufSize);
    printf("g_fileMap = %d\n", p.g_fileMap);
    printf("g_iniFileNameAdc = >%s<\n", p.g_iniFileNameAdc);
    printf("g_info.busType = %d (BRDbus_ETHERNET = %d)\n", p.g_info.busType, BRDbus_ETHERNET);
    printf("g_IoDelay = %d\n", p.g_IoDelay);
    printf("g_isPldLoadAlways = %d\n", p.g_isPldLoadAlways);
    printf("g_IsSysMem = %d\n", p.g_IsSysMem);
    printf("g_IsWriteFile = %d\n", p.g_IsWriteFile);
    printf("g_memorySamplesOfChannel = %d\n", p.g_memorySamplesOfChannel);
    printf("g_MemDrqFlag = %lu\n", p.g_MemDrqFlag);
    printf("g_MemOn = %d\n", p.g_MemOn);
    printf("g_MsTimeout = %lu\n", p.g_MsTimeout);
    printf("g_numChan = %d\n", p.g_numChan);
    printf("g_OnlySetParams = %d\n", p.g_OnlySetParams);
    printf("g_Pause = %d\n", p.g_Pause);
    printf("g_pldFileName = %s\n", p.g_pldFileName);
    printf("g_PretrigMode = %d\n", p.g_PretrigMode);
    printf("g_quick_quit = %d\n", p.g_quick_quit);
    printf("g_regdbg = %d\n", p.g_regdbg);
    printf("g_samplesOfChannel = %d\n", p.g_samplesOfChannel);
    printf("g_SrvName = >%s<\n", p.g_SrvName);
    printf("g_SwitchOutMask = %lu\n", p.g_SwitchOutMask);
    printf("g_transRate = %d\n", p.g_transRate);
    printf("----------------------------------------/n");
}