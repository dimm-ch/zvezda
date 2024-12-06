///
/// \file sync_ctrl.cpp
/// \author Alexander Chernenko (chernenko.a@insys.ru)
/// \brief
/// \version 0.1
/// \date 13.01.2020
///
/// \copyright InSys Copyright (c) 2020
///
///

#include "sync_srv.h"

static CMD_Info GETCFG_CMD { BRDctrl_SYNC_GETCFG, 1, 0, 0, NULL };
static CMD_Info SETCLKMODE_CMD { BRDctrl_SYNC_SETCLKMODE, 0, 0, 0, NULL };
static CMD_Info GETCLKMODE_CMD { BRDctrl_SYNC_GETCLKMODE, 1, 0, 0, NULL };
static CMD_Info SETOUTCLK_CMD { BRDctrl_SYNC_SETOUTCLK, 0, 0, 0, NULL };
static CMD_Info GETOUTCLK_CMD { BRDctrl_SYNC_GETOUTCLK, 1, 0, 0, NULL };
static CMD_Info SETSTBMODE_CMD { BRDctrl_SYNC_SETSTBMODE, 0, 0, 0, NULL };
static CMD_Info GETSTBMODE_CMD { BRDctrl_SYNC_GETSTBMODE, 1, 0, 0, NULL };
static CMD_Info SETINVCLKPHS_CMD { BRDctrl_SYNC_SETINVCLKPHS, 0, 0, 0, NULL };
static CMD_Info GETINVCLKPHS_CMD { BRDctrl_SYNC_GETINVCLKPHS, 1, 0, 0, NULL };
static CMD_Info SETOUTSTB_CMD { BRDctrl_SYNC_SETOUTSTB, 0, 0, 0, NULL };
static CMD_Info GETOUTSTB_CMD { BRDctrl_SYNC_GETOUTSTB, 1, 0, 0, NULL };
static CMD_Info SETDELOUTSTB_CMD { BRDctrl_SYNC_SETDELOUTSTB, 0, 0, 0, NULL };
static CMD_Info GETDELOUTSTB_CMD { BRDctrl_SYNC_GETDELOUTSTB, 1, 0, 0, NULL };
static CMD_Info SETSTARTMODE_CMD { BRDctrl_SYNC_SETSTARTMODE, 0, 0, 0, NULL };
static CMD_Info GETSTARTMODE_CMD { BRDctrl_SYNC_GETSTARTMODE, 1, 0, 0, NULL };
static CMD_Info STARTSTROBE_CMD { BRDctrl_SYNC_STARTSTROBE, 0, 0, 0, NULL };
static CMD_Info STOPSTROBE_CMD { BRDctrl_SYNC_STOPSTROBE, 0, 0, 0, NULL };
static CMD_Info STARTCLOCK_CMD { BRDctrl_SYNC_STARTCLOCK, 0, 0, 0, NULL };
static CMD_Info STOPCLOCK_CMD { BRDctrl_SYNC_STOPCLOCK, 0, 0, 0, NULL };
static CMD_Info GETTEMP_CMD { BRDctrl_SYNC_GETTEMP, 1, 0, 0, NULL };
static CMD_Info GETSYSMON_CMD { BRDctrl_SYNC_GETSYSMON, 1, 0, 0, NULL };
static CMD_Info GETSTBRES_CMD { BRDctrl_SYNC_GETSTBRES, 1, 0, 0, NULL };
static CMD_Info ADJCXO_CMD { BRDctrl_SYNC_ADJCXO, 0, 0, 0, NULL };

CSyncSrv::CSyncSrv(int idx, const BRDCHAR* name, int num, PVOID pModule,
    PVOID pCfg, uint32_t cfgSize)
    : CService(idx, name, num, pModule, pCfg, cfgSize)
{
    Init(&GETCFG_CMD, (CmdEntry)&CSyncSrv::ctrlGetCfg);
    Init(&SETCLKMODE_CMD, (CmdEntry)&CSyncSrv::ctrlClkMode);
    Init(&GETCLKMODE_CMD, (CmdEntry)&CSyncSrv::ctrlClkMode);
    Init(&SETOUTCLK_CMD, (CmdEntry)&CSyncSrv::ctrlOutClk);
    Init(&GETOUTCLK_CMD, (CmdEntry)&CSyncSrv::ctrlOutClk);
    Init(&SETSTBMODE_CMD, (CmdEntry)&CSyncSrv::ctrlStbMode);
    Init(&GETSTBMODE_CMD, (CmdEntry)&CSyncSrv::ctrlStbMode);
    Init(&SETINVCLKPHS_CMD, (CmdEntry)&CSyncSrv::ctrlInvClkPhs);
    Init(&GETINVCLKPHS_CMD, (CmdEntry)&CSyncSrv::ctrlInvClkPhs);
    Init(&SETOUTSTB_CMD, (CmdEntry)&CSyncSrv::ctrlOutStb);
    Init(&GETOUTSTB_CMD, (CmdEntry)&CSyncSrv::ctrlOutStb);
    Init(&SETDELOUTSTB_CMD, (CmdEntry)&CSyncSrv::ctrlDelOutStb);
    Init(&GETDELOUTSTB_CMD, (CmdEntry)&CSyncSrv::ctrlDelOutStb);
    Init(&SETSTARTMODE_CMD, (CmdEntry)&CSyncSrv::ctrlStartMode);
    Init(&GETSTARTMODE_CMD, (CmdEntry)&CSyncSrv::ctrlStartMode);
    Init(&STARTSTROBE_CMD, (CmdEntry)&CSyncSrv::ctrlStartStopStb);
    Init(&STOPSTROBE_CMD, (CmdEntry)&CSyncSrv::ctrlStartStopStb);
    Init(&STARTCLOCK_CMD, (CmdEntry)&CSyncSrv::ctrlStartStopClk);
    Init(&STOPCLOCK_CMD, (CmdEntry)&CSyncSrv::ctrlStartStopClk);
    Init(&GETTEMP_CMD, (CmdEntry)&CSyncSrv::ctrlGetTemp);
    Init(&GETSYSMON_CMD, (CmdEntry)&CSyncSrv::ctrlGetSysMon);
    Init(&GETSTBRES_CMD, (CmdEntry)&CSyncSrv::ctrlGetStbRes);
    Init(&ADJCXO_CMD, (CmdEntry)&CSyncSrv::ctrlAdjCxo);
}

int CSyncSrv::CtrlCapture(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    int status = BRDerr_ERROR;
    if (m_isAvailable) {
        status = capture();
    };
    return status;
}

int CSyncSrv::CtrlIsAvailable(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    int status = BRDerr_ERROR;
    auto& module = ModuleCast<CModule>(pDev);
    auto pServAvailable = (PSERV_CMD_IsAvailable)args;
    pServAvailable->attr = m_attribute;
    m_MainTetrNum = GetTetrNum(&module, m_index, MAIN_TETR_ID);
    m_SyncTetrNum = GetTetrNum(&module, m_index, SYNC_TETR_ID);
    m_isAvailable = 0;
    if (m_MainTetrNum != -1 && m_SyncTetrNum != -1) {
        DEVS_CMD_Reg param;
        param.idxMain = int32_t(m_index);
        param.tetr = m_SyncTetrNum;
        param.reg = ADM2IFnr_STATUS;
        param.val = 0;
        module.RegCtrl(DEVScmd_REGREADDIR, &param);
        const auto ready_bit { uint32_t { 0x02u } };
        if ((param.val & ready_bit) == ready_bit) {
            param.reg = ADM2IFnr_VER;
            param.val = 0;
            module.RegCtrl(DEVScmd_REGREADIND, &param);
            instance(param.val);
            if (m_Service) {
                m_isAvailable = 1;
                status = BRDerr_OK;
            }
        }
    }
    pServAvailable->isAvailable = m_isAvailable;
    return status;
}

int CSyncSrv::CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    return release();
}

int CSyncSrv::ctrlGetCfg(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    int status = BRDerr_ERROR;
    auto& cfg = ArgumentCast<BRD_SyncCfg>(args);
    return getCfg(cfg);
}

int CSyncSrv::ctrlClkMode(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    int status = BRDerr_ERROR;
    auto& clkMode = ArgumentCast<BRD_SyncClkMode>(args);
    if (cmd == BRDctrl_SYNC_SETCLKMODE) {
        status = setClkMode(clkMode);
    } else if (cmd == BRDctrl_SYNC_GETCLKMODE) {
        status = getClkMode(clkMode);
    }
    return status;
}

int CSyncSrv::ctrlStbMode(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    int status = BRDerr_ERROR;
    auto& stbMode = ArgumentCast<BRD_SyncStbMode>(args);
    if (cmd == BRDctrl_SYNC_SETSTBMODE) {
        status = setStbMode(stbMode);
    } else if (cmd == BRDctrl_SYNC_GETSTBMODE) {
        status = getStbMode(stbMode);
    }
    return status;
}

int CSyncSrv::ctrlInvClkPhs(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    int status = BRDerr_ERROR;
    auto& invClkPhs = ArgumentCast<BRD_SyncInvClkPhs>(args);
    if (cmd == BRDctrl_SYNC_SETINVCLKPHS) {
        status = setInvClkPhs(invClkPhs);
    } else if (cmd == BRDctrl_SYNC_GETINVCLKPHS) {
        status = getInvClkPhs(invClkPhs);
    }
    return status;
}

int CSyncSrv::ctrlStartMode(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    int status = BRDerr_ERROR;
    auto& startMode = ArgumentCast<BRD_SyncStartMode>(args);
    if (cmd == BRDctrl_SYNC_SETSTARTMODE) {
        status = setStartMode(startMode);
    } else if (cmd == BRDctrl_SYNC_GETSTARTMODE) {
        status = getStartMode(startMode);
    }
    return status;
}

int CSyncSrv::ctrlOutClk(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    int status = BRDerr_ERROR;
    auto& outClk = ArgumentCast<BRD_SyncOutClk>(args);
    if (cmd == BRDctrl_SYNC_SETOUTCLK) {
        status = setOutClk(outClk);
    } else if (cmd == BRDctrl_SYNC_GETOUTCLK) {
        status = getOutClk(outClk);
    }
    return status;
}

int CSyncSrv::ctrlOutStb(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    int status = BRDerr_ERROR;
    auto& outStb = ArgumentCast<BRD_SyncOutStb>(args);
    if (cmd == BRDctrl_SYNC_SETOUTSTB) {
        status = setOutStb(outStb);
    } else if (cmd == BRDctrl_SYNC_GETOUTSTB) {
        status = getOutStb(outStb);
    }
    return status;
}

int CSyncSrv::ctrlDelOutStb(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    int status = BRDerr_ERROR;
    auto& delOutStb = ArgumentCast<BRD_SyncDelOutStb>(args);
    if (cmd == BRDctrl_SYNC_SETDELOUTSTB) {
        status = setDelOutStb(delOutStb);
    } else if (cmd == BRDctrl_SYNC_GETDELOUTSTB) {
        status = getDelOutStb(delOutStb);
    }
    return status;
}

int CSyncSrv::ctrlStartStopClk(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    int status = BRDerr_ERROR;
    if (cmd == BRDctrl_SYNC_STARTCLOCK) {
        status = startClk();
    } else if (cmd == BRDctrl_SYNC_STOPCLOCK) {
        status = stopClk();
    }
    return status;
}

int CSyncSrv::ctrlStartStopStb(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    int status = BRDerr_ERROR;
    if (cmd == BRDctrl_SYNC_STARTSTROBE) {
        status = startStb();
    } else if (cmd == BRDctrl_SYNC_STOPSTROBE) {
        status = stopStb();
    }
    return status;
}

int CSyncSrv::ctrlGetTemp(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    auto& temp = ArgumentCast<double>(args);
    return getTemp(temp);
}

int CSyncSrv::ctrlGetSysMon(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    int status = BRDerr_ERROR;
    auto& sysmon = ArgumentCast<BRD_SyncSysMon>(args);
    return getSysMon(sysmon);
}

int CSyncSrv::ctrlGetStbRes(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    auto& resolution = ArgumentCast<double>(args);
    return getStbRes(resolution);
}

int CSyncSrv::ctrlAdjCxo(void* pDev, void* pServData, ULONG cmd, void* args) noexcept
{
    auto& adjust = ArgumentCast<uint16_t>(args);
    return adjCxo(adjust);
}
