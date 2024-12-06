/*
 ***************** File DdsCtrl.cpp *******************************
 *
 * CTRL-command for BRD Driver for Dds service on base
 *
 * (C) InSys by Sclyarov A. Mar 2007
 *
 **********************************************************************
*/

#include "module.h"
#include "ddssrv.h"


#define	CURRFILE _BRDC("DDSCTRL")

static CMD_Info SETMASTER_CMD		= { BRDctrl_DDS_SETMASTER,		0, 0, 0, NULL};
static CMD_Info GETMASTER_CMD		= { BRDctrl_DDS_GETMASTER,		1, 0, 0, NULL};
static CMD_Info SETCLKMODE_CMD		= { BRDctrl_DDS_SETCLKMODE,		0, 0, 0, NULL};
static CMD_Info GETCLKMODE_CMD		= { BRDctrl_DDS_GETCLKMODE,		1, 0, 0, NULL};
static CMD_Info SETOUTFREQ_CMD		= { BRDctrl_DDS_SETOUTFREQ,		0, 0, 0, NULL};
static CMD_Info GETOUTFREQ_CMD		= { BRDctrl_DDS_GETOUTFREQ,		1, 0, 0, NULL};
static CMD_Info SETSTBMODE_CMD		= { BRDctrl_DDS_SETSTBMODE,		0, 0, 0, NULL};
static CMD_Info GETSTBMODE_CMD		= { BRDctrl_DDS_GETSTBMODE,		1, 0, 0, NULL};
static CMD_Info STARTSTROBE_CMD		= { BRDctrl_DDS_STARTSTROBE,	0, 0, 0, NULL};
static CMD_Info STOPSTROBE_CMD		= { BRDctrl_DDS_STOPSTROBE,		0, 0, 0, NULL};
static CMD_Info ENABLE_CMD			= { BRDctrl_DDS_ENABLE,			0, 0, 0, NULL};
static CMD_Info DISABLE_CMD			= { BRDctrl_DDS_DISABLE,		0, 0, 0, NULL};
static CMD_Info SETSTARTMODE_CMD	= { BRDctrl_DDS_SETSTARTMODE,	0, 0, 0, NULL};
static CMD_Info GETSTARTMODE_CMD	= { BRDctrl_DDS_GETSTARTMODE,	1, 0, 0, NULL};
static CMD_Info GETSTATUS_CMD		= { BRDctrl_DDS_GETSTATUS,		1, 0, 0, NULL};
static CMD_Info SETSTARTCLK_CMD		= { BRDctrl_DDS_SETSTARTCLK,	0, 0, 0, NULL};
static CMD_Info GETSTARTCLK_CMD		= { BRDctrl_DDS_GETSTARTCLK,	1, 0, 0, NULL};
static CMD_Info SETMUXOUT_CMD		= { BRDctrl_DDS_SETMUXOUT,	0, 0, 0, NULL};
static CMD_Info GETMUXOUT_CMD		= { BRDctrl_DDS_GETMUXOUT,	1, 0, 0, NULL};
static CMD_Info SETDDSPOWER_CMD		= { BRDctrl_DDS_SETDDSPOWER,	0, 0, 0, NULL};
static CMD_Info GETDDSPOWER_CMD		= { BRDctrl_DDS_GETDDSPOWER,	1, 0, 0, NULL};

//***************************************************************************************

CDdsSrv::CDdsSrv(int idx, int srv_num, CModule* pModule, PDDSSRV_CFG pCfg) :
	CService(idx, _BRDC("BASEDDS"), srv_num, pModule, pCfg, sizeof(DDSSRV_CFG))
{
	m_attribute = BRDserv_ATTR_CMPABLE | BRDserv_ATTR_EXCLUSIVE_ONLY;
	
	Init(&SETMASTER_CMD, (CmdEntry)&CDdsSrv::CtrlMaster);
	Init(&GETMASTER_CMD, (CmdEntry)&CDdsSrv::CtrlMaster);
	Init(&SETCLKMODE_CMD, (CmdEntry)&CDdsSrv::CtrlClkMode);
	Init(&GETCLKMODE_CMD, (CmdEntry)&CDdsSrv::CtrlClkMode);
	Init(&SETOUTFREQ_CMD, (CmdEntry)&CDdsSrv::CtrlOutFreq);
	Init(&GETOUTFREQ_CMD, (CmdEntry)&CDdsSrv::CtrlOutFreq);
	Init(&SETSTBMODE_CMD, (CmdEntry)&CDdsSrv::CtrlStbMode);
	Init(&GETSTBMODE_CMD, (CmdEntry)&CDdsSrv::CtrlStbMode);
	Init(&STARTSTROBE_CMD,(CmdEntry)&CDdsSrv::CtrlStartStopStb);
	Init(&STOPSTROBE_CMD, (CmdEntry)&CDdsSrv::CtrlStartStopStb);
	Init(&ENABLE_CMD,	  (CmdEntry)&CDdsSrv::CtrlEnableDisable);
	Init(&DISABLE_CMD,	  (CmdEntry)&CDdsSrv::CtrlEnableDisable);
	Init(&SETSTARTMODE_CMD, (CmdEntry)&CDdsSrv::CtrlStartMode);
	Init(&GETSTARTMODE_CMD, (CmdEntry)&CDdsSrv::CtrlStartMode);
	Init(&GETSTATUS_CMD,    (CmdEntry)&CDdsSrv::CtrlGetStatus);
	Init(&SETSTARTCLK_CMD, (CmdEntry)&CDdsSrv::CtrlStartClk);
	Init(&GETSTARTCLK_CMD, (CmdEntry)&CDdsSrv::CtrlStartClk);
	Init(&SETMUXOUT_CMD, (CmdEntry)&CDdsSrv::CtrlMuxOut);
	Init(&GETMUXOUT_CMD, (CmdEntry)&CDdsSrv::CtrlMuxOut);
	Init(&SETDDSPOWER_CMD, (CmdEntry)&CDdsSrv::CtrlDdsPower);
	Init(&GETDDSPOWER_CMD, (CmdEntry)&CDdsSrv::CtrlDdsPower);

}

//***************************************************************************************
int CDdsSrv::CtrlMaster(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   )
{
	CModule* pModule = (CModule*)pDev;
	ULONG* pMasterMode = (ULONG*)args;
	DEVS_CMD_Reg param;
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	param.idxMain = m_index;
	param.reg = ADM2IFnr_MODE0;
	if(BRDctrl_DDS_SETMASTER == cmd)
	{
		param.tetr = m_DdsTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pMode0->ByBits.Master = *pMasterMode & 0x1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pMode0->ByBits.Master = *pMasterMode >> 1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		param.tetr = m_DdsTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		*pMasterMode = pMode0->ByBits.Master;
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		*pMasterMode |= (pMode0->ByBits.Master << 1);
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDdsSrv::CtrlClkMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ClkMode pClkMode = (PBRD_ClkMode)args;
	if(BRDctrl_DDS_SETCLKMODE == cmd)
	{
		Status = SetClkMode(pModule, pClkMode);
	}
	else
	{
		
		Status = GetClkMode(pModule, pClkMode);
	}
	return Status;
}

//***************************************************************************************
int CDdsSrv::CtrlOutFreq(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_OutFreq pOutFreq = (PBRD_OutFreq)args;
	double *pFreq=&pOutFreq->Freq;
	U32 *pScale=&pOutFreq->CPCurrentScale;
	U32 *pDividerM=&pOutFreq->DividerM;
	if(BRDctrl_DDS_SETOUTFREQ == cmd)
	{
		Status = SetOutFreq(pModule, pFreq,(int*)pScale,(int*)pDividerM);
	}
	else
	{
		if(m_DdsVer)
			Status = GetOutFreq_V2(pModule, pFreq,(int*)pScale,(int*)pDividerM);
		else
			Status = GetOutFreq(pModule, pFreq,(int*)pScale,(int*)pDividerM);
	}
	return Status;
}

//***************************************************************************************
int CDdsSrv::CtrlStbMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_StbMode pStbMode = (PBRD_StbMode)args;
	if(BRDctrl_DDS_SETSTBMODE == cmd)
	{
		Status = SetStbMode(pModule, pStbMode);
	}
	else
	{
		
		Status = GetStbMode(pModule, pStbMode);
	}
	return Status;
}

//***************************************************************************************
int CDdsSrv::CtrlStartStopStb(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_DDS_STARTSTROBE == cmd)
	{
		Status = StartStb(pModule);
	}
	else
	{
		
		Status = StopStb(pModule);
	}
	return Status;
}

//***************************************************************************************
//***************************************************************************************
int CDdsSrv::CtrlEnableDisable (
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_DDS_ENABLE == cmd)
	{
		Status = EnableDds(pModule);
	}
	else
	{
		
		Status = DisableDds(pModule);
	}
	return Status;
}

//***************************************************************************************
int CDdsSrv::CtrlStartMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_DDS_SETSTARTMODE == cmd)
		Status = SetStartMode(pModule, args);
	else
        Status = GetStartMode(pModule, args);
	return Status;
}
//***************************************************************************************
int CDdsSrv::CtrlStartClk(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	if(m_DdsVer)
	{
		U32* pClkSel = (U32*)args;
		CModule* pModule = (CModule*)pDev;
		if(BRDctrl_DDS_SETSTARTCLK == cmd)
			Status = SetStartClk(pModule, pClkSel);
		else
			Status = GetStartClk(pModule, pClkSel);
	}
	return Status;
}
//***************************************************************************************
int CDdsSrv::CtrlMuxOut(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	if(m_DdsVer)
	{
		U32* pMuxOut = (U32*)args;
		CModule* pModule = (CModule*)pDev;
		if(BRDctrl_DDS_SETMUXOUT == cmd)
			Status = SetMuxOut(pModule, pMuxOut);
		else
			Status = GetMuxOut(pModule, pMuxOut);
	}
	return Status;
}
//***************************************************************************************
int CDdsSrv::CtrlDdsPower(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	if(m_DdsVer)
	{
		U32* pPower = (U32*)args;
		CModule* pModule = (CModule*)pDev;
		if(BRDctrl_DDS_SETDDSPOWER == cmd)
			Status = SetDdsPower(pModule, pPower);
		else
			Status = GetDdsPower(pModule, pPower);
	}
	return Status;
}

//***************************************************************************************
int CDdsSrv::CtrlGetStatus(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg = ADM2IFnr_STATUS;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
	ULONG data = pStatus->AsWhole;
	*(PULONG)args = data;
	return BRDerr_OK;
}

//***************************************************************************************
int CDdsSrv::CtrlRelease(
						void			*pDev,		// InfoSM or InfoBM
						void			*pServData,	// Specific Service Data
						ULONG			cmd,		// Command Code (from BRD_ctrl())
						void			*args 		// Command Arguments (from BRD_ctrl())
						)
{
	PDDSSRV_CFG pDdsSrvCfg = (PDDSSRV_CFG)m_pConfig;
	CModule* pModule = (CModule*)pDev;
	U32 power = 0;
	SetDdsPower(pModule, &power);
	pDdsSrvCfg->isAlreadyInit = 0;
	return BRDerr_CMD_UNSUPPORTED; // for free subservice
}

//***************************************************************************************
int CDdsSrv::CtrlIsAvailable(
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
	m_DdsTetrNum = GetTetrNum(pModule, m_index, BASEDDS_TETR_ID);
	m_DdsVer = 0; 
	if(m_DdsTetrNum == -1)
	{
		m_DdsTetrNum = GetTetrNum(pModule, m_index, BDDSV2_TETR_ID);
		m_DdsVer++; 
	}
//	printf("\n m_DdsTetrNum=%d\n",m_DdsTetrNum);
	if(m_MainTetrNum != -1 && m_DdsTetrNum != -1)
	{
		m_isAvailable = 1;
		//PDDSSRV_CFG pDdsSrvCfg = (PDDSSRV_CFG)m_pConfig;
		//if(!pDdsSrvCfg->isAlreadyInit)
		//{
//			pDdsSrvCfg->isAlreadyInit = 1;

			//DEVS_CMD_Reg RegParam;
			//RegParam.idxMain = m_index;
			//RegParam.tetr = m_DdsTetrNum;
			//RegParam.reg = ADM2IFnr_MODE0;
			//RegParam.val = 0;
			//PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&RegParam.val;
			//pMode0->ByBits.Reset = 1;
			//pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			//for(int i = 1; i < 32; i++)
			//{
			//	RegParam.reg = i;
			//	RegParam.val = 0;
			//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			//}
			//RegParam.reg = ADM2IFnr_MODE0;
			//pMode0->ByBits.Reset = 0;
			//pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			//DisableDds(pModule);
			//EnableDds(pModule);
		//}
		//printf("CDdsSrv::CtrlIsAvailable!!\n");
	}
	else
		m_isAvailable = 0;

	pServAvailable->isAvailable = m_isAvailable;
	return BRDerr_OK;
}

//***************************************************************************************
int CDdsSrv::CtrlCapture(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	if(m_MainTetrNum != -1 && m_DdsTetrNum != -1)
	{
		PDDSSRV_CFG pDdsSrvCfg = (PDDSSRV_CFG)m_pConfig;
		if(!pDdsSrvCfg->isAlreadyInit)
		{
			pDdsSrvCfg->isAlreadyInit = 1;

			DEVS_CMD_Reg RegParam;
			RegParam.idxMain = m_index;
			RegParam.tetr = m_DdsTetrNum;
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
			U32 power = 1;
			SetDdsPower(pModule, &power);
			DisableDds(pModule);
			EnableDds(pModule);
			//printf("CDdsSrv::CtrlCapture!!\n");
		}
	}
	return BRDerr_OK;
}

// ***************** End of file DdsCtrl.cpp ***********************