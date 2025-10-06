
#include "adc_ctrl.h"
#include "mu_ctrl.h"
#include <chrono>

#include "../total.h"
#include <math.h>
#include <stdexcept>

#ifdef __linux__
#include <sys/time.h>
#endif

/*
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
#include "gipcy.h"
#include <string.h>
IPC_handle* g_hBufFileMap = NULL;
IPC_handle g_hFlgFileMap = NULL;
IPC_handle g_hPostfixFileMap = NULL;
#else
#include <conio.h>
#include <process.h>
HANDLE* g_hBufFileMap = NULL;
HANDLE g_hFlgFileMap = NULL;
HANDLE g_hPostfixFileMap = NULL;
#endif
*/

void MappingIsviParams(int lid, unsigned long long nNumberOfBytes);

// void DisplayError(S32 status, BRDCHAR* func_name, BRDCHAR* cmd_str)
void DisplayError(S32 status, const char* funcName, const BRDCHAR* cmd_str)
{
    U32 real_status = BRD_errext(status);
    BRDCHAR func_name[MAX_PATH];
#ifdef _WIN64
    mbstowcs(func_name, funcName, MAX_PATH);
#else
    BRDC_strcpy(func_name, funcName);
#endif
    BRDCHAR msg[255];
    switch (real_status) {
    case BRDerr_OK:
        BRDC_sprintf(msg, _BRDC("%s - %s: BRDerr_OK\n"), func_name, cmd_str);
        break;
    case BRDerr_BAD_MODE:
        BRDC_sprintf(msg, _BRDC("%s - %s: BRDerr_BAD_MODE\n"), func_name, cmd_str);
        break;
    case BRDerr_INSUFFICIENT_SERVICES:
        BRDC_sprintf(msg, _BRDC("%s - %s: BRDerr_INSUFFICIENT_SERVICES\n"), func_name, cmd_str);
        break;
    case BRDerr_BAD_PARAMETER:
        BRDC_sprintf(msg, _BRDC("%s - %s: BRDerr_BAD_PARAMETER\n"), func_name, cmd_str);
        break;
    case BRDerr_BUFFER_TOO_SMALL:
        BRDC_sprintf(msg, _BRDC("%s - %s: BRDerr_BUFFER_TOO_SMALL\n"), func_name, cmd_str);
        break;
    case BRDerr_WAIT_TIMEOUT:
        BRDC_sprintf(msg, _BRDC("%s - %s: BRDerr_WAIT_TIMEOUT\n"), func_name, cmd_str);
        break;
    default:
        BRDC_sprintf(msg, _BRDC("%s - %s: Unknown error, status = %8X\n"), func_name, cmd_str, real_status);
        break;
    }
    BRDC_printf(_BRDC("\n%s"), msg);
}

#ifdef _WIN32
int SetMuRun(BRD_Handle hSrv)
{
    typedef S32 STDCALL DEV_API_entry(BRD_Handle hSrv);
    HINSTANCE hLib; // DLL header
    DEV_API_entry* pEntry;
    int err;

    hLib = LoadLibrary(_BRDC("setmu.dll")); // Load helper instatce of DLL
    if (hLib <= (HINSTANCE)HINSTANCE_ERROR) {
        return -1;
    }

    pEntry = (DEV_API_entry*)GetProcAddress(hLib, "SetMuEntry");
    if (pEntry == 0) {
        FreeLibrary(hLib);
        return -1;
    }
    err = (pEntry)(hSrv);
    FreeLibrary(hLib);

    return 0;
}
#endif

// проверка, есть ли тактовая частота, выполняется для служб:
// ADC28X800M, ADC28X1G, ADC10X2G, ADC210X1G, ADC212X1G, FM814X125M
int CheckClock(BRD_Handle hADC, BRDCHAR* AdcSrvName)
{
    if (BRDC_stricmp(AdcSrvName, _BRDC("ADC28X800M")) && BRDC_stricmp(AdcSrvName, _BRDC("ADC28X1G")) && BRDC_stricmp(AdcSrvName, _BRDC("ADC10X2G")) && BRDC_stricmp(AdcSrvName, _BRDC("ADC210X1G")) && BRDC_stricmp(AdcSrvName, _BRDC("ADC212X1G")) && BRDC_stricmp(AdcSrvName, _BRDC("FM814X125M")))
        return 1; // если ни одна из этих служб
    BRDC_printf(_BRDC("Clock checking...\r"));
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
    IPC_delay(1000);
#else
    Sleep(1000);
#endif
    ULONG status = 1;
    if (hADC) {
        for (int i = 0; i < 3; i++) {
            BRD_ClkMode clk_mode;
            status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETCLKMODE, &clk_mode);
            status = BRD_ctrl(hADC, 0, BRDctrl_ADC_SETCLKMODE, &clk_mode);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
            IPC_delay(10);
#else
            Sleep(10);
#endif
            ULONG fifo_status;
            BRD_ctrl(hADC, 0, BRDctrl_ADC_FIFOSTATUS, &fifo_status);
            if (fifo_status & 0x1000) {
                status = 1;
                break;
            } else
                status = 0;
        }
    }
    BRDC_printf(_BRDC("                    \r"));
    if (status)
        BRDC_printf(_BRDC("Clock is OK\n"));
    else
        BRDC_printf(_BRDC("Clock error\n"));
    return status;
}

S32 AdcSettings(BRD_Handle hADC, int lid, int isx, BRDCHAR* srvName, BRDCHAR* iniFileName)
{
    S32 status;
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    BRD_AdcCfg adc_cfg;

    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETCFG, &adc_cfg);
    BRDC_printf(_BRDC("<SRV> AdcSettings: ADC Config: FIFO size = %d kBytes\n"), adc_cfg.FifoSize / 1024);

    // задать параметры из файла
    BRDCHAR iniFilePath[MAX_PATH];
    BRDCHAR iniSectionName[MAX_PATH];
    BRDC_sprintf(iniSectionName, _BRDC("device%d_%s%d"), 0, srvName, (p.g_subNo < 0) ? isx : 0);
    S32 err = IPC_getFullPath(iniFileName, iniFilePath);
    if (0 > err) {
        BRDC_printf(_BRDC("ERROR: Can't find ini-file '%s'\n\n"), iniFileName);
        return -1;
    }

    printf("<SRV> AdcSettings: iniFileName = '%s'\n    iniFilePath = '%s'\n    iniSectionName = '%s'\n",
        iniFileName, iniFilePath, iniSectionName);

    BRD_IniFile ini_file;
    BRDC_strcpy(ini_file.fileName, iniFilePath);
    BRDC_strcpy(ini_file.sectionName, iniSectionName);
    // auto adc_start_time = std::chrono::high_resolution_clock::now();

    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_READINIFILE, &ini_file);
    if (status != BRDerr_OK) {
        printf("<ERR> AdcSettings (BRDctrl_ADC_READINIFILE): status = 0x%X\n", status);
        throw std::invalid_argument("<ERR> AdcSettings (BRDctrl_ADC_READINIFILE): Side-Driver parameters bad!");
    }

#ifdef _WIN32
    // альтернативная установка параметров не из файла, а из структуры SetMU, которая индивидуальна для каждого субмодуля
    SetMuRun(hADC);
#endif

    // получить источник и значение тактовой частоты можно отдельной функцией
    BRD_SyncMode sync_mode;
    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETSYNCMODE, &sync_mode);
    if (BRDC_strstr(srvName, _BRDC("ADC1624X192")) || BRDC_strstr(srvName, _BRDC("ADC1624X128")) || BRDC_strstr(srvName, _BRDC("ADC818X800"))) {
        if (BRD_errcmp(status, BRDerr_OK))
            BRDC_printf(_BRDC("BRDctrl_ADC_GETSYNCMODE: source = %d, value = %.2f MHz, rate = %.3f kHz\n"),
                sync_mode.clkSrc, sync_mode.clkValue / 1000000, sync_mode.rate / 1000);
        else
            DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_ADC_GETSYNCMODE"));
    } else {
        if (BRD_errcmp(status, BRDerr_OK))
            BRDC_printf(_BRDC("BRDctrl_ADC_GETSYNCMODE: source = %d, value = %.4f MHz, rate = %.4f MHz\n"),
                sync_mode.clkSrc, sync_mode.clkValue / 1000000, sync_mode.rate / 1000000);
        else
            DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_ADC_GETSYNCMODE"));
    }
    p.g_samplRate = sync_mode.rate;
    // получить параметры стартовой синхронизации
    // команда BRDctrl_ADC_GETSTARTMODE может получать 2 разные структуры
    // для определения какую из них использует данная служба применяем трюк с массивом )))
    U08 start_struct[40]; // наибольшая из структур имеет размер 40 байт
    memset(start_struct, 0x5A, 40);
    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETSTARTMODE, &start_struct);
    if (BRD_errcmp(status, BRDerr_OK)) {
        if (start_struct[39] == 0x5A) { // стартовая схема на базовом модуле (используется структура по-меньше)
            // службы: ADC414X65M, ADC216X100M, ADC1624X192, ADC818X800, ADC1612X1M
            BRD_StartMode* start = (BRD_StartMode*)start_struct;
            BRDC_printf(_BRDC("<SRV> AdcSettings:(BRDctrl_ADC_GETSTARTMODE): start source = %d (small struct)\n"), start->startSrc);
            //			if(start->startSrc == BRDsts_CMP0 || start->startSrc == BRDsts_CMP1)
            //			{	// старт от компаратора 0 (сигнал канала 0) или от компаратора 1 (сигнал с разъема SDX)
            //				BRDCHAR Buffer[128];
            //				BRD_CmpSC cmp_sc;
            //				cmp_sc.src = BRDcmps_CHAN0CLK;
            // #if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
            //				IPC_getPrivateProfileString(iniSectionName, _BRDC("ComparatorThresholdCHAN0"), _BRDC("0.0"), Buffer, sizeof(Buffer), iniFilePath);
            // #else
            //				GetPrivateProfileString(iniSectionName, _BRDC("ComparatorThresholdCHAN0"), _BRDC("0.0"), Buffer, sizeof(Buffer), iniFilePath);
            // #endif
            //				cmp_sc.thr[0] = BRDC_atof(Buffer);//0.0;
            // #if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
            //				IPC_getPrivateProfileString(iniSectionName, _BRDC("ComparatorThresholdSDX"), _BRDC("0.0"), Buffer, sizeof(Buffer), iniFilePath);
            // #else
            //				GetPrivateProfileString(iniSectionName, _BRDC("ComparatorThresholdSDX"), _BRDC("0.0"), Buffer, sizeof(Buffer), iniFilePath);
            // #endif
            //				cmp_sc.thr[1] = BRDC_atof(Buffer);//0.0;
            //				// задать источникик и пороги для компараторов
            //				status = BRD_ctrl(hADC, 0, BRDctrl_CMPSC_SET, &cmp_sc);
            //				if(BRD_errcmp(status, BRDerr_OK))
            //					BRDC_printf(_BRDC("BRDctrl_CMPSC_SET: comparator source = %d, thresholdCHAN0 = %.2f, thresholdSDX = %.2f\n"),
            //									cmp_sc.src, cmp_sc.thr[0], cmp_sc.thr[1]);
            //				else
            //					DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_CMPSC_SET"));
            //			}
        } else { // стартовая схема на субмодуле (используется большая структура)
            BRD_AdcStartMode* start = (BRD_AdcStartMode*)start_struct;
            // службы: ADC212X200M, ADC10X2G, ADC214X200M, ADC28X1G, ADC214X400M, ADC210X1G,
            // FM814X125M, FM214X250M, FM412X500M, FM212X1G
            BRDC_printf(_BRDC("<SRV> AdcSettings:(BRDctrl_ADC_GETSTARTMODE): start source = %d (big struct)\n"), start->src);
        }
    } else
        DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_ADC_GETSTARTMODE"));

    BRD_StdDelay adc_delay;
    adc_delay.nodeID = 8; // external start delay
    adc_delay.value = 0x3f; // non-valid value
    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_RDDELAY, &adc_delay);
    if (BRD_errcmp(status, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRDctrl_ADC_RDDELAY: Delay External Start = %d\n"), adc_delay.value);
    } else if (BRD_errcmp(status, BRDerr_BAD_NODEID)) {
        BRDC_printf(_BRDC("BRDctrl_ADC_RDDELAY: not used node ID = 8\n"));
    }

    //	if(start->startSrc == BRDsts_CMP0 || start->startSrc == BRDsts_CMP1)
    { // старт от компаратора 0 (сигнал канала 0) или от компаратора 1 (сигнал с разъема SDX)
        BRDCHAR buf0[128];
        BRDCHAR buf1[128];
        BRD_CmpSC cmp_sc;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
        IPC_getPrivateProfileString(iniSectionName, _BRDC("ComparatorSource"), _BRDC("1"), buf0, sizeof(buf0), iniFilePath);
#else
        GetPrivateProfileString(iniSectionName, _BRDC("ComparatorSource"), _BRDC("1"), buf0, sizeof(buf0), iniFilePath);
#endif
        //		cmp_sc.src = BRDcmps_CHAN0CLK;
        cmp_sc.src = BRDC_atoi(buf0);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
        if (cmp_sc.src == BRDcmps_EXTSTCLK) // BRDcmps_EXTSTCLK = 0
            IPC_getPrivateProfileString(iniSectionName, _BRDC("ComparatorThresholdSUBUNIT"), _BRDC(""), buf0, sizeof(buf0), iniFilePath);
        else
            IPC_getPrivateProfileString(iniSectionName, _BRDC("ComparatorThresholdCHAN0"), _BRDC(""), buf0, sizeof(buf0), iniFilePath);
        IPC_getPrivateProfileString(iniSectionName, _BRDC("ComparatorThresholdSDX"), _BRDC(""), buf1, sizeof(buf1), iniFilePath);
#else
        if (cmp_sc.src == BRDcmps_EXTSTCLK) // BRDcmps_EXTSTCLK = 0
            GetPrivateProfileString(iniSectionName, _BRDC("ComparatorThresholdSUBM"), _BRDC(""), buf0, sizeof(buf0), iniFilePath);
        else
            GetPrivateProfileString(iniSectionName, _BRDC("ComparatorThresholdCHAN0"), _BRDC(""), buf0, sizeof(buf0), iniFilePath);
        GetPrivateProfileString(iniSectionName, _BRDC("ComparatorThresholdSDX"), _BRDC(""), buf1, sizeof(buf1), iniFilePath);
#endif
        if (BRDC_strlen(buf0) || BRDC_strlen(buf1)) {
            if (BRDC_strlen(buf0))
                cmp_sc.thr[0] = BRDC_atof(buf0); // 0.0;
            else
                cmp_sc.thr[0] = 0.0;
            if (BRDC_strlen(buf1))
                cmp_sc.thr[1] = BRDC_atof(buf1); // 0.0;
            else
                cmp_sc.thr[1] = 0.0;
            // задать источникик и пороги для компараторов
            status = BRD_ctrl(hADC, 0, BRDctrl_CMPSC_SET, &cmp_sc);
            if (BRD_errcmp(status, BRDerr_OK))
                BRDC_printf(_BRDC("BRDctrl_CMPSC_SET: comparator source = %d, thresholdCHAN0 = %.2f, thresholdSDX = %.2f\n"),
                    cmp_sc.src, cmp_sc.thr[0], cmp_sc.thr[1]);
            else
                DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_CMPSC_SET"));
        }
    }

    // получить маску включенных каналов
    ULONG chan_mask = 0;
    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETCHANMASK, &chan_mask);
    printf("<DBG> AdcSettings: BRDctrl_ADC_GETCHANMASK = 0x%X\n", chan_mask);
    ULONG is_complex = 0;
    BRD_ctrl(hADC, 0, BRDctrl_ADC_ISCOMPLEX, &is_complex);
    printf("<DBG> AdcSettings: BRDctrl_ADC_ISCOMPLEX = 0x%X\n", is_complex);

    // status = BRD_ctrl(hADC, 0, BRDctrl_ADC_SETCHANMASK, &chan_mask);
    if (BRD_errcmp(status, BRDerr_OK))
        BRDC_printf(_BRDC("BRDctrl_ADC_GETCHANMASK: chan_mask = %0X\n"), chan_mask);
    else
        DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_ADC_GETCHANMASK"));
    U32 i = 0;
    S32 numChan = 0;
    // numChan = (chan_mask & 0x1) + ((chan_mask >> 1) & 0x1) + ((chan_mask >> 2) & 0x1) + ((chan_mask >> 3) & 0x1);
    for (i = 0; i < MAX_CHAN; i++)
        numChan += (chan_mask >> i) & 0x1;

    if (is_complex)
        numChan *= 2;

    // получить формат данных
    ULONG format = 0;
    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETFORMAT, &format);
    if (BRD_errcmp(status, BRDerr_OK))
        BRDC_printf(_BRDC("BRDctrl_ADC_GETFORMAT: format = %0X\n"), format);
    else
        DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_ADC_GETFORMAT"));

    BRD_ValChan value_chan;
    for (i = 0; i < adc_cfg.NumChans; i++)
    //	for(i = 0; i < numChan; i++)
    {
        if (((chan_mask >> i) & 1) == 0)
            continue;
        value_chan.chan = i;
        BRDC_printf(_BRDC("Channel %d:\n"), value_chan.chan);

        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETINPRANGE, &value_chan);
        if (BRD_errcmp(status, BRDerr_OK))
            // printf("BRDctrl_ADC_GETINPRANGE: range of channel %d = %f\n", value_chan.chan, value_chan.value);
            BRDC_printf(_BRDC("Range = %f\n"), value_chan.value);
        else
            DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_ADC_GETINPRANGE"));

        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETBIAS, &value_chan);
        if (BRD_errcmp(status, BRDerr_OK))
            //			printf("BRDctrl_ADC_GETBIAS: bias of channel %d = %f\n", value_chan.chan, value_chan.value);
            BRDC_printf(_BRDC("Bias = %f\n"), value_chan.value);
        else
            DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_ADC_GETBIAS"));

        if (BRDC_strstr(srvName, _BRDC("ADC212X200M"))
            || BRDC_strstr(srvName, _BRDC("ADC214X200M"))
            || BRDC_strstr(srvName, _BRDC("ADC214X400M"))
            || BRDC_strstr(srvName, _BRDC("ADC10X2G"))
            || BRDC_strstr(srvName, _BRDC("ADC216X100M"))
            || BRDC_strstr(srvName, _BRDC("FM814X125M"))) {
            status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETINPRESIST, &value_chan);
            if (BRD_errcmp(status, BRDerr_OK)) {
                if (value_chan.value)
                    BRDC_printf(_BRDC("Input resist is 50 Om\n"));
                else {
                    if (BRDC_strstr(srvName, _BRDC("ADC216X100M")))
                        BRDC_printf(_BRDC("Input resist is 1 kOm\n"));
                    else
                        BRDC_printf(_BRDC("Input resist is 1 MOm\n"));
                }
            } else
                DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_ADC_GETINPRESIST"));
        }
        if (BRDC_strstr(srvName, _BRDC("ADC28X1G"))
            || BRDC_strstr(srvName, _BRDC("ADC212X200M"))
            || BRDC_strstr(srvName, _BRDC("ADC214X200M"))
            || BRDC_strstr(srvName, _BRDC("ADC214X400M"))
            || BRDC_strstr(srvName, _BRDC("ADC10X2G"))
            || BRDC_strstr(srvName, _BRDC("ADC216X100M"))
            || BRDC_strstr(srvName, _BRDC("FM814X125M"))) {
            status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETDCCOUPLING, &value_chan);
            if (BRD_errcmp(status, BRDerr_OK)) {
                if (value_chan.value)
                    BRDC_printf(_BRDC("Input is OPENED\n"));
                else
                    BRDC_printf(_BRDC("Input is CLOSED\n"));
            } else
                DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_ADC_GETDCCOUPLING"));
        }
    }

    BRDCHAR Buffer[128];
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
    IPC_getPrivateProfileString(iniSectionName, _BRDC("IsPreTriggerMode"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
    GetPrivateProfileString(iniSectionName, _BRDC("IsPreTriggerMode"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
    p.g_PretrigMode = BRDC_atoi(Buffer);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
    IPC_getPrivateProfileString(iniSectionName, _BRDC("MemPostSamples"), _BRDC("16384"), Buffer, sizeof(Buffer), iniFilePath);
#else
    GetPrivateProfileString(iniSectionName, _BRDC("MemPostSamples"), _BRDC("16384"), Buffer, sizeof(Buffer), iniFilePath);
#endif
    p.g_nPostTrigSamples = BRDC_atoi64(Buffer);

    BRD_PretrigMode premode;
    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETPRETRIGMODE, &premode);
    if (!BRD_errcmp(status, BRDerr_OK))
        // BRDC_printf(_BRDC("Bias = %f\n"), premode.enable);
        // else
        DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_ADC_GETPRETRIGMODE"));

    if (premode.enable) {
        BRDC_printf(_BRDC("Pre-trigger is enabled. "));
        if (premode.assur)
            BRDC_printf(_BRDC("Pre-trigger is assurance. "));
        if (premode.external) {
            BRDC_printf(_BRDC("Post-trigger data size = %lld \n"), p.g_nPostTrigSamples);
        } else {
            BRDC_printf(_BRDC("Pre-trigger data size = %d \n"), premode.size);
        }
    }
    // GetPrivateProfileString("Option", "PreTriggerSamples", "16", Buffer, sizeof(Buffer), iniFilePath);
    // p.g_nPreTrigSamples = _atoi64(Buffer);

    // BRD_PretrigMode pretrigger;
    // pretrigger.enable = 0;
    // pretrigger.assur = 0;
    // pretrigger.external = 0;
    // if(p.g_PretrigMode)
    //{
    //	pretrigger.enable = 1;
    //	pretrigger.assur = (p.g_PretrigMode == 2) ? 1 : 0;
    // }
    // ULONG sample_size = format ? sizeof(char) : sizeof(short);
    // pretrigger.size = ULONG((p.g_nPreTrigSamples * numChan * sample_size) / sizeof(ULONG));
    // status = BRD_ctrl(hADC, 0, BRDctrl_ADC_SETPRETRIGMODE, &pretrigger);

    //	printf("Press any key to continue...\n");
    //	_getch();

    CheckClock(hADC, srvName);

    if (!BRDC_stricmp(srvName, _BRDC("ADC214X200M"))) {
        BRDC_printf(_BRDC("Sleeping 600 ms.\n"));
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
        IPC_delay(200);
        IPC_delay(200);
        IPC_delay(200);
#else
        Sleep(200);
        Sleep(200);
        Sleep(200);
#endif
    }
    BRDC_printf(_BRDC("ADC DRQ flag = %0X\n"), p.g_AdcDrqFlag);
    BRDC_printf(_BRDC("SDRAM DRQ flag = %0X\n"), p.g_MemDrqFlag);

    // отладочные команды
    if (p.g_IoDelay != 128) {
        ULONG stabil = 2; // сброс задержки
        BRD_AdcSpec spec;
        spec.command = ADCcmd_ADJUST;
        spec.arg = &stabil;
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_SETSPECIFIC, &spec);
        BRDC_printf(_BRDC("IoDelay reset!\n"));
        if (p.g_IoDelay) {
            stabil = (p.g_IoDelay >= 0) ? 1 : 0;
            spec.arg = &stabil;
            int num = abs(p.g_IoDelay);
            for (int i = 0; i < num; i++) {
                BRD_ctrl(hADC, 0, BRDctrl_ADC_SETSPECIFIC, &spec);
            }
            BRDC_printf(_BRDC("IoDelay = %d\n"), p.g_IoDelay);
        }
    }

    if (p.g_fileMap || p.g_DirWriteFile == -1) {
        if (!p.g_hPostfixFileMap) {
            BRDCHAR namePostfixMap[64] = _BRDC("data_postfix");
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
            p.g_hPostfixFileMap = IPC_createSharedMemory(namePostfixMap, 32768);
            p.g_pPostfix = (char*)IPC_mapSharedMemory(p.g_hPostfixFileMap);
#else
            p.g_hPostfixFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,
                NULL, PAGE_READWRITE,
                0, 32768,
                namePostfixMap);
            p.g_pPostfix = (char*)MapViewOfFile(p.g_hPostfixFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
#endif
            ULONG sample_size = format ? format : sizeof(short);
            p.g_bBufSize = (p.g_samplesOfChannel * numChan) * sample_size; // получить размер собираемых данных в байтах
        }
        if (p.g_DirWriteFile == -1)
            MappingIsviParams(lid, p.g_FileBufSize);
        else if (p.g_MemOn == 1)
            MappingIsviParams(lid, p.g_bMemBufSize);
        else
            MappingIsviParams(lid, p.g_bBufSize);
    }

    return numChan;
}

void MappingIsviParams(int lid, unsigned long long nNumberOfBytes)
{
    char str_buf[32768];
    BRD_Handle hADC = DevicesLid[lid].adc.handle();
    ParamsAdc& p = DevicesLid[lid].paramsAdc;

    sprintf(p.g_pPostfix, "\r\nDEVICE_NAME_________ ");
#ifdef _WIN64
    char srvName[32];
    wcstombs(srvName, p.g_AdcSrvName, 32);
    lstrcatA(p.g_pPostfix, srvName);
#else
    lstrcatA(p.g_pPostfix, p.g_AdcSrvName);
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
    sprintf(str_buf, "\r\nNUMBER_OF_CHANNELS__ %ld", long(num_chan));
    lstrcatA(p.g_pPostfix, str_buf);

    sprintf(str_buf, "\r\nNUMBERS_OF_CHANNELS_ ");
    lstrcatA(p.g_pPostfix, str_buf);
    str_buf[0] = 0;
    for (int iChan = 0; iChan < num_chan; iChan++) {
        char buf[512];
        sprintf(buf, "%d,", chans[iChan]);
        lstrcatA(str_buf, buf);
    }
    lstrcatA(p.g_pPostfix, str_buf);

    ULONG sample_size = format ? format : sizeof(short);
    ULONG samples = ULONG(nNumberOfBytes / sample_size / num_chan);

    if (p.g_MemOn)
        samples = p.g_memorySamplesOfChannel;

    if (p.g_Cycle > 1)
        samples *= p.g_Cycle;
    sprintf(str_buf, "\r\nNUMBER_OF_SAMPLES___ %ld", long(samples));
    lstrcatA(p.g_pPostfix, str_buf);

    sprintf(str_buf, "\r\nSAMPLINp.g_RATE_______ %f", p.g_samplRate);
    lstrcatA(p.g_pPostfix, str_buf);

    sprintf(str_buf, "\r\nBYTES_PER_SAMPLES___ %ld", long(sample_size));
    lstrcatA(p.g_pPostfix, str_buf);

    lstrcatA(p.g_pPostfix, "\r\nSAMPLES_PER_BYTES___ 1");

    if (is_complex)
        sprintf(str_buf, "\r\nIS_COMPLEX_SIGNAL?__ YES");
    else
        sprintf(str_buf, "\r\nIS_COMPLEX_SIGNAL?__ NO");
    lstrcatA(p.g_pPostfix, str_buf);

    double fc[MAX_CHAN];
    sprintf(str_buf, "\r\nSHIFT_FREQUENCY_____ ");
    lstrcatA(p.g_pPostfix, str_buf);
    str_buf[0] = 0;
    int num_fc = num_chan;
    for (int iChan = 0; iChan < num_fc; iChan++) {
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
    lstrcatA(p.g_pPostfix, str_buf);

    sprintf(str_buf, "\r\nGAINS_______________ ");
    lstrcatA(p.g_pPostfix, str_buf);
    str_buf[0] = 0;
    for (int iChan = 0; iChan < num_chan; iChan++) {
        char buf[16];
        sprintf(buf, "%f,", gains[chans[iChan]]);
        lstrcatA(str_buf, buf);
    }
    lstrcatA(p.g_pPostfix, str_buf);

    sprintf(str_buf, "\r\nVOLTAGE_OFFSETS_____ ");
    lstrcatA(p.g_pPostfix, str_buf);
    str_buf[0] = 0;
    for (int iChan = 0; iChan < num_chan; iChan++) {
        char buf[16];
        sprintf(buf, "%f,", volt_offset[iChan]);
        lstrcatA(str_buf, buf);
    }
    lstrcatA(p.g_pPostfix, str_buf);

    sprintf(str_buf, "\r\nVOLTAGE_RANGE_______ %f", adc_cfg.InpRange / 1000.);
    lstrcatA(p.g_pPostfix, str_buf);

    //	int BitsPerSample = !format ? adc_cfg.Bits : 8;
    //	if(is_complex)
    //		BitsPerSample = 16;
    int BitsPerSample = (format == 1) ? 8 : adc_cfg.Bits;
    if (BRDC_strstr(p.g_AdcSrvName, _BRDC("ADC1624X192")) || BRDC_strstr(p.g_AdcSrvName, _BRDC("ADC818X800")))
        if (format == 0 || format == 2)
            BitsPerSample = 16;
    sprintf(str_buf, "\r\nBIT_RANGE___________ %d", BitsPerSample);
    lstrcatA(p.g_pPostfix, str_buf);
    lstrcatA(p.g_pPostfix, "\r\n");

    // int len = lstrlenA(p.g_pPostfix);
}

// выполнить сбор данных в FIFO с программным методом передачи в ПК
S32 DaqIntoFifo(int lid, PVOID pSig, ULONG bBufSize, int DspMode)
{
    S32 status;
    ULONG Status = 0;
    ULONG Enable = 1;
    BRD_Handle hADC = DevicesLid[lid].adc.handle();
    ParamsAdc& p = DevicesLid[lid].paramsAdc;

    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_FIFORESET, NULL); // сборс FIFO АЦП
    //	if(DspMode)
    //		status = BRD_ctrl(hADC, 0, BRDctrl_DSPNODE_FIFORESET, NULL); // сброс FIFO ПЛИС ЦОС
    if (p.g_MemAsFifo) {
        status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_FIFORESET, NULL); // сборс FIFO SDRAM
        status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_ENABLE, &Enable); // разрешение записи в SDRAM
    }
    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_ENABLE, &Enable); // разрешение работы АЦП

    // дожидаемся заполнения FIFO
    do {
        //		if(DspMode)
        //			status = BRD_ctrl(hADC, 0, BRDctrl_DSPNODE_FIFOSTATUS, &Status);
        //		else
        if (p.g_MemAsFifo)
            status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_FIFOSTATUS, &Status);
        else
            status = BRD_ctrl(hADC, 0, BRDctrl_ADC_FIFOSTATUS, &Status);
    } while (Status & 0x40);

    Enable = 0;
    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_ENABLE, &Enable); // запрет работы АЦП

    if (p.g_MemAsFifo)
        status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_ENABLE, &Enable); // запрет записи в SDRAM

    BRD_DataBuf data_buf;
    data_buf.pData = pSig;
    data_buf.size = bBufSize;
    //	if(DspMode)
    //		status = BRD_ctrl(hADC, 0, BRDctrl_DSPNODE_GETDATA, &data_buf);
    //	else
    if (p.g_MemAsFifo)
        status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_GETDATA, &data_buf);
    else
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETDATA, &data_buf);

    return status;
}

// выполнить сбор данных в FIFO с ПДП-методом передачи в ПК
// S32 DaqIntoFifoDMA(BRD_Handle hADC, int idx, ULONG bBufSize, int DspMode)
S32 DaqIntoFifoDMA(int lid)
{
    //	printf("DAQ into ADC FIFO\n");
    S32 status;
    ULONG adc_status = 0;
    ULONG Enable = 1;
    BRD_Handle hADC = DevicesLid[lid].adc.handle();
    ParamsAdc& p = DevicesLid[lid].paramsAdc;

    // установить источник для работы стрима
    ULONG tetrad;
    if (p.g_MemAsFifo)
        status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_GETSRCSTREAM, &tetrad); // стрим будет работать с SDRAM
    else
        status = BRD_ctrl(hADC, 0, BRDctrl_ADC_GETSRCSTREAM, &tetrad); // стрим будет работать с АЦП
    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_SETSRC, &tetrad);

    // устанавливать флаг для формирования запроса ПДП надо после установки источника (тетрады) для работы стрима
    //	ULONG flag = BRDstrm_DRQ_ALMOST; // FIFO почти пустое
    //	ULONG flag = BRDstrm_DRQ_READY;
    //	ULONG flag = BRDstrm_DRQ_HALF; // рекомендуется флаг - FIFO наполовину заполнено
    ULONG flag = p.g_AdcDrqFlag;
    if (p.g_MemAsFifo)
        flag = p.g_MemDrqFlag;
    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_SETDRQ, &flag);
    if (!BRD_errcmp(status, BRDerr_OK))
        DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_STREAM_SETDRQ"));

    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_FIFORESET, NULL); // сброс FIFO АЦП
    if (p.g_MemAsFifo) {
        status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_FIFORESET, NULL); // сброс FIFO SDRAM
        status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_ENABLE, &Enable); // разрешение записи в SDRAM
    }
    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_RESETFIFO, NULL);

    BRDctrl_StreamCBufStart start_pars;
    start_pars.isCycle = 0; // без зацикливания
    // start_pars.isCycle = 1; // с зацикливанием
    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_START, &start_pars); // старт ПДП

#ifdef _WIN32
    // определение скорости сбора данных
    LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
    int bHighRes = QueryPerformanceFrequency(&Frequency);
    QueryPerformanceCounter(&StartPerformCount);
#else
    struct timeval start;
    struct timeval stop;
    struct timeval dt;
    gettimeofday(&start, 0);
#endif
    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_ENABLE, &Enable); // разрешение работы АЦП

    ULONG msTimeout = p.g_MsTimeout; // ждать окончания сбора данных до p.g_MsTimeout мсек.
    // ULONG msTimeout = 20000; // ждать окончания передачи данных до 20 сек.
    // while(1) // при старте с зацикливанием
    //{
    //	status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_WAITBLOCK, &msTimeout);
    //	if(BRD_errcmp(status, BRDerr_WAIT_TIMEOUT))
    //	{	// если вышли по тайм-ауту, то остановимся
    //		status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
    //		DisplayError(status, __FUNCTION__, _BRDC("TIME-OUT"));
    //		break;
    //	}
    //	BRDctrl_StreamCBufState buf_state;
    //	buf_state.timeout = 0;
    //	status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_STATE, &buf_state);
    //	printf("State Total Counter = %d\r", buf_state.blkNumTotal);
    //	//printf("Total Counter = %d\r", p.g_buf_dscr.pStub->totalCounter);
    //	if(GetAsyncKeyState(VK_ESCAPE))
    //	{
    //		status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
    //		printf("\n\n", buf_state.blkNumTotal);
    //		_getch();
    //		break;
    //	}
    // }

    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout);
#ifdef _WIN32
    QueryPerformanceCounter(&StopPerformCount);
#else
    gettimeofday(&stop, 0);
#endif
    if (BRD_errcmp(status, BRDerr_WAIT_TIMEOUT)) { // если вышли по тайм-ауту, то остановимся
        status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
        // DisplayError(status, __FUNCTION__, _BRDC("TIME-OUT"));
        ULONG AdcStatus = 0;
        ULONG Status = 0;
        BRDCHAR msg[255];
        if (p.g_MemAsFifo) {
            status = BRD_ctrl(hADC, 0, BRDctrl_ADC_FIFOSTATUS, &AdcStatus);
            status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_FIFOSTATUS, &Status);
            BRDC_sprintf(msg, _BRDC("BRDctrl_STREAM_CBUF_WAITBUF is TIME-OUT(%d sec.)\n AdcFifoStatus = %08X SdramFifoStatus = %08X"),
                msTimeout / 1000, AdcStatus, Status);
            // IPC_getch();
        } else {
            status = BRD_ctrl(hADC, 0, BRDctrl_ADC_FIFOSTATUS, &Status);
            BRDC_sprintf(msg, _BRDC("BRDctrl_STREAM_CBUF_WAITBUF is TIME-OUT(%d sec.)\n AdcFifoStatus = %08X"),
                msTimeout / 1000, Status);
            // IPC_getch();
        }
        DisplayError(status, __FUNCTION__, msg);
    }
    /*
            ULONG msTimeout = 0; // только проверка (без ожидания) - чтобы срабатывал Ctrl-Break
            int i = 0;
            for(i = 0; i < 200; i++)
            {
                    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_WAITBUF, &msTimeout);
                    if(BRD_errcmp(status, BRDerr_OK))
                            break;
    #if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
                    IPC_delay(100);
    #else
                    Sleep(100);
    #endif
            }
            if(i>=200)
            {	// если вышли по тайм-ауту, то остановимся
                    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
                    DisplayError(status, __FUNCTION__, _BRDC("TIME-OUT"));
            }
            QueryPerformanceCounter (&StopPerformCount);
    */
    Enable = 0;
    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_ENABLE, &Enable); // запрет работы АЦП

    if (p.g_MemAsFifo)
        status = BRD_ctrl(hADC, 0, BRDctrl_SDRAM_ENABLE, &Enable); // запрет записи в SDRAM

#ifdef _WIN32
    double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
#else
    dt.tv_sec = stop.tv_sec - start.tv_sec;
    dt.tv_usec = stop.tv_usec - start.tv_usec;
    if (dt.tv_usec < 0) {
        dt.tv_sec--;
        dt.tv_usec += 1000000;
    }
    double msTime = dt.tv_sec * 1000 + (double)dt.tv_usec / 1000;

#endif
    if (p.g_transRate)
        printf("DAQ & Transfer by bus rate is %.2f Mbytes/sec\r", ((double)p.g_bBufSize / msTime) / 1000.);

    status = BRD_ctrl(hADC, 0, BRDctrl_ADC_ISBITSOVERFLOW, &adc_status);
    // status = BRD_ctrl(hADC, 0, BRDctrl_ADC_FIFOSTATUS, &adc_status);
    // printf("ADC status = 0x%X ", adc_status);
    if (adc_status)
        printf("ADC Bits OVERFLOW %lX  ", adc_status);
    return status;
}

// размещение буфера для получения данных с АЦП через Стрим
//	hADC - дескриптор службы АЦП (IN)
//	pSig - указатель на массив указателей (IN), каждый элемент массива является указателем на блок (OUT)
//	pbytesBufSize - общий размер данных (всех блоков составного буфера), которые должны быть выделены (IN/OUT - может меняться внутри функции)
//	bufType - тип памяти для данных (IN):
//		0 - пользовательская память выделяется в драйвере (точнее, в DLL базового модуля)
//		1 - системная память выделяется драйвере 0-го кольца
//		2 - пользовательская память выделяется в приложении
//	pBlkNum - число блоков составного буфера (OUT)
S32 AllocDaqBuf(int lid, PVOID*& pSig, unsigned long long* pbytesBufSize, ULONG bufType, ULONG* pBlkNum)
{
    S32 status;
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    BRD_Handle hADC = DevicesLid[lid].adc.handle();

    unsigned long long bBufSize = *pbytesBufSize;
    ULONG bBlkSize;
    ULONG blkNum = 1;
    // определяем число блоков составного буфера
//	if(bBufSize > 2147483648) // максимальный размер блока = 2 Гбайта
// if(bBufSize > 524288) // максимальный размер блока = 512 Kбайт
#ifdef _WIN32
    if (bBufSize > 1073741824) // максимальный размер блока = 1 Гбайт
#else // LINUX
    if (bBufSize > 4194304) // максимальный размер блока = 4 Mбайтa
#endif
    {
        do {
            blkNum <<= 1;
            bBufSize >>= 1;
            //}while(bBufSize > 2147483648);
            //}while(bBufSize > 524288);
#ifdef _WIN32
        } while (bBufSize > 1073741824);
#else // LINUX
        } while (bBufSize > 4194304);
#endif
    }
    bBlkSize = (ULONG)bBufSize;

    void** pBuffer = NULL;
    if (2 == bufType) {
#ifdef __linux__
        ULONG nBlkSize = (p.g_MemOn == 1) ? p.g_bMemBufSize : bBlkSize * blkNum;
        ULONG nBlkNum = 1;
#else
        ULONG nBlkSize = bBlkSize;
        ULONG nBlkNum = blkNum;
#endif
        // pBuffer = malloc(*pbytesBufSize);
        pBuffer = new PVOID[nBlkNum];
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
        p.g_hBufFileMap = new IPC_handle[nBlkNum];
#else
        p.g_hBufFileMap = new HANDLE[nBlkNum];
#endif
        for (ULONG i = 0; i < nBlkNum; i++) {
            if (p.g_fileMap) {
                BRDCHAR nameBufMap[64];
                BRDC_sprintf(nameBufMap, _BRDC("data_blk%d"), i);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
                p.g_hBufFileMap[i] = IPC_createSharedMemory(nameBufMap, nBlkSize);
                pBuffer[i] = IPC_mapSharedMemory(p.g_hBufFileMap[i]);
#else
                p.g_hBufFileMap[i] = CreateFileMapping(INVALID_HANDLE_VALUE,
                    NULL, PAGE_READWRITE,
                    0, nBlkSize,
                    nameBufMap);
                pBuffer[i] = MapViewOfFile(p.g_hBufFileMap[i], FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
#endif
            }
#ifdef _WIN32
            else {
                pBuffer[i] = VirtualAlloc(NULL, nBlkSize, MEM_COMMIT, PAGE_READWRITE);
                if (!pBuffer[i]) {
                    BRDC_printf(_BRDC("VirtualAlloc() by allocating buffer %d is error!!!\n"), i);
                    return -1; // error
                }
            }
#endif
        }
        if (p.g_fileMap) {
            BRDCHAR nameFlagMap[64] = _BRDC("data_flg");
            // BRDC_sprintf(nameFlagMap, _BRDC("data_flg"), i);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
            p.g_hFlgFileMap = IPC_createSharedMemory(nameFlagMap, 3 * sizeof(ULONG));
            p.g_pFlags_adc = (ULONG*)IPC_mapSharedMemory(p.g_hFlgFileMap);

            p.g_pMapBuf = pBuffer[0];

            char str_buf[64];
            sprintf(str_buf, "BLKNUM %d", nBlkNum);
            lstrcatA(p.g_pPostfix, str_buf);
            sprintf(str_buf, "\r\nBLKSIZE %d\r\n", nBlkSize);
            lstrcatA(p.g_pPostfix, str_buf);
#else
            p.g_hFlgFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,
                NULL, PAGE_READWRITE,
                0, 3 * sizeof(ULONG),
                nameFlagMap);
            p.g_pFlags_adc = (ULONG*)MapViewOfFile(p.g_hFlgFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
#endif
        }
    }

#ifdef __linux__
    if (bufType == 2)
        bufType = 1;
#endif

    p.g_buf_dscr.dir = BRDstrm_DIR_IN;
    p.g_buf_dscr.isCont = bufType; // 0 - буфер размещается в пользовательской памяти ПК, 1 - в системной
    p.g_buf_dscr.blkNum = blkNum;
    p.g_buf_dscr.blkSize = bBlkSize; //*pbytesBufSize;
    p.g_buf_dscr.ppBlk = new PVOID[p.g_buf_dscr.blkNum];
    if (p.g_buf_dscr.isCont == 2) {
        for (ULONG i = 0; i < blkNum; i++)
            p.g_buf_dscr.ppBlk[i] = pBuffer[i];
        delete[] pBuffer;
    }
    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_ALLOC, &p.g_buf_dscr);
    if (!BRD_errcmp(status, BRDerr_OK) && !BRD_errcmp(status, BRDerr_PARAMETER_CHANGED))
        DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_STREAM_CBUF_ALLOC"));
    if (BRD_errcmp(status, BRDerr_PARAMETER_CHANGED)) { // может быть выделено меньшее количество памяти
        BRDC_printf(_BRDC("Warning!!! BRDctrl_STREAM_CBUF_ALLOC: BRDerr_PARAMETER_CHANGED\n"));
        status = BRDerr_OK;
    }
    //*pSig = p.g_buf_dscr.ppBlk[0];
    pSig = new PVOID[blkNum];
    for (ULONG i = 0; i < blkNum; i++) {
        pSig[i] = p.g_buf_dscr.ppBlk[i];
    }
    *pbytesBufSize = (unsigned long long)p.g_buf_dscr.blkSize * blkNum;
    *pBlkNum = blkNum;
    BRDC_printf(_BRDC("Allocated memory for Stream:: Number of blocks = %d, Block size = %d kBytes\n"),
        blkNum, p.g_buf_dscr.blkSize / 1024);
    return status;
}

// освобождение буфера для получения данных с АЦП через Стрим
S32 FreeDaqBuf(int lid, ULONG blkNum)
{
    S32 status;
    BRD_Handle hADC = DevicesLid[lid].adc.handle();
    ParamsAdc& p = DevicesLid[lid].paramsAdc;

    status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_FREE, NULL);
    if (!BRD_errcmp(status, BRDerr_OK))
        DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_STREAM_CBUF_FREE"));
    if (p.g_buf_dscr.isCont == 2) {
        for (ULONG i = 0; i < blkNum; i++) {
            if (p.g_fileMap) {
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
#ifdef __linux__
                blkNum = 1;
#endif
                IPC_unmapSharedMemory(p.g_hBufFileMap[i]);
                IPC_deleteSharedMemory(p.g_hBufFileMap[i]);
#else
                UnmapViewOfFile(p.g_buf_dscr.ppBlk[i]);
                CloseHandle(p.g_hBufFileMap[i]);
#endif
            }
#ifdef _WIN32
            else
                VirtualFree(p.g_buf_dscr.ppBlk[i], 0, MEM_RELEASE);
#endif
        }
        if (p.g_fileMap) {
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
            IPC_unmapSharedMemory(p.g_hFlgFileMap);
            IPC_deleteSharedMemory(p.g_hFlgFileMap);
#else
            UnmapViewOfFile(p.g_pFlags_adc);
            CloseHandle(p.g_hFlgFileMap);
#endif
        }
    }
    delete[] p.g_buf_dscr.ppBlk;
    // delete pSig;
    return status;
}

// размещение 1-блокового буфера для получения данных с АЦП через Стрим
// S32 AllocDaqBuf(int lid, PVOID* pSig, unsigned long long* pbytesBufSize, ULONG bufType)
//{
//	S32		status;
//
//	void* pBuffer = NULL;
//	if(2 == bufType)
//	{
//		pBuffer = malloc(*pbytesBufSize);
////		pBuffer = VirtualAlloc(NULL, bBufSize, MEM_COMMIT, PAGE_READWRITE);
//		if(!pBuffer)
//		{
//			BRDC_printf(_BRDC("VirtualAlloc() is error!!!\n"));
//			return -1; // error
//		}
//	}
//	p.g_buf_dscr.dir = BRDstrm_DIR_IN;
//	p.g_buf_dscr.isCont = bufType; // 0 - буфер размещается в пользовательской памяти ПК, 1 - в системной
//	p.g_buf_dscr.blkNum = 1;
//	p.g_buf_dscr.blkSize = *pbytesBufSize;
//	p.g_buf_dscr.ppBlk = new PVOID[p.g_buf_dscr.blkNum];
//	if(p.g_buf_dscr.isCont == 2)
//		p.g_buf_dscr.ppBlk[0] = pBuffer;
//	status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_ALLOC, &p.g_buf_dscr);
//	if(!BRD_errcmp(status, BRDerr_OK) && !BRD_errcmp(status, BRDerr_PARAMETER_CHANGED))
//		DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_STREAM_CBUF_ALLOC"));
//	if(BRD_errcmp(status, BRDerr_PARAMETER_CHANGED))
//	{
//		BRDC_printf(_BRDC("Warning!!! BRDctrl_STREAM_CBUF_ALLOC: BRDerr_PARAMETER_CHANGED\n"));
//		status = BRDerr_OK;
//	}
//	*pSig = p.g_buf_dscr.ppBlk[0];
//	*pbytesBufSize = p.g_buf_dscr.blkSize;
////	printf("BRDctrl_STREAM_CBUF_ALLOC:: %d kBytes, memory type = %d\n", (*pbytesBufSize)/1024, bufType);
//	return status;
//}

// освобождение 1-блокового буфера для получения данных с АЦП через Стрим
// S32 FreeDaqBuf(BRD_Handle hADC)
//{
//	S32		status;
//	status = BRD_ctrl(hADC, 0, BRDctrl_STREAM_CBUF_FREE, NULL);
//	if(!BRD_errcmp(status, BRDerr_OK))
//		DisplayError(status, __FUNCTION__, _BRDC("BRDctrl_STREAM_CBUF_FREE"));
//	if(p.g_buf_dscr.isCont == 2)
//		free(p.g_buf_dscr.ppBlk[0]);
////		VirtualFree(p.g_buf_dscr.ppBlk[0], 0, MEM_RELEASE);
//	delete[] p.g_buf_dscr.ppBlk;
//	return status;
//}

void MapWriteData(int lid, PVOID* pBuf, unsigned long long nNumberOfBytes)
{
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    DWORD blockSize = DWORD(nNumberOfBytes / p.g_bBlkNum);
    for (ULONG i = 0; i < p.g_bBlkNum; i++) {
        char* pDst = (char*)p.g_pMapBuf;
        memcpy((void*)(pDst + i * blockSize), pBuf[i], blockSize);
    }
}

void MapWrFlagSinc(int lid, int flg, int isNewParam)
{
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    p.g_pFlags_adc[0] = flg;
    p.g_pFlags_adc[1] = isNewParam;
    p.g_pFlags_adc[2] = p.g_buf_dscr.blkSize;
}

int MapRdFlagSinc(int lid)
{
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    int flg = p.g_pFlags_adc[0];
    return flg;
}

S32 MapDataFromMemWriteData(int lid, PVOID* pBuf, unsigned long long bBufSize, unsigned long long bMemBufSize, ULONG DmaOn)
{
    BRD_Handle hADC = DevicesLid[lid].adc.handle();
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    char* pDst = (char*)p.g_pMapBuf;
    S32 status = BRDerr_OK;

    //    if(p.g_PretrigMode == 3)
    //    {
    //        // получить параметры, актуальные в режиме претриггера
    //        status = GetPostrigData(hADC);
    //    }

    int nCnt = int(bMemBufSize / bBufSize);
    for (int iCnt = 0; iCnt < nCnt; iCnt++) {
        status = DataFromMem(hADC, *pBuf, (ULONG)bBufSize, DmaOn);
        if (!BRD_errcmp(status, BRDerr_OK))
            return status;
        memcpy((void*)pDst, *pBuf, (ULONG)bBufSize);
        pDst += bBufSize;
    }
    ULONG ostSize = ULONG(bMemBufSize % bBufSize);
    if (ostSize) {
        status = DataFromMem(hADC, *pBuf, (ULONG)bBufSize, DmaOn);
        memcpy((void*)pDst, *pBuf, ostSize);
    }

    return status;
}

typedef struct _THREAD_PARAM {
    BRD_Handle handle;
    int idx;
} THREAD_PARAM, *PTHREAD_PARAM;

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
thread_value __IPC_API DirWriteIntoFile(void* pParams);
int SimpleProcWrDir(BRD_Handle hSrv, IPC_handle hfile, int idx, BRDctrl_StreamCBufAlloc* buf_dscr);
int MultiBlkProcWrDir(BRD_Handle hSrv, IPC_handle hfile, int idx, BRDctrl_StreamCBufAlloc* buf_dscr);
#else
unsigned __stdcall DirWriteIntoFile(void* pParams);
int SimpleProcWrDir(BRD_Handle hSrv, HANDLE hfile, int idx, BRDctrl_StreamCBufAlloc* buf_dscr);
int MultiBlkProcWrDir(BRD_Handle hSrv, HANDLE hfile, int idx, BRDctrl_StreamCBufAlloc* buf_dscr);
#endif

void DspFunc(void* buf, ULONG size);

/*
int p.g_flbreak_adc = 0;

ULONG p.g_bufType;
ULONG p.g_fileBufSize;
ULONG p.g_fileBufNum;
ULONG p.g_fileBlkNum;
*/

void DirectFile(int lid, ULONG bufType, ULONG FileBufSize, ULONG FileBufNum, ULONG FileBlkNum)
{
    THREAD_PARAM thread_par;
    ParamsAdc& p = DevicesLid[lid].paramsAdc;
    printf("<DBG> DirectFile: ADC-%d, bufType=%lu, FileBufSize=%lu, FileBufNum=%lu? FileBlkNum=%ul \n", lid, bufType, FileBufSize, FileBufNum, FileBlkNum);

#ifdef _WIN32
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    DWORD prior_class = GetPriorityClass(GetCurrentProcess());
    BRDC_printf(_BRDC("Process Priority = %d\n"), prior_class);
#endif

    p.g_bufType = bufType;
    p.g_fileBufSize = FileBufSize;
    p.g_fileBufNum = FileBufNum;
    p.g_fileBlkNum = FileBlkNum;

    p.g_flbreak_adc = 0;
    thread_par.handle = DevicesLid[lid].adc.handle(); // x_hADC;
    thread_par.idx = lid; // 0;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
    IPC_handle hThread = IPC_createThread(_BRDC("DirWriteIntoFile"), &DirWriteIntoFile, &thread_par);
    IPC_waitThread(hThread, INFINITE); // Wait until threads terminates
    IPC_deleteThread(hThread);
#else
    unsigned threadID;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &DirWriteIntoFile, &(thread_par), 0, &(threadID));
    WaitForSingleObject(hThread, INFINITE); // Wait until threads terminates
    CloseHandle(hThread);
#endif
}

// extern BRDCHAR g_dirFileName[];
//  extern ULONG p.g_flDirFileName;

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
thread_value __IPC_API DirWriteIntoFile(void* pParams)
#else
unsigned __stdcall DirWriteIntoFile(void* pParams)
#endif
{
    S32 status = BRDerr_OK;
    ULONG Status = 0;
#ifdef _WIN32
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    int prior = GetThreadPriority(GetCurrentThread());
    BRDC_printf(_BRDC("Thread Priority = %d\n"), prior);
#endif
    PTHREAD_PARAM pThreadParam = (PTHREAD_PARAM)pParams;
    BRD_Handle hSrv = pThreadParam->handle;
    int idx = pThreadParam->idx;
    ParamsAdc& p = DevicesLid[idx].paramsAdc;

    BRDCHAR fileName[MAX_PATH];
    if (idx != 0)
        BRDC_sprintf(fileName, _BRDC("%s_%d.bin"), p.g_dirFileName, idx);
    else
        BRDC_strcpy(fileName, p.g_dirFileName);

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
    IPC_handle hfile = IPC_openFileEx(fileName,
        IPC_CREATE_FILE | IPC_FILE_WRONLY,
        IPC_FILE_NOBUFFER);
    if (!hfile) {
        BRDC_printf(_BRDC("Create file error\n"));
        return (thread_value)status;
    }
#else
    HANDLE hfile = CreateFile(fileName,
        GENERIC_WRITE,
        //								FILE_SHARE_WRITE | FILE_SHARE_READ,
        0,
        NULL,
        CREATE_ALWAYS,
        //								FILE_ATTRIBUTE_NORMAL,
        FILE_FLAG_NO_BUFFERING, // | FILE_FLAG_WRITE_THROUGH,
        NULL);
    if (hfile == INVALID_HANDLE_VALUE) {
        BRDC_printf(_BRDC("Create file error\n"));
        return status;
    }
#endif

    BRDctrl_StreamCBufAlloc buf_dscr;
    buf_dscr.dir = BRDstrm_DIR_IN;
    buf_dscr.isCont = p.g_bufType; // 0/1 - буфер размещается в пользовательской/системной памяти ПК
    buf_dscr.blkNum = p.g_fileBlkNum;
    buf_dscr.blkSize = p.g_fileBufSize;
    buf_dscr.ppBlk = new PVOID[buf_dscr.blkNum];
    status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_CBUF_ALLOC, &buf_dscr);
    if (!BRD_errcmp(status, BRDerr_OK) && !BRD_errcmp(status, BRDerr_PARAMETER_CHANGED)) {
        BRDC_printf(_BRDC("ERROR!!! BRDctrl_STREAM_CBUF_ALLOC\n"));
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
        return (thread_value)status;
#else
        return status;
#endif
    }
    if (BRD_errcmp(status, BRDerr_PARAMETER_CHANGED)) {
        BRDC_printf(_BRDC("Warning!!! BRDctrl_STREAM_CBUF_ALLOC: BRDerr_PARAMETER_CHANGED\n"));
        status = BRDerr_OK;
    }
    BRDC_printf(_BRDC("Block size = %d Mbytes, Block num = %d, Total blocks = %d\n"), buf_dscr.blkSize / 1024 / 1024, buf_dscr.blkNum, p.g_fileBufNum);

    // установить источник для работы стрима
    ULONG tetrad;
    if (p.g_MemAsFifo)
        status = BRD_ctrl(hSrv, 0, BRDctrl_SDRAM_GETSRCSTREAM, &tetrad); // стрим будет работать с SDRAM
    else
        status = BRD_ctrl(hSrv, 0, BRDctrl_ADC_GETSRCSTREAM, &tetrad); // стрим будет работать с АЦП
    status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_SETSRC, &tetrad);

    if (p.g_adjust_mode)
        status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_CBUF_ADJUST, &p.g_adjust_mode);

    // устанавливать флаг для формирования запроса ПДП надо после установки источника (тетрады) для работы стрима
    //	ULONG flag = BRDstrm_DRQ_ALMOST; // FIFO почти пустое
    //	ULONG flag = BRDstrm_DRQ_READY;
    //	ULONG flag = BRDstrm_DRQ_HALF; // рекомендуется флаг - FIFO наполовину заполнено
    ULONG flag = p.g_AdcDrqFlag;
    if (p.g_MemAsFifo)
        flag = p.g_MemDrqFlag;
    status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_SETDRQ, &flag);

    ULONG Enable = 1;
    status = BRD_ctrl(hSrv, 0, BRDctrl_ADC_FIFORESET, NULL); // сброс FIFO АЦП
    status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_RESETFIFO, NULL);
    if (p.g_MemAsFifo) {
        status = BRD_ctrl(hSrv, 0, BRDctrl_SDRAM_FIFORESET, NULL); // сборс FIFO SDRAM
        status = BRD_ctrl(hSrv, 0, BRDctrl_SDRAM_ENABLE, &Enable); // разрешение записи в SDRAM
    }

    BRDctrl_StreamCBufStart start_pars;
    start_pars.isCycle = 1; // с зацикливанием
    status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_CBUF_START, &start_pars); // старт ПДП
    status = BRD_ctrl(hSrv, 0, BRDctrl_ADC_ENABLE, &Enable); // разрешение работы АЦП

    int errCnt = 0;
    BRDC_printf(_BRDC("Data writing into file %s...\n"), fileName);
    if (p.g_fileBlkNum == 2)
        errCnt = SimpleProcWrDir(hSrv, hfile, idx, &buf_dscr);
    else
        errCnt = MultiBlkProcWrDir(hSrv, hfile, idx, &buf_dscr);

    Enable = 0;
    status = BRD_ctrl(hSrv, 0, BRDctrl_ADC_ENABLE, &Enable); // запрет работы АЦП
    if (p.g_MemAsFifo)
        status = BRD_ctrl(hSrv, 0, BRDctrl_SDRAM_ENABLE, &Enable); // запрет записи в SDRAM
    //	printf("                                             \r");
    if (errCnt)
        BRDC_printf(_BRDC("ERROR (%s): buffers skiped %d\n"), fileName, errCnt);
    BRDC_printf(_BRDC("Total Buffer Counter (%s) = %d\n"), fileName, buf_dscr.pStub->totalCounter);
    status = BRD_ctrl(hSrv, 0, BRDctrl_ADC_FIFOSTATUS, &Status);
    if (Status & 0x80)
        BRDC_printf(_BRDC("ERROR (%s): ADC FIFO is overflow\n"), fileName);
    if (p.g_MemAsFifo) {
        status = BRD_ctrl(hSrv, 0, BRDctrl_SDRAM_FIFOSTATUS, &Status);
        if (Status & 0x8000)
            BRDC_printf(_BRDC("ERROR (%s): SDRAM FIFO is overflow\n"), fileName);
    }
    status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_CBUF_FREE, &buf_dscr);
    delete[] buf_dscr.ppBlk;

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
    IPC_closeFile(hfile);
    return (thread_value)status;
#else
    CloseHandle(hfile);
    _endthreadex(0);
    return status;
#endif
}

int SimpleProcWrDir(BRD_Handle hSrv, IPC_handle hfile, int idx, BRDctrl_StreamCBufAlloc* buf_dscr)
{
    S32 status = BRDerr_OK;
    ULONG Status = 0;
    ULONG curbuf = 0;
    int cnt = 0;
    int errcnt = 0;
    ULONG msTimeout = 40000; // ждать окончания передачи данных до 40 сек.
    ParamsAdc& p = DevicesLid[idx].paramsAdc;

    for (ULONG i = 0; i < p.g_fileBufNum; i++) {
        status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_CBUF_WAITBLOCK, &msTimeout);
        if (BRD_errcmp(status, BRDerr_WAIT_TIMEOUT)) { // если вышли по тайм-ауту, то остановимся
            status = BRD_ctrl(hSrv, 0, BRDctrl_ADC_FIFOSTATUS, &Status);
            BRDC_printf(_BRDC("ADC FIFO Status = 0x%04X\n"), Status);
            status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
            DisplayError(status, __FUNCTION__, _BRDC("TIME-OUT"));
            break;
        }
        if (!idx && IPC_kbhit()) {
            int ch = IPC_getch(); // получает клавишу
            if (0x1B == ch) // если Esc
            {
                p.g_flbreak_adc = 1;
                break;
            }
        }
        IPC_writeFile(hfile, buf_dscr->ppBlk[curbuf], buf_dscr->blkSize);
        curbuf ^= 1;
        if (cnt + 1 != (int)buf_dscr->pStub->totalCounter)
            errcnt++;
        //			printf("ERROR: buffer %d skip\n", cnt);
        cnt = buf_dscr->pStub->totalCounter;
        if (i % 4 == 0)
            BRDC_printf(_BRDC("Current buffer = %d\r"), cnt);
        //			printf("Current buffer = %d, MO = %d\r", cnt, mo);
    }
    return errcnt;
}

int MultiBlkProcWrDir(BRD_Handle hSrv, IPC_handle hfile, int idx, BRDctrl_StreamCBufAlloc* buf_dscr)
{
    S32 status = BRDerr_OK;
    ULONG Status = 0;
    ULONG total_cnt = 0;
    LONG delta_cnt = 0;
    LONG twice = 1;
    LONG cur_buf = 0;
    // LONG daq_buf = 0;
    ULONG write_cnt = 0;
    ULONG first_err = 0;
    int err_cnt = 0;
    int waitblk_cnt = 0;
    ULONG msTimeout = 40000; // ждать окончания передачи данных до 20 сек.
    ParamsAdc& p = DevicesLid[idx].paramsAdc;

    do {
        total_cnt = buf_dscr->pStub->totalCounter;
        delta_cnt = total_cnt - write_cnt;
        // printf("Delta = %d, total %d\n", delta_cnt, total_cnt);
        if (delta_cnt > (S32)buf_dscr->blkNum) {
            if (!err_cnt)
                first_err = write_cnt;
            err_cnt++;
            delta_cnt = 0;
        }
        if (delta_cnt <= 0 || !(delta_cnt % buf_dscr->blkNum)) {
            // printf("Waiting...\n");
            waitblk_cnt++;
            status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_CBUF_WAITBLOCK, &msTimeout);
            if (BRD_errcmp(status, BRDerr_WAIT_TIMEOUT)) { // если вышли по тайм-ауту, то остановимся
                status = BRD_ctrl(hSrv, 0, BRDctrl_ADC_FIFOSTATUS, &Status);
                BRDC_printf(_BRDC("ADC FIFO Status = 0x%04X\n"), Status);
                status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
                DisplayError(status, __FUNCTION__, _BRDC("TIME-OUT"));
                break;
            }
            if (twice) {
                status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_CBUF_WAITBLOCK, &msTimeout);
                if (BRD_errcmp(status, BRDerr_WAIT_TIMEOUT)) { // если вышли по тайм-ауту, то остановимся
                    status = BRD_ctrl(hSrv, 0, BRDctrl_ADC_FIFOSTATUS, &Status);
                    BRDC_printf(_BRDC("ADC FIFO Status = 0x%04X\n"), Status);
                    status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_CBUF_STOP, NULL);
                    DisplayError(status, __FUNCTION__, _BRDC("TIME-OUT"));
                    break;
                }
            }
            delta_cnt = 1;
            twice = 0;
            // total_cnt++;
        } else
            twice = 1;
        if (!idx && IPC_kbhit()) {
            int ch = IPC_getch(); // получает клавишу
            if (0x1B == ch) // если Esc
            {
                p.g_flbreak_adc = 1;
                break;
            }
        }
        cur_buf = write_cnt % buf_dscr->blkNum;
        for (int j = 0; j < delta_cnt; j++) {
            // daq_buf = buf_dscr.pStub->lastBlock + 1;
            // if(daq_buf == buf_dscr.blkNum)
            //	daq_buf = 0;
            // if(daq_buf == cur_buf)
            //	err_cnt++;
            // printf("Writing %d, daq %d, total %d\n", cur_buf, daq_buf, buf_dscr.pStub->totalCounter);
            // DspFunc(buf_dscr->ppBlk[cur_buf], buf_dscr->blkSize);
            if (p.g_MemAsFifo) {
                status = BRD_ctrl(hSrv, 0, BRDctrl_SDRAM_FIFOSTATUS, &Status);
                if (Status & 0x8000)
                    BRDC_printf(_BRDC("ERROR: SDRAM FIFO is overflow (SDRAM FIFO Status = 0x%04X)\n"), Status);
            } else {
                status = BRD_ctrl(hSrv, 0, BRDctrl_ADC_FIFOSTATUS, &Status);
                if (Status & 0x80)
                    BRDC_printf(_BRDC("ERROR: ADC FIFO is overflow (ADC FIFO Status = 0x%04X)\n"), Status);
            }
            IPC_writeFile(hfile, buf_dscr->ppBlk[cur_buf], buf_dscr->blkSize);
            if (p.g_adjust_mode)
                status = BRD_ctrl(hSrv, 0, BRDctrl_STREAM_CBUF_DONE, &cur_buf);
            cur_buf++;
            if (cur_buf == (LONG)buf_dscr->blkNum)
                cur_buf = 0;
            write_cnt++;
            if (write_cnt == p.g_fileBufNum)
                break;
        }
        BRDC_printf(_BRDC("Status = 0x%04X, Delta = %ld, written buffers = %d, errors = %d\r"), Status, delta_cnt, write_cnt, err_cnt);
        // delta_cnt = buf_dscr.pStub->totalCounter - write_cnt;
        // if(delta_cnt > buf_dscr.blkNum)
        //	err_cnt += delta_cnt - 1;
        // if(write_cnt % 4 == 0)
        //{
        // total_cnt = buf_dscr.pStub->totalCounter;
        // printf("%s: Total buffers = %d, written buffers = %d\r", fileName, total_cnt, write_cnt);
        //	printf("%s: written buffers = %d\r", fileName, write_cnt);
        //}
    } while (write_cnt < p.g_fileBufNum);
    return err_cnt;
}

// void DspFunc(void* buf, ULONG size)
//{
//	USHORT* pSig = (USHORT*)buf;
//	int num = size >> 1;
//	for(int i = 0; i < num; i++)
//		pSig[i] += 1;
// }
