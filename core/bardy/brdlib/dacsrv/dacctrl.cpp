/*
 ***************** File DacCtrl.cpp ************
 *
 * CTRL-command for BRD Driver for ADРЎ service on ADРЎ submodule
 *
 * (C) InSys by Dorokhin A. Apr 2004
 *
 ******************************************************
*/
#ifndef __linux__
#include <windows.h>
#include <winioctl.h>
#endif

#include "module.h"
#include "dacsrv.h"

#define	CURRFILE "DACCTRL"

static CMD_Info GETTRACETEXT_CMD	= { BRDctrl_GETTRACETEXT,    1, 0, 0, NULL};
static CMD_Info GETTRACEPARAM_CMD	= { BRDctrl_GETTRACEPARAM,   1, 0, 0, NULL};
static CMD_Info SETCHANMASK_CMD	= { BRDctrl_DAC_SETCHANMASK,	0, 0, 0, NULL};
static CMD_Info GETCHANMASK_CMD	= { BRDctrl_DAC_GETCHANMASK,	1, 0, 0, NULL};
static CMD_Info SETGAIN_CMD		= { BRDctrl_DAC_SETGAIN,		0, 0, 0, NULL};
static CMD_Info GETGAIN_CMD		= { BRDctrl_DAC_GETGAIN,		1, 0, 0, NULL};
static CMD_Info SETOUTRANGE_CMD	= { BRDctrl_DAC_SETOUTRANGE,	0, 0, 0, NULL};
static CMD_Info GETOUTRANGE_CMD	= { BRDctrl_DAC_GETOUTRANGE,	1, 0, 0, NULL};
static CMD_Info SETBIAS_CMD		= { BRDctrl_DAC_SETBIAS,		0, 0, 0, NULL};
static CMD_Info GETBIAS_CMD		= { BRDctrl_DAC_GETBIAS,		1, 0, 0, NULL};
static CMD_Info SETFORMAT_CMD	= { BRDctrl_DAC_SETFORMAT,		0, 0, 0, NULL};
static CMD_Info GETFORMAT_CMD	= { BRDctrl_DAC_GETFORMAT,		1, 0, 0, NULL};
static CMD_Info SETRATE_CMD		= { BRDctrl_DAC_SETRATE,		0, 0, 0, NULL};
static CMD_Info GETRATE_CMD		= { BRDctrl_DAC_GETRATE,		1, 0, 0, NULL};
static CMD_Info SETCLKMODE_CMD	= { BRDctrl_DAC_SETCLKMODE,		0, 0, 0, NULL};
static CMD_Info GETCLKMODE_CMD	= { BRDctrl_DAC_GETCLKMODE,		1, 0, 0, NULL};
static CMD_Info SETSYNCMODE_CMD	= { BRDctrl_DAC_SETSYNCMODE,	0, 0, 0, NULL};
static CMD_Info GETSYNCMODE_CMD	= { BRDctrl_DAC_GETSYNCMODE,	1, 0, 0, NULL};
static CMD_Info SETMASTER_CMD	= { BRDctrl_DAC_SETMASTER,		0, 0, 0, NULL};
static CMD_Info GETMASTER_CMD	= { BRDctrl_DAC_GETMASTER,		1, 0, 0, NULL};
static CMD_Info SETSTARTMODE_CMD= { BRDctrl_DAC_SETSTARTMODE,	0, 0, 0, NULL};
static CMD_Info GETSTARTMODE_CMD= { BRDctrl_DAC_GETSTARTMODE,	1, 0, 0, NULL};
static CMD_Info SETSTDELAY_CMD	= { BRDctrl_DAC_SETSTDELAY,		0, 0, 0, NULL};
static CMD_Info GETSTDELAY_CMD	= { BRDctrl_DAC_GETSTDELAY,		1, 0, 0, NULL};
static CMD_Info SETOUTDATA_CMD	= { BRDctrl_DAC_SETOUTDATA,		0, 0, 0, NULL};
static CMD_Info GETOUTDATA_CMD	= { BRDctrl_DAC_GETOUTDATA,		1, 0, 0, NULL};
static CMD_Info SETSKIPDATA_CMD	= { BRDctrl_DAC_SETSKIPDATA,	0, 0, 0, NULL};
static CMD_Info GETSKIPDATA_CMD	= { BRDctrl_DAC_GETSKIPDATA,	1, 0, 0, NULL};
static CMD_Info SETCYCLMODE_CMD	= { BRDctrl_DAC_SETCYCLMODE,	0, 0, 0, NULL};
static CMD_Info GETCYCLMODE_CMD	= { BRDctrl_DAC_GETCYCLMODE,	1, 0, 0, NULL};
static CMD_Info SETSOURCE_CMD	= { BRDctrl_DAC_SETSOURCE,		0, 0, 0, NULL};
static CMD_Info GETSOURCE_CMD	= { BRDctrl_DAC_GETSOURCE,		1, 0, 0, NULL};
static CMD_Info SELFCLBR_CMD	= { BRDctrl_DAC_SELFCLBR,		0, 0, 0, NULL};
static CMD_Info SETCODE_CMD		= { BRDctrl_DAC_SETCODE,		0, 0, 0, NULL};
static CMD_Info GETCODE_CMD		= { BRDctrl_DAC_GETCODE,		1, 0, 0, NULL};
static CMD_Info GETCFG_CMD		= { BRDctrl_DAC_GETCFG,			1, 0, 0, NULL};
static CMD_Info WRITEINIFILE_CMD= { BRDctrl_DAC_WRITEINIFILE,	1, 0, 0, NULL};
static CMD_Info READINIFILE_CMD	= { BRDctrl_DAC_READINIFILE,	0, 0, 0, NULL};
static CMD_Info SETTESTMODE_CMD	= { BRDctrl_DAC_SETTESTMODE,	0, 0, 0, NULL};
static CMD_Info GETTESTMODE_CMD	= { BRDctrl_DAC_GETTESTMODE,	1, 0, 0, NULL};

static CMD_Info WRDELAY_CMD			= { BRDctrl_DAC_WRDELAY,		0, 0, 0, NULL};
static CMD_Info RDDELAY_CMD			= { BRDctrl_DAC_RDDELAY,		1, 0, 0, NULL};
static CMD_Info RSTDELAY_CMD		= { BRDctrl_DAC_RSTDELAY,		0, 0, 0, NULL};

static CMD_Info SETSPECIFIC_CMD	= { BRDctrl_DAC_SETSPECIFIC,	0, 0, 0, NULL};
static CMD_Info GETSPECIFIC_CMD	= { BRDctrl_DAC_GETSPECIFIC,	1, 0, 0, NULL};

static CMD_Info GETDLGPAGES_CMD	= { BRDctrl_DAC_GETDLGPAGES,	0, 0, 0, NULL};
//CMD_Info ENDPAGESDLG_CMD	= { BRDctrl_DAC_ENDPAGESDLG,	0, 0, 0, NULL};
//CMD_Info SETPROPDLG_CMD		= { BRDctrl_DAC_SETPROPDLG,		0, 0, 0, NULL};
//CMD_Info GETPROPDLG_CMD		= { BRDctrl_DAC_GETPROPDLG,		1, 0, 0, NULL};

static CMD_Info FIFORESET_CMD	= { BRDctrl_DAC_FIFORESET,	 0, 0, 0, NULL};
static CMD_Info ENABLE_CMD		= { BRDctrl_DAC_ENABLE,		 0, 0, 0, NULL};
static CMD_Info PREPARESTART_CMD= { BRDctrl_DAC_PREPARESTART,0, 0, 0, NULL };
static CMD_Info FIFOSTATUS_CMD	= { BRDctrl_DAC_FIFOSTATUS,	 1, 0, 0, NULL};
static CMD_Info PUTDATA_CMD		= { BRDctrl_DAC_PUTDATA,	 0, 0, 0, NULL};
//CMD_Info ISBITSUNDERFLOW_CMD	= { BRDctrl_DAC_ISBITSUNDERFLOW, 1, 0, 0, NULL};
static CMD_Info GETSRCSTREAM_CMD= { BRDctrl_DAC_GETSRCSTREAM, 1, 0, 0, NULL};
static CMD_Info SETCLKREFMODE_CMD	= { BRDctrl_DAC_SETCLKREFMODE,		0, 0, 0, NULL};
static CMD_Info GETCLKREFMODE_CMD	= { BRDctrl_DAC_GETCLKREFMODE,		1, 0, 0, NULL};
static CMD_Info SETDIVCLK_CMD		= { BRDctrl_DAC_SETDIVCLK,		0, 0, 0, NULL};
static CMD_Info GETDIVCLK_CMD		= { BRDctrl_DAC_GETDIVCLK,		1, 0, 0, NULL};
static CMD_Info OUTSYNC_CMD		    = { BRDctrl_DAC_OUTSYNC,		0, 0, 0, NULL};

static CMD_Info SETINTERP_CMD		= { BRDctrl_DAC_SETINTERP,		0, 0, 0, NULL};
static CMD_Info GETINTERP_CMD		= { BRDctrl_DAC_GETINTERP,		1, 0, 0, NULL};

//***************************************************************************************
CDacSrv::CDacSrv(int idx, const BRDCHAR* name, int srv_num, CModule* pModule, PVOID pCfg, ULONG cfgSize) :
	CService(idx, name, srv_num, pModule, pCfg, cfgSize)
{
	m_attribute = 
			BRDserv_ATTR_DIRECTION_OUT |
			BRDserv_ATTR_STREAMABLE_OUT |
			BRDserv_ATTR_SDRAMABLE |
			BRDserv_ATTR_CMPABLE |
//			BRDserv_ATTR_DSPABLE |
			BRDserv_ATTR_EXCLUSIVE_ONLY;
	
	Init(&GETTRACETEXT_CMD, (CmdEntry)&CDacSrv::CtrlGetTraceText);
	Init(&GETTRACEPARAM_CMD, (CmdEntry)&CDacSrv::CtrlGetTraceParam);

	Init(&GETCFG_CMD, (CmdEntry)&CDacSrv::CtrlCfg);
	Init(&WRITEINIFILE_CMD, (CmdEntry)&CDacSrv::CtrlIniFile);
	Init(&READINIFILE_CMD, (CmdEntry)&CDacSrv::CtrlIniFile);

	Init(&GETDLGPAGES_CMD, (CmdEntry)&CDacSrv::CtrlGetPages);
	//Init(&ENDPAGESDLG_CMD, (CmdEntry)CtrlInitEndDlg);
	//Init(&SETPROPDLG_CMD, (CmdEntry)CtrlDlgProperty);
	//Init(&GETPROPDLG_CMD, (CmdEntry)CtrlDlgProperty);

	Init(&SETCHANMASK_CMD, (CmdEntry)&CDacSrv::CtrlChanMask);
	Init(&GETCHANMASK_CMD, (CmdEntry)&CDacSrv::CtrlChanMask);
	Init(&SETGAIN_CMD, (CmdEntry)&CDacSrv::CtrlGain);
	Init(&GETGAIN_CMD, (CmdEntry)&CDacSrv::CtrlGain);
	Init(&SETOUTRANGE_CMD, (CmdEntry)&CDacSrv::CtrlOutRange);
	Init(&GETOUTRANGE_CMD, (CmdEntry)&CDacSrv::CtrlOutRange);
	Init(&SETBIAS_CMD, (CmdEntry)&CDacSrv::CtrlBias);
	Init(&GETBIAS_CMD, (CmdEntry)&CDacSrv::CtrlBias);
	Init(&SETFORMAT_CMD, (CmdEntry)&CDacSrv::CtrlFormat);
	Init(&GETFORMAT_CMD, (CmdEntry)&CDacSrv::CtrlFormat);
	Init(&SETCODE_CMD, (CmdEntry)&CDacSrv::CtrlCode);
	Init(&GETCODE_CMD, (CmdEntry)&CDacSrv::CtrlCode);
	Init(&SETRATE_CMD, (CmdEntry)&CDacSrv::CtrlRate);
	Init(&GETRATE_CMD, (CmdEntry)&CDacSrv::CtrlRate);
	Init(&SETCLKMODE_CMD, (CmdEntry)&CDacSrv::CtrlClkMode);
	Init(&GETCLKMODE_CMD, (CmdEntry)&CDacSrv::CtrlClkMode);
	Init(&SETSYNCMODE_CMD, (CmdEntry)&CDacSrv::CtrlSyncMode);
	Init(&GETSYNCMODE_CMD, (CmdEntry)&CDacSrv::CtrlSyncMode);
	Init(&SETMASTER_CMD, (CmdEntry)&CDacSrv::CtrlMaster);
	Init(&GETMASTER_CMD, (CmdEntry)&CDacSrv::CtrlMaster);

	Init(&SELFCLBR_CMD, (CmdEntry)&CDacSrv::CtrlSelfClbr);

	Init(&SETCYCLMODE_CMD, (CmdEntry)&CDacSrv::CtrlCyclingMode);
	Init(&GETCYCLMODE_CMD, (CmdEntry)&CDacSrv::CtrlCyclingMode);

	Init(&SETTESTMODE_CMD, (CmdEntry)&CDacSrv::CtrlTestMode);
	Init(&GETTESTMODE_CMD, (CmdEntry)&CDacSrv::CtrlTestMode);
	Init(&SETSPECIFIC_CMD, (CmdEntry)&CDacSrv::CtrlSpecific);
	Init(&GETSPECIFIC_CMD, (CmdEntry)&CDacSrv::CtrlSpecific);

	Init(&SETSOURCE_CMD, (CmdEntry)&CDacSrv::CtrlSource);
	Init(&GETSOURCE_CMD, (CmdEntry)&CDacSrv::CtrlSource);

	Init(&SETSTARTMODE_CMD, (CmdEntry)&CDacSrv::CtrlStartMode);
	Init(&GETSTARTMODE_CMD, (CmdEntry)&CDacSrv::CtrlStartMode);

	Init(&WRDELAY_CMD, (CmdEntry)&CDacSrv::CtrlDelay);
	Init(&RDDELAY_CMD, (CmdEntry)&CDacSrv::CtrlDelay);
	Init(&RSTDELAY_CMD, (CmdEntry)&CDacSrv::CtrlDelay);

	Init(&SETSTDELAY_CMD, (CmdEntry)&CDacSrv::CtrlStDelay);
	Init(&GETSTDELAY_CMD, (CmdEntry)&CDacSrv::CtrlStDelay);
	Init(&SETOUTDATA_CMD, (CmdEntry)&CDacSrv::CtrlOutData);
	Init(&GETOUTDATA_CMD, (CmdEntry)&CDacSrv::CtrlOutData);
	Init(&SETSKIPDATA_CMD, (CmdEntry)&CDacSrv::CtrlSkipData);
	Init(&GETSKIPDATA_CMD, (CmdEntry)&CDacSrv::CtrlSkipData);

	Init(&FIFORESET_CMD, (CmdEntry)&CDacSrv::CtrlFifoReset);
	Init(&ENABLE_CMD, (CmdEntry)&CDacSrv::CtrlEnable);
	Init(&PREPARESTART_CMD, (CmdEntry)&CDacSrv::CtrlPrepareStart);
	Init(&FIFOSTATUS_CMD, (CmdEntry)&CDacSrv::CtrlFifoStatus);
	Init(&PUTDATA_CMD, (CmdEntry)&CDacSrv::CtrlPutData);
//	Init(&ISBITSUNDERFLOW_CMD, (CmdEntry)CtrlIsBitsUnderflow);

	Init(&GETSRCSTREAM_CMD, (CmdEntry)&CDacSrv::CtrlGetAddrData);

	Init(&SETCLKREFMODE_CMD, (CmdEntry)&CDacSrv::CtrlClkRefMode);
	Init(&GETCLKREFMODE_CMD, (CmdEntry)&CDacSrv::CtrlClkRefMode);
	Init(&SETDIVCLK_CMD, (CmdEntry)&CDacSrv::CtrlDivClk);
	Init(&GETDIVCLK_CMD, (CmdEntry)&CDacSrv::CtrlDivClk);
	Init(&OUTSYNC_CMD, (CmdEntry)&CDacSrv::CtrlOutSync);

	Init(&SETINTERP_CMD, (CmdEntry)&CDacSrv::CtrlInterp);
	Init(&GETINTERP_CMD, (CmdEntry)&CDacSrv::CtrlInterp);
}

//***************************************************************************************
int CDacSrv::ExtraInit(CModule* pModule)
{
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlRelease(
						void			*pDev,		// InfoSM or InfoBM
						void			*pServData,	// Specific Service Data
						ULONG			cmd,		// Command Code (from BRD_ctrl())
						void			*args 		// Command Arguments (from BRD_ctrl())
						)
{
	PDACSRV_CFG pDacSrvCfg = (PDACSRV_CFG)m_pConfig;
	pDacSrvCfg->isAlreadyInit = 0;
	return BRDerr_CMD_UNSUPPORTED; // РґР»СЏ РѕСЃРІРѕР±РѕР¶РґРµРЅРёСЏ РїРѕРґСЃР»СѓР¶Р±
}

//***************************************************************************************
int CDacSrv::CtrlIsAvailable(
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
	GetDacTetrNum(pModule);

	PDACSRV_CFG pDacSrvCfg = (PDACSRV_CFG)m_pConfig;
	if(pDacSrvCfg->TetrNum != -1)
		m_DacTetrNum = pDacSrvCfg->TetrNum;

	if(m_MainTetrNum != -1 && m_DacTetrNum != -1)
	{
		m_isAvailable = 1;
	//	PDACSRV_CFG pDacSrvCfg = (PDACSRV_CFG)m_pConfig;
	//	if(!pDacSrvCfg->isAlreadyInit)
	//	{
	//		pDacSrvCfg->isAlreadyInit = 1;

	//		DEVS_CMD_Reg RegParam;
	//		RegParam.idxMain = m_index;
	//		RegParam.tetr = m_DacTetrNum;
	//		pDacSrvCfg->FifoType = 1; // РІРЅРµС€РЅРµРµ
	//		if(!pDacSrvCfg->FifoSize)
	//		{
	//			RegParam.reg = ADM2IFnr_FTYPE;
	//			pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
	//			int widthFifo = RegParam.val >> 3;
	//			RegParam.reg = ADM2IFnr_FSIZE;
	//			pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
	////			pDacSrvCfg->FifoSize = RegParam.val << 3;
	//			pDacSrvCfg->FifoSize = RegParam.val * widthFifo;
	//			pDacSrvCfg->FifoType = 0; // РІРЅСѓС‚СЂРµРЅРЅРµРµ
	//		}

	//		RegParam.tetr = m_DacTetrNum;
	//		RegParam.reg = ADM2IFnr_MODE0;
	//		RegParam.val = 0;
	//	//	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
	//		PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&RegParam.val;
	//		pMode0->ByBits.Reset = 1;
	//		pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
	//		for(int i = 1; i < 32; i++)
	//		{
	//			RegParam.reg = i;
	//			RegParam.val = 0;
	//			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
	//		}
	//		RegParam.reg = ADM2IFnr_MODE0;
	////		pMode0->ByBits.AdmClk = 1;
	//		pMode0->ByBits.Reset = 0;
	//		pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

	//		RegParam.reg = ADM2IFnr_FDIV;
	//		RegParam.val = 2;
	//		pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
	//		RegParam.tetr = m_MainTetrNum;
	//		pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

	//		ULONG Status = ExtraInit(pModule);
	//		if(Status != BRDerr_OK)
	//			return Status;
	//	}
	}
	else
		m_isAvailable = 0;

	pServAvailable->isAvailable = m_isAvailable;
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlCapture(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	if(m_MainTetrNum != -1 && m_DacTetrNum != -1)
	{
		PDACSRV_CFG pDacSrvCfg = (PDACSRV_CFG)m_pConfig;
		if(!pDacSrvCfg->isAlreadyInit)
		{
			pDacSrvCfg->isAlreadyInit = 1;

			DEVS_CMD_Reg RegParam;
			RegParam.idxMain = m_index;
			RegParam.tetr = m_DacTetrNum;
			pDacSrvCfg->FifoType = 1; // РІРЅРµС€РЅРµРµ
			if(!pDacSrvCfg->FifoSize)
			{
				RegParam.reg = ADM2IFnr_FTYPE;
				pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
				int widthFifo = RegParam.val >> 3;
				RegParam.reg = ADM2IFnr_FSIZE;
				pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
	//			pDacSrvCfg->FifoSize = RegParam.val << 3;
				pDacSrvCfg->FifoSize = RegParam.val * widthFifo;
				pDacSrvCfg->FifoType = 0; // РІРЅСѓС‚СЂРµРЅРЅРµРµ
			}

			RegParam.tetr = m_DacTetrNum;
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

			RegParam.reg = ADM2IFnr_FDIV;
			RegParam.val = 2;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			RegParam.tetr = m_MainTetrNum;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

			ULONG Status = ExtraInit(pModule);
			if(Status != BRDerr_OK)
				return Status;
		}
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlGetAddrData(
							void			*pDev,		// InfoSM or InfoBM
							void			*pServData,	// Specific Service Data
							ULONG			cmd,		// Command Code (from BRD_ctrl())
							void			*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	*(ULONG*)args = GetParamForStream(pModule);
//	*(ULONG*)args = (m_AdmNum << 16) | m_DacTetrNum;
//	*(ULONG*)args = m_DacTetrNum;
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlGetTraceText( void *pDev, void *pServData, ULONG cmd, void *args )
{
	CModule* pModule = (CModule*)pDev;
	BRD_TraceText	*pTT = (BRD_TraceText*)args;
	
	return GetTraceText( pModule, pTT->traceId, pTT->sizeb, pTT->pText );
}

//***************************************************************************************
int CDacSrv::CtrlGetTraceParam( void *pDev, void *pServData, ULONG cmd, void *args )
{
	CModule* pModule = (CModule*)pDev;
	BRD_TraceParam	*pTP = (BRD_TraceParam*)args;

	return GetTraceParam( pModule, pTP->traceId, pTP->sizeb, &(pTP->sizebReal), pTP->pParam );
}

//***************************************************************************************
int CDacSrv::CtrlGetPages(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
#ifdef _WIN32
	CModule* pModule = (CModule*)pDev;
	PBRD_PropertyList pList = (PBRD_PropertyList)args;
//	ULONG dlg_mode = *(PULONG)args;

	if(cmd == BRDctrl_DAC_GETDLGPAGES)
	{
		// Open Library
		PDACSRV_CFG pSrvCfg = (PDACSRV_CFG)m_pConfig;
		pList->hLib = LoadLibrary(pSrvCfg->DlgDllName);
		if(pList->hLib <= (HINSTANCE)HINSTANCE_ERROR)
			return BRDerr_BAD_DEVICE_VITAL_DATA;

		INFO_InitPages_Type* pDlgFunc; 
		pDlgFunc = (INFO_InitPages_Type*)GetProcAddress(pList->hLib, "INFO_InitPages");
		if(!pDlgFunc)
		{
			FreeLibrary(pList->hLib);
			return BRDerr_BAD_DEVICE_VITAL_DATA;
		}
		PVOID pInfo = GetInfoForDialog(pModule);

		ULONG num_dlg;
//		int num_pages = (pDlgFunc)(pDev, pSrvCfg->pPropDlg, this, &num_dlg, pInfo);
		int num_pages = (pDlgFunc)(pDev, m_pPropDlg, this, &num_dlg, pInfo);
		pList->PagesCnt = num_pages;
		pList->NumDlg = num_dlg;

		FreeInfoForDialog(pInfo);
	}
#else
	fprintf(stderr, "%s, %d: - %s not implemented\n", __FILE__, __LINE__, __FUNCTION__);
	return -ENOSYS;
#endif
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlIniFile(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_IniFile pFile = (PBRD_IniFile)args;
	if(BRDctrl_DAC_WRITEINIFILE == cmd)
		Status = SaveProperties(pModule, pFile->fileName, pFile->sectionName);
	else
		Status = SetProperties(pModule, pFile->fileName, pFile->sectionName);
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlCfg(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	Status = GetCfg((PBRD_DacCfg)args);
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlChanMask(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_DAC_SETCHANMASK == cmd)
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
int CDacSrv::CtrlFormat(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_FORMAT;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_FORMAT pFormat = (PADM2IF_FORMAT)&param.val;
	if(BRDctrl_DAC_SETFORMAT == cmd)
	{
		pFormat->ByBits.Pack = *(ULONG*)args;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
		*(ULONG*)args = pFormat->ByBits.Pack;
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlCode(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_DAC_SETCODE == cmd)
	{
		ULONG type = *(ULONG*)args;
		Status = SetCode(pModule, type);
	}
	else
	{
		ULONG type;
		Status = GetCode(pModule, type);
		*(ULONG*)args = type;
	}
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlGain(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_ValChan pValChan = (PBRD_ValChan)args;
	PDACSRV_CFG pDacSrvCfg = (PDACSRV_CFG)m_pConfig;
	if(pValChan->chan < 0 || pValChan->chan >= pDacSrvCfg->MaxChan)
		return BRDerr_DAC_ILLEGAL_CHANNEL | BRDerr_WARN;
	int Status = BRDerr_OK;
	if(BRDctrl_DAC_SETGAIN == cmd)
		Status = SetGain(pModule, pValChan->value, pValChan->chan);
	else
		Status = GetGain(pModule, pValChan->value, pValChan->chan);
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlOutRange(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_ValChan pValChan = (PBRD_ValChan)args;
	PDACSRV_CFG pDacSrvCfg = (PDACSRV_CFG)m_pConfig;
	if(pValChan->chan < 0 || pValChan->chan >= pDacSrvCfg->MaxChan)
		return BRDerr_DAC_ILLEGAL_CHANNEL | BRDerr_WARN;
	int Status = BRDerr_OK;
	if(BRDctrl_DAC_SETOUTRANGE == cmd)
		Status = SetOutRange(pModule, pValChan->value, pValChan->chan);
	else
		Status = GetOutRange(pModule, pValChan->value, pValChan->chan);
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlBias(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
        int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ValChan pValChan = (PBRD_ValChan)args;
	PDACSRV_CFG pDacSrvCfg = (PDACSRV_CFG)m_pConfig;
	if(pValChan->chan < 0 || pValChan->chan >= pDacSrvCfg->MaxChan)
		return BRDerr_DAC_ILLEGAL_CHANNEL | BRDerr_WARN;
	if(BRDctrl_DAC_SETBIAS == cmd)
		Status = SetBias(pModule, pValChan->value, pValChan->chan);
	else
		Status = GetBias(pModule, pValChan->value, pValChan->chan);
        return Status;
}

//***************************************************************************************
int CDacSrv::CtrlRate(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG ClkSrc;

	Status = GetClkSource(pModule, ClkSrc);
	double ClkValue;
	Status = GetClkValue(pModule, ClkSrc, ClkValue);
	double Rate = *(double*)args;

	if(BRDctrl_DAC_SETRATE == cmd)
	{
		Status = SetRate(pModule, Rate, ClkSrc, ClkValue);
		*(double*)args = Rate;
	}
	else
	{
		Status = GetRate(pModule, Rate, ClkValue);
		*(double*)args = Rate;
	}
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlClkMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ClkMode pDacClk = (PBRD_ClkMode)args;
	if(BRDctrl_DAC_SETCLKMODE == cmd)
	{
		Status = SetClkSource(pModule, pDacClk->src);
		Status = SetClkValue(pModule, pDacClk->src, pDacClk->value);
	}
	else
	{
		ULONG src = pDacClk->src;
		Status = GetClkSource(pModule, src);
		pDacClk->src = src;
		Status = GetClkValue(pModule, pDacClk->src, pDacClk->value);
	}
	return Status;
}
//***************************************************************************************
int CDacSrv::CtrlClkRefMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ClkRefMode pClkRefMode = (PBRD_ClkRefMode)args;

	if(BRDctrl_DAC_SETCLKREFMODE == cmd)
	{
		Status = SetClkRefSource(pModule, pClkRefMode->srcClk,pClkRefMode->srcRef);
		Status = SetClkRefValue(pModule, pClkRefMode->srcClk,pClkRefMode->srcRef, pClkRefMode->valueClk,pClkRefMode->valueRef);
	}
	else
	{
		ULONG srcClk = pClkRefMode->srcClk;
		ULONG srcRef = pClkRefMode->srcRef;
		Status = GetClkRefSource(pModule, srcClk,srcRef);
		pClkRefMode->srcClk = srcClk;
		pClkRefMode->srcRef = srcRef;
		Status = GetClkRefValue(pModule, pClkRefMode->srcClk,pClkRefMode->srcRef, pClkRefMode->valueClk,pClkRefMode->valueRef);
	}

	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlDivClk(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
        int Status = BRDerr_CMD_UNSUPPORTED;
	PBRD_DivClk pDivClk=(PBRD_DivClk)args;
	ULONG divClk = pDivClk->divClk;
	if(BRDctrl_DAC_SETDIVCLK == cmd)
	{
		Status = SetDivClk(pModule,divClk,pDivClk->outRate);
	}
	else
	{
		Status = GetDivClk(pModule,divClk,pDivClk->outRate);
	}
	pDivClk->divClk=divClk;	
        return Status;

}

//***************************************************************************************

//***************************************************************************************
int CDacSrv::CtrlCyclingMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(BRDctrl_DAC_SETCYCLMODE == cmd)
	{
		pMode0->ByBits.Cycle = *(ULONG*)args;
		pMode0->ByBits.Reset = 0;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
		*(ULONG*)args = pMode0->ByBits.Cycle;
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlSyncMode(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   )
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_SyncMode pSyncMode = (PBRD_SyncMode)args;
	if(BRDctrl_DAC_SETSYNCMODE == cmd)
	{
		Status = SetClkSource(pModule, pSyncMode->clkSrc);
		Status = SetClkValue(pModule, pSyncMode->clkSrc, pSyncMode->clkValue);
		Status = SetRate(pModule, pSyncMode->rate, pSyncMode->clkSrc, pSyncMode->clkValue);
	}
	else
	{
		ULONG src = pSyncMode->clkSrc;
		Status = GetClkSource(pModule, src);
		pSyncMode->clkSrc = src;
		Status = GetClkValue(pModule, pSyncMode->clkSrc, pSyncMode->clkValue);
		Status = GetRate(pModule, pSyncMode->rate, pSyncMode->clkValue);
	}
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlMaster(
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
	if(BRDctrl_DAC_SETMASTER == cmd)
	{
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pMode0->ByBits.Master = *pMasterMode >> 1;
		if(pMode0->ByBits.Master)
			pMode0->ByBits.Start = 0;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.tetr = m_DacTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);

		//param.val = 0;
		//pMode0->ByBits.Reset = 1;
		//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		//for(int i = 1; i < 32; i++)
		//{
		//	param.reg = i;
		//	param.val = 0;
		//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		//}
		//param.reg = ADM2IFnr_MODE0;
		//pMode0->ByBits.Reset = 0;
		//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		//param.reg = ADM2IFnr_FDIV;
		//param.val = 2;
		//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		//param.reg = ADM2IFnr_MODE0;
		//pModule->RegCtrl(DEVScmd_REGREADIND, &param);

		pMode0->ByBits.Master = *pMasterMode & 0x1;
		pMode0->ByBits.Reset = 0;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		param.tetr = m_DacTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		*pMasterMode = pMode0->ByBits.Master;
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		*pMasterMode |= (pMode0->ByBits.Master << 1);
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlStartMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
//	PBRD_DacStartMode pDacStartMode = (PBRD_DacStartMode)args;
	if(BRDctrl_DAC_SETSTARTMODE == cmd)
		Status = SetStartMode(pModule, args);
	else
        Status = GetStartMode(pModule, args);
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlDelay(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_StdDelay delay = (PBRD_StdDelay)args;
	if(BRDctrl_DAC_WRDELAY == cmd)
		Status = StdDelay(pModule, 1, delay);
	else
		if(BRDctrl_DAC_RSTDELAY == cmd)
			Status = StdDelay(pModule, 2, delay);
		else
			Status = StdDelay(pModule, 0, delay);
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlStDelay(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   )
{
	CModule* pModule = (CModule*)pDev;
	PBRD_EnVal pEnVal = (PBRD_EnVal)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;//ADM2IF_ntBASEDAC;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(BRDctrl_DAC_SETSTDELAY == cmd)
	{
		pMode0->ByBits.Cnt0En = pEnVal->enable;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.reg = ADM2IFnr_CNT0;
		param.val = pEnVal->value;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		pEnVal->enable = pMode0->ByBits.Cnt0En;
		param.reg = ADM2IFnr_CNT0;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pEnVal->value = param.val;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlOutData(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   )
{
	CModule* pModule = (CModule*)pDev;
	PBRD_EnVal pEnVal = (PBRD_EnVal)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(BRDctrl_DAC_SETOUTDATA == cmd)
	{
		pMode0->ByBits.Cnt1En = pEnVal->enable;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.reg = ADM2IFnr_CNT1;
		param.val = pEnVal->value;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		pEnVal->enable = pMode0->ByBits.Cnt1En;
		param.reg = ADM2IFnr_CNT1;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pEnVal->value = param.val;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlSkipData(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   )
{
	CModule* pModule = (CModule*)pDev;
	PBRD_EnVal pEnVal = (PBRD_EnVal)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;//ADM2IF_ntBASEDAC;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(BRDctrl_DAC_SETSKIPDATA == cmd)
	{
		pMode0->ByBits.Cnt2En = pEnVal->enable;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.reg = ADM2IFnr_CNT2;
		param.val = pEnVal->value;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		pEnVal->enable = pMode0->ByBits.Cnt2En;
		param.reg = ADM2IFnr_CNT2;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pEnVal->value = param.val;
	}
	return BRDerr_OK;
}
/*
// ***************************************************************************************
int CDacSrv::CtrlIsBitsUnderflow(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	//param.reg = ADM2IFnr_STATUS;
	//pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//PDAC_STATUS pStatus = (PDAC_STATUS)&param.val;
	//ULONG data = pStatus->ByBits.OverDac0;
	//data |= (pStatus->ByBits.OverDac1 << 1);
    // *(PULONG)args = data;
	param.reg = DACnr_UNDERFLOW;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	*(PULONG)args = param.val;
	return BRDerr_OK;
}
*/
//***************************************************************************************
int CDacSrv::CtrlTestMode(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG mode = *(PULONG)args;
	if(BRDctrl_DAC_SETTESTMODE == cmd)
		Status = SetTestMode(pModule, mode);
	else
		Status = GetTestMode(pModule, mode);
	*(PULONG)args = mode;
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlSelfClbr(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status = SelfClbr(pModule);
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlSpecific(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_DacSpec pSpec = (PBRD_DacSpec)args;
	if(BRDctrl_DAC_SETSPECIFIC == cmd)
		Status = SetSpecific(pModule, pSpec);
	else
		Status = GetSpecific(pModule, pSpec);
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlSource(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG target = *(ULONG*)args;
	if(BRDctrl_DAC_SETSOURCE == cmd)
	{
		Status = SetSource(pModule, target);
	}
	else
	{
		Status = GetSource(pModule, target);
		*(ULONG*)args = target;
	}
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlFifoReset (
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.FifoRes = 1;
	pMode0->ByBits.Reset = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
    IPC_delay(1);
#else
	Sleep(1);
#endif
	pMode0->ByBits.FifoRes = 0;
	pMode0->ByBits.Reset = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
    IPC_delay(1);
#else
	Sleep(1);
#endif
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlEnable (
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	//
	// Ekk изменил 10.08.2016
	//
	CModule* pModule = (CModule*)pDev;
	ULONG enable = *(PULONG)args;
	return DacEnable(pModule, enable);

	//CModule* pModule = (CModule*)pDev;
	//ULONG enable = *(PULONG)args;
	//DEVS_CMD_Reg param;
	//param.idxMain = m_index;
	//param.tetr = m_DacTetrNum;
	//if(enable)
	//{
	//	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
	//	param.reg = ADM2IFnr_STATUS;
	//	for(int i = 0; i < 20; i++)
	//	{
	//		pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//		if(pStatus->ByBits.Empty)
	//			break;
	//		#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	//		    IPC_delay(1);
	//		#else
	//			Sleep(1);
	//		#endif
	//	}
	//	//if(!pStatus->ByBits.Empty)
	//	//	return BRDerr_NOT_READY;
	//}
	//param.reg = ADM2IFnr_MODE0;
	//pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	//pMode0->ByBits.Start = enable;
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//if(!pMode0->ByBits.Master)
	//{ // Master/Slave
	//	param.tetr = m_MainTetrNum;
	//	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//	if(pMode0->ByBits.Master)
	//	{ // Master
	//		if(enable)
	//		{
	//			/*
	//			param.reg = ADM2IFnr_MODE1;
	//			pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//			PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	//			pMode1->ByBits.MsSync = 0;
	//			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//			Sleep(1);
	//			pMode1->ByBits.MsSync = 1;
	//			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//			Sleep(1);
	//			pMode1->ByBits.MsSync = 0;
	//			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//			Sleep(1);
	//		*/
	//			param.reg = ADM2IFnr_MODE0;
	//			pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//		}
	//		pMode0->ByBits.Start = enable;
	//		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//	}
	//}
	//return BRDerr_OK;
}
//***************************************************************************************
int CDacSrv::CtrlPrepareStart(void *pDev, void *pServData, ULONG cmd, void *args)
{
	CModule* pModule = (CModule*)pDev;
	PrepareStart(pModule, args);
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlResist(void * pDev, void * pServData, ULONG cmd, void * args)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule			*pModule = (CModule*)pDev;
	BRD_UvalChan		target = *(BRD_UvalChan*)args;

	if(BRDctrl_DAC_SETRESIST == cmd)
	{
		Status = SetResist( pModule, target.value, target.chan );
	}
	else
	{
		Status = GetResist( pModule, (ULONG&)target.value, target.chan );
		*(BRD_UvalChan*)args = target;
	}
	return Status;
}

//***************************************************************************************
int CDacSrv::CtrlOutSync (
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MainTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	pMode1->ByBits.MsSync = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//Sleep(1);
	pMode1->ByBits.MsSync = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//Sleep(1);
	pMode1->ByBits.MsSync = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	// Sleep(1);
			
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlFifoStatus(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_STATUS;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
//	ULONG data = pStatus->ByBits.Underflow;
	ULONG data = pStatus->AsWhole;
	*(PULONG)args = data;
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::CtrlPutData(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
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
	pModule->RegCtrl(DEVScmd_REGWRITESDIR, &param);
/*
	PULONG buf = (PULONG)pBuf->pData;
	for(ULONG i = 0; i < pBuf->size / sizeof(ULONG); i++)
//	while(1)
	{
		param.val = buf[i];
//		param.val = buf[0];
		pModule->RegCtrl(DEVScmd_REGWRITEDIR, &param);
	}
*/	
	//param.reg = ADM2IFnr_STATUS;
	//pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//pStatus = (PADM2IF_STATUS)&param.val;

	return BRDerr_OK;
}

/*

 _mra

*/
int CDacSrv::CtrlInterp(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
        int Status = BRDerr_CMD_UNSUPPORTED;

	ULONG InterpFactor = *(U32*)args;
	if(BRDctrl_DAC_SETINTERP == cmd)
	{
		Status = SetInterpFactor( pModule, InterpFactor );
	}
	else
	{
		Status = GetInterpFactor( pModule, InterpFactor );
	}

	*(U32*)args=InterpFactor;	

        return Status;

}

// ***************** End of file DacCtrl.cpp ***********************
