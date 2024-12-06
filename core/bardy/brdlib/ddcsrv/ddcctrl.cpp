/*
 ***************** File DdcCtrl.cpp ************
 *
 * CTRL-command for BRD Driver for DDÑ service on DDÑ submodule
 *
 * (C) InSys by Dorokhin A. Jan 2006
 *
 * 12.10.2006 - submodule ver 3.0 - synchronous mode
 *
 ******************************************************
*/

#include "module.h"
#include "ddcsrv.h"

#include "gipcy.h"

#define	CURRFILE "DDCCTRL"

static CMD_Info SETCHANMASK_CMD		= { BRDctrl_DDC_SETCHANMASK,	0, 0, 0, NULL};
static CMD_Info GETCHANMASK_CMD		= { BRDctrl_DDC_GETCHANMASK,	1, 0, 0, NULL};
static CMD_Info SETFORMAT_CMD		= { BRDctrl_DDC_SETFORMAT,		0, 0, 0, NULL};
static CMD_Info GETFORMAT_CMD		= { BRDctrl_DDC_GETFORMAT,		1, 0, 0, NULL};
static CMD_Info SETRATE_CMD			= { BRDctrl_DDC_SETRATE,		0, 0, 0, NULL};
static CMD_Info GETRATE_CMD			= { BRDctrl_DDC_GETRATE,		1, 0, 0, NULL};
static CMD_Info SETCLKMODE_CMD		= { BRDctrl_DDC_SETCLKMODE,		0, 0, 0, NULL};
static CMD_Info GETCLKMODE_CMD		= { BRDctrl_DDC_GETCLKMODE,		1, 0, 0, NULL};
static CMD_Info SETSYNCMODE_CMD		= { BRDctrl_DDC_SETSYNCMODE,	0, 0, 0, NULL};
static CMD_Info GETSYNCMODE_CMD		= { BRDctrl_DDC_GETSYNCMODE,	1, 0, 0, NULL};
static CMD_Info SETMASTER_CMD		= { BRDctrl_DDC_SETMASTER,		0, 0, 0, NULL};
static CMD_Info GETMASTER_CMD		= { BRDctrl_DDC_GETMASTER,		1, 0, 0, NULL};
static CMD_Info SETSTARTMODE_CMD	= { BRDctrl_DDC_SETSTARTMODE,	0, 0, 0, NULL};
static CMD_Info GETSTARTMODE_CMD	= { BRDctrl_DDC_GETSTARTMODE,	1, 0, 0, NULL};
static CMD_Info SETSTDELAY_CMD		= { BRDctrl_DDC_SETSTDELAY,		0, 0, 0, NULL};
static CMD_Info GETSTDELAY_CMD		= { BRDctrl_DDC_GETSTDELAY,		1, 0, 0, NULL};
static CMD_Info SETACQDATA_CMD		= { BRDctrl_DDC_SETACQDATA,		0, 0, 0, NULL};
static CMD_Info GETACQDATA_CMD		= { BRDctrl_DDC_GETACQDATA,		1, 0, 0, NULL};
static CMD_Info SETSKIPDATA_CMD		= { BRDctrl_DDC_SETSKIPDATA,	0, 0, 0, NULL};
static CMD_Info GETSKIPDATA_CMD		= { BRDctrl_DDC_GETSKIPDATA,	1, 0, 0, NULL};
static CMD_Info SETTITLEMODE_CMD	= { BRDctrl_DDC_SETTITLEMODE,	0, 0, 0, NULL};
static CMD_Info GETTITLEMODE_CMD	= { BRDctrl_DDC_GETTITLEMODE,	1, 0, 0, NULL};
static CMD_Info SETINPSRC_CMD		= { BRDctrl_DDC_SETINPSRC,		0, 0, 0, NULL};
static CMD_Info GETINPSRC_CMD		= { BRDctrl_DDC_GETINPSRC,		1, 0, 0, NULL};
static CMD_Info SETFC_CMD			= { BRDctrl_DDC_SETFC,			0, 0, 0, NULL};
static CMD_Info GETFC_CMD			= { BRDctrl_DDC_GETFC,			1, 0, 0, NULL};
static CMD_Info SETPROGRAM_CMD		= { BRDctrl_DDC_SETPROGRAM,		0, 0, 0, NULL};
static CMD_Info SETDDCSYNC_CMD		= { BRDctrl_DDC_SETDDCSYNC,		0, 0, 0, NULL};
static CMD_Info GETDDCSYNC_CMD		= { BRDctrl_DDC_GETDDCSYNC,		1, 0, 0, NULL};
static CMD_Info SETDECIM_CMD		= { BRDctrl_DDC_SETDECIM,		0, 0, 0, NULL};
static CMD_Info GETDECIM_CMD		= { BRDctrl_DDC_GETDECIM,		1, 0, 0, NULL};
static CMD_Info SETTARGET_CMD		= { BRDctrl_DDC_SETTARGET,		0, 0, 0, NULL};
static CMD_Info GETTARGET_CMD		= { BRDctrl_DDC_GETTARGET,		1, 0, 0, NULL};
static CMD_Info SETFRAME_CMD		= { BRDctrl_DDC_SETFRAME,		0, 0, 0, NULL};
static CMD_Info GETFRAME_CMD		= { BRDctrl_DDC_GETFRAME,		1, 0, 0, NULL};
static CMD_Info GETCFG_CMD			= { BRDctrl_DDC_GETCFG,			1, 0, 0, NULL};
static CMD_Info WRITEINIFILE_CMD	= { BRDctrl_DDC_WRITEINIFILE,	1, 0, 0, NULL};
static CMD_Info READINIFILE_CMD		= { BRDctrl_DDC_READINIFILE,	0, 0, 0, NULL};
static CMD_Info SETTESTMODE_CMD		= { BRDctrl_DDC_SETTESTMODE,	0, 0, 0, NULL};
static CMD_Info GETTESTMODE_CMD		= { BRDctrl_DDC_GETTESTMODE,	1, 0, 0, NULL};
static CMD_Info SETSPECIFIC_CMD		= { BRDctrl_DDC_SETSPECIFIC,	0, 0, 0, NULL};
static CMD_Info GETSPECIFIC_CMD		= { BRDctrl_DDC_GETSPECIFIC,	1, 0, 0, NULL};

static CMD_Info GETDLGPAGES_CMD		= { BRDctrl_DDC_GETDLGPAGES,	0, 0, 0, NULL};

static CMD_Info FIFORESET_CMD		= { BRDctrl_DDC_FIFORESET,	 0, 0, 0, NULL};
static CMD_Info ENABLE_CMD			= { BRDctrl_DDC_ENABLE,		 0, 0, 0, NULL};
static CMD_Info PREPARESTART_CMD    = { BRDctrl_DDC_PREPARESTART,0, 0, 0, NULL };
static CMD_Info FIFOSTATUS_CMD		= { BRDctrl_DDC_FIFOSTATUS,	 1, 0, 0, NULL};
static CMD_Info GETDATA_CMD			= { BRDctrl_DDC_GETDATA,	 0, 0, 0, NULL};
static CMD_Info GETSRCSTREAM_CMD	= { BRDctrl_DDC_GETSRCSTREAM, 1, 0, 0, NULL};
static CMD_Info SETTLDATA_CMD		= { BRDctrl_DDC_SETTITLEDATA, 0, 0, 0, NULL};
static CMD_Info GETTLDATA_CMD		= { BRDctrl_DDC_GETTITLEDATA, 1, 0, 0, NULL};

//***************************************************************************************
CDdcSrv::CDdcSrv(int idx, const BRDCHAR* name, int srv_num, CModule* pModule, PVOID pCfg, ULONG cfgSize) :
	CService(idx, name, srv_num, pModule, pCfg, cfgSize)
{
	m_attribute = 
			BRDserv_ATTR_DIRECTION_IN |
			BRDserv_ATTR_STREAMABLE_IN |
			BRDserv_ATTR_SDRAMABLE |
			BRDserv_ATTR_CMPABLE |
			BRDserv_ATTR_DSPABLE |
			BRDserv_ATTR_ADCABLE |
			BRDserv_ATTR_EXCLUSIVE_ONLY;
	
	Init(&GETCFG_CMD, (CmdEntry)&CDdcSrv::CtrlCfg);
	Init(&WRITEINIFILE_CMD, (CmdEntry)&CDdcSrv::CtrlIniFile);
	Init(&READINIFILE_CMD, (CmdEntry)&CDdcSrv::CtrlIniFile);

	Init(&GETDLGPAGES_CMD, (CmdEntry)&CDdcSrv::CtrlGetPages);
	//Init(&ENDPAGESDLG_CMD, (CmdEntry)CtrlInitEndDlg);
	//Init(&SETPROPDLG_CMD, (CmdEntry)CtrlDlgProperty);
	//Init(&GETPROPDLG_CMD, (CmdEntry)CtrlDlgProperty);

	Init(&SETCHANMASK_CMD, (CmdEntry)&CDdcSrv::CtrlChanMask);
	Init(&GETCHANMASK_CMD, (CmdEntry)&CDdcSrv::CtrlChanMask);
	Init(&SETFORMAT_CMD, (CmdEntry)&CDdcSrv::CtrlFormat);
	Init(&GETFORMAT_CMD, (CmdEntry)&CDdcSrv::CtrlFormat);
	Init(&SETRATE_CMD, (CmdEntry)&CDdcSrv::CtrlRate);
	Init(&GETRATE_CMD, (CmdEntry)&CDdcSrv::CtrlRate);
	Init(&SETCLKMODE_CMD, (CmdEntry)&CDdcSrv::CtrlClkMode);
	Init(&GETCLKMODE_CMD, (CmdEntry)&CDdcSrv::CtrlClkMode);
	Init(&SETSYNCMODE_CMD, (CmdEntry)&CDdcSrv::CtrlSyncMode);
	Init(&GETSYNCMODE_CMD, (CmdEntry)&CDdcSrv::CtrlSyncMode);
	Init(&SETMASTER_CMD, (CmdEntry)&CDdcSrv::CtrlMaster);
	Init(&GETMASTER_CMD, (CmdEntry)&CDdcSrv::CtrlMaster);

	Init(&SETINPSRC_CMD, (CmdEntry)&CDdcSrv::CtrlInpSrc);
	Init(&GETINPSRC_CMD, (CmdEntry)&CDdcSrv::CtrlInpSrc);
	Init(&SETFC_CMD, (CmdEntry)&CDdcSrv::CtrlFC);
	Init(&GETFC_CMD, (CmdEntry)&CDdcSrv::CtrlFC);
	Init(&SETPROGRAM_CMD, (CmdEntry)&CDdcSrv::CtrlProgram);
	Init(&SETDDCSYNC_CMD, (CmdEntry)&CDdcSrv::CtrlDDCSync);
	Init(&GETDDCSYNC_CMD, (CmdEntry)&CDdcSrv::CtrlDDCSync);
	Init(&SETDECIM_CMD, (CmdEntry)&CDdcSrv::CtrlDecim);
	Init(&GETDECIM_CMD, (CmdEntry)&CDdcSrv::CtrlDecim);
	Init(&SETFRAME_CMD, (CmdEntry)&CDdcSrv::CtrlFrame);
	Init(&GETFRAME_CMD, (CmdEntry)&CDdcSrv::CtrlFrame);

	Init(&SETTESTMODE_CMD, (CmdEntry)&CDdcSrv::CtrlTestMode);
	Init(&GETTESTMODE_CMD, (CmdEntry)&CDdcSrv::CtrlTestMode);
	Init(&SETSPECIFIC_CMD, (CmdEntry)&CDdcSrv::CtrlSpecific);
	Init(&GETSPECIFIC_CMD, (CmdEntry)&CDdcSrv::CtrlSpecific);

	Init(&SETTARGET_CMD, (CmdEntry)&CDdcSrv::CtrlTarget);
	Init(&GETTARGET_CMD, (CmdEntry)&CDdcSrv::CtrlTarget);

	Init(&SETSTARTMODE_CMD, (CmdEntry)&CDdcSrv::CtrlStartMode);
	Init(&GETSTARTMODE_CMD, (CmdEntry)&CDdcSrv::CtrlStartMode);
	Init(&SETTITLEMODE_CMD, (CmdEntry)&CDdcSrv::CtrlTitleMode);
	Init(&GETTITLEMODE_CMD, (CmdEntry)&CDdcSrv::CtrlTitleMode);
	Init(&SETTLDATA_CMD, (CmdEntry)&CDdcSrv::CtrlTitleData);
	Init(&GETTLDATA_CMD, (CmdEntry)&CDdcSrv::CtrlTitleData);

	Init(&SETSTDELAY_CMD, (CmdEntry)&CDdcSrv::CtrlCnt);
	Init(&GETSTDELAY_CMD, (CmdEntry)&CDdcSrv::CtrlCnt);
	Init(&SETACQDATA_CMD, (CmdEntry)&CDdcSrv::CtrlCnt);
	Init(&GETACQDATA_CMD, (CmdEntry)&CDdcSrv::CtrlCnt);
	Init(&SETSKIPDATA_CMD, (CmdEntry)&CDdcSrv::CtrlCnt);
	Init(&GETSKIPDATA_CMD, (CmdEntry)&CDdcSrv::CtrlCnt);

	Init(&FIFORESET_CMD, (CmdEntry)&CDdcSrv::CtrlFifoReset);
	Init(&ENABLE_CMD, (CmdEntry)&CDdcSrv::CtrlEnable);
	Init(&PREPARESTART_CMD, (CmdEntry)&CDdcSrv::CtrlPrepareStart);
	Init(&FIFOSTATUS_CMD, (CmdEntry)&CDdcSrv::CtrlFifoStatus);
	Init(&GETDATA_CMD, (CmdEntry)&CDdcSrv::CtrlGetData);

	Init(&GETSRCSTREAM_CMD, (CmdEntry)&CDdcSrv::CtrlGetAddrData);

}

//#include <conio.h>
//***************************************************************************************
int CDdcSrv::CtrlIsAvailable(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PSERV_CMD_IsAvailable pServAvailable = (PSERV_CMD_IsAvailable)args;
	pServAvailable->attr = m_attribute;
/*	long m_FpgaTetrNum = GetTetrNum(pModule, m_index, ADMFPGA_TETR_ID);
        if(m_FpgaTetrNum != -1)
	{
		DEVS_CMD_Reg RegParam;
		RegParam.idxMain = m_index;
		RegParam.tetr = m_FpgaTetrNum;
		RegParam.reg = ADM2IFnr_STATUS;
		for(int i = 0; i < 50; i++)
		{
			Sleep(100);
			pModule->RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			if(RegParam.val & 0x8000)
				break;
		}
//		Sleep(5000);
	}
*/	
	m_MainTetrNum = GetTetrNum(pModule, m_index, MAIN_TETR_ID);
	GetDdcTetrNum(pModule);

	if(m_MainTetrNum != -1 && m_DdcTetrNum != -1)
	{
		m_isAvailable = 1;
/*		PDDCSRV_CFG pDdcSrvCfg = (PDDCSRV_CFG)m_pConfig;
		//printf("isAlreadyInit(PID=%d) =  0x%X\n", pModule->GetPID(), pDdcSrvCfg->isAlreadyInit);
		if(!pDdcSrvCfg->isAlreadyInit)
		{
			pDdcSrvCfg->isAlreadyInit = 1;

			DEVS_CMD_Reg RegParam;
			RegParam.idxMain = m_index;

		//	RegParam.tetr = m_MainTetrNum;
		//	RegParam.reg = ADM2IFnr_MODE0;
		//	RegParam.val = 0;
		////	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
		//	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&RegParam.val;
		//	pMode0->ByBits.Reset = 1;
		//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
		//	pMode0->ByBits.Reset = 0;
		//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);


			RegParam.tetr = m_DdcTetrNum;
			if(!pDdcSrvCfg->FifoSize)
			{
				RegParam.reg = ADM2IFnr_FTYPE;
				pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
				int widthFifo = RegParam.val >> 3;
				RegParam.reg = ADM2IFnr_FSIZE;
				pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
	//			pDdcSrvCfg->FifoSize = RegParam.val << 3;
				pDdcSrvCfg->FifoSize = RegParam.val * widthFifo;
			}
			PADM2IF_STATUS pStatus;
			//RegParam.reg = ADM2IFnr_STATUS;
			//pModule->RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			//pStatus = (PADM2IF_STATUS)&param.val;

			RegParam.tetr = m_DdcTetrNum;
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

			//RegParam.tetr = m_MainTetrNum;
			//RegParam.reg = ADM2IFnr_MODE0;
			//RegParam.val = 0;
			//pMode0 = (PADM2IF_MODE0)&RegParam.val;
			//pMode0->ByBits.Reset = 1;
			//pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			//pMode0->ByBits.Reset = 0;
			//pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

			Sleep(1200);

			//RegParam.reg = ADM2IFnr_MODE1;
			//PDDC_MODE1 pMode1 = (PDDC_MODE1)&RegParam.val;
			//pMode1->ByBits.Sleep = 1;
			//pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			//pMode1->ByBits.Sleep = 0;
			//pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

			RegParam.reg = DDCnr_ADCEN;
			PDDC_ADCEN pAdcEn = (PDDC_ADCEN)&RegParam.val;
			pAdcEn->ByBits.Enbl = 0x0f;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

			RegParam.reg = DDCnr_ADMCONST;
			pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
			PDDC_CONST adm_const = (PDDC_CONST)&RegParam.val;
			m_AdmConst.AsWhole = adm_const->AsWhole & 0xffff;
			//printf("CONST =  0x%X\n", adm_const->AsWhole & 0xffff);
			//printf("Press any key for continue...\n");
			//_getch();
			
			RegParam.reg = ADM2IFnr_STATUS;
			pModule->RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			pStatus = (PADM2IF_STATUS)&RegParam.val;

		}*/
	}
	else
		m_isAvailable = 0;

	pServAvailable->isAvailable = m_isAvailable;
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::CtrlCapture(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	long m_FpgaTetrNum = GetTetrNum(pModule, m_index, ADMFPGA_TETR_ID);
	if(m_FpgaTetrNum != -1)
	{
		DEVS_CMD_Reg RegParam;
		RegParam.idxMain = m_index;
		RegParam.tetr = m_FpgaTetrNum;
		RegParam.reg = ADM2IFnr_STATUS;
		for(int i = 0; i < 50; i++)
		{
            IPC_delay(100);
			pModule->RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			if(RegParam.val & 0x8000)
				break;
		}
//		Sleep(5000);
	}
	
	m_MainTetrNum = GetTetrNum(pModule, m_index, MAIN_TETR_ID);
	GetDdcTetrNum(pModule);

	if(m_MainTetrNum != -1 && m_DdcTetrNum != -1)
	{
		PDDCSRV_CFG pDdcSrvCfg = (PDDCSRV_CFG)m_pConfig;
		//printf("isAlreadyInit(PID=%d) =  0x%X\n", pModule->GetPID(), pDdcSrvCfg->isAlreadyInit);
		if(!pDdcSrvCfg->isAlreadyInit)
		{
			pDdcSrvCfg->isAlreadyInit = 1;

			DEVS_CMD_Reg RegParam;
			RegParam.idxMain = m_index;

		//	RegParam.tetr = m_MainTetrNum;
		//	RegParam.reg = ADM2IFnr_MODE0;
		//	RegParam.val = 0;
		////	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
		//	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&RegParam.val;
		//	pMode0->ByBits.Reset = 1;
		//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
		//	pMode0->ByBits.Reset = 0;
		//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);


			RegParam.tetr = m_DdcTetrNum;
			if(!pDdcSrvCfg->FifoSize)
			{
				RegParam.reg = ADM2IFnr_FTYPE;
				pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
				int widthFifo = RegParam.val >> 3;
				RegParam.reg = ADM2IFnr_FSIZE;
				pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
	//			pDdcSrvCfg->FifoSize = RegParam.val << 3;
				pDdcSrvCfg->FifoSize = RegParam.val * widthFifo;
			}
                        //PADM2IF_STATUS pStatus;
			//RegParam.reg = ADM2IFnr_STATUS;
			//pModule->RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			//pStatus = (PADM2IF_STATUS)&param.val;

			RegParam.tetr = m_DdcTetrNum;
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

			//RegParam.tetr = m_MainTetrNum;
			//RegParam.reg = ADM2IFnr_MODE0;
			//RegParam.val = 0;
			//pMode0 = (PADM2IF_MODE0)&RegParam.val;
			//pMode0->ByBits.Reset = 1;
			//pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			//pMode0->ByBits.Reset = 0;
			//pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

            IPC_delay(1200);

			//RegParam.reg = ADM2IFnr_MODE1;
			//PDDC_MODE1 pMode1 = (PDDC_MODE1)&RegParam.val;
			//pMode1->ByBits.Sleep = 1;
			//pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			//pMode1->ByBits.Sleep = 0;
			//pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

			RegParam.reg = DDCnr_ADCEN;
			PDDC_ADCEN pAdcEn = (PDDC_ADCEN)&RegParam.val;
			pAdcEn->ByBits.Enbl = 0x0f;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

			RegParam.reg = DDCnr_ADMCONST;
			pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
			PDDC_CONST adm_const = (PDDC_CONST)&RegParam.val;
			m_AdmConst.AsWhole = adm_const->AsWhole & 0xffff;
			//printf("CONST =  0x%X\n", adm_const->AsWhole & 0xffff);
			//printf("Press any key for continue...\n");
			//_getch();
			
			RegParam.reg = ADM2IFnr_STATUS;
			pModule->RegCtrl(DEVScmd_REGREADDIR, &RegParam);
                        //pStatus = (PADM2IF_STATUS)&RegParam.val;

		}
	}

	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::CtrlGetAddrData(
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
	*(ULONG*)args = (m_AdmNum << 16) | m_DdcTetrNum;
//	*(ULONG*)args = m_DdcTetrNum;
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::CtrlGetPages(
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

	if(cmd == BRDctrl_DDC_GETDLGPAGES)
	{
		// Open Library
		PDDCSRV_CFG pSrvCfg = (PDDCSRV_CFG)m_pConfig;
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

	//	int num_pages = (pDlgFunc)(pDev, this, dlg_mode, pInfo);
		ULONG num_dlg;
//		int num_pages = (pDlgFunc)(pDev, pSrvCfg->pPropDlg, this, &num_dlg, pInfo);
		int num_pages = (pDlgFunc)(pDev, m_pPropDlg, this, &num_dlg, pInfo);
		pList->PagesCnt = num_pages;
		pList->NumDlg = num_dlg;

		FreeInfoForDialog(pInfo);
	}
	return BRDerr_OK;
	//if(cmd == BRDctrl_DDC_ENDPAGESDLG)
	//{
	//	INFO_DeletePages_Type* pDlgFunc; 
	//	pDlgFunc = (INFO_DeletePages_Type*)GetProcAddress(pList->hLib, _T("INFO_DeletePages"));
	//	(pDlgFunc)(pList->NumDlg);

	//	FreeLibrary(pList->hLib);
	//}
#else
	fprintf(stderr, "%s, %d: - %s not implemented\n", __FILE__, __LINE__, __FUNCTION__);
	return -ENOSYS;
#endif
}

//***************************************************************************************
int CDdcSrv::CtrlIniFile(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_IniFile pFile = (PBRD_IniFile)args;
	if(BRDctrl_DDC_WRITEINIFILE == cmd)
		Status = SaveProperties(pModule, pFile->fileName, pFile->sectionName);
	else
		Status = SetProperties(pModule, pFile->fileName, pFile->sectionName);
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlCfg(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	Status = GetCfg((PBRD_DdcCfg)args);
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlChanMask(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_DDC_SETCHANMASK == cmd)
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
int CDdcSrv::CtrlFormat(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
        //int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_DDC_SETFORMAT == cmd)
	{
		ULONG fmt = *(ULONG*)args;
                SetFormat(pModule, fmt);
	}
	else
	{
		ULONG fmt;
                GetFormat(pModule, fmt);
		*(ULONG*)args = fmt;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::CtrlRate(
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
	if(BRDctrl_DDC_SETRATE == cmd)
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
int CDdcSrv::CtrlClkMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ClkMode pDdcClk = (PBRD_ClkMode)args;
	if(BRDctrl_DDC_SETCLKMODE == cmd)
	{
		Status = SetClkSource(pModule, pDdcClk->src);
		Status = SetClkValue(pModule, pDdcClk->src, pDdcClk->value);
	}
	else
	{
		ULONG src = pDdcClk->src;
		Status = GetClkSource(pModule, src);
		pDdcClk->src = src;
		Status = GetClkValue(pModule, pDdcClk->src, pDdcClk->value);
	}
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlSyncMode(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   )
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_SyncMode pSyncMode = (PBRD_SyncMode)args;
	if(BRDctrl_DDC_SETSYNCMODE == cmd)
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
int CDdcSrv::CtrlMaster(
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
	if(BRDctrl_DDC_SETMASTER == cmd)
	{
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pMode0->ByBits.Master = *pMasterMode >> 1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.tetr = m_DdcTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pMode0->ByBits.Master = *pMasterMode & 0x1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		param.tetr = m_DdcTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		*pMasterMode = pMode0->ByBits.Master;
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		*pMasterMode |= (pMode0->ByBits.Master << 1);
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::CtrlStartMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
//	PBRD_DdcStartMode pDdcStartMode = (PBRD_DdcStartMode)args;
	if(BRDctrl_DDC_SETSTARTMODE == cmd)
		Status = SetStartMode(pModule, args);
	else
        Status = GetStartMode(pModule, args);
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlTitleMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_EnVal pTitleMode = (PBRD_EnVal)args;
	if(BRDctrl_DDC_SETTITLEMODE == cmd)
		Status = SetTitleMode(pModule, pTitleMode);
	else
        Status = GetTitleMode(pModule, pTitleMode);
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlTitleData(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PULONG pTitleData = (PULONG)args;
	if(BRDctrl_DDC_SETTITLEDATA == cmd)
		Status = SetTitleData(pModule, pTitleData);
	else
        Status = GetTitleData(pModule, pTitleData);
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlCnt(
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
	case BRDctrl_DDC_SETSTDELAY:
		SetCnt(pModule, 0, pEnVal);
		break;
	case BRDctrl_DDC_GETSTDELAY:
		GetCnt(pModule, 0, pEnVal);
		break;
	case BRDctrl_DDC_SETACQDATA:
		SetCnt(pModule, 1, pEnVal);
		break;
	case BRDctrl_DDC_GETACQDATA:
		GetCnt(pModule, 1, pEnVal);
		break;
	case BRDctrl_DDC_SETSKIPDATA:
		SetCnt(pModule, 2, pEnVal);
		break;
	case BRDctrl_DDC_GETSKIPDATA:
		GetCnt(pModule, 2, pEnVal);
		break;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::CtrlFC(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ValChan pValChan = (PBRD_ValChan)args;
	if(BRDctrl_DDC_SETFC == cmd)
		Status = SetFC(pModule, pValChan->value, pValChan->chan);
	else
        Status = GetFC(pModule, pValChan->value, pValChan->chan);
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlDecim(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ValChan pValChan = (PBRD_ValChan)args;
//	ULONG Decim = (ULONG)pValChan->value;
	double Decim = pValChan->value;
	if(BRDctrl_DDC_SETDECIM == cmd)
		Status = SetDecim(pModule, Decim, pValChan->chan);
	else
        Status = GetDecim(pModule, Decim, pValChan->chan);
	pValChan->value = Decim;
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlFrame(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG len = *(PULONG)args;
	if(BRDctrl_DDC_SETFRAME == cmd)
		Status = SetFrame(pModule, len);
	else
        Status = GetFrame(pModule, len);
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlInpSrc(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_EnVal pEnVal = (PBRD_EnVal)args;
	ULONG AdcNum = pEnVal->enable;
	if(BRDctrl_DDC_SETINPSRC == cmd)
		Status = SetInpSrc(pModule, AdcNum, pEnVal->value);
	else
        Status = GetInpSrc(pModule, AdcNum, pEnVal->value);
	pEnVal->enable = AdcNum;
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlProgram(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status = SetProgram(pModule, args);
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlDDCSync(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_DdcSync pSync = (PBRD_DdcSync)args;
	if(BRDctrl_DDC_SETDDCSYNC == cmd)
		Status = SetDDCSync(pModule, pSync->mode, pSync->prog, pSync->async);
	else
	{
		ULONG mode = pSync->mode;
		ULONG async = pSync->async;
        Status = GetDDCSync(pModule, mode, async);
		pSync->prog = 0;
		pSync->mode = mode;
		pSync->async = async;
	}
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlTestMode(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG mode = *(PULONG)args;
	if(BRDctrl_DDC_SETTESTMODE == cmd)
		Status = SetTestMode(pModule, mode);
	else
		Status = GetTestMode(pModule, mode);
	*(PULONG)args = mode;
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlSpecific(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_DdcSpec pSpec = (PBRD_DdcSpec)args;
	if(BRDctrl_DDC_SETSPECIFIC == cmd)
		Status = SetSpecific(pModule, pSpec);
	else
		Status = GetSpecific(pModule, pSpec);
	return Status;
}

//***************************************************************************************
int CDdcSrv::CtrlTarget(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG target = *(ULONG*)args;
	if(BRDctrl_DDC_SETTARGET == cmd)
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
int CDdcSrv::CtrlFifoReset (
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
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
int CDdcSrv::CtrlEnable (
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG Enbl = *(PULONG)args;
	Status = StartEnable(pModule, Enbl);
	return Status;
	//DEVS_CMD_Reg param;
	//param.idxMain = m_index;
	//param.tetr = m_DdcTetrNum;
	//param.reg = ADM2IFnr_MODE0;
	//pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	//pMode0->ByBits.Start = *(PULONG)args;
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::CtrlPrepareStart(void *pDev, void *pServData, ULONG cmd, void *args)
{
	CModule* pModule = (CModule*)pDev;
	PrepareStart(pModule, args);
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::CtrlFifoStatus(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = ADM2IFnr_STATUS;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
//	ULONG data = pStatus->ByBits.Underflow;
	ULONG data = pStatus->AsWhole;
	*(PULONG)args = data;

	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	param.reg = ADM2IFnr_STMODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);


	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::CtrlGetData(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
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

// ***************** End of file DdcCtrl.cpp ***********************
