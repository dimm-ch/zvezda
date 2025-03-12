#pragma once

#include "brdapi.h"
#include "exam_edac.h"
#include "gipcy.h"
#include <atomic>
#include <thread>

// All total definition

extern S32 x_DevNum; // кол-во у-в

typedef struct FU_Triad {
    FU_Triad()
        : handle_(nullptr)
        , ok(false)
        , mode_(BRDopen_SHARED)
    {
    }
    BRD_Handle handle()
    {
        lastError = "";
        if (ok)
            return handle_;
        lastError = "<ERR> Bad handle ...";
        return NULL;
    }

    void setHandle(BRD_Handle hndl)
    {
    }

    int mode() { return mode_; }

    void setMode(int md) { mode_ = md };

    std::string error()
    {
        return lastError;
    }

    bool isOpen() { return ok; }

protected:
    BRD_Handle handle_;
    bool ok;
    int mode_;
    std::string lastError;
} TRIads;

typedef struct FU_TriadDev : public FU_Triad {

    FU_Triad()
        : mode_(BRDopen_SHARED) { };

    BRD_Handle handle()
    {
        lastError = "";
        if (ok)
            return handle_;
        lastError = "<ERR> Bad handle device ...";
        return NULL;
    }
    BRD_Handle open(int lid)
    {
        lastError = "";
        if (lid < 0 || lid >= MAX_LID_DEVICES) {
            lastError = "<ERR> Bad LID number!";
        }
        if (ok)
            close();
        handle_ = BRD_open(x_lid, mode_, &mode_);
        ok = (handle_ > 0);
        if (!ok)
            lastError = "<ERR> device lid#" + std::to_string(lid) + " not opened!";
        return handle_;
    }
    bool close()
    {
        return (BRD_close(handle_) != 0);
    }

} TRIadsDevs;

typedef struct FU_Threads {
    std::thread* thread;
    std::atomic<bool> stop;
    std::atomic<bool> isStoped;
} FuThread;

typedef struct FU_Devices {
    TRIadsDevs device;
    //
    TRIads adc;
    FuThread adcCtrlThr;
    // DAC
    vector<TDacParam> dac;
    FuThread dacCtrlThr;
    // REGs
    TRIads servRegs;

} FuDevs;

#define MAX_LID_DEVICES 10
#define LID_DEFAULT 1
extern FuDevs DevicesLid[10]; // допускаем до 10 лид идентификаторов

// extern BRD_Handle x_hADC; // сервис АЦП
// extern BRD_Handle x_hDAC; // сервис ЦAП
// extern BRD_Handle x_hREG;
extern int x_lid;
extern BRD_Handle x_handleDevice;
// extern int x_mode;

static int version_srv_hi = 1;
static int version_srv_lo = 2;