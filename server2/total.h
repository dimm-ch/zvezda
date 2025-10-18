#pragma once

#include "brdapi.h"
#include "exam_edac.h"
#include "gipcy.h"
#include <atomic>
#include <thread>
#include <vector>

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
#include "gipcy.h"
#include <string.h>
#else
#include <conio.h>
#include <process.h>
#endif
// All total definition

extern S32 x_DevNum; // кол-во у-в
#define MAX_LID_DEVICES 10
#define LID_DEFAULT 1

// параметры DAC
struct ParamsDACs {
    ParamsDACs()
        : g_nMsTimeout(5000)
        , g_iniFileName(_BRDC("exam_edac.ini"))
        , g_pFileDataBin(nullptr)

    {
    }
    BRDCHAR g_sFullName[MAX_PATH];
    BRDCHAR g_iniFileName[256];
    BRDCHAR g_sServiceName[256]; // без номера службы
    BRDCHAR g_sPldFileName[MAX_PATH];
    BRD_Handle g_hDev[MAX_DEVICE];
    int g_exitRightAway;
    int g_nWorkMode;
    int g_nIsAlwaysWriteSdram;
    SBIG g_nSamplesPerChannel;
    int g_nIsSystemMemory;
    int g_nSdramWriteBufSize;
    int g_nIsAlwaysLoadPld;
    int g_nMasterSlave;
    int g_nDmaBufFactor;
    int g_nCycle;
    int g_nQuickQuit;
    S32 g_nMsTimeout;
    // S32 g_idx[MAX_DAC];
    BRD_Handle g_idx[MAX_DAC]; // Рассортированные номера ЦАПов для MASTER/SLAVE
    double g_dSamplingRate;
    double g_dSignalFreq;
    U32 g_dSignalType;
    FILE* g_pFileDataBin;
    int g_nIsDebugMarker;
    int g_nIsNew;
};

// параметры ADC
struct ParamsAdc {
    ParamsAdc()
        : g_fileMap(0)
        , g_isPldLoadAlways(0)
        , g_OnlySetParams(0)
        , g_subNo(0)
        , g_times(0)
        , g_DmaOn(1)
        , g_Cycle(0)
        , g_Pause(0)
        , g_MemOn(1)
        , g_IsWriteFile(1)
        , g_IsSysMem(0)
        , g_samplesOfChannel(16384)
        , g_memorySamplesOfChannel(16384)
        , DspMode(0)
        , g_MemAsFifo(0)
        , g_dirFileName(_BRDC("adcdir.bin"))
        , g_quick_quit(0)
        , g_PretrigMode(0)
        , g_nPostTrigSamples(16384)
        , g_bPostTrigSize(32768)
        , g_iniFileNameAdc(_BRDC("exam_adc.ini"))
        , g_StopFlag(0)
        , g_flbreak_cont(0)
        , g_flbreak_adc(0)
        , g_hBufFileMap(nullptr)
        , g_hBufFileMap_cont(nullptr)
        , g_hFlgFileMap_cont(nullptr)
        , g_hFlgFileMap(nullptr)
        , g_hPostfixFileMap(nullptr)
#ifdef _WIN32
        , g_hThread(nullptr)
        , g_hUserEvent(nullptr)
#endif
    {
    }
    int g_subNo; // номер службы АЦП из командной строки
    int g_fileMap;
    ULONG* g_pFlags_cont;
    ULONG* g_pFlags_adc;
    char* g_pPostfix;
    ULONG g_MsTimeout;
    BRDctrl_StreamCBufAlloc g_buf_dscr;
    ULONG g_MemAsFifo;
    ULONG g_AdcDrqFlag;
    ULONG g_MemDrqFlag;
    double g_samplRate;
    int g_IoDelay;
    BRDCHAR g_AdcSrvName[64]; // с номером службы
    BRDCHAR g_SrvName[64]; // без номера службы
    int g_AdcSrvNum; // номер службы
    BRDCHAR g_pldFileName[MAX_PATH];
    BRDCHAR g_directory[MAX_PATH];
    int g_isPldLoadAlways;
    int g_OnlySetParams; // 1 - только установка параметров АЦП
    ULONG g_Cycle;
    ULONG g_Pause;
    ULONG g_DmaOn;
    unsigned long long g_samplesOfChannel;
    unsigned long long g_memorySamplesOfChannel;
    unsigned long long g_bBufSize;
    unsigned long long g_bMemBufSize;
    int g_DirWriteFile;
    ULONG g_FileBufSize;
    ULONG g_adjust_mode;
    int g_transRate;
    int g_PretrigMode;
    // long long  g_nPreTrigSamples = 16;
    long long g_nPostTrigSamples;
    void* g_pMapBuf;
    ULONG g_bBlkNum;
    ULONG g_FileBlkNum;
    ULONG g_MemOn;
    ULONG g_IsWriteFile;
    ULONG g_IsSysMem;
    ULONG g_times;
    ULONG DspMode;
    ULONG g_SwitchOutMask; // маска включенных выходов коммутатора для FMC-модулей
    BRDCHAR g_dirFileName[MAX_PATH];
    ULONG g_quick_quit;
    long long g_bPostTrigSize;
    long long g_bStartEvent;
    long long g_bRealSize;
    ULONG g_isPassMemEnd;
    int g_numChan;
    BRDCHAR g_iniFileNameAdc[FILENAME_MAX];
    BRD_Info g_info;
    int g_regdbg;
    int g_StopFlag;
    int g_flbreak_cont;
    int g_flbreak_adc;
    ULONG g_BlkSize;
    ULONG g_BlkNum;
    void* g_pBufFileMap;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
    IPC_handle g_hBufFileMap_cont;
    IPC_handle g_hFlgFileMap_cont;
    IPC_handle* g_hBufFileMap = NULL;
    IPC_handle g_hFlgFileMap;
    IPC_handle g_hPostfixFileMap = NULL;
#else
    HANDLE g_hBufFileMap_cont;
    HANDLE g_hFlgFileMap_cont;
    HANDLE* g_hBufFileMap = NULL;
    HANDLE g_hFlgFileMap;
    HANDLE g_hPostfixFileMap = NULL;
#endif

    ULONG g_bufType;
    ULONG g_fileBufNum;
    ULONG g_fileBlkNum;
    ULONG g_fileBufSize;
#ifdef _WIN32
    HANDLE g_hThread;
    HANDLE g_hUserEvent;
#endif
};

/////////////////////////////////////// Service
typedef struct FU_TriadSrv {
    FU_TriadSrv()
        : handle_(0)
        , ok(false)
        , mode_(BRDcapt_EXCLUSIVE)
    {
    }
    BRD_Handle handle()
    {
        lastError = "";
        if (ok)
            return handle_;
        lastError = "<ERR> Bad handle ...";
        return 0;
    }

    BRD_Handle capture(BRD_Handle devHndl, char* sname, int delay)
    {
        // printf("<DEBUG:capture()> Service %s captured! Handle = %X \n", sname, handle_);
        lastError = "";
        if (ok) {
            if (!BRDC_stricmp(sname, servName.name))
                return handle_;
        }
        U32 modeOpened = mode_;
        handle_ = BRD_capture(devHndl, 0, &modeOpened, sname, delay);
        if (handle_ > 0) {
            ok = true;
            strcpy(servName.name, sname);
            // printf("<DEBUG> Service %s captured! Handle = %X \n", sname, handle_);
            if (modeOpened != mode_) {
                release();
                lastError = "service " + std::string(sname) + "captured in different mode! Released!";
            }
            return handle_;
        }
        lastError = "<ERR> service (name=" + std::string(sname) + ") not captured!";
        ok = false;
        strcpy(servName.name, "");
        return 0;
    }

    bool release()
    {
        // printf("<DEBUG:release()> Service %s captured! Handle = %X \n", servName.name, handle_);
        lastError = "";
        if (ok) {
            BRD_release(handle_, 0);
            ok = false;
            strcpy(servName.name, "");
        }
        return true;
    }

    std::string name() { return servName.name; };

    U32 mode() { return mode_; }

    void setMode(U32 md) { mode_ = md; };

    std::string error()
    {
        return lastError;
    }

    bool isCaptured() { return ok; }

    std::string strMode()
    {
        std::string retVal = std::string("Service ") + servName.name + " captured in mode - ";
        switch (mode_) {
        case BRDcapt_SHARED:
            retVal += "SHARED";
            break;
        case BRDcapt_EXCLUSIVE:
            retVal += "EXCLUSIVE";
            break;
        case BRDcapt_SPY:
            retVal += "SPY";
            break;
        default:
            retVal += "UNDEFINE";
        }
        retVal += "\n";
        return retVal;
    }

protected:
    BRD_Handle handle_;
    bool ok;
    U32 mode_;
    std::string lastError;
    BRD_ServList servName;
} TRIadSrv;

//////////////////////////////////// Device
typedef struct FU_TriadDev {

    FU_TriadDev()
        : mode_(BRDopen_SHARED)
        , ok(false)
        , handle_(0) { };

    BRD_Handle handle()
    {
        lastError = "";
        if (ok)
            return handle_;
        lastError = "Bad handle device ...";
        return 0;
    }
    BRD_Handle open(int lid)
    {
        lastError = "";
        if (lid < 0 || lid >= MAX_LID_DEVICES) {
            lastError = "Bad LID number!";
            return 0;
        }
        int modeOpened = 0;
        handle_ = BRD_open(lid, mode_, &modeOpened);
        ok = (handle_ > 0);
        if (!ok)
            lastError = "device lid#" + std::to_string(lid) + " not opened!";
        else if (mode_ != modeOpened) {
            close();
            lastError = "device lid#" + std::to_string(lid) + "opened in different mode! Closed!";
        }
        return handle_;
    }

    BRD_Handle reopen(int lid)
    {
        lastError = "";
        if (lid <= 0 || lid >= MAX_LID_DEVICES) {
            lastError = "Bad LID number!";
            return 0;
        }
        if (ok)
            close();
        return open(lid);
    }

    bool close()
    {
        U32 e = BRD_close(handle_);
        handle_ = 0;
        ok = false;
        return (e != 0);
    }

    U32 mode() { return mode_; }

    void setMode(int md) { mode_ = md; };

    bool isOpen() { return ok; }

    std::string strMode()
    {
        std::string retVal;
        switch (mode_) {
        case BRDopen_SHARED:
            retVal += "SHARED";
            break;
        case BRDopen_EXCLUSIVE:
            retVal += "EXCLUSIVE";
            break;
        case BRDopen_SPY:
            retVal += "SPY";
            break;
        case BRDopen_HIGHEST:
            retVal += "HIGHEST";
            break;
        default:
            retVal += "UNDEFINE";
        }
        return retVal;
    }

    std::string error() { return lastError; }

protected:
    BRD_Handle handle_;
    bool ok;
    int mode_;
    std::string lastError;

} TRIadsDevs;

typedef struct FU_Threads {
    std::thread* thread;
    std::atomic<bool> stop;
    std::atomic<bool> isStoped;
} FuThread;

typedef struct DacDevice {
    DacDevice()
        : outText(true)
    {
    }
    TRIadSrv servDac;
    TDacParam paramDac;
    U32 dacMode;
    bool outText;
    BRD_Handle getService()
    {
        if (!servDac.isCaptured())
            return 0;
        return (paramDac.handle = servDac.handle());
    }
};

typedef struct FU_Devices {
    FU_Devices() { }
    TRIadsDevs device;
    // ADC
    ParamsAdc paramsAdc;
    TRIadSrv adc;
    FuThread adcCtrlThr;
    // DAC
    ParamsDACs paramsDac;
    std::vector<DacDevice> dac;
    FuThread dacCtrlThr;
    // REGs
    TRIadSrv servRegs;
    //

} FuDevs;

extern FuDevs DevicesLid[MAX_LID_DEVICES]; // допускаем до 10 лид идентификаторов
extern int x_lid;
extern BRD_Handle x_handleDevice;

static int version_srv_hi = 3;
static int version_srv_lo = 942;

S32 GetInifileString(const BRDCHAR* sSectionName, const BRDCHAR* sParamName, const BRDCHAR* defValue,
    BRDCHAR* strValue, int strSize, const BRDCHAR* sFullName);
S32 GetInifileInt(const BRDCHAR* sSectionName, const BRDCHAR* sParamName, const BRDCHAR* defValue,
    int& value, const BRDCHAR* sFullName);
S32 GetInifileBig(const BRDCHAR* sSectionName, const BRDCHAR* sParamName, const BRDCHAR* defValue,
    long long& value, const BRDCHAR* sFullName);
S32 GetInifileFloat(const BRDCHAR* sSectionName, const BRDCHAR* sParamName, const BRDCHAR* defValue,
    double& value, const BRDCHAR* sFullName);