///
/// \file sync_srv.h
/// \author Alexander Chernenko (chernenko.a@insys.ru)
/// \brief
/// \version 0.1
/// \date 13.01.2020
///
/// \copyright InSys Copyright (c) 2020
///
///

#pragma once

#include "adm2if.h"
#include "service.h"
#include "submodule.h"

#include "ctrl_sync.h"

#define CTRL_ADD_HELPER(ctrl_name)                      \
    int ctrl_name(void* pDev, /* InfoSM or InfoBM */    \
        void* pServData, /* Specific Service Data */    \
        ULONG cmd, /* Command Code (from BRD_ctrl()) */ \
        void* args) noexcept /* Command Arguments (from BRD_ctrl()) */

class CSyncVer final {
    const int _major {};
    const int _minor {};

public:
    constexpr CSyncVer() = default;
    constexpr CSyncVer(int major, int minor)
        : _major { major }
        , _minor { minor }
    {
    }
    constexpr int version() const noexcept { return (_major << 8 | _minor); }
    ~CSyncVer() = default;
    constexpr int major() const noexcept { return _major; };
    constexpr int minor() const noexcept { return _minor; };
};

struct ISyncSrv {
    using UniquePtr = std::unique_ptr<ISyncSrv>;

    virtual int capture() noexcept = 0;
    virtual int release() noexcept = 0;
    virtual CSyncVer version() const noexcept = 0;

    virtual int getCfg(BRD_SyncCfg&) noexcept = 0;
    virtual int setClkMode(const BRD_SyncClkMode&) noexcept = 0;
    virtual int getClkMode(BRD_SyncClkMode&) noexcept = 0;
    virtual int setStbMode(const BRD_SyncStbMode&) noexcept = 0;
    virtual int getStbMode(BRD_SyncStbMode&) noexcept = 0;
    virtual int setInvClkPhs(const BRD_SyncInvClkPhs&) noexcept = 0;
    virtual int getInvClkPhs(BRD_SyncInvClkPhs&) noexcept = 0;
    virtual int setStartMode(const BRD_SyncStartMode&) noexcept = 0;
    virtual int getStartMode(BRD_SyncStartMode&) noexcept = 0;
    virtual int setOutClk(const BRD_SyncOutClk&) noexcept = 0;
    virtual int getOutClk(BRD_SyncOutClk&) noexcept = 0;
    virtual int setOutStb(const BRD_SyncOutStb&) noexcept = 0;
    virtual int getOutStb(BRD_SyncOutStb&) noexcept = 0;
    virtual int setDelOutStb(const BRD_SyncDelOutStb&) noexcept = 0;
    virtual int getDelOutStb(BRD_SyncDelOutStb&) noexcept = 0;
    virtual int startClk() noexcept = 0;
    virtual int stopClk() noexcept = 0;
    virtual int startStb() noexcept = 0;
    virtual int stopStb() noexcept = 0;
    virtual int getTemp(double&) noexcept = 0;
    virtual int getSysMon(BRD_SyncSysMon&) noexcept = 0;
    virtual int getStbRes(double&) noexcept = 0;
    virtual int adjCxo(uint16_t) noexcept = 0;
    virtual ~ISyncSrv() noexcept = default;
};

/// Служба Sync
class CSyncSrv : public CService, ISyncSrv {
    CSyncVer version() const noexcept override { return {}; };

protected:
    static const uint32_t SYNC_TETR_ID { 0xFB };

    int32_t m_SyncTetrNum;
    int32_t m_MainTetrNum;

    typename ISyncSrv::UniquePtr m_Service {};

    virtual void instance(uint16_t) = 0;

public:
    CSyncSrv(int, const BRDCHAR*, int, PVOID, PVOID, uint32_t);
    virtual ~CSyncSrv() noexcept = default;

    CTRL_ADD_HELPER(CtrlCapture)
    override;
    CTRL_ADD_HELPER(CtrlIsAvailable)
    override;
    CTRL_ADD_HELPER(CtrlRelease)
    override;

    CTRL_ADD_HELPER(ctrlGetCfg);
    CTRL_ADD_HELPER(ctrlClkMode);
    CTRL_ADD_HELPER(ctrlStbMode);
    CTRL_ADD_HELPER(ctrlInvClkPhs);
    CTRL_ADD_HELPER(ctrlStartMode);
    CTRL_ADD_HELPER(ctrlOutClk);
    CTRL_ADD_HELPER(ctrlOutStb);
    CTRL_ADD_HELPER(ctrlDelOutStb);
    CTRL_ADD_HELPER(ctrlStartStopClk);
    CTRL_ADD_HELPER(ctrlStartStopStb);
    CTRL_ADD_HELPER(ctrlGetTemp);
    CTRL_ADD_HELPER(ctrlGetSysMon);
    CTRL_ADD_HELPER(ctrlGetStbRes);
    CTRL_ADD_HELPER(ctrlAdjCxo);
};
