/*
 ***************** File TestSrv.cpp ***********************
 *
 * BRD Driver for ADM-interface Test service
 *
 * (C) InSys by Dorokhin A. Aug 2005
 *
 * 02.10.2007 - add TESTIN
 *
 ******************************************************
*/

#include "module.h"
#include "testsrv.h"

#define	CURRFILE "TESTSRV"

static CMD_Info SETMODE_CMD		= { BRDctrl_TEST_SETMODE,		0, 0, 0, NULL};
static CMD_Info GETMODE_CMD		= { BRDctrl_TEST_GETMODE,		1, 0, 0, NULL};

static CMD_Info GETCFG_CMD		= { BRDctrl_TEST_GETCFG,			1, 0, 0, NULL};
//static CMD_Info GETDLGPAGES_CMD	= { BRDctrl_TEST_GETDLGPAGES,	1, 0, 0, NULL};

static CMD_Info FIFORESET_CMD	= { BRDctrl_TEST_FIFORESET,		0, 0, 0, NULL};
static CMD_Info ENABLE_CMD		= { BRDctrl_TEST_ENABLE,   		0, 0, 0, NULL};
static CMD_Info FIFOSTATUS_CMD	= { BRDctrl_TEST_FIFOSTATUS,	1, 0, 0, NULL};
static CMD_Info GETDATA_CMD		= { BRDctrl_TEST_GETDATA,		0, 0, 0, NULL};
static CMD_Info SETDATA_CMD		= { BRDctrl_TEST_SETDATA,		0, 0, 0, NULL};

static CMD_Info GETSRCSTREAM_CMD = { BRDctrl_TEST_GETSRCSTREAM,	1, 0, 0, NULL};

//***************************************************************************************
CTestSrv::CTestSrv(int idx, int srv_num, CModule* pModule, PTESTSRV_CFG pCfg) :
	CService(idx, _BRDC("TEST"), srv_num, pModule, pCfg, sizeof(TESTSRV_CFG))
{
	m_attribute = 
			BRDserv_ATTR_DIRECTION_IN |
			BRDserv_ATTR_DIRECTION_OUT |
			BRDserv_ATTR_STREAMABLE_IN |
			BRDserv_ATTR_STREAMABLE_OUT |
//			BRDserv_ATTR_UNVISIBLE |
			BRDserv_ATTR_SUBSERVICE_ONLY |
			BRDserv_ATTR_EXCLUSIVE_ONLY;

	Init(&SETMODE_CMD, (CmdEntry)&CTestSrv::CtrlMode);
	Init(&GETMODE_CMD, (CmdEntry)&CTestSrv::CtrlMode);

	Init(&GETCFG_CMD, (CmdEntry)&CTestSrv::CtrlCfg);
//	Init(&GETDLGPAGES_CMD, (CmdEntry)CtrlGetPages);

	Init(&FIFORESET_CMD, (CmdEntry)&CTestSrv::CtrlFifoReset);
	Init(&ENABLE_CMD, (CmdEntry)&CTestSrv::CtrlEnable);
	Init(&FIFOSTATUS_CMD, (CmdEntry)&CTestSrv::CtrlFifoStatus);
	Init(&GETDATA_CMD, (CmdEntry)&CTestSrv::CtrlGetData);
	Init(&SETDATA_CMD, (CmdEntry)&CTestSrv::CtrlSetData);

	Init(&GETSRCSTREAM_CMD, (CmdEntry)&CTestSrv::CtrlGetAddrData);
}

//***************************************************************************************
void CTestSrv::TetradInit(CModule* pModule, ULONG tetrNum)
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
ULONG CTestSrv::GetFifoSize(CModule* pModule, ULONG tetrNum)
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
int CTestSrv::CtrlIsAvailable(
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
	m_TestOutTetrNum = GetTetrNum(pModule, m_index, TESTOUT_TETR_ID);
	m_TestInTetrNum = GetTetrNum(pModule, m_index, TESTIN_TETR_ID);
	if(m_MainTetrNum != -1 && (m_TestOutTetrNum != -1 || m_TestInTetrNum != -1))
	{
//		printf("Available: %d %d %d\n", m_MainTetrNum, m_TestOutTetrNum, m_TestInTetrNum);
		m_isAvailable = 1;
	}
	else
	{
//		printf("NOT Available: %d %d %d\n", m_MainTetrNum, m_TestOutTetrNum, m_TestInTetrNum);
		m_isAvailable = 0;
	}
	pServAvailable->isAvailable = m_isAvailable;
	return BRDerr_OK;
}

//***************************************************************************************
int CTestSrv::CtrlCapture(
							  void		*pDev,		// InfoSM or InfoBM
							  void		*pServData,	// Specific Service Data
							  ULONG		cmd,		// Command Code (from BRD_ctrl())
							  void		*args 		// Command Arguments (from BRD_ctrl())
							  )
{
	CModule* pModule = (CModule*)pDev;

	if(m_MainTetrNum != -1 && (m_TestOutTetrNum != -1 || m_TestInTetrNum != -1))
	{
		PTESTSRV_CFG pSrvCfg = (PTESTSRV_CFG)m_pConfig;
		if(m_TestOutTetrNum != -1)
			pSrvCfg->OutFifoSize = GetFifoSize(pModule, m_TestOutTetrNum);
		if(m_TestInTetrNum != -1)
			pSrvCfg->InFifoSize = GetFifoSize(pModule, m_TestInTetrNum);

		if(!pSrvCfg->isAlreadyInit)
		{
			pSrvCfg->isAlreadyInit = 1;
			if (m_TestInTetrNum != -1)
				TetradInit(pModule, m_TestInTetrNum);
			if(m_TestOutTetrNum != -1)
				TetradInit(pModule, m_TestOutTetrNum);
		}
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CTestSrv::CtrlRelease(
						void			*pDev,		// InfoSM or InfoBM
						void			*pServData,	// Specific Service Data
						ULONG			cmd,		// Command Code (from BRD_ctrl())
						void			*args 		// Command Arguments (from BRD_ctrl())
						)
{
	PTESTSRV_CFG pSrvCfg = (PTESTSRV_CFG)m_pConfig;
	pSrvCfg->isAlreadyInit = 0;
	return BRDerr_CMD_UNSUPPORTED; // for free subservice
}

//***************************************************************************************
int CTestSrv::CtrlGetAddrData(
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
	if(BRDC_strchr(pBuf, '1'))
//	if(_tcschr(m_name, '1'))
		m_AdmNum = 1;
	else
		m_AdmNum = 0;
	if (m_TestInTetrNum != -1)
		*(ULONG*)args = (m_AdmNum << 16) | m_TestInTetrNum;
	if(m_TestOutTetrNum != -1)
		*(ULONG*)args = (m_AdmNum << 16) | m_TestOutTetrNum;

	return BRDerr_OK;
}

//***************************************************************************************
int CTestSrv::CtrlCfg(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	PBRD_TestCfg pCfg = (PBRD_TestCfg)args;
	PTESTSRV_CFG pSrvCfg = (PTESTSRV_CFG)m_pConfig;
	if (m_TestInTetrNum != -1)
		pCfg->FifoSize = pSrvCfg->InFifoSize;
	if(m_TestOutTetrNum != -1)
		pCfg->FifoSize = pSrvCfg->OutFifoSize;
	return BRDerr_OK;
}

//***************************************************************************************
int CTestSrv::CtrlMode(
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
int CTestSrv::CtrlFifoReset(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	if (m_TestInTetrNum != -1)
		param.tetr = m_TestInTetrNum;
	if(m_TestOutTetrNum != -1)
		param.tetr = m_TestOutTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.FifoRes = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pMode0->ByBits.FifoRes = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
#ifdef _WIN32
	Sleep(1);
#else
	IPC_delay(1);
#endif
	return BRDerr_OK;
}

//***************************************************************************************
int CTestSrv::CtrlEnable(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	//CModule* pModule = (CModule*)pDev;
	//DEVS_CMD_Reg param;
	//param.idxMain = m_index;
	//param.tetr = m_MemTetrNum;
	//param.reg = ADM2IFnr_MODE0;
	//pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	//pMode0->ByBits.Start = *(PULONG)args;
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CTestSrv::CtrlFifoStatus(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	if (m_TestInTetrNum != -1)
		param.tetr = m_TestInTetrNum;
	if(m_TestOutTetrNum != -1)
		param.tetr = m_TestOutTetrNum;
	param.reg = ADM2IFnr_STATUS;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
//	ULONG data = pStatus->ByBits.Underflow;
	ULONG data = pStatus->AsWhole;
	*(PULONG)args = data;
	return BRDerr_OK;
}

//***************************************************************************************
int CTestSrv::CtrlGetData(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_TestInTetrNum;
	param.reg = ADM2IFnr_DATA;
	PBRD_DataBuf pBuf = (PBRD_DataBuf)args;
	param.pBuf = pBuf->pData;
	param.bytes = pBuf->size;
	pModule->RegCtrl(DEVScmd_REGREADSDIR, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CTestSrv::CtrlSetData(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_TestOutTetrNum;
	param.reg = ADM2IFnr_DATA;
	PBRD_DataBuf pBuf = (PBRD_DataBuf)args;
	param.pBuf = pBuf->pData;
	param.bytes = pBuf->size;
	//pModule->RegCtrl(DEVScmd_REGWRITESDIR, &param);
	
	ULONG* pDataBuf = (ULONG*)param.pBuf;
	int cnt = param.bytes / 4;
	for(int i = 0; i < cnt; i++)
	{
		param.val = pDataBuf[i];
		pModule->RegCtrl(DEVScmd_REGWRITEDIR, &param);
	}

	//UCHAR* pDataBuf = (UCHAR*)pBuf->pData;
	//int cnt = pBuf->size / 64;
	//param.bytes = 64;
	//for(int i = 0; i < cnt; i++)
	//{
	//	param.pBuf = pDataBuf + i * 64;
	//	pModule->RegCtrl(DEVScmd_REGWRITESDIR, &param);
	//}
	return BRDerr_OK;
}

// ***************** End of file TestSrv.cpp ***********************
