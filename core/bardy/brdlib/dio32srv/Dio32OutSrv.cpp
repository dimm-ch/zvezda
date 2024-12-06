/*
 ***************** File Dio32OutSrv.cpp ***********************
 *
 * BRD Driver for DIO32OUT service
 *
 * (C) InSys by Dorokhin A. Sep 2007
 *
 ******************************************************
*/

//#include <windows.h>
//#include <winioctl.h>
//#include <tchar.h>

#include "module.h"
#include "Dio32OutSrv.h"

#define	CURRFILE "DIO32OUTSRV"

static CMD_Info SETCHANMASK_CMD		= { BRDctrl_DIO32OUT_SETCHANMASK, 0, 0, 0, NULL};
static CMD_Info GETCHANMASK_CMD		= { BRDctrl_DIO32OUT_GETCHANMASK, 1, 0, 0, NULL};
static CMD_Info SETRATE_CMD			= { BRDctrl_DIO32OUT_SETRATE,	  0, 0, 0, NULL};
static CMD_Info GETRATE_CMD			= { BRDctrl_DIO32OUT_GETRATE,	  1, 0, 0, NULL};
static CMD_Info SETCLKMODE_CMD		= { BRDctrl_DIO32OUT_SETCLKMODE,  0, 0, 0, NULL};
static CMD_Info GETCLKMODE_CMD		= { BRDctrl_DIO32OUT_GETCLKMODE,  1, 0, 0, NULL};
static CMD_Info SETMASTER_CMD		= { BRDctrl_DIO32OUT_SETMASTER,	  0, 0, 0, NULL};
static CMD_Info GETMASTER_CMD		= { BRDctrl_DIO32OUT_GETMASTER,	  1, 0, 0, NULL};
static CMD_Info SETSTARTMODE_CMD	= { BRDctrl_DIO32OUT_SETSTARTMODE,0, 0, 0, NULL};
static CMD_Info GETSTARTMODE_CMD	= { BRDctrl_DIO32OUT_GETSTARTMODE,1, 0, 0, NULL};

static CMD_Info SETPNPKMODE_CMD		= { BRDctrl_DIO32OUT_SETPNPKMODE, 0, 0, 0, NULL};
static CMD_Info GETPNPKMODE_CMD		= { BRDctrl_DIO32OUT_GETPNPKMODE, 1, 0, 0, NULL};
static CMD_Info SETINITMODE_CMD		= { BRDctrl_DIO32OUT_SETINITMODE, 0, 0, 0, NULL};
static CMD_Info GETINITMODE_CMD		= { BRDctrl_DIO32OUT_GETINITMODE, 1, 0, 0, NULL};

static CMD_Info FIFORESET_CMD		= { BRDctrl_DIO32OUT_FIFORESET,	  0, 0, 0, NULL};
static CMD_Info ENABLE_CMD			= { BRDctrl_DIO32OUT_ENABLE,	  0, 0, 0, NULL};
static CMD_Info FIFOSTATUS_CMD		= { BRDctrl_DIO32OUT_FIFOSTATUS,  1, 0, 0, NULL};
static CMD_Info PUTDATA_CMD			= { BRDctrl_DIO32OUT_PUTDATA,	  0, 0, 0, NULL};
static CMD_Info GETSRCSTREAM_CMD	= { BRDctrl_DIO32OUT_GETSRCSTREAM, 1, 0, 0, NULL};
static CMD_Info FLAGCLR_CMD			= { BRDctrl_DIO32OUT_FLAGCLR,	  0, 0, 0, NULL};

//***************************************************************************************
CDio32OutSrv::CDio32OutSrv(int idx, int srv_num, CModule* pModule, PDIO32OUTSRV_CFG pCfg) :
	CService(idx, _BRDC("DIO32OUT"), srv_num, pModule, pCfg, sizeof(DIO32OUTSRV_CFG))
{
	m_attribute = 
			BRDserv_ATTR_DIRECTION_OUT |
			BRDserv_ATTR_STREAMABLE_OUT |
			BRDserv_ATTR_CMPABLE |
			BRDserv_ATTR_EXCLUSIVE_ONLY;

	Init(&SETCHANMASK_CMD, (CmdEntry)&CDio32OutSrv::CtrlChanMask);
	Init(&GETCHANMASK_CMD, (CmdEntry)&CDio32OutSrv::CtrlChanMask);
	Init(&SETRATE_CMD, (CmdEntry)&CDio32OutSrv::CtrlRate);
	Init(&GETRATE_CMD, (CmdEntry)&CDio32OutSrv::CtrlRate);
	Init(&SETCLKMODE_CMD, (CmdEntry)&CDio32OutSrv::CtrlClkMode);
	Init(&GETCLKMODE_CMD, (CmdEntry)&CDio32OutSrv::CtrlClkMode);
	Init(&SETMASTER_CMD, (CmdEntry)&CDio32OutSrv::CtrlMaster);
	Init(&GETMASTER_CMD, (CmdEntry)&CDio32OutSrv::CtrlMaster);
	Init(&SETSTARTMODE_CMD, (CmdEntry)&CDio32OutSrv::CtrlStartMode);
	Init(&GETSTARTMODE_CMD, (CmdEntry)&CDio32OutSrv::CtrlStartMode);

	Init(&SETPNPKMODE_CMD, (CmdEntry)&CDio32OutSrv::CtrlPnpkMode);
	Init(&GETPNPKMODE_CMD, (CmdEntry)&CDio32OutSrv::CtrlPnpkMode);
	Init(&SETINITMODE_CMD, (CmdEntry)&CDio32OutSrv::CtrlInitMode);
	Init(&GETINITMODE_CMD, (CmdEntry)&CDio32OutSrv::CtrlInitMode);

	Init(&FIFORESET_CMD, (CmdEntry)&CDio32OutSrv::CtrlFifoReset);
	Init(&ENABLE_CMD, (CmdEntry)&CDio32OutSrv::CtrlEnable);
	Init(&FIFOSTATUS_CMD, (CmdEntry)&CDio32OutSrv::CtrlFifoStatus);
	Init(&PUTDATA_CMD, (CmdEntry)&CDio32OutSrv::CtrlPutData);
	Init(&GETSRCSTREAM_CMD, (CmdEntry)&CDio32OutSrv::CtrlGetAddrData);
	Init(&FLAGCLR_CMD, (CmdEntry)&CDio32OutSrv::CtrlFlagClear);

}

//***************************************************************************************
int CDio32OutSrv::CtrlIsAvailable(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	PSERV_CMD_IsAvailable pServAvailable = (PSERV_CMD_IsAvailable)args;
	pServAvailable->attr = m_attribute;
	m_MainTetrNum = GetTetrNum(pModule, m_index, MAIN_TETR_ID);
	m_Dio32OutTetrNum = GetTetrNum(pModule, m_index, DIO32OUT_TETR_ID);
	if(m_MainTetrNum != -1 && m_Dio32OutTetrNum != -1)
	{
		m_isAvailable = 1;
		PDIO32OUTSRV_CFG pSrvCfg = (PDIO32OUTSRV_CFG)m_pConfig;
		if(!pSrvCfg->isAlreadyInit)
		{
			pSrvCfg->isAlreadyInit = 1;
			DEVS_CMD_Reg RegParam;
			RegParam.idxMain = m_index;
			RegParam.tetr = m_Dio32OutTetrNum;
			if(!pSrvCfg->FifoSize)
			{
				RegParam.reg = ADM2IFnr_FTYPE;
				pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
				int widthFifo = RegParam.val >> 3;
				RegParam.reg = ADM2IFnr_FSIZE;
				pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
				pSrvCfg->FifoSize = RegParam.val * widthFifo;
			}
			RegParam.tetr = m_Dio32OutTetrNum;
			RegParam.reg = ADM2IFnr_MODE0;
			RegParam.val = 0;
			PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&RegParam.val;
			pMode0->ByBits.Reset = 1;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			for(int i = 1; i < 32; i++)
			{
				RegParam.reg = i;
				RegParam.val = 0;
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			}
			RegParam.reg = ADM2IFnr_MODE0;
			pMode0->ByBits.Reset = 0;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

		}
	}
	else
		m_isAvailable = 0;
	pServAvailable->isAvailable = m_isAvailable;

	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::CtrlGetAddrData(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	ULONG m_AdmNum;
	BRDCHAR buf[SERVNAME_SIZE];
	BRDC_strcpy(buf, m_name);
	BRDCHAR* pBuf = buf + (BRDC_strlen(buf) - 2);
	if(BRDC_strchr(pBuf, '1'))
		m_AdmNum = 1;
	else
		m_AdmNum = 0;
	*(ULONG*)args = (m_AdmNum << 16) | m_Dio32OutTetrNum;
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::CtrlChanMask(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDIO32OUT_MODE1 pMode1 = (PDIO32OUT_MODE1)&param.val;
	if(BRDctrl_DIO32OUT_SETCHANMASK == cmd)
	{
		pMode1->ByBits.ChanSel = *(PULONG)args;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		*(PULONG)args = pMode1->ByBits.ChanSel;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::GetClkSource(CModule* pModule, ULONG& ClkSrc)
{
	ULONG source;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_FMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		source = param.val;
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.reg = ADM2IFnr_FMODE;
			pModule->RegCtrl(DEVScmd_REGREADIND, &param);
			source = param.val;
		}
		else
		{ // Slave
			source = BRDclks_EXTSYNX;
		}
	}
	ClkSrc = source;
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::GetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue)
{
	PDIO32OUTSRV_CFG pSrvCfg = (PDIO32OUTSRV_CFG)m_pConfig;
	double Clk;
	switch(ClkSrc)
	{
	case BRDclks_SYSGEN:		// System Generator
		Clk = pSrvCfg->SysRefGen;
		break;
	case BRDclks_BASEGEN0:		// Generator 0
		Clk = pSrvCfg->BaseRefGen[0];
		break;
	case BRDclks_BASEGEN1:		// Generator 1
		Clk = pSrvCfg->BaseRefGen[1];
		break;
	case BRDclks_EXTSDX:	// External clock from SDX
	case BRDclks_EXTSYNX:	// External clock from SYNX
		Clk = pSrvCfg->BaseExtClk;
		break;
	//case BRDclks_SMCLK:		// 
	//case BRDclks_INFREQ:		// 
	//case BRDclks_SMOUTFREQ:	//
	//	Clk = pSrvCfg->SubExtClk;
	//	break;
	default:
		Clk = 0.0;
		break;
	}
	ClkValue = Clk;
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::SetClkSource(CModule* pModule, ULONG ClkSrc)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_FMODE;
		param.val = ClkSrc;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.reg = ADM2IFnr_FMODE;
			param.val = ClkSrc;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		}
		else
			return BRDerr_NOT_ACTION; // функция в режиме Slave не выполнима
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::SetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue)
{
	PDIO32OUTSRV_CFG pSrvCfg = (PDIO32OUTSRV_CFG)m_pConfig;
	switch(ClkSrc)
	{
	case BRDclks_SYSGEN:		// System Generator
		ClkValue = pSrvCfg->SysRefGen;
		break;
	case BRDclks_BASEGEN0:		// Generator 0
		ClkValue = pSrvCfg->BaseRefGen[0];
		break;
	case BRDclks_BASEGEN1:		// Generator 1
		ClkValue = pSrvCfg->BaseRefGen[1];
		break;
	case BRDclks_EXTSDX:	// External clock from SDX
	case BRDclks_EXTSYNX:	// External clock from SYNX
		pSrvCfg->BaseExtClk = ROUND(ClkValue);
		break;
//	case BRDclks_SMCLK:		// 
//	case BRDclks_INFREQ:		// Sinchro with input
//	case BRDclks_SMOUTFREQ:	// Sinchro with output
//		pSrvCfg->SubExtClk = ROUND(ClkValue);
//		break;
	default:
//		ClkValue = 0.0;
		break;
	}
	return BRDerr_OK;
}
	
//***************************************************************************************
int CDio32OutSrv::SetStartMode(CModule* pModule, PVOID pStMode)
{
	PBRD_StartMode pStartMode = (PBRD_StartMode)pStMode;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_STMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
		pStMode->ByBits.SrcStart  = pStartMode->startSrc;
		pStMode->ByBits.SrcStop   = pStartMode->trigStopSrc;
		pStMode->ByBits.InvStart  = pStartMode->startInv;
		pStMode->ByBits.InvStop   = pStartMode->stopInv;
		pStMode->ByBits.TrigStart = pStartMode->trigOn;
		pStMode->ByBits.Restart   = pStartMode->reStartMode;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		ULONG master = pMode0->ByBits.Master;
		param.reg = ADM2IFnr_STMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
		pStMode->ByBits.SrcStart = pStartMode->startSrc;
		if(!master)
			pStMode->ByBits.SrcStart = BRDsts_EXTSYNX; // SLAVE
		pStMode->ByBits.SrcStop   = pStartMode->trigStopSrc;
		pStMode->ByBits.InvStart  = pStartMode->startInv;
		pStMode->ByBits.InvStop   = pStartMode->stopInv;
		pStMode->ByBits.TrigStart = pStartMode->trigOn;
		pStMode->ByBits.Restart   = pStartMode->reStartMode;
		if(master && pStMode->ByBits.Restart == 1)
			pStMode->ByBits.SrcStop   = BRDsts_TRDADC;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

		param.tetr = m_Dio32OutTetrNum;
		pStMode->ByBits.SrcStart  = BRDsts_EXTSYNX; // SLAVE
		pStMode->ByBits.SrcStop   = BRDsts_PRG;
		pStMode->ByBits.InvStart  = 0;
		pStMode->ByBits.InvStop   = 0;
		pStMode->ByBits.TrigStart = 1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::GetStartMode(CModule* pModule, PVOID pStMode)
{
	PBRD_StartMode pStartMode = (PBRD_StartMode)pStMode;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_STMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
		pStartMode->startSrc	= pStMode->ByBits.SrcStart;
		pStartMode->trigStopSrc	= pStMode->ByBits.SrcStop;
		pStartMode->startInv	= pStMode->ByBits.InvStart;
		pStartMode->stopInv		= pStMode->ByBits.InvStop;
		pStartMode->trigOn		= pStMode->ByBits.TrigStart;
		pStartMode->reStartMode	= pStMode->ByBits.Restart;

		//param.reg = ADM2IFnr_PRTMODE;
		//pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		//PADM2IF_PRTMODE pPreMode = (PADM2IF_PRTMODE)&param.val;
		//pStartMode->pretrig	 = pPreMode->ByBits.Enable;

	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;//ADM2IF_ntMAIN;
//		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//		ULONG master = pMode0->ByBits.Master;
		param.reg = ADM2IFnr_STMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
		pStartMode->startSrc	= pStMode->ByBits.SrcStart;
//		if(!master)
//			pStartMode->startSrc = BRDsts_EXTSYNX;
		pStartMode->trigStopSrc	= pStMode->ByBits.SrcStop;
		pStartMode->startInv	= pStMode->ByBits.InvStart;
		pStartMode->stopInv		= pStMode->ByBits.InvStop;
		pStartMode->trigOn		= pStMode->ByBits.TrigStart;
		pStartMode->reStartMode	= pStMode->ByBits.Restart;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::CtrlRate(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   )
{
	CModule* pModule = (CModule*)pDev;
	ULONG ClkSrc;
	GetClkSource(pModule, ClkSrc);
	double ClkValue;
	GetClkValue(pModule, ClkSrc, ClkValue);

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;
	param.reg = ADM2IFnr_FDIV;
	if(BRDctrl_DIO32OUT_SETRATE == cmd)
	{
		double rate = *(double*)args;
		ULONG RateDivider = ROUND(ClkValue / rate);
		param.val = RateDivider;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		*(double*)args = ClkValue / RateDivider;
	}
	else
	{
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		double rate = ROUND(ClkValue / param.val);
		*(double*)args = rate;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::CtrlClkMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Dio32outClkMode pClk = (PBRD_Dio32outClkMode)args;
	if(BRDctrl_DIO32OUT_SETCLKMODE == cmd)
	{
		SetClkSource(pModule, pClk->src);
		SetClkValue(pModule, pClk->src, pClk->value);

	}
	else
	{
		ULONG ClkSrc;
		GetClkSource(pModule, ClkSrc);
		double ClkValue;
		GetClkValue(pModule, ClkSrc, ClkValue);
		pClk->src = ClkSrc;
		pClk->value = ClkValue;
	}
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDIO32OUT_MODE1 pMode1 = (PDIO32OUT_MODE1)&param.val;
	if(BRDctrl_DIO32OUT_SETCLKMODE == cmd)
	{
		pMode1->ByBits.OutClkInv = pClk->inv;
		pMode1->ByBits.OutClkMode = pClk->mode;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		pClk->inv = pMode1->ByBits.OutClkInv;
		pClk->mode = pMode1->ByBits.OutClkMode;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::CtrlStartMode(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   )
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_DIO32OUT_SETSTARTMODE == cmd)
		Status = SetStartMode(pModule, args);
	else
        Status = GetStartMode(pModule, args);
	return Status;
}

//***************************************************************************************
int CDio32OutSrv::CtrlMaster(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   )
{
	CModule* pModule = (CModule*)pDev;
	ULONG mode = *(ULONG*)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.reg = ADM2IFnr_MODE0;
	if(BRDctrl_DIO32OUT_SETMASTER == cmd)
	{
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
		pMode0->ByBits.Master = mode >> 1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

		param.tetr = m_Dio32OutTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pMode0->ByBits.Master = mode & 0x1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		param.tetr = m_Dio32OutTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
		mode = pMode0->ByBits.Master;

		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		mode |= (pMode0->ByBits.Master << 1);

		*(ULONG*)args = mode;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::CtrlPnpkMode(
							  void		*pDev,		// InfoSM or InfoBM
							  void		*pServData,	// Specific Service Data
							  ULONG		cmd,		// Command Code (from BRD_ctrl())
							  void		*args 		// Command Arguments (from BRD_ctrl())
							  )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDIO32OUT_MODE1 pMode1 = (PDIO32OUT_MODE1)&param.val;
	if(BRDctrl_DIO32OUT_SETPNPKMODE == cmd)
	{
		pMode1->ByBits.PnpkMode = *(PULONG)args;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
		*(PULONG)args = pMode1->ByBits.PnpkMode;
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::CtrlInitMode(
							  void		*pDev,		// InfoSM or InfoBM
							  void		*pServData,	// Specific Service Data
							  ULONG		cmd,		// Command Code (from BRD_ctrl())
							  void		*args 		// Command Arguments (from BRD_ctrl())
							  )
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Dio32outInitMode initMode = (PBRD_Dio32outInitMode)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDIO32OUT_MODE1 pMode1 = (PDIO32OUT_MODE1)&param.val;
	if(BRDctrl_DIO32OUT_SETINITMODE == cmd)
	{
		pMode1->ByBits.InitMode = initMode->mode;
		pMode1->ByBits.InitOut = initMode->out;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		initMode->mode = pMode1->ByBits.InitMode;
		initMode->out = pMode1->ByBits.InitOut;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::CtrlFifoReset (
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.FifoRes = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pMode0->ByBits.FifoRes = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::CtrlEnable(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.Start = *(PULONG)args;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::CtrlFifoStatus(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;//ADM2IF_ntBASEDAC;
	param.reg = ADM2IFnr_STATUS;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
	ULONG data = pStatus->AsWhole;
	*(PULONG)args = data;
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::CtrlPutData(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;

	//param.reg = ADM2IFnr_MODE0;
	//pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	//pMode0->ByBits.FifoRes = 1;
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//pMode0->ByBits.FifoRes = 0;
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//param.reg = ADM2IFnr_STATUS;
	//pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;

	param.reg = ADM2IFnr_DATA;
	PBRD_DataBuf pBuf = (PBRD_DataBuf)args;
	param.pBuf = pBuf->pData;
	param.bytes = pBuf->size;
	pModule->RegCtrl(DEVScmd_REGWRITESDIR, &param);

	//PULONG buf = (PULONG)pBuf->pData;
	//for(ULONG i = 0; i < pBuf->size / sizeof(ULONG); i++)
	//{
	//	param.val = buf[i];
	//	pModule->RegCtrl(DEVScmd_REGWRITEDIR, &param);
	//}
	//
	//param.reg = ADM2IFnr_STATUS;
	//pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//pStatus = (PADM2IF_STATUS)&param.val;

	return BRDerr_OK;
}

//***************************************************************************************
int CDio32OutSrv::CtrlFlagClear (
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32OutTetrNum;
	param.reg = DIO32OUT_nrFLAGCLR;
	param.val = *(PULONG)args;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

// ***************** End of file Dio32OutSrv.cpp ***********************
