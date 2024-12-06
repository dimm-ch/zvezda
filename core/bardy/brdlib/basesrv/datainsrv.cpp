/*
 ***************** File DataInSrv.cpp ***********************
 *
 * BRD Driver for ADM-interface DataIn service
 *
 * (C) InSys by Dorokhin A. Mar 2007
 *
 ******************************************************
*/

#include <chrono>

#include "module.h"
#include "datainsrv.h"

#define	CURRFILE _BRDC("DATAINSRV")

static CMD_Info SETMODE_CMD		= { BRDctrl_DATAIN_SETMODE,		0, 0, 0, NULL};
static CMD_Info GETMODE_CMD		= { BRDctrl_DATAIN_GETMODE,		1, 0, 0, NULL};
static CMD_Info GETCFG_CMD		= { BRDctrl_DATAIN_GETCFG,		1, 0, 0, NULL};

static CMD_Info FIFORESET_CMD	= { BRDctrl_DATAIN_FIFORESET,	0, 0, 0, NULL};
static CMD_Info ENABLE_CMD		= { BRDctrl_DATAIN_ENABLE,   	0, 0, 0, NULL};
static CMD_Info FIFOSTATUS_CMD	= { BRDctrl_DATAIN_FIFOSTATUS,	1, 0, 0, NULL};
static CMD_Info GETDATA_CMD		= { BRDctrl_DATAIN_GETDATA,		0, 0, 0, NULL};

static CMD_Info GETSRCSTREAM_CMD = { BRDctrl_DATAIN_GETSRCSTREAM, 1, 0, 0, NULL};

//***************************************************************************************
CDataInSrv::CDataInSrv(int idx, int srv_num, CModule* pModule, PDATAINSRV_CFG pCfg) :
	CService(idx, _BRDC("DATAIN"), srv_num, pModule, pCfg, sizeof(DATAINSRV_CFG))
{
	m_attribute = 
			BRDserv_ATTR_DIRECTION_IN |
//			BRDserv_ATTR_DIRECTION_OUT |
			BRDserv_ATTR_STREAMABLE_IN |
//			BRDserv_ATTR_STREAMABLE_OUT |
//			BRDserv_ATTR_UNVISIBLE |
//			BRDserv_ATTR_SUBSERVICE_ONLY |
			BRDserv_ATTR_EXCLUSIVE_ONLY;

//	Init(&GETCFG_CMD, (CmdEntry)CtrlCfg);

	Init(&SETMODE_CMD, (CmdEntry)&CDataInSrv::CtrlMode);
	Init(&GETMODE_CMD, (CmdEntry)&CDataInSrv::CtrlMode);
	Init(&GETCFG_CMD, (CmdEntry)&CDataInSrv::CtrlCfg);

	Init(&FIFORESET_CMD, (CmdEntry)&CDataInSrv::CtrlFifoReset);
	Init(&ENABLE_CMD, (CmdEntry)&CDataInSrv::CtrlEnable);
	Init(&FIFOSTATUS_CMD, (CmdEntry)&CDataInSrv::CtrlFifoStatus);
	Init(&GETDATA_CMD, (CmdEntry)&CDataInSrv::CtrlGetData);

	Init(&GETSRCSTREAM_CMD, (CmdEntry)&CDataInSrv::CtrlGetAddrData);
}

//***************************************************************************************
void CDataInSrv::TetradInit(CModule* pModule, ULONG tetrNum)
{
		DEVS_CMD_Reg RegParam;
		RegParam.idxMain = m_index;
		RegParam.tetr = tetrNum;

		RegParam.reg = ADM2IFnr_MODE0;
		RegParam.val = 0;
	//	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
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

		RegParam.reg = ADM2IFnr_MODE0;
		pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
//		PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&RegParam.val;
		pMode0->ByBits.Master = 1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
}

//***************************************************************************************
ULONG CDataInSrv::GetFifoSize(CModule* pModule, ULONG tetrNum)
{
		DEVS_CMD_Reg RegParam;
		RegParam.idxMain = m_index;
		RegParam.tetr = tetrNum;
		RegParam.reg = ADM2IFnr_FTYPE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
		int widthFifo = RegParam.val >> 3;
		RegParam.reg = ADM2IFnr_FSIZE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
		return (RegParam.val * widthFifo);
}

//***************************************************************************************
int CDataInSrv::CtrlIsAvailable(
							  void		*pDev,		// InfoSM or InfoBM
							  void		*pServData,	// Specific Service Data
							  ULONG		cmd,		// Command Code (from BRD_ctrl())
							  void		*args 		// Command Arguments (from BRD_ctrl())
							  )
{
	CModule* pModule = (CModule*)pDev;
	PSERV_CMD_IsAvailable pServAvailable = (PSERV_CMD_IsAvailable)args;
	pServAvailable->attr = m_attribute;
	pServAvailable->isAvailable = 0;

	m_MainTetrNum = GetTetrNum(pModule, m_index, MAIN_TETR_ID);
	m_DataInTetrNum = GetTetrNum(pModule, m_index, DATAIN_TETR_ID);
	if(m_MainTetrNum != -1 && m_DataInTetrNum != -1)
	{
		m_isAvailable = 1;

		PDATAINSRV_CFG pSrvCfg = (PDATAINSRV_CFG)m_pConfig;
		pSrvCfg->InFifoSize = GetFifoSize(pModule, m_DataInTetrNum);

		if(!pSrvCfg->isAlreadyInit)
		{
			pSrvCfg->isAlreadyInit = 1;
			TetradInit(pModule, m_DataInTetrNum);
		}
	}
	else
		m_isAvailable = 0;

	pServAvailable->isAvailable = m_isAvailable;
	return BRDerr_OK;
}

//***************************************************************************************
int CDataInSrv::CtrlGetAddrData(
							void			*pDev,		// InfoSM or InfoBM
							void			*pServData,	// Specific Service Data
							ULONG			cmd,		// Command Code (from BRD_ctrl())
							void			*args 		// Command Arguments (from BRD_ctrl())
							)
{
	ULONG m_AdmNum;
	BRDCHAR buf[SERVNAME_SIZE];
	BRDC_strcpy(buf, m_name);
	BRDCHAR* pBuf = buf + (BRDC_strlen(buf) - 2);
	if(BRDC_strchr(pBuf, _BRDC('1')))
//	if(_tcschr(m_name, '1'))
		m_AdmNum = 1;
	else
		m_AdmNum = 0;
	*(ULONG*)args = (m_AdmNum << 16) | m_DataInTetrNum;
	return BRDerr_OK;
}

//***************************************************************************************
int CDataInSrv::CtrlMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DataInTetrNum;
	param.reg = ADM2IFnr_MODE1;
	if(BRDctrl_DATAIN_SETMODE == cmd)
	{
		param.val = *(ULONG*)args;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		*(ULONG*)args = param.val;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDataInSrv::CtrlFifoReset(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DataInTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.FifoRes = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pMode0->ByBits.FifoRes = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	return BRDerr_OK;
}

//***************************************************************************************
int CDataInSrv::CtrlEnable(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DataInTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.Start = *(PULONG)args;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDataInSrv::CtrlFifoStatus(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DataInTetrNum;
	param.reg = ADM2IFnr_STATUS;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
//	ULONG data = pStatus->ByBits.Underflow;
	ULONG data = pStatus->AsWhole;
	*(PULONG)args = data;
	return BRDerr_OK;
}

//***************************************************************************************
int CDataInSrv::CtrlGetData(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DataInTetrNum;
	param.reg = ADM2IFnr_DATA;
	PBRD_DataBuf pBuf = (PBRD_DataBuf)args;
	param.pBuf = pBuf->pData;
	param.bytes = pBuf->size;
	pModule->RegCtrl(DEVScmd_REGREADSDIR, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDataInSrv::CtrlCfg(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_OK;
	PBRD_DataInOutCfg pCfg = (PBRD_DataInOutCfg)args;
	PDATAINSRV_CFG pSrvCfg = (PDATAINSRV_CFG)m_pConfig;
	pCfg->FifoSize = pSrvCfg->InFifoSize;

	return BRDerr_OK;
}

// ***************** End of file DataInSrv.cpp ***********************
