/*
 ***************** File Gc5016Ctrl.cpp *******************************
 *
 * CTRL-command for BRD Driver for GC5016 service on ADMDDC5016 submodule
 *
 * (C) InSys by Sclyarov A. Nov 2006
 *
 **********************************************************************
*/

#include "module.h"
#include "gc5016srv.h"

#define	CURRFILE "GC5016CTRL"

CMD_Info SETMASTER_CMD		= { BRDctrl_GC5016_SETMASTER,		0, 0, 0, NULL};
CMD_Info GETMASTER_CMD		= { BRDctrl_GC5016_GETMASTER,		1, 0, 0, NULL};
CMD_Info SETCHANMASK_CMD	= { BRDctrl_GC5016_SETCHANMASK,	0, 0, 0, NULL};
CMD_Info GETCHANMASK_CMD	= { BRDctrl_GC5016_GETCHANMASK,	1, 0, 0, NULL};
CMD_Info SETCLKMODE_CMD		= { BRDctrl_GC5016_SETCLKMODE,		0, 0, 0, NULL};
CMD_Info GETCLKMODE_CMD		= { BRDctrl_GC5016_GETCLKMODE,		1, 0, 0, NULL};
CMD_Info SETSTARTMODE_CMD	= { BRDctrl_GC5016_SETSTARTMODE,	0, 0, 0, NULL};
CMD_Info GETSTARTMODE_CMD	= { BRDctrl_GC5016_GETSTARTMODE,	1, 0, 0, NULL};
CMD_Info FIFORESET_CMD		= { BRDctrl_GC5016_FIFORESET,	 0, 0, 0, NULL};
CMD_Info START_CMD			= { BRDctrl_GC5016_START,		 0, 0, 0, NULL};
CMD_Info STOP_CMD			= { BRDctrl_GC5016_STOP,		 0, 0, 0, NULL};
CMD_Info FIFOSTATUS_CMD		= { BRDctrl_GC5016_FIFOSTATUS,	 1, 0, 0, NULL};
CMD_Info GETDATA_CMD		= { BRDctrl_GC5016_GETDATA,	 0, 0, 0, NULL};
CMD_Info GETSRCSTREAM_CMD	= { BRDctrl_GC5016_GETSRCSTREAM, 1, 0, 0, NULL};
CMD_Info SETTARGET_CMD		= { BRDctrl_GC5016_SETTARGET,		0, 0, 0, NULL};
CMD_Info GETTARGET_CMD		= { BRDctrl_GC5016_GETTARGET,		1, 0, 0, NULL};
CMD_Info SETTESTMODE_CMD	= { BRDctrl_GC5016_SETTESTMODE,	0, 0, 0, NULL};
CMD_Info GETTESTMODE_CMD	= { BRDctrl_GC5016_GETTESTMODE,	1, 0, 0, NULL};
CMD_Info GCSETREG_CMD		= { BRDctrl_GC5016_GCSETREG, 0, 0, 0, NULL};
CMD_Info GCGETREG_CMD		= { BRDctrl_GC5016_GCGETREG, 1, 0, 0, NULL};
CMD_Info SETADCGAIN_CMD		= { BRDctrl_GC5016_SETADCGAIN, 0, 0, 0, NULL};
CMD_Info GETADCOVER_CMD		= { BRDctrl_GC5016_GETADCOVER, 0, 0, 0, NULL};
CMD_Info ADCENABLE_CMD		= { BRDctrl_GC5016_ADCENABLE, 0, 0, 0, NULL};
CMD_Info SETDDCFC_CMD		= { BRDctrl_GC5016_SETDDCFC, 0, 0, 0, NULL};
CMD_Info SETADMMODE_CMD		= { BRDctrl_GC5016_SETADMMODE, 0, 0, 0, NULL};
CMD_Info SETDDCMODE_CMD		= { BRDctrl_GC5016_SETDDCMODE, 0, 0, 0, NULL};
CMD_Info SETDDCSYNC_CMD		= { BRDctrl_GC5016_SETDDCSYNC, 0, 0, 0, NULL};
CMD_Info PROGRAMDDC_CMD		= { BRDctrl_GC5016_PROGRAMDDC, 0, 0, 0, NULL};
CMD_Info STARTDDC_CMD		= { BRDctrl_GC5016_STARTDDC, 0, 0, 0, NULL};
CMD_Info STOPDDC_CMD		= { BRDctrl_GC5016_STOPDDC, 0, 0, 0, NULL};
CMD_Info SETSTDELAY_CMD		= { BRDctrl_GC5016_SETSTDELAY,		0, 0, 0, NULL};
CMD_Info GETSTDELAY_CMD		= { BRDctrl_GC5016_GETSTDELAY,		1, 0, 0, NULL};
CMD_Info SETACQDATA_CMD		= { BRDctrl_GC5016_SETACQDATA,		0, 0, 0, NULL};
CMD_Info GETACQDATA_CMD		= { BRDctrl_GC5016_GETACQDATA,		1, 0, 0, NULL};
CMD_Info SETSKIPDATA_CMD	= { BRDctrl_GC5016_SETSKIPDATA,	0, 0, 0, NULL};
CMD_Info GETSKIPDATA_CMD	= { BRDctrl_GC5016_GETSKIPDATA,	1, 0, 0, NULL};
CMD_Info SETTITLEMODE_CMD	= { BRDctrl_GC5016_SETTITLEMODE,	0, 0, 0, NULL};
CMD_Info GETTITLEMODE_CMD	= { BRDctrl_GC5016_GETTITLEMODE,	1, 0, 0, NULL};
CMD_Info SETTLDATA_CMD		= { BRDctrl_GC5016_SETTITLEDATA, 0, 0, 0, NULL};
CMD_Info GETTLDATA_CMD		= { BRDctrl_GC5016_GETTITLEDATA, 1, 0, 0, NULL};
CMD_Info SETSPECIFIC_CMD	= { BRDctrl_GC5016_SETSPECIFIC, 0, 0, 0, NULL};
CMD_Info GETSPECIFIC_CMD	= { BRDctrl_GC5016_GETSPECIFIC, 1, 0, 0, NULL};
CMD_Info GETCFG_CMD			= { BRDctrl_GC5016_GETCFG, 1, 0, 0, NULL};
CMD_Info PROGRAMDDCEXT_CMD	= { BRDctrl_GC5016_PROGRAMDDCEXT, 0, 0, 0, NULL};
CMD_Info STARTDDCEXT_CMD	= { BRDctrl_GC5016_STARTDDCEXT, 0, 0, 0, NULL};
CMD_Info STOPDDCEXT_CMD		= { BRDctrl_GC5016_STOPDDCEXT, 0, 0, 0, NULL};
CMD_Info SETDDCFCEXT_CMD	= { BRDctrl_GC5016_SETDDCFCEXT, 0, 0, 0, NULL};
CMD_Info SETFLT_CMD			= { BRDctrl_GC5016_SETFLT, 0, 0, 0, NULL};
CMD_Info SETFFT_CMD			= { BRDctrl_GC5016_SETFFT, 0, 0, 0, NULL};
CMD_Info SETFRAME_CMD		= { BRDctrl_GC5016_SETFRAME, 0, 0, 0, NULL};
CMD_Info SETNCHANS_CMD		= { BRDctrl_GC5016_SETNCHANS, 0, 0, 0, NULL};

//***************************************************************************************
CGc5016Srv::CGc5016Srv(int idx, int srv_num, CModule* pModule, PGC5016SRV_CFG pCfg) :
	CService(idx, _BRDC("GC5016"), srv_num, pModule, pCfg, sizeof(PGC5016SRV_CFG))
{
	m_attribute = 
			BRDserv_ATTR_DIRECTION_IN |
			BRDserv_ATTR_STREAMABLE_IN |
			BRDserv_ATTR_SDRAMABLE |
			BRDserv_ATTR_CMPABLE |
//			BRDserv_ATTR_DSPABLE |
			BRDserv_ATTR_EXCLUSIVE_ONLY;
	

	Init(&SETMASTER_CMD, (CmdEntry)&CGc5016Srv::CtrlMaster);
	Init(&GETMASTER_CMD, (CmdEntry)&CGc5016Srv::CtrlMaster);
	Init(&SETCLKMODE_CMD, (CmdEntry)&CGc5016Srv::CtrlClkMode);
	Init(&GETCLKMODE_CMD, (CmdEntry)&CGc5016Srv::CtrlClkMode);
	Init(&SETCHANMASK_CMD, (CmdEntry)&CGc5016Srv::CtrlChanMask);
	Init(&GETCHANMASK_CMD, (CmdEntry)&CGc5016Srv::CtrlChanMask);
	Init(&SETSTARTMODE_CMD, (CmdEntry)&CGc5016Srv::CtrlStartMode);
	Init(&GETSTARTMODE_CMD, (CmdEntry)&CGc5016Srv::CtrlStartMode);
	Init(&FIFORESET_CMD, (CmdEntry)&CGc5016Srv::CtrlFifoReset);
	Init(&START_CMD, (CmdEntry)&CGc5016Srv::CtrlStart);
	Init(&STOP_CMD, (CmdEntry)&CGc5016Srv::CtrlStart);
	Init(&FIFOSTATUS_CMD, (CmdEntry)&CGc5016Srv::CtrlFifoStatus);
	Init(&GETDATA_CMD, (CmdEntry)&CGc5016Srv::CtrlGetData);
	Init(&GETSRCSTREAM_CMD, (CmdEntry)&CGc5016Srv::CtrlGetAddrData);
	Init(&SETTARGET_CMD, (CmdEntry)&CGc5016Srv::CtrlTarget);
	Init(&GETTARGET_CMD, (CmdEntry)&CGc5016Srv::CtrlTarget);
	Init(&SETTESTMODE_CMD, (CmdEntry)&CGc5016Srv::CtrlTestMode);
	Init(&GETTESTMODE_CMD, (CmdEntry)&CGc5016Srv::CtrlTestMode);
	Init(&GCSETREG_CMD, (CmdEntry)&CGc5016Srv::CtrlGcSetReg);
	Init(&GCGETREG_CMD, (CmdEntry)&CGc5016Srv::CtrlGcGetReg);
	Init(&SETADCGAIN_CMD, (CmdEntry)&CGc5016Srv::CtrlSetAdcGain);
	Init(&GETADCOVER_CMD, (CmdEntry)&CGc5016Srv::CtrlGetAdcOver);
	Init(&ADCENABLE_CMD, (CmdEntry)&CGc5016Srv::CtrlAdcEnable);
	Init(&SETDDCFC_CMD, (CmdEntry)&CGc5016Srv::CtrlSetDdcFc);
	Init(&SETADMMODE_CMD, (CmdEntry)&CGc5016Srv::CtrlSetAdmMode);
	Init(&SETDDCMODE_CMD, (CmdEntry)&CGc5016Srv::CtrlSetDdcMode);
	Init(&SETDDCSYNC_CMD, (CmdEntry)&CGc5016Srv::CtrlSetDdcSync);
	Init(&PROGRAMDDC_CMD, (CmdEntry)&CGc5016Srv::CtrlProgramDdc);
	Init(&STARTDDC_CMD, (CmdEntry)&CGc5016Srv::CtrlStartDdc);
	Init(&STOPDDC_CMD, (CmdEntry)&CGc5016Srv::CtrlStopDdc);
	Init(&SETSTDELAY_CMD, (CmdEntry)&CGc5016Srv::CtrlCnt);
	Init(&GETSTDELAY_CMD, (CmdEntry)&CGc5016Srv::CtrlCnt);
	Init(&SETACQDATA_CMD, (CmdEntry)&CGc5016Srv::CtrlCnt);
	Init(&GETACQDATA_CMD, (CmdEntry)&CGc5016Srv::CtrlCnt);
	Init(&SETSKIPDATA_CMD, (CmdEntry)&CGc5016Srv::CtrlCnt);
	Init(&GETSKIPDATA_CMD, (CmdEntry)&CGc5016Srv::CtrlCnt);
	Init(&SETTITLEMODE_CMD, (CmdEntry)&CGc5016Srv::CtrlTitleMode);
	Init(&GETTITLEMODE_CMD, (CmdEntry)&CGc5016Srv::CtrlTitleMode);
	Init(&SETTLDATA_CMD, (CmdEntry)&CGc5016Srv::CtrlTitleData);
	Init(&GETTLDATA_CMD, (CmdEntry)&CGc5016Srv::CtrlTitleData);
	Init(&SETSPECIFIC_CMD, (CmdEntry)&CGc5016Srv::CtrlSpecific);
	Init(&GETSPECIFIC_CMD, (CmdEntry)&CGc5016Srv::CtrlSpecific);
	Init(&GETCFG_CMD, (CmdEntry)&CGc5016Srv::CtrlGetCfg);
	Init(&PROGRAMDDCEXT_CMD, (CmdEntry)&CGc5016Srv::CtrlProgramDdcExt);
	Init(&STARTDDCEXT_CMD, (CmdEntry)&CGc5016Srv::CtrlStartDdcExt);
	Init(&STOPDDCEXT_CMD, (CmdEntry)&CGc5016Srv::CtrlStopDdcExt);
	Init(&SETDDCFCEXT_CMD, (CmdEntry)&CGc5016Srv::CtrlSetDdcFcExt);
	Init(&SETFLT_CMD, (CmdEntry)&CGc5016Srv::CtrlSetFlt);
	Init(&SETFFT_CMD, (CmdEntry)&CGc5016Srv::CtrlSetFft);
	Init(&SETFRAME_CMD, (CmdEntry)&CGc5016Srv::CtrlSetFrame);
	Init(&SETNCHANS_CMD, (CmdEntry)&CGc5016Srv::CtrlSetNchans);
}

//***************************************************************************************
int CGc5016Srv::CtrlMaster(
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
	if(BRDctrl_GC5016_SETMASTER == cmd)
	{
		param.tetr = m_GcTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pMode0->ByBits.Master = *pMasterMode & 0x1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pMode0->ByBits.Master = *pMasterMode >> 1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		if(pMode0->ByBits.Master)
		{
			pMode0->ByBits.AdmClk=0;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		    param.reg = ADM2IFnr_FMODE;
		    param.val =12 |0x8000;
//		    param.val =7 |0x8000;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		    param.reg = ADM2IFnr_FDIV;
		    param.val =1;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		}
	}
	else
	{
		param.tetr = m_GcTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		*pMasterMode = pMode0->ByBits.Master;
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		*pMasterMode |= (pMode0->ByBits.Master << 1);
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::CtrlChanMask(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_GC5016_SETCHANMASK == cmd)
	{
		ULONG mask = *(ULONG*)args;
		Status = SetChanMask(pModule, mask);
	}
	else
	{
		ULONG mask;
		Status = GetChanMask(pModule, mask);
		*(ULONG*)args = mask;
	}
	return Status;
}


//***************************************************************************************
int CGc5016Srv::CtrlClkMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ClkMode pClkMode = (PBRD_ClkMode)args;
	if(BRDctrl_GC5016_SETCLKMODE == cmd)
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
int CGc5016Srv::CtrlStartMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_GC5016_SETSTARTMODE == cmd)
		Status = SetStartMode(pModule, args);
	else
        Status = GetStartMode(pModule, args);
	return Status;
}

//***************************************************************************************

//***************************************************************************************
int CGc5016Srv::CtrlFifoReset (
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.FifoRes = 1;
	pMode0->ByBits.Reset = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//Sleep(1);
	pMode0->ByBits.FifoRes = 0;
	pMode0->ByBits.Reset = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//Sleep(1);
	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::CtrlStart (
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
//	ULONG start;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(BRDctrl_GC5016_START == cmd)
	{
		pMode0->ByBits.Start = 1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		if(!pMode0->ByBits.Master)
		{ // Master/Slave
			param.tetr = m_MainTetrNum;
			pModule->RegCtrl(DEVScmd_REGREADIND, &param);
			if(pMode0->ByBits.Master)
			{ // Master
				pMode0->ByBits.Start = 1;
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			}
		}
	}else
	{
//		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pMode0->ByBits.Start = 0;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
//		if(!pMode0->ByBits.Master)
		{ // Master/Slave
			param.tetr = m_MainTetrNum;
			pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//			if(pMode0->ByBits.Master)
			{ // Master
				pMode0->ByBits.Start = 0;
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			}
		}
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::CtrlFifoStatus(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = ADM2IFnr_STATUS;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
	ULONG data = pStatus->AsWhole;
	*(PULONG)args = data;
	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::CtrlGetData(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;

	param.reg = ADM2IFnr_DATA;
	PBRD_DataBuf pBuf = (PBRD_DataBuf)args;
	param.pBuf = pBuf->pData;
	param.bytes = pBuf->size;
	pModule->RegCtrl(DEVScmd_REGREADSDIR, &param);

//	PULONG buf = (PULONG)pBuf->pData;
//	for(ULONG i = 0; i < pBuf->size / sizeof(ULONG); i++)
//	while(1)
	{
//		buf[i]=0x2000;
//			param.val = buf[i];
//		param.val = buf[0];
//		pModule->RegCtrl(DEVScmd_REGWRITEDIR, &param);
	}
	
	//param.reg = ADM2IFnr_STATUS;
	//pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//pStatus = (PADM2IF_STATUS)&param.val;

	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::CtrlIsAvailable(
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
	GetGcTetrNum(pModule);
	
	if(m_MainTetrNum != -1 && m_GcTetrNum != -1)
	{
		m_isAvailable = 1;
		PGC5016SRV_CFG pDacSrvCfg = (PGC5016SRV_CFG)m_pConfig;
		if(!pDacSrvCfg->isAlreadyInit)
		{
			pDacSrvCfg->isAlreadyInit = 1;

			DEVS_CMD_Reg RegParam;
			RegParam.idxMain = m_index;
			RegParam.tetr = m_GcTetrNum;
			if(!pDacSrvCfg->FifoSize)
			{
				RegParam.reg = ADM2IFnr_FTYPE;
				pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
				int widthFifo = RegParam.val >> 3;
				RegParam.reg = ADM2IFnr_FSIZE;
				pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
	//			pDacSrvCfg->FifoSize = RegParam.val << 3;
				pDacSrvCfg->FifoSize = RegParam.val * widthFifo;
			}

			RegParam.tetr = m_GcTetrNum;
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
	//		pMode0->ByBits.AdmClk = 1;
			pMode0->ByBits.Reset = 0;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

		}
		DEVS_CMD_Reg RegParam;
		RegParam.idxMain = m_index;
		RegParam.tetr = m_GcTetrNum;
		RegParam.reg = GC5016nr_ADMCONST;
		pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
		PDDC_CONST adm_const = (PDDC_CONST)&RegParam.val;
		m_AdmConst.AsWhole = adm_const->AsWhole & 0xffff;

	}
	else
		m_isAvailable = 0;

	pServAvailable->isAvailable = m_isAvailable;
	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::CtrlGetAddrData(
							void			*pDev,		// InfoSM or InfoBM
							void			*pServData,	// Specific Service Data
							ULONG			cmd,		// Command Code (from BRD_ctrl())
							void			*args 		// Command Arguments (from BRD_ctrl())
							)
{
//	CModule* pModule = (CModule*)pDev;
//	*(ULONG*)args = (m_AdmNum << 16) | m_QmTetrNum;
	*(ULONG*)args = m_GcTetrNum;
	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::CtrlTarget(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG target = *(ULONG*)args;
	if(BRDctrl_GC5016_SETTARGET == cmd)
	{
		Status = SetTarget(pModule, target);
	}
	else
	{
		Status = GetTarget(pModule, target);
		*(ULONG*)args = target;
	}
	return Status;
}
//***************************************************************************************
int CGc5016Srv::CtrlTestMode(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG mode = *(PULONG)args;
	if(BRDctrl_GC5016_SETTESTMODE == cmd)
		Status = SetTestMode(pModule, mode);
	else
		Status = GetTestMode(pModule, mode);
	*(PULONG)args = mode;
	return Status;
}
//***************************************************************************************
int CGc5016Srv::CtrlGcSetReg(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status = WriteGcReg(pModule,args);

	return Status;
}

//***************************************************************************************
int CGc5016Srv::CtrlGcGetReg(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status = ReadGcReg(pModule,args);

	return Status;
}
//***************************************************************************************
int CGc5016Srv::CtrlSetAdcGain(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status = SetAdcGain(pModule,args);

	return Status;
}

//***************************************************************************************
//***************************************************************************************
int CGc5016Srv::CtrlGetAdcOver(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status = GetAdcOver(pModule,args);
	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::CtrlAdcEnable(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status =AdcEnable( pModule,args);
	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::CtrlSetDdcFc(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status =SetDdcFc( pModule,args);
	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::CtrlSetDdcFcExt(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status =SetDdcFcExt( pModule,args);
	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::CtrlSetAdmMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status =SetAdmMode( pModule,args);
	return BRDerr_OK;
}
//***************************************************************************************

int CGc5016Srv::CtrlSetDdcMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status =SetDdcMode( pModule,args);
	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::CtrlSetDdcSync(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status = SetDdcSync(pModule,args);

	return Status;
}
//***************************************************************************************
//***************************************************************************************
int CGc5016Srv::CtrlProgramDdc(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status = ProgramDdc(pModule,args);

	return Status;
}
//***************************************************************************************
//***************************************************************************************
int CGc5016Srv::CtrlProgramDdcExt(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status = ProgramDdcExt(pModule,args);

	return Status;
}
//***************************************************************************************
int CGc5016Srv::CtrlStartDdc(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status =StartDdc( pModule);
	return BRDerr_OK;
}
//***************************************************************************************
//***************************************************************************************
int CGc5016Srv::CtrlStopDdc(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status =StopDdc( pModule);
	return BRDerr_OK;
}
//***************************************************************************************
//***************************************************************************************
int CGc5016Srv::CtrlStartDdcExt(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				)
{
	U32	mask= *(U32*)args;
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status =StartDdcExt( pModule,mask);
	return BRDerr_OK;
}
//***************************************************************************************
//***************************************************************************************
int CGc5016Srv::CtrlStopDdcExt(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				)
{
	U32	mask= *(U32*)args;
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status =StopDdcExt( pModule,mask);
	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::CtrlCnt(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   )
{
	CModule* pModule = (CModule*)pDev;
	PBRD_EnVal pEnVal = (PBRD_EnVal)args;
	switch(cmd)
	{
	case BRDctrl_GC5016_SETSTDELAY:
		SetCnt(pModule, 0, pEnVal);
		break;
	case BRDctrl_GC5016_GETSTDELAY:
		GetCnt(pModule, 0, pEnVal);
		break;
	case BRDctrl_GC5016_SETACQDATA:
		SetCnt(pModule, 1, pEnVal);
		break;
	case BRDctrl_GC5016_GETACQDATA:
		GetCnt(pModule, 1, pEnVal);
		break;
	case BRDctrl_GC5016_SETSKIPDATA:
		SetCnt(pModule, 2, pEnVal);
		break;
	case BRDctrl_GC5016_GETSKIPDATA:
		GetCnt(pModule, 2, pEnVal);
		break;
	}
	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::CtrlTitleMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_EnVal pTitleMode = (PBRD_EnVal)args;
	if(BRDctrl_GC5016_SETTITLEMODE == cmd)
		Status = SetTitleMode(pModule, pTitleMode);
	else
        Status = GetTitleMode(pModule, pTitleMode);
	return Status;
}

//***************************************************************************************
int CGc5016Srv::CtrlTitleData(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PULONG pTitleData = (PULONG)args;
	if(BRDctrl_GC5016_SETTITLEDATA == cmd)
		Status = SetTitleData(pModule, pTitleData);
	else
        Status = GetTitleData(pModule, pTitleData);
	return Status;
}
//***************************************************************************************
int CGc5016Srv::CtrlSpecific(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_GC5016Spec pSpec = (PBRD_GC5016Spec)args;
	if(BRDctrl_GC5016_SETSPECIFIC == cmd)
		Status = SetSpecific(pModule, pSpec);
	else
		Status = GetSpecific(pModule, pSpec);
	return Status;
}

//***************************************************************************************
int CGc5016Srv::CtrlGetCfg(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	PBRD_GC5016Cfg pCfg = (PBRD_GC5016Cfg)args;
	Status = GetCfg(pCfg);
	return Status;
}
//***************************************************************************************
int CGc5016Srv::CtrlSetFlt(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_SetFlt pSetFlt = (PBRD_SetFlt)args;
		Status = SetFlt(pModule, pSetFlt);
	return Status;
}

//***************************************************************************************
//***************************************************************************************
int CGc5016Srv::CtrlSetFft(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_SetFft pSetFft = (PBRD_SetFft)args;
		Status = SetFft(pModule, pSetFft);
	return Status;
}
//***************************************************************************************
int CGc5016Srv::CtrlSetFrame(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_SetFrame pSetFrame = (PBRD_SetFrame)args;
		Status = SetFrame(pModule, pSetFrame);
	return Status;
}

//***************************************************************************************
int CGc5016Srv::CtrlSetNchans(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status = SetNchans(pModule, args);
	return Status;
}

//***************************************************************************************
// ***************** End of file Gc5016Ctrl.cpp ***********************
