/*
 ***************** File PioSrv.cpp ***********************
 *
 * BRD Driver for PIO service
 *
 * (C) InSys by Dorokhin A. Mar 2006
 *
 *********************************************************
*/

#include "module.h"
#include "piosrv.h"

#define	CURRFILE _BRDC("PIOSRV")

static CMD_Info SETDIR_CMD		= { BRDctrl_PIO_SETDIR,		0, 0, 0, NULL};
static CMD_Info GETDIR_CMD		= { BRDctrl_PIO_GETDIR,		1, 0, 0, NULL};
static CMD_Info SETMODE_CMD		= { BRDctrl_PIO_SETMODE,	0, 0, 0, NULL};
static CMD_Info GETMODE_CMD		= { BRDctrl_PIO_GETMODE,	1, 0, 0, NULL};
static CMD_Info WRITEDATA_CMD	= { BRDctrl_PIO_WRITEDATA,	0, 0, 0, NULL};
static CMD_Info READDATA_CMD	= { BRDctrl_PIO_READDATA,	0, 0, 0, NULL};
static CMD_Info ENABLE_CMD		= { BRDctrl_PIO_ENABLE,		0, 0, 0, NULL};
static CMD_Info GETISLVDS_CMD	= { BRDctrl_PIO_ISLVDS,		1, 0, 0, NULL};
static CMD_Info FIFOSTATUS_CMD	= { BRDctrl_PIO_STATUS,		1, 0, 0, NULL};
static CMD_Info FLAGCLR_CMD		= { BRDctrl_PIO_FLAGCLR,   	0, 0, 0, NULL};

//***************************************************************************************
CPioSrv::CPioSrv(int idx, int srv_num, CModule* pModule, PPIOSRV_CFG pCfg) :
	CService(idx, _BRDC("PIOSTD"), srv_num, pModule, pCfg, sizeof(PIOSRV_CFG))
{
	m_attribute = 
			BRDserv_ATTR_DIRECTION_IN |
			BRDserv_ATTR_DIRECTION_OUT	|
			BRDserv_ATTR_EXCLUSIVE_ONLY;

	Init(&SETDIR_CMD, (CmdEntry)&CPioSrv::CtrlDir);
	Init(&GETDIR_CMD, (CmdEntry)&CPioSrv::CtrlDir);
	Init(&SETMODE_CMD, (CmdEntry)&CPioSrv::CtrlMode);
	Init(&GETMODE_CMD, (CmdEntry)&CPioSrv::CtrlMode);
	Init(&WRITEDATA_CMD, (CmdEntry)&CPioSrv::CtrlWriteData);
	Init(&READDATA_CMD, (CmdEntry)&CPioSrv::CtrlReadData);

	Init(&ENABLE_CMD, (CmdEntry)&CPioSrv::CtrlEnable);
	Init(&GETISLVDS_CMD, (CmdEntry)&CPioSrv::CtrlIsLvds);
	Init(&FIFOSTATUS_CMD, (CmdEntry)&CPioSrv::CtrlStatus);
	Init(&FLAGCLR_CMD, (CmdEntry)&CPioSrv::CtrlFlagClear);
}

//***************************************************************************************
int CPioSrv::CtrlIsAvailable(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	PSERV_CMD_IsAvailable pServAvailable = (PSERV_CMD_IsAvailable)args;
	pServAvailable->attr = m_attribute;
	m_PioTetrNum = GetTetrNum(pModule, m_index, PIOSTD_TETR_ID);

	if(m_PioTetrNum != -1)
	{
		//PPIOSRV_CFG pSrvCfg = (PPIOSRV_CFG)m_pConfig;
		//if(!pSrvCfg->isAlreadyInit)
		//{
		//	pSrvCfg->isAlreadyInit = 1;

		//	DEVS_CMD_Reg RegParam;
		//	RegParam.idxMain = m_index;
		//	RegParam.tetr = m_PioTetrNum;
		//	RegParam.reg = ADM2IFnr_MODE0;
		//	RegParam.val = 0;
		//	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&RegParam.val;
		//	pMode0->ByBits.Reset = 1;
		//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
		//	for(int i = 1; i < 32; i++)
		//	{
		//		RegParam.reg = i;
		//		RegParam.val = 0;
		//		pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
		//	}
		//	RegParam.reg = ADM2IFnr_MODE0;
		//	pMode0->ByBits.Reset = 0;
		//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

		//	RegParam.reg = ADM2IFnr_MODE1;
		//	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
		//	PPIO_MODE1 pMode1 = (PPIO_MODE1)&RegParam.val;
		//	pMode1->ByBits.Enbl = 1;
		//	if(pSrvCfg->PioType == 2)
		//		pMode1->ByBits.Lvds = 1;
		//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
		//}
		m_isAvailable = 1;
		//printf("CPioSrv::CtrlIsAvailable!!\n");
	}
	else
		m_isAvailable = 0;
	pServAvailable->isAvailable = m_isAvailable;

	return BRDerr_OK;
}

//***************************************************************************************
int CPioSrv::CtrlCapture(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	if(m_PioTetrNum != -1)
	{
		PPIOSRV_CFG pSrvCfg = (PPIOSRV_CFG)m_pConfig;
		if(!pSrvCfg->isAlreadyInit)
		{
			pSrvCfg->isAlreadyInit = 1;

			DEVS_CMD_Reg RegParam;
			RegParam.idxMain = m_index;
			RegParam.tetr = m_PioTetrNum;
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

			RegParam.reg = ADM2IFnr_MODE1;
			pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
			PPIO_MODE1 pMode1 = (PPIO_MODE1)&RegParam.val;
			pMode1->ByBits.Enbl = 1;
			if(pSrvCfg->PioType == 2)
				pMode1->ByBits.Lvds = 1;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			//printf("CPioSrv::CtrlCapture!!\n");
		}
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CPioSrv::CtrlDir(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   )
{
	CModule* pModule = (CModule*)pDev;
	PBRD_PioDir pPioDir = (PBRD_PioDir)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_PioTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PPIO_MODE1 pMode1 = (PPIO_MODE1)&param.val;

	if(BRDctrl_PIO_SETDIR == cmd)
	{
		pMode1->ByBits.LBDir = pPioDir->lbDir;
		pMode1->ByBits.HBDir = pPioDir->hbDir;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		pPioDir->lbDir = pMode1->ByBits.LBDir;
		pPioDir->hbDir = pMode1->ByBits.HBDir;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CPioSrv::CtrlMode(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   )
{
	CModule* pModule = (CModule*)pDev;
	PBRD_PioMode pPioMode = (PBRD_PioMode)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_PioTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PPIO_MODE1 pMode1 = (PPIO_MODE1)&param.val;

	if(BRDctrl_PIO_SETMODE == cmd)
	{
		pMode1->ByBits.RdMode = pPioMode->rdMode;
		pMode1->ByBits.WrMode = pPioMode->wrMode;
		pMode1->ByBits.PioCtrl = pPioMode->strobeFlow;
		pMode1->ByBits.AckCtrl = pPioMode->ackFlow;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

		param.reg = ADM2IFnr_MODE2;
		PPIO_MODE2 pMode2 = (PPIO_MODE2)&param.val;
		pMode2->ByBits.CntWr = pPioMode->wrStrobe;
		pMode2->ByBits.CntRd = pPioMode->rdStrobe;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		pPioMode->rdMode = pMode1->ByBits.RdMode;
		pPioMode->wrMode = pMode1->ByBits.WrMode;
		pPioMode->strobeFlow = pMode1->ByBits.PioCtrl;
		pPioMode->ackFlow = pMode1->ByBits.AckCtrl;

		param.reg = ADM2IFnr_MODE2;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PPIO_MODE2 pMode2 = (PPIO_MODE2)&param.val;
		pPioMode->wrStrobe = pMode2->ByBits.CntWr;
		pPioMode->rdStrobe = pMode2->ByBits.CntRd;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CPioSrv::CtrlEnable(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_PioTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PPIO_MODE1 pMode1 = (PPIO_MODE1)&param.val;
	pMode1->ByBits.Enbl = *(PULONG)args;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CPioSrv::CtrlIsLvds(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_PioTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PPIO_MODE1 pMode1 = (PPIO_MODE1)&param.val;
	*(PULONG)args = pMode1->ByBits.Lvds;
	return BRDerr_OK;
}

//***************************************************************************************
int CPioSrv::CtrlStatus(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_PioTetrNum;
	param.reg = ADM2IFnr_STATUS;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
	ULONG data = pStatus->AsWhole;
	*(PULONG)args = data;
	return BRDerr_OK;
}

//***************************************************************************************
int CPioSrv::CtrlWriteData(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	ULONG Status = BRDerr_OK;
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_PioTetrNum;
	param.reg = ADM2IFnr_STATUS;
	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
//	do	{
//		pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
//	} while(!pStatus->ByBits.CmdRdy);
	int i = 0;
	for(i = 0; i < 100; i++)
	{
		pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
		if(pStatus->ByBits.CmdRdy)
			break;
	} 
	if(i >= 100)
		Status = BRDerr_WAIT_TIMEOUT;
	else
	{
		param.reg = ADM2IFnr_DATA;
		param.val = *(PULONG)args;
		pModule->RegCtrl(DEVScmd_REGWRITEDIR, &param);
	}
	return Status;
}

//***************************************************************************************
int CPioSrv::CtrlFlagClear(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_PioTetrNum;
	param.reg = PIO_nrFLAGCLR;
	param.val = 0xFFFF;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CPioSrv::CtrlReadData(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   )
{
	CModule* pModule = (CModule*)pDev;
	ULONG Status = BRDerr_OK;

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_PioTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PPIO_MODE1 pMode1 = (PPIO_MODE1)&param.val;
	ULONG read_mode = pMode1->ByBits.RdMode;
	switch(read_mode)
	{
	case 0:
		{
			param.reg = ADM2IFnr_STATUS;
			PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
			int i = 0;
			for(i = 0; i < 100; i++)
			{
				pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
				if(pStatus->ByBits.CmdRdy)
					break;
			} 
			if(i >= 100)
				Status = BRDerr_WAIT_TIMEOUT;
			else
			{
				param.reg = PIO_nrSTARTRD;
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
				param.reg = ADM2IFnr_STATUS;
				for(i = 0; i < 100; i++)
				{
					pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
					if(pStatus->ByBits.CmdRdy)
						break;
				} 
				if(i >= 100)
					Status = BRDerr_WAIT_TIMEOUT;
			}
		}
		break;
	case 1:
		{
			param.reg = ADM2IFnr_STATUS;
			PPIO_STATUS pStatus = (PPIO_STATUS)&param.val;
			int i = 0;
			for(i = 0; i < 100; i++)
			{
				pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
				if(pStatus->ByBits.FAckRd)
					break;
			} 
			if(i >= 100)
				Status = BRDerr_WAIT_TIMEOUT;
		}
		break;
	case 2:
		break;
	}
	if(Status == BRDerr_OK)
	{
		param.reg = ADM2IFnr_DATA;
		pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
		*(PULONG)args = param.val;
		if(read_mode == 1)
		{
			param.reg = PIO_nrFLAGCLR;
			PPIO_STATUS pStatus = (PPIO_STATUS)&param.val;
			pStatus->ByBits.FAckRd = 1;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		}
	}
	return Status;
}

// ***************** End of file PioSrv.cpp ***********************
