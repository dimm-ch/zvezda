/*
 ***************** File Dio32InSrv.cpp ***********************
 *
 * BRD Driver for DIO32IN service
 *
 * (C) InSys by Dorokhin A. Sep 2007
 *
 ******************************************************
*/

//#include <windows.h>
//#include <winioctl.h>
//#include <tchar.h>

#include "module.h"
#include "Dio32InSrv.h"

#define	CURRFILE "DIO32INSRV"

static CMD_Info SETCHANMASK_CMD		= { BRDctrl_DIO32IN_SETCHANMASK, 0, 0, 0, NULL};
static CMD_Info GETCHANMASK_CMD		= { BRDctrl_DIO32IN_GETCHANMASK, 1, 0, 0, NULL};
static CMD_Info SETFORMAT_CMD		= { BRDctrl_DIO32IN_SETFORMAT, 0, 0, 0, NULL};
static CMD_Info GETFORMAT_CMD		= { BRDctrl_DIO32IN_GETFORMAT, 1, 0, 0, NULL};
static CMD_Info SETCLKMODE_CMD		= { BRDctrl_DIO32IN_SETCLKMODE,	 0, 0, 0, NULL};
static CMD_Info GETCLKMODE_CMD		= { BRDctrl_DIO32IN_GETCLKMODE,	 1, 0, 0, NULL};
static CMD_Info SETMASTER_CMD		= { BRDctrl_DIO32IN_SETMASTER,	  0, 0, 0, NULL};
static CMD_Info GETMASTER_CMD		= { BRDctrl_DIO32IN_GETMASTER,	  1, 0, 0, NULL};

static CMD_Info FIFORESET_CMD		= { BRDctrl_DIO32IN_FIFORESET,	 0, 0, 0, NULL};
static CMD_Info ENABLE_CMD			= { BRDctrl_DIO32IN_ENABLE,		 0, 0, 0, NULL};
static CMD_Info FIFOSTATUS_CMD		= { BRDctrl_DIO32IN_FIFOSTATUS,	 1, 0, 0, NULL};
static CMD_Info GETDATA_CMD			= { BRDctrl_DIO32IN_GETDATA,	 0, 0, 0, NULL};
static CMD_Info GETSRCSTREAM_CMD	= { BRDctrl_DIO32IN_GETSRCSTREAM, 1, 0, 0, NULL};
static CMD_Info FLAGCLR_CMD			= { BRDctrl_DIO32IN_FLAGCLR,	 0, 0, 0, NULL};

//***************************************************************************************
CDio32InSrv::CDio32InSrv(int idx, int srv_num, CModule* pModule, PDIO32INSRV_CFG pCfg) :
	CService(idx, _BRDC("DIO32IN"), srv_num, pModule, pCfg, sizeof(DIO32INSRV_CFG))
{
	m_attribute = 
			BRDserv_ATTR_DIRECTION_IN |
			BRDserv_ATTR_STREAMABLE_IN |
			BRDserv_ATTR_CMPABLE |
			BRDserv_ATTR_EXCLUSIVE_ONLY;

	Init(&SETCHANMASK_CMD, (CmdEntry)&CDio32InSrv::CtrlChanMask);
	Init(&GETCHANMASK_CMD, (CmdEntry)&CDio32InSrv::CtrlChanMask);
	Init(&SETFORMAT_CMD, (CmdEntry)&CDio32InSrv::CtrlFormat);
	Init(&GETFORMAT_CMD, (CmdEntry)&CDio32InSrv::CtrlFormat);
	Init(&SETCLKMODE_CMD, (CmdEntry)&CDio32InSrv::CtrlClkMode);
	Init(&GETCLKMODE_CMD, (CmdEntry)&CDio32InSrv::CtrlClkMode);
	Init(&SETMASTER_CMD, (CmdEntry)&CDio32InSrv::CtrlMaster);
	Init(&GETMASTER_CMD, (CmdEntry)&CDio32InSrv::CtrlMaster);

	Init(&FIFORESET_CMD, (CmdEntry)&CDio32InSrv::CtrlFifoReset);
	Init(&ENABLE_CMD, (CmdEntry)&CDio32InSrv::CtrlEnable);
	Init(&FIFOSTATUS_CMD, (CmdEntry)&CDio32InSrv::CtrlFifoStatus);
	Init(&GETDATA_CMD, (CmdEntry)&CDio32InSrv::CtrlGetData);
	Init(&GETSRCSTREAM_CMD, (CmdEntry)&CDio32InSrv::CtrlGetAddrData);
	Init(&FLAGCLR_CMD, (CmdEntry)&CDio32InSrv::CtrlFlagClear);

}

//***************************************************************************************
int CDio32InSrv::CtrlIsAvailable(
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
	m_Dio32InTetrNum = GetTetrNum(pModule, m_index, DIO32IN_TETR_ID);
	if(m_MainTetrNum != -1 && m_Dio32InTetrNum != -1)
	{
		m_isAvailable = 1;
		PDIO32INSRV_CFG pSrvCfg = (PDIO32INSRV_CFG)m_pConfig;
		if(!pSrvCfg->isAlreadyInit)
		{
			pSrvCfg->isAlreadyInit = 1;
			DEVS_CMD_Reg RegParam;
			RegParam.idxMain = m_index;
			RegParam.tetr = m_Dio32InTetrNum;
			if(!pSrvCfg->FifoSize)
			{
				RegParam.reg = ADM2IFnr_FTYPE;
				pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
				int widthFifo = RegParam.val >> 3;
				RegParam.reg = ADM2IFnr_FSIZE;
				pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
				pSrvCfg->FifoSize = RegParam.val * widthFifo;
			}
			RegParam.tetr = m_Dio32InTetrNum;
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
int CDio32InSrv::CtrlGetAddrData(
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
	*(ULONG*)args = (m_AdmNum << 16) | m_Dio32InTetrNum;
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32InSrv::CtrlChanMask(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32InTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDIO32IN_MODE1 pMode1 = (PDIO32IN_MODE1)&param.val;
	if(BRDctrl_DIO32IN_SETCHANMASK == cmd)
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
int CDio32InSrv::CtrlFormat(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32InTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDIO32IN_MODE1 pMode1 = (PDIO32IN_MODE1)&param.val;
	if(BRDctrl_DIO32IN_SETFORMAT == cmd)
	{
		pMode1->ByBits.Packing = *(PULONG)args;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
		*(PULONG)args = pMode1->ByBits.Packing;
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32InSrv::CtrlClkMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32InTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDIO32IN_MODE1 pMode1 = (PDIO32IN_MODE1)&param.val;
	if(BRDctrl_DIO32IN_SETCLKMODE == cmd)
	{
		pMode1->ByBits.ExtClkInv = *(PULONG)args;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		*(PULONG)args = pMode1->ByBits.ExtClkInv;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32InSrv::CtrlMaster(
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
	if(BRDctrl_DIO32IN_SETMASTER == cmd)
	{
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
		pMode0->ByBits.Master = mode >> 1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

		param.tetr = m_Dio32InTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pMode0->ByBits.Master = mode & 0x1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		param.tetr = m_Dio32InTetrNum;
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
int CDio32InSrv::CtrlFifoReset (
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32InTetrNum;
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
int CDio32InSrv::CtrlEnable(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32InTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.Start = *(PULONG)args;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32InSrv::CtrlFifoStatus(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32InTetrNum;
	param.reg = ADM2IFnr_STATUS;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
	ULONG data = pStatus->AsWhole;
	*(PULONG)args = data;
	return BRDerr_OK;
}

//***************************************************************************************
int CDio32InSrv::CtrlGetData(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32InTetrNum;
/*	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.FifoRes = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pMode0->ByBits.FifoRes = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
*/
	//PADM2IF_STATUS pStatus;
	//do
	//{
	//	param.reg = ADM2IFnr_STATUS;
	//	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//	pStatus = (PADM2IF_STATUS)&param.val;
	//} while(pStatus->ByBits.HalfFull);

	param.reg = ADM2IFnr_DATA;
	PBRD_DataBuf pBuf = (PBRD_DataBuf)args;
	param.pBuf = pBuf->pData;
	param.bytes = pBuf->size;
	pModule->RegCtrl(DEVScmd_REGREADSDIR, &param);

	//PULONG buf = (PULONG)pBuf->pData;
	//for(ULONG i = 0; i < pBuf->size / sizeof(ULONG); i++)
	//{
	//	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//	buf[i] = param.val;
	//}
	
	//param.reg = ADM2IFnr_STATUS;
	//pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//pStatus = (PADM2IF_STATUS)&param.val;

	return BRDerr_OK;
}

//***************************************************************************************
int CDio32InSrv::CtrlFlagClear (
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_Dio32InTetrNum;
	param.reg = DIO32INnr_FLAGCLR;
	param.val = *(PULONG)args;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

// ***************** End of file Dio32InSrv.cpp ***********************
