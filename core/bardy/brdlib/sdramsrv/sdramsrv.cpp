/*
 ***************** File SdramSrv.cpp ***********************
 *
 * BRD Driver for ADM-interface Sdram service
 *
 * (C) InSys by Dorokhin A. May 2004
 *
 ******************************************************
*/

#include "module.h"
#include "sdramsrv.h"

#define	CURRFILE "SDRAMSRV"

static CMD_Info SETREADMODE_CMD		= { BRDctrl_SDRAM_SETREADMODE,   	0, 0, 0, NULL};
static CMD_Info GETREADMODE_CMD		= { BRDctrl_SDRAM_GETREADMODE,   	1, 0, 0, NULL};
static CMD_Info SETMEMSIZE_CMD		= { BRDctrl_SDRAM_SETMEMSIZE,    	0, 0, 0, NULL};
static CMD_Info GETMEMSIZE_CMD		= { BRDctrl_SDRAM_GETMEMSIZE,    	1, 0, 0, NULL};
static CMD_Info SETSTARTADDR_CMD	= { BRDctrl_SDRAM_SETSTARTADDR,  	0, 0, 0, NULL};
static CMD_Info GETSTARTADDR_CMD	= { BRDctrl_SDRAM_GETSTARTADDR,  	1, 0, 0, NULL};
static CMD_Info SETREADADDR_CMD		= { BRDctrl_SDRAM_SETREADADDR,   	0, 0, 0, NULL};
static CMD_Info GETREADADDR_CMD		= { BRDctrl_SDRAM_GETREADADDR,	 	1, 0, 0, NULL};
static CMD_Info SETPOSTTRIG_CMD		= { BRDctrl_SDRAM_SETPOSTTRIGER,	0, 0, 0, NULL};
static CMD_Info GETPOSTTRIG_CMD		= { BRDctrl_SDRAM_GETPOSTTRIGER,	1, 0, 0, NULL};
static CMD_Info GETACQSIZE_CMD		= { BRDctrl_SDRAM_GETACQSIZE,		1, 0, 0, NULL};
static CMD_Info ISACQCOMPLETE_CMD	= { BRDctrl_SDRAM_ISACQCOMPLETE,	1, 0, 0, NULL};
static CMD_Info ISPASSMEMEND_CMD	= { BRDctrl_SDRAM_ISPASSMEMEND,		1, 0, 0, NULL};
static CMD_Info GETPRETRIGEVENT_CMD	= { BRDctrl_SDRAM_GETPRETRIGEVENT,	1, 0, 0, NULL};
static CMD_Info GETCFG_CMD			= { BRDctrl_SDRAM_GETCFG,			1, 0, 0, NULL};
static CMD_Info GETCFGEX_CMD		= { BRDctrl_SDRAM_GETCFGEX,			1, 0, 0, NULL};
static CMD_Info SETFIFOMODE_CMD		= { BRDctrl_SDRAM_SETFIFOMODE,   	0, 0, 0, NULL};
static CMD_Info GETFIFOMODE_CMD		= { BRDctrl_SDRAM_GETFIFOMODE,   	1, 0, 0, NULL};
static CMD_Info SETSEL_CMD			= { BRDctrl_SDRAM_SETSEL,   		0, 0, 0, NULL};
static CMD_Info GETSEL_CMD			= { BRDctrl_SDRAM_GETSEL,   		1, 0, 0, NULL};
static CMD_Info SETTESTSEQ_CMD		= { BRDctrl_SDRAM_SETTESTSEQ,		0, 0, 0, NULL};
static CMD_Info GETTESTSEQ_CMD		= { BRDctrl_SDRAM_GETTESTSEQ,		1, 0, 0, NULL};

static CMD_Info GETDLGPAGES_CMD		= { BRDctrl_SDRAM_GETDLGPAGES,		0, 0, 0, NULL};

static CMD_Info FIFORESET_CMD		= { BRDctrl_SDRAM_FIFORESET,		0, 0, 0, NULL};
static CMD_Info ENABLE_CMD			= { BRDctrl_SDRAM_ENABLE,   		0, 0, 0, NULL};
static CMD_Info FIFOSTATUS_CMD		= { BRDctrl_SDRAM_FIFOSTATUS,		1, 0, 0, NULL};
static CMD_Info GETDATA_CMD			= { BRDctrl_SDRAM_GETDATA,			0, 0, 0, NULL};
static CMD_Info PUTDATA_CMD			= { BRDctrl_SDRAM_PUTDATA,			0, 0, 0, NULL};
static CMD_Info READENABLE_CMD		= { BRDctrl_SDRAM_READENABLE,   	0, 0, 0, NULL};
static CMD_Info FLAGCLR_CMD			= { BRDctrl_SDRAM_FLAGCLR,   		0, 0, 0, NULL};

static CMD_Info GETSRCSTREAM_CMD	= { BRDctrl_SDRAM_GETSRCSTREAM,		1, 0, 0, NULL};

static CMD_Info IRQACQCOMPLETE_CMD	= { BRDctrl_SDRAM_IRQACQCOMPLETE,	0, 0, 0, NULL};
static CMD_Info WAITACQCOMPLETE_CMD	= { BRDctrl_SDRAM_WAITACQCOMPLETE,	1, 0, 0, NULL};
static CMD_Info WAITACQCOMPLEEX_CMD	= { BRDctrl_SDRAM_WAITACQCOMPLETEEX,1, 0, 0, NULL};

//***************************************************************************************
//CSdramSrv::CSdramSrv(int idx, int srv_num, CModule* pModule, PSDRAMSRV_CFG pCfg) :
//	CService(idx, _BRDC("BASESDRAM"), srv_num, pModule, pCfg, sizeof(SDRAMSRV_CFG))
CSdramSrv::CSdramSrv(int idx, const BRDCHAR *name, int srv_num, CModule* pModule, PVOID pCfg, ULONG cfgSize) :
	CService(idx, name, srv_num, pModule, pCfg, cfgSize)
{
	m_attribute = 
//			BRDserv_ATTR_SDRAM |
			BRDserv_ATTR_DIRECTION_IN |
			BRDserv_ATTR_DIRECTION_OUT |
			BRDserv_ATTR_STREAMABLE_IN |
			BRDserv_ATTR_STREAMABLE_OUT |
//			BRDserv_ATTR_UNVISIBLE |
//			BRDserv_ATTR_SUBSERVICE_ONLY |
			BRDserv_ATTR_EXCLUSIVE_ONLY;

	Init(&GETCFG_CMD, (CmdEntry)&CSdramSrv::CtrlCfg);
	Init(&GETCFGEX_CMD, (CmdEntry)&CSdramSrv::CtrlCfgEx);

	Init(&GETDLGPAGES_CMD, (CmdEntry)&CSdramSrv::CtrlGetPages);
	//Init(&ENDPAGESDLG_CMD, (CmdEntry)CtrlInitEndDlg);
	//Init(&SETPROPDLG_CMD, (CmdEntry)CtrlDlgProperty);
	//Init(&GETPROPDLG_CMD, (CmdEntry)CtrlDlgProperty);

	Init(&SETREADMODE_CMD, (CmdEntry)&CSdramSrv::CtrlReadMode);
	Init(&GETREADMODE_CMD, (CmdEntry)&CSdramSrv::CtrlReadMode);
	Init(&SETMEMSIZE_CMD, (CmdEntry)&CSdramSrv::CtrlSetMemSize);
	Init(&GETMEMSIZE_CMD, (CmdEntry)&CSdramSrv::CtrlGetMemSize);
	Init(&SETSTARTADDR_CMD, (CmdEntry)&CSdramSrv::CtrlSetStartAddr);
	Init(&GETSTARTADDR_CMD, (CmdEntry)&CSdramSrv::CtrlGetStartAddr);
	Init(&SETREADADDR_CMD, (CmdEntry)&CSdramSrv::CtrlSetReadAddr);
	Init(&GETREADADDR_CMD, (CmdEntry)&CSdramSrv::CtrlGetReadAddr);
	Init(&SETPOSTTRIG_CMD, (CmdEntry)&CSdramSrv::CtrlSetPostTrig);
	Init(&GETPOSTTRIG_CMD, (CmdEntry)&CSdramSrv::CtrlGetPostTrig);
	Init(&SETFIFOMODE_CMD, (CmdEntry)&CSdramSrv::CtrlFifoMode);
	Init(&GETFIFOMODE_CMD, (CmdEntry)&CSdramSrv::CtrlFifoMode);
	Init(&SETSEL_CMD,	(CmdEntry)&CSdramSrv::CtrlSel);
	Init(&GETSEL_CMD,	(CmdEntry)&CSdramSrv::CtrlSel);
	Init(&SETTESTSEQ_CMD, (CmdEntry)&CSdramSrv::CtrlTestSeq);
	Init(&GETTESTSEQ_CMD, (CmdEntry)&CSdramSrv::CtrlTestSeq);

	Init(&GETACQSIZE_CMD, (CmdEntry)&CSdramSrv::CtrlGetAcqSize);
	Init(&ISACQCOMPLETE_CMD, (CmdEntry)&CSdramSrv::CtrlIsAcqComplete);
	Init(&ISPASSMEMEND_CMD, (CmdEntry)&CSdramSrv::CtrlIsPassMemEnd);
	Init(&GETPRETRIGEVENT_CMD, (CmdEntry)&CSdramSrv::CtrlGetPretrigEvent);

	//CService::Init(&SETSTARTMODE_CMD, (CmdEntry)CtrlSetStartMode);
	//CService::Init(&GETSTARTMODE_CMD, (CmdEntry)CtrlGetStartMode);

	Init(&FIFORESET_CMD, (CmdEntry)&CSdramSrv::CtrlFifoReset);
	Init(&ENABLE_CMD, (CmdEntry)&CSdramSrv::CtrlEnable);
	Init(&FIFOSTATUS_CMD, (CmdEntry)&CSdramSrv::CtrlFifoStatus);
	Init(&GETDATA_CMD, (CmdEntry)&CSdramSrv::CtrlGetData);
	Init(&PUTDATA_CMD, (CmdEntry)&CSdramSrv::CtrlPutData);
	Init(&READENABLE_CMD, (CmdEntry)&CSdramSrv::CtrlReadEnable);
	Init(&FLAGCLR_CMD, (CmdEntry)&CSdramSrv::CtrlFlagClear);

	Init(&GETSRCSTREAM_CMD, (CmdEntry)&CSdramSrv::CtrlGetAddrData);

	Init(&IRQACQCOMPLETE_CMD, (CmdEntry)&CSdramSrv::CtrlIrqAcqComplete);
	Init(&WAITACQCOMPLETE_CMD, (CmdEntry)&CSdramSrv::CtrlWaitAcqComplete);
	Init(&WAITACQCOMPLEEX_CMD, (CmdEntry)&CSdramSrv::CtrlWaitAcqCompleteEx);
	
#ifdef _WIN32
	BRDCHAR nameEvent[MAX_PATH];
	BRDC_sprintf(nameEvent, _BRDC("irqevent_%s_%d"), m_name, pModule->GetPID());
//	m_hAcqCompleteEvent = CreateEvent(NULL, TRUE, FALSE, nameEvent); // начальное состояние Non-Signaled
	m_hAcqCompleteEvent = CreateEvent(NULL, TRUE, TRUE, nameEvent); // начальное состояние Signaled
	//m_hAcqCompleteEvent = IPC_createEvent(nameEvent, TRUE, TRUE);
#endif
}

//***************************************************************************************
CSdramSrv::~CSdramSrv()
{
#ifdef _WIN32
	if(m_hAcqCompleteEvent)
		CloseHandle(m_hAcqCompleteEvent);
		//IPC_deleteEvent(m_hAcqCompleteEvent);
#endif
}

//***************************************************************************************
void CSdramSrv::GetSdramTetrNum(CModule* pModule)
{
	m_MemTetrNum = 0;//GetTetrNum(pModule, m_index, SDRAM_TETR_ID);
}

//***************************************************************************************
void CSdramSrv::GetSdramTetrNumEx(CModule* pModule, ULONG TetrInstNum)
{
	if(TetrInstNum == 1)
		GetSdramTetrNum(pModule);
	else
		m_MemTetrNum = -1;
}

//***************************************************************************************
ULONG CSdramSrv::CheckCfg(CModule* pModule)
{
	PSDRAMSRV_CFG pMemSrvCfg = (PSDRAMSRV_CFG)m_pConfig;
	pMemSrvCfg->ModuleCnt &= 0x7F;
	pMemSrvCfg->RowAddrBits &= 0x7F;
	pMemSrvCfg->ColAddrBits &= 0x7F;
	pMemSrvCfg->ModuleBanks &= 0x7F;
	pMemSrvCfg->ChipBanks &= 0x7F;
	if(!pMemSrvCfg->ModuleCnt)
		return BRDerr_SDRAM_NO_MEMORY;
	if(pMemSrvCfg->RowAddrBits < 11 || pMemSrvCfg->RowAddrBits > 14)
	    return BRDerr_SDRAM_BAD_PARAMETER;
	if(pMemSrvCfg->ColAddrBits < 8 || pMemSrvCfg->ColAddrBits > 13)
	    return BRDerr_SDRAM_BAD_PARAMETER;
	if(pMemSrvCfg->ModuleBanks < 1 || pMemSrvCfg->ModuleBanks > 2)
	    return BRDerr_SDRAM_BAD_PARAMETER;
	if(pMemSrvCfg->ChipBanks != 2 && pMemSrvCfg->ChipBanks != 4)
	    return BRDerr_SDRAM_BAD_PARAMETER;

	m_dwPhysMemSize =	(1 << pMemSrvCfg->RowAddrBits) *
						(1 << pMemSrvCfg->ColAddrBits) * 
						pMemSrvCfg->ModuleBanks * 
						pMemSrvCfg->ChipBanks *
						pMemSrvCfg->ModuleCnt * 2; // in 32-bit Words
	return BRDerr_OK;
}

//***************************************************************************************
void CSdramSrv::MemInit(CModule* pModule, ULONG init)
{
	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = m_index;
	RegParam.tetr = m_MemTetrNum;
	RegParam.reg = SDRAMnr_CFG;
	PSDRAM_CFG pMemCfg = (PSDRAM_CFG)&RegParam.val;
	PSDRAMSRV_CFG pMemSrvCfg = (PSDRAMSRV_CFG)m_pConfig;
	pMemCfg->ByBits.NumSlots = pMemSrvCfg->ModuleCnt ? (pMemSrvCfg->ModuleCnt - 1) : 0;
	pMemCfg->ByBits.RowAddr = pMemSrvCfg->RowAddrBits - 8;
	pMemCfg->ByBits.ColAddr = pMemSrvCfg->ColAddrBits - 8;
	pMemCfg->ByBits.BankModule = pMemSrvCfg->ModuleBanks - 1;
	pMemCfg->ByBits.BankChip = pMemSrvCfg->ChipBanks >> 2;
	pMemCfg->ByBits.InitMemCmd = init; //1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
//	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
//	pMemCfg->ByBits.InitMemCmd = 0;
//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
}

//***************************************************************************************
int CSdramSrv::CtrlIsAvailable(
							  void		*pDev,		// InfoSM or InfoBM
							  void		*pServData,	// Specific Service Data
							  ULONG		cmd,		// Command Code (from BRD_ctrl())
							  void		*args 		// Command Arguments (from BRD_ctrl())
							  )
{
	CModule* pModule = (CModule*)pDev;
	PSERV_CMD_IsAvailable pServAvailable = (PSERV_CMD_IsAvailable)args;
	//pServAvailable->attr = m_attribute;
	pServAvailable->isAvailable = 0;
	m_MemTetrModif = 0;

	m_MainTetrNum = GetTetrNum(pModule, m_index, MAIN_TETR_ID);

	ULONG TetrInstantNum = 1;
	BRDCHAR* pBuf = m_name + (BRDC_strlen(m_name) - 1);
	TetrInstantNum = BRDC_atoi(pBuf) + 1;
	//if(BRDC_strchr(pBuf, '1'))
	//	TetrInstantNum = 2;

	//GetSdramTetrNum(pModule);
	GetSdramTetrNumEx(pModule, TetrInstantNum);

	if(m_MainTetrNum != -1 && m_MemTetrNum != -1)
	{
		ULONG Status = CheckCfg(pModule);
		if (Status != BRDerr_OK)
		{
			//BRDCHAR dbg_buf[128];
			//BRDC_sprintf(dbg_buf, _BRDC("Debug ERROR: SDRAM CheckCfg ERROR! 0x%08X\n"), Status);
			//OutputDebugString(dbg_buf); // выводим в окно Visual Studio Output
			return Status;
		}
		if ((pServAvailable->attr & BRDserv_ATTR_STREAMABLE_IN) &&
			(m_attribute & BRDserv_ATTR_DIRECTION_IN))
				m_isAvailable = 1;
		else
			if ((pServAvailable->attr & BRDserv_ATTR_STREAMABLE_OUT) &&
				(m_attribute & BRDserv_ATTR_DIRECTION_OUT))
					m_isAvailable = 1;
			else
					m_isAvailable = 0;

		if(pServAvailable->attr == 0)
			m_isAvailable = 1;


//		PSDRAMSRV_CFG pMemSrvCfg = (PSDRAMSRV_CFG)m_pConfig;
//
//		if(!pMemSrvCfg->isAlreadyInit)
//		{
//			pMemSrvCfg->isAlreadyInit = 1;
//			DEVS_CMD_Reg RegParam;
//			RegParam.idxMain = m_index;
//			RegParam.tetr = m_MemTetrNum;
//
//			RegParam.reg = ADM2IFnr_MODE0;
//			RegParam.val = 0;
//		//	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
//			PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&RegParam.val;
//			pMode0->ByBits.Reset = 1;
//			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
//			for(int i = 1; i < 32; i++)
//			{
//				RegParam.reg = i;
//				RegParam.val = 0;
//				pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
//			}
//			RegParam.reg = ADM2IFnr_MODE0;
//			pMode0->ByBits.Reset = 0;
//			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
//
//			RegParam.reg = ADM2IFnr_MODE0;
//			pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
//	//		PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&RegParam.val;
//			pMode0->ByBits.Master = 1;
//			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
//
//			RegParam.reg = 0;
//			RegParam.val = 0;
//#ifdef _WIN32
//			RegParam.pBuf = m_hAcqCompleteEvent;
//#else
//			RegParam.pBuf = NULL;
//#endif
//			pModule->RegCtrl(DEVScmd_CLEARSTATIRQ, &RegParam);
//
//			RegParam.reg = ADM2IFnr_STATUS;
//			pModule->RegCtrl(DEVScmd_REGREADDIR, &RegParam);
//			PSDRAM_STATUS pStatus = (PSDRAM_STATUS)&RegParam.val;
//			if(!pStatus->ByBits.InitMem)
//			{
//				MemInit(pModule, 1);
//
//				RegParam.reg = ADM2IFnr_STATUS;
//				pStatus = (PSDRAM_STATUS)&RegParam.val;
//				for(int i = 0; i < 100; i++)
//				{
//					pModule->RegCtrl(DEVScmd_REGREADDIR, &RegParam);
//					if(pStatus->ByBits.InitMem)
//						break;
//					IPC_delay(10);
//				}
//				if(!pStatus->ByBits.InitMem)
//					return BRDerr_WAIT_TIMEOUT;
//			}
//			else
//				MemInit(pModule, 0);
//		}
	}
	else
		m_isAvailable = 0;

	pServAvailable->isAvailable = m_isAvailable;
	pServAvailable->attr = m_attribute;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::ExtraInit(CModule* pModule)
{
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlCapture(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	if(m_MainTetrNum != -1 && m_MemTetrNum != -1)
	{
		PSDRAMSRV_CFG pMemSrvCfg = (PSDRAMSRV_CFG)m_pConfig;

		if(!pMemSrvCfg->isAlreadyInit)
		{
			pMemSrvCfg->isAlreadyInit = 1;
			DEVS_CMD_Reg RegParam;
			RegParam.idxMain = m_index;
			RegParam.tetr = m_MemTetrNum;

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

			RegParam.reg = 0;
			RegParam.val = 0;
#ifdef _WIN32
			RegParam.pBuf = m_hAcqCompleteEvent;
#else
			RegParam.pBuf = NULL;
#endif
			pModule->RegCtrl(DEVScmd_CLEARSTATIRQ, &RegParam);

			ULONG Status = ExtraInit(pModule);
			if (Status != BRDerr_OK)
				return Status;

			RegParam.reg = ADM2IFnr_STATUS;
			pModule->RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			PSDRAM_STATUS pStatus = (PSDRAM_STATUS)&RegParam.val;
			if(!pStatus->ByBits.InitMem)
			{
				MemInit(pModule, 1);
#ifdef _WIN32
					Sleep(50);
#else
					IPC_delay(10);
#endif

				//BRDC_printf(_BRDC("Init Memory.......!\n"));
				RegParam.reg = ADM2IFnr_STATUS;
				pStatus = (PSDRAM_STATUS)&RegParam.val;
				for(int i = 0; i < 100; i++)
				{
					pModule->RegCtrl(DEVScmd_REGREADDIR, &RegParam);
					if (pStatus->ByBits.InitMem)
					{
						//BRDC_printf(_BRDC("Init Memory  OK!\n"));
						break;
					}
#ifdef _WIN32
					Sleep(50);
#else
					IPC_delay(50);
#endif
				}
				if(!pStatus->ByBits.InitMem)
				{
					//BRDC_printf(_BRDC("NOT Init Memory!\n"));
					return BRDerr_WAIT_TIMEOUT;
				}
			}
			else
			{
				MemInit(pModule, 0); // необходимо для памяти DDR2 и DDR
				pModule->RegCtrl(DEVScmd_REGREADDIR, &RegParam);
                                //int init = pStatus->ByBits.InitMem;
			}
		}
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlRelease(
						void			*pDev,		// InfoSM or InfoBM
						void			*pServData,	// Specific Service Data
						ULONG			cmd,		// Command Code (from BRD_ctrl())
						void			*args 		// Command Arguments (from BRD_ctrl())
						)
{
	PSDRAMSRV_CFG pMemSrvCfg = (PSDRAMSRV_CFG)m_pConfig;
	pMemSrvCfg->isAlreadyInit = 0;
	return BRDerr_CMD_UNSUPPORTED; // for free subservice
}

//***************************************************************************************
int CSdramSrv::CtrlGetAddrData(
							void			*pDev,		// InfoSM or InfoBM
							void			*pServData,	// Specific Service Data
							ULONG			cmd,		// Command Code (from BRD_ctrl())
							void			*args 		// Command Arguments (from BRD_ctrl())
							)
{
	ULONG m_AdmNum;
//	BRDCHAR buf[SERVNAME_SIZE];
//	BRDC_strcpy(buf, m_name);
//	BRDCHAR* pBuf = buf + (BRDC_strlen(buf) - 2);
//	if(BRDC_strchr(pBuf, '1'))
////	if(_tcschr(m_name, '1'))
//		m_AdmNum = 1;
//	else
		m_AdmNum = 0;
	*(ULONG*)args = (m_AdmNum << 16) | m_MemTetrNum;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlGetPages(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
#ifdef _WIN32
	CModule* pModule = (CModule*)pDev;
	PBRD_PropertyList pList = (PBRD_PropertyList)args;

	if(cmd == BRDctrl_SDRAM_GETDLGPAGES)
	{
		// Open Library
		PSDRAMSRV_CFG pSrvCfg = (PSDRAMSRV_CFG)m_pConfig;
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
#else
	fprintf(stderr, "%s, %d: - %s not implemented\n", __FILE__, __LINE__, __FUNCTION__);
	return -ENOSYS;
#endif
	return BRDerr_OK;
}

//***************************************************************************************
void CSdramSrv::FreeInfoForDialog(PVOID pInfo)
{
	PSDRAMSRV_INFO pSrvInfo = (PSDRAMSRV_INFO)pInfo;
	delete pSrvInfo;
}

//	ULONG		StartAddr;			// начальный адрес сбора данных (активной зоны) (в 32-битных словах)
//	ULONG		ReadAddr;			// начальный адрес чтения (только в произвольном режиме)(в 32-битных словах)
//	ULONG		PostTrigSize;		// размер пост-триггера (только в режиме претриггера) (в 32-битных словах)
//***************************************************************************************
PVOID CSdramSrv::GetInfoForDialog(CModule* pDev)
{
	PSDRAMSRV_CFG pSrvCfg = (PSDRAMSRV_CFG)m_pConfig;
	PSDRAMSRV_INFO pSrvInfo = new SDRAMSRV_INFO;
	pSrvInfo->Size = sizeof(SDRAMSRV_INFO);
	pSrvInfo->SlotCnt = pSrvCfg->SlotCnt;			// количество установленных слотов
	pSrvInfo->ModuleCnt = pSrvCfg->ModuleCnt;		// количество установленных DIMM-модулей(занятых слотов)
	pSrvInfo->RowAddrBits = pSrvCfg->RowAddrBits;	// количество разрядов адреса строк
	pSrvInfo->ColAddrBits = pSrvCfg->ColAddrBits;	// количество разрядов адреса столбцов
	pSrvInfo->ModuleBanks = pSrvCfg->ModuleBanks;	// количество банков в DIMM-модуле
	pSrvInfo->ChipBanks = pSrvCfg->ChipBanks;		// количество банков в микросхемах DIMM-модуля

	CSdramSrv::GetReadMode(pDev, pSrvInfo->ReadMode); // режим чтения памяти (авто/произвольный)

	BRDCHAR module_name[40];
//	sprintf(module_name, " (%s)", pDev->m_name);
	BRDC_sprintf(module_name, _BRDC(" (%s)"), pDev->GetName());
//	PBASE_Info binfo = pModule->GetBaseInfo();
//	sprintf(module_name, " (%s)", binfo->boardName);
	BRDC_strcpy(pSrvInfo->Name, m_name);
	BRDC_strcat(pSrvInfo->Name, module_name); //_tcscat_s

	return pSrvInfo;
//	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::SetPropertyFromDialog(void	*pDev, void	*args)
{
	CModule* pModule = (CModule*)pDev;
	PSDRAMSRV_INFO pInfo = (PSDRAMSRV_INFO)args;
	CSdramSrv::SetReadMode(pModule, pInfo->ReadMode);

	return BRDerr_OK;
}

//***************************************************************************************
ULONG CSdramSrv::GetStartAddr(CModule* pModule)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_STADDRL;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG start_addr = param.val & 0xffff;
	param.reg = SDRAMnr_STADDRH;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	start_addr |= (param.val << 16);
	return start_addr;
}

//***************************************************************************************
ULONG CSdramSrv::GetEndAddr(CModule* pModule)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_ENDADDRL;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG end_addr = param.val & 0xffff;
	param.reg = SDRAMnr_ENDADDRH;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	end_addr |= (param.val << 16);
	return end_addr;
}

//***************************************************************************************
ULONG CSdramSrv::GetTrigCnt(CModule* pModule)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_TRIGCNTL;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG trig_cnt = param.val & 0xffff;
	param.reg = SDRAMnr_TRIGCNTH;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	trig_cnt |= (param.val << 16);
	return trig_cnt;
}
/*
// ***************************************************************************************
void CSdramSrv::GetStartStopMode(CModule* pModule, PBRD_StartMode pStartStopMode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;//ADM2IF_ntBASEDAC;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_STMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
		pStartStopMode->startSrc	= pStMode->ByBits.SrcStart;
		pStartStopMode->trigStopSrc	= pStMode->ByBits.SrcStop;
		pStartStopMode->startInv	= pStMode->ByBits.InvStart;
		pStartStopMode->stopInv		= pStMode->ByBits.InvStop;
		pStartStopMode->trigOn		= pStMode->ByBits.TrigStart;
		pStartStopMode->pretrig		= pStMode->ByBits.Pretrig;
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;//ADM2IF_ntMAIN;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.reg = ADM2IFnr_STMODE;
			pModule->RegCtrl(DEVScmd_REGREADIND, &param);
			PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
			pStartStopMode->startSrc	= pStMode->ByBits.SrcStart;
			pStartStopMode->trigStopSrc	= pStMode->ByBits.SrcStop;
			pStartStopMode->startInv	= pStMode->ByBits.InvStart;
			pStartStopMode->stopInv		= pStMode->ByBits.InvStop;
			pStartStopMode->trigOn		= pStMode->ByBits.TrigStart;
			pStartStopMode->pretrig		= pStMode->ByBits.Pretrig;
		}
//		else
//		{ // Slave
//			source = BRD_clksEXTSYNX;
//		}
	}
}

// ***************************************************************************************
void CSdramSrv::SetStartStopMode(CModule* pModule, PBRD_StartMode pStartStopMode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_STMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
		pStMode->ByBits.SrcStart  = pStartStopMode->startSrc;
		pStMode->ByBits.SrcStop   = pStartStopMode->trigStopSrc;
		pStMode->ByBits.InvStart  = pStartStopMode->startInv;
		pStMode->ByBits.InvStop   = pStartStopMode->stopInv;
		pStMode->ByBits.TrigStart = pStartStopMode->trigOn;
		pStMode->ByBits.Pretrig   = pStartStopMode->pretrig;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.reg = ADM2IFnr_STMODE;
			pModule->RegCtrl(DEVScmd_REGREADIND, &param);
			PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
			pStMode->ByBits.SrcStart  = pStartStopMode->startSrc;
			pStMode->ByBits.SrcStop   = pStartStopMode->trigStopSrc;
			pStMode->ByBits.InvStart  = pStartStopMode->startInv;
			pStMode->ByBits.InvStop   = pStartStopMode->stopInv;
			pStMode->ByBits.TrigStart = pStartStopMode->trigOn;
			pStMode->ByBits.Pretrig   = pStartStopMode->pretrig;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		}
//		else
//			return BRDerr_; // функция в режиме Slave не выполнима
	}
}
*/
//***************************************************************************************
int CSdramSrv::GetCfg(PBRD_SdramCfg pCfg)
{
	PSDRAMSRV_CFG pSrvCfg = (PSDRAMSRV_CFG)m_pConfig;
	pCfg->SlotCnt	  = pSrvCfg->SlotCnt;
	pCfg->ModuleCnt	  = pSrvCfg->ModuleCnt;	
	pCfg->RowAddrBits = pSrvCfg->RowAddrBits;
	pCfg->ColAddrBits = pSrvCfg->ColAddrBits;
	pCfg->ModuleBanks = pSrvCfg->ModuleBanks;
	pCfg->ChipBanks = pSrvCfg->ChipBanks;
	pCfg->MemType = pSrvCfg->MemType;
	pCfg->Mode = pSrvCfg->Mode;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlCfg(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	Status = GetCfg((PBRD_SdramCfg)args);
	return Status;
}

//***************************************************************************************
int CSdramSrv::GetCfgEx(PBRD_SdramCfgEx pCfg)
{
	PSDRAMSRV_CFG pSrvCfg = (PSDRAMSRV_CFG)m_pConfig;
	pCfg->Size = sizeof(BRD_SdramCfgEx);
	pCfg->MemType = pSrvCfg->MemType;
	pCfg->Mode = pSrvCfg->Mode;
	pCfg->ModuleCnt	  = pSrvCfg->ModuleCnt;	
	pCfg->ModuleBanks = pSrvCfg->ModuleBanks;
	pCfg->RowAddrBits = pSrvCfg->RowAddrBits;
	pCfg->ColAddrBits = pSrvCfg->ColAddrBits;
	pCfg->ChipBanks = pSrvCfg->ChipBanks;
	pCfg->PrimWidth = 0;
	pCfg->ChipWidth = 0;
	pCfg->CapacityMbits = 0;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlCfgEx(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	Status = GetCfgEx((PBRD_SdramCfgEx)args);
	return Status;
}

//***************************************************************************************
int CSdramSrv::CtrlSetEnable(
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
	param.reg = SDRAMnr_MODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PSDRAM_MODE pMode = (PSDRAM_MODE)&param.val;
	pMode->ByBits.ReadEnbl = *(ULONG*)args;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlGetEnable(
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
	param.reg = SDRAMnr_MODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PSDRAM_MODE pMode = (PSDRAM_MODE)&param.val;
	*(ULONG*)args = pMode->ByBits.ReadEnbl;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlReadMode(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_SDRAM_SETREADMODE == cmd)
	{
		ULONG mode = *(ULONG*)args;
		Status = SetReadMode(pModule, mode);
	}
	else
	{
		ULONG mode;
		Status = GetReadMode(pModule, mode);
		*(ULONG*)args = mode;
	}
	return Status;
}

//***************************************************************************************
int CSdramSrv::SetReadMode(CModule* pModule, ULONG mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_MODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PSDRAM_MODE pMode = (PSDRAM_MODE)&param.val;
	pMode->ByBits.ReadMode = mode;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::GetReadMode(CModule* pModule, ULONG& mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_MODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PSDRAM_MODE pMode = (PSDRAM_MODE)&param.val;
	mode = pMode->ByBits.ReadMode;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlFifoMode(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_SDRAM_SETFIFOMODE == cmd)
	{
		ULONG mode = *(ULONG*)args;
		Status = SetFifoMode(pModule, mode);
	}
	else
	{
		ULONG mode;
		Status = GetFifoMode(pModule, mode);
		*(ULONG*)args = mode;
	}
	return Status;
}

//***************************************************************************************
int CSdramSrv::CtrlSel(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG sel = *(ULONG*)args;
	Status = SetSel(pModule, sel, cmd);
	*(ULONG*)args = sel;
	return Status;
}

//***************************************************************************************
int CSdramSrv::SetSel(CModule* pModule, ULONG& sel, ULONG cmd)
{
	return BRDerr_CMD_UNSUPPORTED;
}

//***************************************************************************************
int CSdramSrv::SetFifoMode(CModule* pModule, ULONG mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_MODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PSDRAM_MODE pMode = (PSDRAM_MODE)&param.val;
	pMode->ByBits.FifoMode = mode;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.val = 0;
	param.reg = SDRAMnr_ENDADDRL;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = SDRAMnr_ENDADDRH;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = SDRAMnr_STADDRL;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = SDRAMnr_STADDRH;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = SDRAMnr_RDADDRL;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = SDRAMnr_RDADDRH;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::GetFifoMode(CModule* pModule, ULONG& mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_MODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PSDRAM_MODE pMode = (PSDRAM_MODE)&param.val;
	mode = pMode->ByBits.FifoMode;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlReadEnable(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	if (3 == m_MemTetrModif) // SDRAMDDR3X_TETR_ID (0x9B) tetrada of memory
	{
		param.reg = SDRAMnr_RDSTART;
		param.val = 1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		//param.val = 0;
		//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		param.reg = SDRAMnr_MODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PSDRAM_MODE pMode = (PSDRAM_MODE)&param.val;
		pMode->ByBits.ReadEnbl = *(ULONG*)args;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlSetStartAddr(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	ULONG start_addr = *(ULONG*)args;
	ULONG active_size = GetEndAddr(pModule) - start_addr + 1;
	if(start_addr + active_size > m_dwPhysMemSize) 
		start_addr = m_dwPhysMemSize - active_size;
	if(2 == m_MemTetrModif) // ADS10x2G
		start_addr = (start_addr >> 11) << 11; // align on 8192 bytes (1024 64-bit words)
	else
		start_addr = (start_addr >> 8) << 8; // align on 1024 bytes (256 32-bit words)
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_STADDRL;
	param.val = start_addr & 0xffff;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = SDRAMnr_STADDRH;
	param.val = start_addr >> 16;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	*(ULONG*)args = start_addr;
	// trig_cnt
	ULONG tirg_cnt = GetTrigCnt(pModule);
	if(tirg_cnt > active_size - 16)
	{
		tirg_cnt = active_size - 16;
		CtrlSetPostTrig(pDev, NULL, 0, &tirg_cnt);
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlGetStartAddr(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	*(ULONG*)args = GetStartAddr(pModule);
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlSetMemSize(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	ULONG start_addr = GetStartAddr(pModule);
	ULONG active_size = *(ULONG*)args;
	if(2 == m_MemTetrModif) // ADS10x2G
	{
		if(active_size > 0xfffff800) 
			active_size = 0xfffff800;
		if(active_size & 0x7ff)
			active_size = ((active_size >> 11) + 1) << 11;
		if(start_addr + active_size > m_dwPhysMemSize) 
			active_size = m_dwPhysMemSize - start_addr;
		active_size = (active_size >> 11) << 11; // align on 8192 bytes (1024 64-bit words)
	}
	else
	{
		if(active_size > 0xffffff00) 
			active_size = 0xffffff00;
		if(active_size & 0xff)
			active_size = ((active_size >> 8) + 1) << 8;
		if(start_addr + active_size > m_dwPhysMemSize) 
			active_size = m_dwPhysMemSize - start_addr;
		active_size = (active_size >> 8) << 8; // align on 1024 bytes (256 32-bit words)
	}
	if(!active_size && pServData)
	{
		PUNDERSERV_Cmd pReleaseCmd = (PUNDERSERV_Cmd)pServData;
		pReleaseCmd->code = SERVcmd_SYS_RELEASE;
		PSDRAMSRV_CFG pMemSrvCfg = (PSDRAMSRV_CFG)m_pConfig;
		pMemSrvCfg->isAlreadyInit = 0;
	}
	else
	{
		ULONG end_addr = start_addr + active_size - 1;
		DEVS_CMD_Reg param;
		param.idxMain = m_index;
		param.tetr = m_MemTetrNum;
		param.reg = SDRAMnr_ENDADDRL;
		param.val = end_addr & 0xffff;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.reg = SDRAMnr_ENDADDRH;
		param.val = end_addr >> 16;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		*(ULONG*)args = active_size;

		// trig_cnt
		ULONG tirg_cnt = GetTrigCnt(pModule);
		if(tirg_cnt > active_size - 16)
		{
			tirg_cnt = active_size - 16;
			CtrlSetPostTrig(pDev, NULL, 0, &tirg_cnt);
		}
	}

	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlGetMemSize(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	ULONG start_addr = GetStartAddr(pModule);
	ULONG end_addr = GetEndAddr(pModule);
	*(ULONG*)args = end_addr - start_addr + 1;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlSetReadAddr(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	ULONG read_addr = *(ULONG*)args;
	if(read_addr > m_dwPhysMemSize) 
		read_addr = m_dwPhysMemSize;
	if(2 == m_MemTetrModif) // ADS10x2G
		read_addr = (read_addr >> 11) << 11; // align on 8192 bytes (1024 64-bit words)
	else
		read_addr = (read_addr >> 8) << 8; // align on 1024 bytes (256 32-bit words)
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_RDADDRL;
	param.val = read_addr & 0xffff;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = SDRAMnr_RDADDRH;
	param.val = read_addr >> 16;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	*(ULONG*)args = read_addr;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlGetReadAddr(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_RDADDRL;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG read_addr = param.val & 0xffff;
	param.reg = SDRAMnr_RDADDRH;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	read_addr |= (param.val << 16);
	*(ULONG*)args = read_addr;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlSetPostTrig(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	ULONG trig_cnt = *(ULONG*)args;
	ULONG start_addr = GetStartAddr(pModule);
	ULONG end_addr = GetEndAddr(pModule);
	ULONG active_size = end_addr - start_addr + 1;
	ULONG max_trig_cnt = active_size - 16;
	if(trig_cnt > max_trig_cnt)
		trig_cnt = max_trig_cnt;
	trig_cnt = (trig_cnt >> 1) << 1; // align on 2 words
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_TRIGCNTL;
	param.val = trig_cnt & 0xffff;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = SDRAMnr_TRIGCNTH;
	param.val = trig_cnt >> 16;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	*(ULONG*)args = trig_cnt;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlGetPostTrig(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	*(ULONG*)args = GetTrigCnt(pModule);
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::SetTestSeq(CModule* pModule, ULONG mode)
{
	return BRDerr_CMD_UNSUPPORTED;
}

//***************************************************************************************
int CSdramSrv::GetTestSeq(CModule* pModule, ULONG& mode)
{
	return BRDerr_CMD_UNSUPPORTED;
}

//***************************************************************************************
int CSdramSrv::CtrlTestSeq(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG mode = *(PULONG)args;
	if(BRDctrl_SDRAM_SETTESTSEQ == cmd)
		Status = SetTestSeq(pModule, mode);
	else
		Status = GetTestSeq(pModule, mode);
	*(PULONG)args = mode;
	return Status;
}

////***************************************************************************************
//int CSdramSrv::CtrlSetStartMode(
//					void		*pDev,		// InfoSM or InfoBM
//					void		*pServData,	// Specific Service Data
//					ULONG		cmd,		// Command Code (from BRD_ctrl())
//					void		*args 		// Command Arguments (from BRD_ctrl())
//				   )
//{
//	CModule* pModule = (CModule*)pDev;
//	PBRD_StartMode pStartMode = (PBRD_StartMode)args;
//	SetStartStopMode(pModule, pStartMode);
//	return BRDerr_OK;
//}
//
////***************************************************************************************
//int CSdramSrv::CtrlGetStartMode(
//					void		*pDev,		// InfoSM or InfoBM
//					void		*pServData,	// Specific Service Data
//					ULONG		cmd,		// Command Code (from BRD_ctrl())
//					void		*args 		// Command Arguments (from BRD_ctrl())
//					)
//{
//	CModule* pModule = (CModule*)pDev;
//	PBRD_StartMode pStartMode = (PBRD_StartMode)args;
//	GetStartStopMode(pModule, pStartMode);
//	return BRDerr_OK;
//}

//***************************************************************************************
int CSdramSrv::CtrlGetAcqSize(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	ULONG start_addr = GetStartAddr(pModule);
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_ACQCNTL;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG acq_size = param.val & 0xffff;
	param.reg = SDRAMnr_ACQCNTH;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	acq_size |= (param.val << 16);
	acq_size -= start_addr;
	if(!acq_size)
	{
		ULONG end_addr = GetEndAddr(pModule);
		acq_size = end_addr - start_addr + 1;
	}
	*(ULONG*)args = acq_size;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlIsAcqComplete(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = ADM2IFnr_STATUS;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	PSDRAM_STATUS pStatus = (PSDRAM_STATUS)&param.val;
	ULONG data = pStatus->ByBits.AcqComplete;

	//if(data)
	//{
	//	//Sleep(1);
	//	param.reg = SDRAMnr_FLAGCLR;//SDRAMnr_ACQCNTL;
	//	param.val = 0x2000;
	//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//	//printf("AcqComplete = %d, %d\n", data, *(PULONG)args);
	//}
	*(PULONG)args = data;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlFlagClear(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_FLAGCLR;
	param.val = 0x2000;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlIsPassMemEnd(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = ADM2IFnr_STATUS;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	PSDRAM_STATUS pStatus = (PSDRAM_STATUS)&param.val;
	ULONG data = pStatus->ByBits.PassMem;
	*(PULONG)args = data;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlGetPretrigEvent(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
//	param.reg = SDRAMnr_SYNCEVENTL;
	param.reg = SDRAMnr_PRTEVENTLO;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG sync_event = param.val & 0xffff;
//	param.reg = SDRAMnr_SYNCEVENTH;
	param.reg = SDRAMnr_PRTEVENTHI;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	sync_event |= (param.val << 16);
	ULONG start_addr = GetStartAddr(pModule);
	*(ULONG*)args = sync_event - start_addr;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlFifoReset(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.FifoRes = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pMode0->ByBits.FifoRes = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	pMode0->ByBits.IrqEnbl = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlEnable(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.Start = *(PULONG)args;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

#ifdef _WIN32
	if(pMode0->ByBits.Start)
		ResetEvent(m_hAcqCompleteEvent); // сброс в состояние Non-Signaled (после разрешения записи в память, но перед разрешением АЦП)
#endif
	//IPC_resetEvent(m_hAcqCompleteEvent);
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlIrqAcqComplete(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	ULONG IrqEn = *(PULONG)args;
	// разрешим/запретим прерывания от флага завершения сбора
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;

	//param.reg = ADM2IFnr_MODE0;
	//pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	//pMode0->ByBits.IrqEnbl = IrqEn;
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = ADM2IFnr_IRQMASK;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PSDRAM_STATUS pIrqMask = (PSDRAM_STATUS)&param.val;
	pIrqMask->ByBits.AcqComplete = IrqEn;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	if(IrqEn)
	{
		param.reg = SDRAM_statACQCOMPLETE;	// irqmask
		param.val = 0;						// irqinv
#ifdef _WIN32
		param.pBuf = m_hAcqCompleteEvent;	// hEvent
#else 
		param.pBuf = NULL;	// hEvent
#endif
		//param.pBuf = IPC_getEvent(m_hAcqCompleteEvent);	// hEvent
		pModule->RegCtrl(DEVScmd_SETSTATIRQ, &param);
	}
	else
    {
#ifdef __linux__
        param.reg = SDRAM_statACQCOMPLETE; //> ????? Merge with else
#else
        param.reg = 0;
#endif
        param.val = 0;
#ifdef _WIN32
		param.pBuf = m_hAcqCompleteEvent;	// hEvent
#else 
		param.pBuf = NULL;	// hEvent
#endif
		//param.pBuf = IPC_getEvent(m_hAcqCompleteEvent);	// hEvent
		pModule->RegCtrl(DEVScmd_CLEARSTATIRQ, &param);
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlWaitAcqComplete(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = ADM2IFnr_IRQMASK;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PSDRAM_STATUS pIrqMask = (PSDRAM_STATUS)&param.val;
	
	if(pIrqMask->ByBits.AcqComplete)
	{
#ifdef _WIN32
		ULONG msTimeout = *(PULONG)args;
		int Status = WaitForSingleObject(m_hAcqCompleteEvent, msTimeout);
		//int Status = IPC_waitEvent(m_hAcqCompleteEvent, msTimeout);
		if(Status == WAIT_TIMEOUT) 
			return BRDerr_WAIT_TIMEOUT;
		else
			ResetEvent(m_hAcqCompleteEvent); // сброс в состояние Non-Signaled после завершения сбора
			//IPC_resetEvent(m_hAcqCompleteEvent);
#else
    param.reg = SDRAM_statACQCOMPLETE; // Use in set/clear
    param.val = *(PULONG)args; //Timeout;
    int Status = pModule->RegCtrl(DEVScmd_WAITSTATIRQ, &param);
    if(Status == -ETIMEDOUT)
        return BRDerr_WAIT_TIMEOUT;
#endif
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlWaitAcqCompleteEx(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = ADM2IFnr_IRQMASK;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PSDRAM_STATUS pIrqMask = (PSDRAM_STATUS)&param.val;
	
	if(pIrqMask->ByBits.AcqComplete)
	{
#ifdef _WIN32
		PBRD_WaitEvent pWaitEvent = (PBRD_WaitEvent)args;
		ULONG msTimeout = pWaitEvent->timeout;
		HANDLE Events[2];
		Events[0] = pWaitEvent->hAppEvent;
		Events[1] = m_hAcqCompleteEvent;
//		int Status = WaitForSingleObject(m_hAcqCompleteEvent, msTimeout);
		int Status = WaitForMultipleObjects( 2, Events, FALSE, msTimeout);
		if(Status == WAIT_TIMEOUT) 
			return BRDerr_WAIT_TIMEOUT;
		else
		{
			if(Status - WAIT_OBJECT_0)
				ResetEvent(m_hAcqCompleteEvent); // сброс в состояние Non-Signaled после завершения сбора
				//IPC_resetEvent(m_hAcqCompleteEvent);
			else
				return BRDerr_SIGNALED_APPEVENT;
		}
#else
        param.reg = SDRAM_statACQCOMPLETE; // Use in set/clear
        param.val = *(PULONG)args; //Timeout;
        int Status = pModule->RegCtrl(DEVScmd_WAITSTATIRQ, &param);
        if(Status == -ETIMEDOUT)
            return BRDerr_WAIT_TIMEOUT;
#endif
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlFifoStatus(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = ADM2IFnr_STATUS;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	PSDRAM_STATUS pStatus = (PSDRAM_STATUS)&param.val;
//	ULONG data = pStatus->ByBits.Underflow;
	ULONG data = pStatus->AsWhole;
	*(PULONG)args = data;
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlGetData(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
/*	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.FifoRes = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pMode0->ByBits.FifoRes = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
*/
	param.reg = ADM2IFnr_FSIZE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG HalfFifoSize = param.val << 2;

	PBRD_DataBuf pBuf = (PBRD_DataBuf)args;
	PUCHAR buf = (PUCHAR)pBuf->pData;
	ULONG num = pBuf->size / HalfFifoSize;
	ULONG tail_bytes = pBuf->size % HalfFifoSize;

	PADM2IF_STATUS pStatus;
	for(ULONG i = 0; i < num; i++)
	{
		// кручусь пока флаг HalfFull=1, то есть пока пол-FIFO не заполнено
		// когда флаг HalfFull=0, то есть пол-FIFO заполнено, то читаю
		do
		{
			param.reg = ADM2IFnr_STATUS;
			pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
			pStatus = (PADM2IF_STATUS)&param.val;
		} while(pStatus->ByBits.HalfFull);

		param.reg = ADM2IFnr_DATA;
		param.pBuf = buf;
		param.bytes = HalfFifoSize;
		pModule->RegCtrl(DEVScmd_REGREADSDIR, &param);
		buf += HalfFifoSize;
	}
	if(tail_bytes)
	{
		do
		{
			param.reg = ADM2IFnr_STATUS;
			pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
			pStatus = (PADM2IF_STATUS)&param.val;
		} while(pStatus->ByBits.HalfFull);

		param.reg = ADM2IFnr_DATA;
		param.pBuf = buf;
		param.bytes = tail_bytes;
		pModule->RegCtrl(DEVScmd_REGREADSDIR, &param);
	}
		//param.reg = ADM2IFnr_DATA;
		//PBRD_DataBuf pBuf = (PBRD_DataBuf)args;
		//param.pBuf = pBuf->pData;
		//param.bytes = pBuf->size;
		//pModule->RegCtrl(DEVScmd_REGREADSDIR, &param);

	//PULONG buf = (PULONG)pBuf->pData;
	//for(ULONG i = 0; i < pBuf->size / sizeof(ULONG); i++)
	//{
	//	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//	buf[i] = param.val;
	//}
	return BRDerr_OK;
}

//***************************************************************************************
int CSdramSrv::CtrlPutData(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
/*	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.FifoRes = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pMode0->ByBits.FifoRes = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
*/
	param.reg = ADM2IFnr_FSIZE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG HalfFifoSize = param.val << 2;

	PBRD_DataBuf pBuf = (PBRD_DataBuf)args;
	PUCHAR buf = (PUCHAR)pBuf->pData;
	ULONG num = pBuf->size / HalfFifoSize;
	ULONG tail_bytes = pBuf->size % HalfFifoSize;

	PADM2IF_STATUS pStatus;
	for(ULONG i = 0; i < num; i++)
	{
		// кручусь пока флаг HalfFull=0, то есть пока пол-FIFO не освободится
		// когда флаг HalfFull=1, то есть пол-FIFO свободно, то пишу
		do
		{
			param.reg = ADM2IFnr_STATUS;
			pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
			pStatus = (PADM2IF_STATUS)&param.val;
		} while(!pStatus->ByBits.HalfFull);

		param.reg = ADM2IFnr_DATA;
		param.pBuf = buf;
		param.bytes = HalfFifoSize;
		pModule->RegCtrl(DEVScmd_REGWRITESDIR, &param);
		buf += HalfFifoSize;
	}
	if(tail_bytes)
	{
		do
		{
			param.reg = ADM2IFnr_STATUS;
			pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
			pStatus = (PADM2IF_STATUS)&param.val;
		} while(!pStatus->ByBits.HalfFull);

		param.reg = ADM2IFnr_DATA;
		param.pBuf = buf;
		param.bytes = tail_bytes;
		pModule->RegCtrl(DEVScmd_REGWRITESDIR, &param);
	}
		//param.reg = ADM2IFnr_DATA;
		//PBRD_DataBuf pBuf = (PBRD_DataBuf)args;
		//param.pBuf = pBuf->pData;
		//param.bytes = pBuf->size;
		//pModule->RegCtrl(DEVScmd_REGREADSDIR, &param);

	//PULONG buf = (PULONG)pBuf->pData;
	//for(ULONG i = 0; i < pBuf->size / sizeof(ULONG); i++)
	//{
	//	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//	buf[i] = param.val;
	//}
	return BRDerr_OK;
}

// ***************** End of file SdramSrv.cpp ***********************
