#pragma once

#include "brdapi.h"
#include "exam_edac.h"
#include "gipcy.h"
#include <atomic>
#include <thread>
#include <vector>

// All total definition

extern S32 x_DevNum; // кол-во у-в
#define MAX_LID_DEVICES 10
#define LID_DEFAULT 1

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

        handle_ = BRD_capture(devHndl, 0, &mode_, sname, delay);
        if (handle_ > 0) {
            ok = true;
            strcpy(servName.name, sname);
            // printf("<DEBUG> Service %s captured! Handle = %X \n", sname, handle_);
            return handle_;
        }
        lastError = "<ERR> service (name=" + std::string(sname) + ") not captured!";
        // printf("<DEBUG> %s\n", lastError.c_str());
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
        lastError = "<ERR> Bad handle device ...";
        return 0;
    }
    BRD_Handle open(int lid)
    {
        lastError = "";
        if (lid < 0 || lid >= MAX_LID_DEVICES) {
            lastError = "<ERR> Bad LID number!";
            return 0;
        }
        handle_ = BRD_open(lid, mode_, &mode_);
        ok = (handle_ > 0);
        if (!ok)
            lastError = "<ERR> device lid#" + std::to_string(lid) + " not opened!";
        return handle_;
    }

    BRD_Handle reopen(int lid)
    {
        lastError = "";
        if (lid <= 0 || lid >= MAX_LID_DEVICES) {
            lastError = "<ERR> Bad LID number!";
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
    TRIadSrv servDac;
    TDacParam paramDac;
    U32 dacMode;
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
    //
    TRIadSrv adc;
    FuThread adcCtrlThr;
    // DAC
    std::vector<DacDevice> dac;
    FuThread dacCtrlThr;
    // REGs
    TRIadSrv servRegs;

} FuDevs;

extern FuDevs DevicesLid[MAX_LID_DEVICES]; // допускаем до 10 лид идентификаторов
extern int x_lid;
extern BRD_Handle x_handleDevice;

static int version_srv_hi = 3;
static int version_srv_lo = 89;