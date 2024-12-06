/*
 ***************** File RegSrv.cpp ***********************
 *
 * CTRL-command for BRD Driver for REG service
 *
 * (C) InSys by Dorokhin A. Nov 2007
 *
 ******************************************************
*/

#include "module.h"
#include "regsrv.h"

#define	CURRFILE _BRDC("REGSRV")

static CMD_Info READDIR_CMD			= { BRDctrl_REG_READDIR,		1, 0, 0, NULL};
static CMD_Info READSDIR_CMD		= { BRDctrl_REG_READSDIR,		1, 0, 0, NULL};
static CMD_Info READIND_CMD			= { BRDctrl_REG_READIND,		1, 0, 0, NULL};
static CMD_Info READSIND_CMD		= { BRDctrl_REG_READSIND,		1, 0, 0, NULL};
static CMD_Info WRITEDIR_CMD		= { BRDctrl_REG_WRITEDIR,		0, 0, 0, NULL};
static CMD_Info WRITESDIR_CMD		= { BRDctrl_REG_WRITESDIR,		0, 0, 0, NULL};
static CMD_Info WRITEIND_CMD		= { BRDctrl_REG_WRITEIND,		0, 0, 0, NULL};
static CMD_Info WRITESIND_CMD		= { BRDctrl_REG_WRITESIND,		0, 0, 0, NULL};
static CMD_Info SETSTATIRQ_CMD		= { BRDctrl_REG_SETSTATIRQ,		0, 0, 0, NULL};
static CMD_Info CLEARSTATIRQ_CMD	= { BRDctrl_REG_CLEARSTATIRQ,	0, 0, 0, NULL};

static CMD_Info WAITSTATIRQ_CMD = { BRDctrl_REG_WAITSTATIRQ,	1, 0, 0, NULL };

static CMD_Info READSPD_CMD	= { BRDctrl_REG_READSPD,	0, 0, 0, NULL};
static CMD_Info WRITESPD_CMD	= { BRDctrl_REG_WRITESPD,	0, 0, 0, NULL};

static CMD_Info PACKEXECUTE_CMD	= { BRDctrl_REG_PACKEXECUTE,	0, 0, 0, NULL};

static CMD_Info READHOST_CMD = { BRDctrl_REG_READHOST,	1, 0, 0, NULL };
static CMD_Info WRITEHOST_CMD = { BRDctrl_REG_WRITEHOST,	0, 0, 0, NULL };

//***************************************************************************************
CRegSrv::CRegSrv(int idx, int srv_num, CModule* pModule, PREGSRV_CFG pCfg) :
	CService(idx, _BRDC("REG"), srv_num, pModule, pCfg, sizeof(REGSRV_CFG))
{
	m_attribute = 
			BRDserv_ATTR_DIRECTION_IN |
			BRDserv_ATTR_STREAMABLE_IN |
			BRDserv_ATTR_DIRECTION_OUT |
			BRDserv_ATTR_STREAMABLE_OUT;// |
			//BRDserv_ATTR_EXCLUSIVE_ONLY;

	Init(&READDIR_CMD, (CmdEntry)&CRegSrv::CtrlReadDir);
	Init(&READSDIR_CMD, (CmdEntry)&CRegSrv::CtrlReadsDir);
	Init(&READIND_CMD, (CmdEntry)&CRegSrv::CtrlReadInd);
	Init(&READSIND_CMD, (CmdEntry)&CRegSrv::CtrlReadsInd);
	Init(&WRITEDIR_CMD, (CmdEntry)&CRegSrv::CtrlWriteDir);
	Init(&WRITESDIR_CMD, (CmdEntry)&CRegSrv::CtrlWritesDir);
	Init(&WRITEIND_CMD, (CmdEntry)&CRegSrv::CtrlWriteInd);
	Init(&WRITESIND_CMD, (CmdEntry)&CRegSrv::CtrlWritesInd);
	Init(&SETSTATIRQ_CMD, (CmdEntry)&CRegSrv::CtrlSetStatIrq);
	Init(&CLEARSTATIRQ_CMD, (CmdEntry)&CRegSrv::CtrlClearStatIrq);
	Init(&WAITSTATIRQ_CMD, (CmdEntry)&CRegSrv::CtrlWaitStatIrq);

	Init(&PACKEXECUTE_CMD, (CmdEntry)&CRegSrv::CtrlPackExec);

	Init(&READSPD_CMD, (CmdEntry)&CRegSrv::CtrlSpd);
	Init(&WRITESPD_CMD, (CmdEntry)&CRegSrv::CtrlSpd);

	Init(&READHOST_CMD, (CmdEntry)&CRegSrv::CtrlReadHost);
	Init(&WRITEHOST_CMD, (CmdEntry)&CRegSrv::CtrlWriteHost);
}

//***************************************************************************************
int CRegSrv::CtrlIsAvailable(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								)
{
	CModule* pModule = (CModule*)pDev;
	PSERV_CMD_IsAvailable pServAvailable = (PSERV_CMD_IsAvailable)args;
	pServAvailable->attr = m_attribute;
	m_MainTetrNum = 1;//GetTetrNum(pModule, m_index, MAIN_TETR_ID);
	if(m_MainTetrNum != -1)
	{
		m_isAvailable = 1;
		//PREGSRV_CFG pSrvCfg = (PREGSRV_CFG)m_pConfig;
		//if(!pSrvCfg->isAlreadyInit)
		//	pSrvCfg->isAlreadyInit = 1;
	}
	else
		m_isAvailable = 0;

	pServAvailable->isAvailable = m_isAvailable;
	return BRDerr_OK;
}

//***************************************************************************************
int CRegSrv::CtrlCapture(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	if(m_MainTetrNum != -1)
	{
		PREGSRV_CFG pSrvCfg = (PREGSRV_CFG)m_pConfig;
		if (!pSrvCfg->isAlreadyInit)
			pSrvCfg->isAlreadyInit = 1;

		// DEVScmd_GETBASEADR - есть только в bambpex
		DEVS_CMD_Reg param;
		S32 ret = pModule->RegCtrl(DEVScmd_GETBASEADR, &param);
		PULONG* pParam = (PULONG*)&param;
		m_BaseAdrOper = pParam[0];
		m_BaseAdrTetr = pParam[1];
		//printf("CDevReg:: BAR0 = %08p, BAR1 = %08p\n", m_BaseAdrOper, m_BaseAdrTetr);
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CRegSrv::CtrlReadDir(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Reg pReg = (PBRD_Reg)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = pReg->tetr;
	param.reg =	pReg->reg;
	LONG ret = pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	pReg->val = param.val;
	return ret;
}

//***************************************************************************************
int CRegSrv::CtrlReadsDir(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Reg pReg = (PBRD_Reg)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = pReg->tetr;
	param.reg =	pReg->reg;
	param.pBuf = pReg->pBuf;
	param.bytes = pReg->bytes;
	LONG ret = pModule->RegCtrl(DEVScmd_REGREADSDIR, &param);
	return ret;
}

//***************************************************************************************
int CRegSrv::CtrlReadInd(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Reg pReg = (PBRD_Reg)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = pReg->tetr;
	param.reg =	pReg->reg;
	LONG ret = pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	pReg->val = param.val;
	return ret;
}

//***************************************************************************************
int CRegSrv::CtrlReadsInd(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Reg pReg = (PBRD_Reg)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = pReg->tetr;
	param.reg =	pReg->reg;
	param.pBuf = pReg->pBuf;
	param.bytes = pReg->bytes;
	LONG ret = pModule->RegCtrl(DEVScmd_REGREADSIND, &param);
	return ret;
}

//***************************************************************************************
int CRegSrv::CtrlWriteDir(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Reg pReg = (PBRD_Reg)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = pReg->tetr;
	param.reg =	pReg->reg;
	param.val = pReg->val;
	LONG ret = pModule->RegCtrl(DEVScmd_REGWRITEDIR, &param);
	return ret;
}

//***************************************************************************************
int CRegSrv::CtrlWritesDir(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Reg pReg = (PBRD_Reg)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = pReg->tetr;
	param.reg =	pReg->reg;
	param.pBuf = pReg->pBuf;
	param.bytes = pReg->bytes;
	LONG ret = pModule->RegCtrl(DEVScmd_REGWRITESDIR, &param);
	return ret;
}

//***************************************************************************************
int CRegSrv::CtrlWriteInd(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Reg pReg = (PBRD_Reg)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = pReg->tetr;
	param.reg =	pReg->reg;
	param.val = pReg->val;
	LONG ret = pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return ret;
}

//***************************************************************************************
int CRegSrv::CtrlWritesInd(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Reg pReg = (PBRD_Reg)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = pReg->tetr;
	param.reg =	pReg->reg;
	param.pBuf = pReg->pBuf;
	param.bytes = pReg->bytes;
	LONG ret = pModule->RegCtrl(DEVScmd_REGWRITESIND, &param);
	return ret;
}

//***************************************************************************************
int CRegSrv::CtrlSetStatIrq(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Irq pIrq = (PBRD_Irq)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;

	param.tetr = pIrq->tetr;
	param.reg = 1; // IRQMASK
	param.val = pIrq->irqMask;
	LONG ret = pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = 2; // IRQINV
	param.val = pIrq->irqInv;
	ret = pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg =	pIrq->irqMask;
	param.val =	pIrq->irqInv;
	param.pBuf = (void*)pIrq->hEvent;
	ret = pModule->RegCtrl(DEVScmd_SETSTATIRQ, &param);
	return ret;
}

//***************************************************************************************
int CRegSrv::CtrlClearStatIrq(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	//ULONG pIrq = *(PULONG)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = *(PULONG)args;
	param.reg =	0;
	param.val =	0;
	param.pBuf = NULL;
	LONG ret = pModule->RegCtrl(DEVScmd_CLEARSTATIRQ, &param);
	return ret;
}

//***************************************************************************************
int CRegSrv::CtrlWaitStatIrq(
	void		*pDev,		// InfoSM or InfoBM
	void		*pServData,	// Specific Service Data
	ULONG		cmd,		// Command Code (from BRD_ctrl())
	void		*args 		// Command Arguments (from BRD_ctrl())
)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Irq pIrq = (PBRD_Irq)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = pIrq->tetr;

#ifdef _WIN32
	ULONG msTimeout = pIrq->irqInv;
	DWORD Status = WaitForSingleObject(pIrq->hEvent, msTimeout);
	if (Status == WAIT_FAILED || Status == WAIT_ABANDONED)
		return BRDerr_BAD_PARAMETER;
	if (Status == WAIT_TIMEOUT)
		return BRDerr_WAIT_TIMEOUT;
	else
		ResetEvent(pIrq->hEvent); // сброс в состояние Non-Signaled после завершения сбора
								 //IPC_resetEvent(pIrq->hEvent);
#else
	param.reg = pIrq->irqMask;
	param.val = pIrq->irqInv; //Timeout;
	int Status = pModule->RegCtrl(DEVScmd_WAITSTATIRQ, &param);
	if (Status == ETIMEDOUT)
		return BRDerr_WAIT_TIMEOUT;
#endif
	return BRDerr_OK;
}

static int devscmd_remap[]= {

	DEVScmd_REGREADDIR,
	DEVScmd_REGREADSDIR,
	DEVScmd_REGREADIND,
	DEVScmd_REGREADSIND,

	0,
	0,

	DEVScmd_REGWRITEDIR,
	DEVScmd_REGWRITESDIR,

	DEVScmd_REGWRITEIND,
	DEVScmd_REGWRITESIND,
	DEVScmd_SETSTATIRQ,
	DEVScmd_CLEARSTATIRQ,
	DEVScmd_GETBASEADR,
    DEVScmd_WAITSTATIRQ,
	DEVScmd_PACKEXECUTE

};

int CRegSrv::CtrlPackExec(
			void		*pDev,		// InfoSM or InfoBM
			void		*pServData,	// Specific Service Data
			ULONG		cmd,		// Command Code (from BRD_ctrl())
			void		*args 		// Command Arguments (from BRD_ctrl())
		   )
{
	LONG ret = 0;
	CModule* pModule = (CModule*)pDev;
	PBRD_ItemArray pReg = (PBRD_ItemArray)args;
	DEVS_CMD_Reg param;

	int _iobuf_len = pReg->item;
	BRD_PckReg *_iobuf = (BRD_PckReg *)pReg->pItem;

	while( _iobuf_len > 0 )
	{
		DEVS_CMD_Reg realreg[128];

		int nitem;
		int nitemMax = (_iobuf_len<128)?_iobuf_len:128;

		for( nitem=0; nitem<nitemMax; nitem++ )
		{
			
			realreg[nitem].idxMain = devscmd_remap[_iobuf[nitem].cmd - BRDctrl_CMN];

			realreg[nitem].tetr = _iobuf[nitem].tetr;
			realreg[nitem].reg = _iobuf[nitem].reg;
			realreg[nitem].val = _iobuf[nitem].val;

			_iobuf_len--;
			
		}

		param.pBuf = realreg;
		param.bytes = nitem*sizeof(DEVS_CMD_Reg);
		ret = pModule->RegCtrl(DEVScmd_PACKEXECUTE, &param);

		if( BRD_errcmp( ret, BRDerr_FUNC_UNIMPLEMENTED ) )
		{
		
			for( int i=0; i<nitem; i++ )
			{
				pModule->RegCtrl( realreg[i].idxMain, (PDEVS_CMD_Reg)&realreg[i] );
			}

		}
		else
			for( int i=0; i<nitem; i++ )
			{
				_iobuf[i].val = realreg[i].val;
			}

	}

	return ret;
}

int CRegSrv::CtrlSpd(
			void		*pDev,		// InfoSM or InfoBM
			void		*pServData,	// Specific Service Data
			ULONG		cmd,		// Command Code (from BRD_ctrl())
			void		*args 		// Command Arguments (from BRD_ctrl())
		   )
{
	CModule* pModule = (CModule*)pDev;
	PREGSRV_CFG pSrvCfg = (PREGSRV_CFG)m_pConfig;
	PBRD_Spd pReg = (PBRD_Spd)args;
	
	DEVS_CMD_Reg regdata;

	regdata.idxMain = m_index;
	regdata.tetr = pReg->tetr;

	U32 dev_baseadr = pSrvCfg->baseAdr; //0x203;
	U32 num = pReg->num;
	U32 fl_32bits = pReg->mode;
	U32 synchr = pReg->sync;

	regdata.reg = 0;

	S32 status = 0;

	int cmdrdy;

	do
	{// читаем статус тетрады
		status = pModule->RegCtrl( DEVScmd_REGREADDIR, &regdata);
		cmdrdy = regdata.val & 1;
	}while(!cmdrdy);

	// выбираем устройство
	regdata.reg = dev_baseadr; //0x203;//SPD_DEVICE
	regdata.val = pReg->dev;
	status = pModule->RegCtrl( DEVScmd_REGWRITEIND, &regdata);
	
	// записываем адрес
	regdata.reg = dev_baseadr+2; //0x205;//SPD_ADDR
	regdata.val = pReg->reg;
	status = pModule->RegCtrl( DEVScmd_REGWRITEIND, &regdata);
	
	if( cmd == BRDctrl_REG_WRITESPD )
	{
		// записываем данные
		regdata.reg = dev_baseadr+3; //0x206;//SPD_DATA
		regdata.val = pReg->val & 0xFFFF;
		status = pModule->RegCtrl( DEVScmd_REGWRITEIND, &regdata);
		if(fl_32bits)
		{
			regdata.reg = dev_baseadr+4; //0x207;//SPD_DATAH
			regdata.val = pReg->val >> 16;
			status = pModule->RegCtrl( DEVScmd_REGWRITEIND, &regdata);
		}
		regdata.val = (synchr << 12) | (num << 4) | 0x2; // write
	}
	else
	{
		regdata.val = (synchr << 12) | (num << 4) | 0x1; // read
	}

	//Выбор режима
	regdata.reg = dev_baseadr+1; //0x204;//SPD_CTRL
	status = pModule->RegCtrl( DEVScmd_REGWRITEIND, &regdata);

	// ожидаем готовности тетрады
	regdata.reg = 0;
	do
	{// читаем статус тетрады
		status = pModule->RegCtrl( DEVScmd_REGREADDIR, &regdata);
		cmdrdy = regdata.val & 1;
	}while(!cmdrdy);

	S32 val = 0;

	if( cmd == BRDctrl_REG_READSPD )
	{
		// считываем данные
		regdata.reg = dev_baseadr+3; //0x206;//SPD_DATA
		status = pModule->RegCtrl( DEVScmd_REGREADIND, &regdata);
		
		if(BRD_errcmp(status, BRDerr_OK))
			val = regdata.val;
		
		if(fl_32bits)
		{
				regdata.reg = dev_baseadr+4; //0x207;//SPD_DATAH
				status = pModule->RegCtrl( DEVScmd_REGREADIND, &regdata);
				
				if(BRD_errcmp(status, BRDerr_OK))
					val |= regdata.val << 16;
		}
	}

	pReg->val = val;

	return status;
}

//***************************************************************************************
//int CRegSrv::CtrlGetBlk(
//	void		*pDev,		// InfoSM or InfoBM
//	void		*pServData,	// Specific Service Data
//	ULONG		cmd,		// Command Code (from BRD_ctrl())
//	void		*args 		// Command Arguments (from BRD_ctrl())
//)
//{
//	CModule* pModule = (CModule*)pDev;
//	ULONG* pBar = (ULONG*)args;
//	DEVS_CMD_Reg param;
//	LONG ret = pModule->RegCtrl(DEVScmd_GETBASEADR, &param);
//	ULONG* pParam = (ULONG*)&param;
//	pBar[0] = pParam[0];
//	pBar[1] = pParam[1];
//	return ret;
//}

//***************************************************************************************
int CRegSrv::CtrlReadHost(
	void		*pDev,		// InfoSM or InfoBM
	void		*pServData,	// Specific Service Data
	ULONG		cmd,		// Command Code (from BRD_ctrl())
	void		*args 		// Command Arguments (from BRD_ctrl())
)
{
	CModule* pModule = (CModule*)pDev;
	//PREGSRV_CFG pSrvCfg = (PREGSRV_CFG)m_pConfig;
	//PULONG MainBlock = pSrvCfg->Bar0;
	//printf("CtrlReadHost:: BAR0 = %08p\n", pSrvCfg->Bar0);

	PULONG MainBlock = m_BaseAdrOper;
	//printf("CtrlReadHost:: BAR0 = %08p\n", m_BaseAdrOper);

	BRD_Host* pArg = (BRD_Host*)args;
	ULONG BlockCnt = MainBlock[10];
	if (ULONG(pArg->blk) >= BlockCnt)
		return BRDerr_BAD_PARAMETER;
	PULONG BlockAdr = MainBlock + pArg->blk * 64;
	pArg->val = BlockAdr[pArg->reg * 2];
	return BRDerr_OK;
}

//***************************************************************************************
int CRegSrv::CtrlWriteHost(
	void		*pDev,		// InfoSM or InfoBM
	void		*pServData,	// Specific Service Data
	ULONG		cmd,		// Command Code (from BRD_ctrl())
	void		*args 		// Command Arguments (from BRD_ctrl())
)
{
	CModule* pModule = (CModule*)pDev;
	//PREGSRV_CFG pSrvCfg = (PREGSRV_CFG)m_pConfig;
	//PULONG MainBlock = pSrvCfg->Bar0;
	PULONG MainBlock = m_BaseAdrOper;
	BRD_Host* pArg = (BRD_Host*)args;
	ULONG BlockCnt = MainBlock[10];
	if (ULONG(pArg->blk) >= BlockCnt)
		return BRDerr_BAD_PARAMETER;
	PULONG BlockAdr = MainBlock + pArg->blk * 64;
	BlockAdr[pArg->reg * 2] = pArg->val;
	return BRDerr_OK;
}

// ***************** End of file RegSrv.cpp ***********************
