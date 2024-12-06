/*
 ***************** File DataOutSrv.cpp ***********************
 *
 * BRD Driver for ADM-interface DataOut service
 *
 * (C) InSys by Dorokhin A. Mar 2007
 *
 ******************************************************
*/

#include <chrono>

#include "module.h"
#include "dataoutsrv.h"

#define	CURRFILE _BRDC("DATAOUTSRV")

static CMD_Info SETMODE_CMD		= { BRDctrl_DATAOUT_SETMODE,		0, 0, 0, NULL};
static CMD_Info GETMODE_CMD		= { BRDctrl_DATAOUT_GETMODE,		1, 0, 0, NULL};
static CMD_Info GETCFG_CMD		= { BRDctrl_DATAOUT_GETCFG,		1, 0, 0, NULL};

static CMD_Info FIFORESET_CMD	= { BRDctrl_DATAOUT_FIFORESET,	0, 0, 0, NULL};
static CMD_Info ENABLE_CMD		= { BRDctrl_DATAOUT_ENABLE,   	0, 0, 0, NULL};
static CMD_Info FIFOSTATUS_CMD	= { BRDctrl_DATAOUT_FIFOSTATUS,	1, 0, 0, NULL};
static CMD_Info PUTDATA_CMD		= { BRDctrl_DATAOUT_PUTDATA,	0, 0, 0, NULL};

static CMD_Info GETSRCSTREAM_CMD = { BRDctrl_DATAOUT_GETSRCSTREAM, 1, 0, 0, NULL};

//***************************************************************************************
CDataOutSrv::CDataOutSrv(int idx, int srv_num, CModule* pModule, PDATAOUTSRV_CFG pCfg) :
	CService(idx, _BRDC("DATAOUT"), srv_num, pModule, pCfg, sizeof(DATAOUTSRV_CFG))
{
	m_attribute = 
//			BRDserv_ATTR_DIRECTION_IN |
			BRDserv_ATTR_DIRECTION_OUT |
//			BRDserv_ATTR_STREAMABLE_IN |
			BRDserv_ATTR_STREAMABLE_OUT |
//			BRDserv_ATTR_UNVISIBLE |
//			BRDserv_ATTR_SUBSERVICE_ONLY |
			BRDserv_ATTR_EXCLUSIVE_ONLY;

	Init(&SETMODE_CMD, (CmdEntry)&CDataOutSrv::CtrlMode);
	Init(&GETMODE_CMD, (CmdEntry)&CDataOutSrv::CtrlMode);
	Init(&GETCFG_CMD, (CmdEntry)&CDataOutSrv::CtrlCfg);

	Init(&FIFORESET_CMD, (CmdEntry)&CDataOutSrv::CtrlFifoReset);
	Init(&ENABLE_CMD, (CmdEntry)&CDataOutSrv::CtrlEnable);
	Init(&FIFOSTATUS_CMD, (CmdEntry)&CDataOutSrv::CtrlFifoStatus);
	Init(&PUTDATA_CMD, (CmdEntry)&CDataOutSrv::CtrlPutData);

	Init(&GETSRCSTREAM_CMD, (CmdEntry)&CDataOutSrv::CtrlGetAddrData);
}

//***************************************************************************************
void CDataOutSrv::TetradInit(CModule* pModule, ULONG tetrNum)
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
ULONG CDataOutSrv::GetFifoSize(CModule* pModule, ULONG tetrNum)
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
int CDataOutSrv::CtrlIsAvailable(
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
	m_DataOutTetrNum = GetTetrNum(pModule, m_index, DATAOUT_TETR_ID);
	if(m_MainTetrNum != -1 && m_DataOutTetrNum != -1)
	{
		m_isAvailable = 1;

		PDATAOUTSRV_CFG pSrvCfg = (PDATAOUTSRV_CFG)m_pConfig;
		pSrvCfg->OutFifoSize = GetFifoSize(pModule, m_DataOutTetrNum);

		if(!pSrvCfg->isAlreadyInit)
		{
			pSrvCfg->isAlreadyInit = 1;
			TetradInit(pModule, m_DataOutTetrNum);
		}
	}
	else
		m_isAvailable = 0;

	pServAvailable->isAvailable = m_isAvailable;
	return BRDerr_OK;
}

//***************************************************************************************
int CDataOutSrv::CtrlGetAddrData(
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
	*(ULONG*)args = (m_AdmNum << 16) | m_DataOutTetrNum;
	return BRDerr_OK;
}

//***************************************************************************************
int CDataOutSrv::CtrlMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	//CModule* pModule = (CModule*)pDev;
	//if(BRDctrl_DSPNODE_SETMODE == cmd)
	//{
	//	ULONG mode = *(ULONG*)args;
	//	Status = SetMode(pModule, mode);
	//	if(!mode && pServData)
	//	{
	//		PUNDERSERV_Cmd pReleaseCmd = (PUNDERSERV_Cmd)pServData;
	//		pReleaseCmd->code = SERVcmd_SYS_RELEASE;
	//	}
	//}
	//else
	//{
	//	ULONG mode;
	//	Status = GetMode(pModule, mode);
	//	*(ULONG*)args = mode;
	//}
	return Status;
}

//***************************************************************************************
int CDataOutSrv::CtrlFifoReset(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DataOutTetrNum;
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
int CDataOutSrv::CtrlEnable(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DataOutTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.Start = *(PULONG)args;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDataOutSrv::CtrlFifoStatus(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DataOutTetrNum;
	param.reg = ADM2IFnr_STATUS;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
//	ULONG data = pStatus->ByBits.Underflow;
	ULONG data = pStatus->AsWhole;
	*(PULONG)args = data;
	return BRDerr_OK;
}

//***************************************************************************************
int CDataOutSrv::CtrlPutData(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DataOutTetrNum;
	param.reg = ADM2IFnr_DATA;
	PBRD_DataBuf pBuf = (PBRD_DataBuf)args;
	param.pBuf = pBuf->pData;
	param.bytes = pBuf->size;
	pModule->RegCtrl(DEVScmd_REGWRITESDIR, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDataOutSrv::CtrlCfg(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_OK;
	PBRD_DataInOutCfg pCfg = (PBRD_DataInOutCfg)args;
	PDATAOUTSRV_CFG pSrvCfg = (PDATAOUTSRV_CFG)m_pConfig;
	pCfg->FifoSize = pSrvCfg->OutFifoSize;

	return BRDerr_OK;
}

// ***************** End of file DataOutSrv.cpp ***********************
