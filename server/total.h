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
        , mode_(BRDcapt_SHARED)
    {
    }
    BRD_Handle handle()
    {
        lastError = "";
        printf("<DEBUG:handle()> Service %s captured! Handle = %X \n", servName.name, handle_);
        if (ok)
            return handle_;
        lastError = "<ERR> Bad handle ...";
        return 0;
    }

    BRD_Handle capture(BRD_Handle devHndl, char* sname, int delay)
    {
        printf("<DEBUG:capture()> Service %s captured! Handle = %X \n", sname, handle_);
        lastError = "";
        if (ok) {
            if (!BRDC_stricmp(sname, servName.name))
                return handle_;
        }

        handle_ = BRD_capture(devHndl, 0, &mode_, sname, delay);
        if (handle_ > 0) {
            ok = true;
            strcpy(servName.name, sname);
            printf("<DEBUG> Service %s captured! Handle = %X \n", sname, handle_);
            return handle_;
        }
        lastError = "<ERR> service (name=" + std::string(sname) + ") not captured!";
        printf("<DEBUG> %s\n", lastError.c_str());
        ok = false;
        strcpy(servName.name, "");
        return 0;
    }

    bool release()
    {
        printf("<DEBUG:release()> Service %s captured! Handle = %X \n", servName.name, handle_);
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

    bool isOpen() { return ok; }

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
        , curLid_(0)
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
        }
        if (ok) {
            if (lid != curLid_)
                close();
            else
                return handle_;
        }
        handle_ = BRD_open(lid, mode_, &mode_);
        ok = (handle_ > 0);
        if (!ok)
            lastError = "<ERR> device lid#" + std::to_string(lid) + " not opened!";
        curLid_ = ok ? lid : 0;
        return handle_;
    }

    BRD_Handle reopen(int lid)
    {
        lastError = "";
        if (lid <= 0 || lid >= MAX_LID_DEVICES) {
            lastError = "<ERR> Bad LID number!";
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
        curLid_ = 0;
        return (e != 0);
    }

    int curLid() { return curLid_; }

    int mode() { return mode_; }

    void setMode(int md) { mode_ = md; };

    bool isOpen() { return ok; }

    std::string error() { return lastError; }

protected:
    BRD_Handle handle_;
    bool ok;
    int mode_;
    int curLid_;
    std::string lastError;

} TRIadsDevs;

typedef struct FU_Threads {
    std::thread* thread;
    std::atomic<bool> stop;
    std::atomic<bool> isStoped;
} FuThread;

typedef struct FU_Devices {
    TRIadsDevs device;
    //
    TRIadSrv adc;
    FuThread adcCtrlThr;
    // DAC
    std::vector<TDacParam> dac;
    FuThread dacCtrlThr;
    // REGs
    TRIadSrv servRegs;

} FuDevs;

extern FuDevs DevicesLid[MAX_LID_DEVICES]; // допускаем до 10 лид идентификаторов
extern int x_lid;
extern BRD_Handle x_handleDevice;

static int version_srv_hi = 3;
static int version_srv_lo = 0;