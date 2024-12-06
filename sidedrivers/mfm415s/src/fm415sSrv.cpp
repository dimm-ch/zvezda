#include "module.h"
#include "fm415sSrv.h"

#define CURRFILE "FM415SSRV"

static CMD_Info SETCHANMASK_CMD = { BRDctrl_FM415S_SETCHANMASK, 0, 0, 0, NULL };
static CMD_Info GETCHANMASK_CMD = { BRDctrl_FM415S_GETCHANMASK, 1, 0, 0, NULL };
static CMD_Info SETCLKMODE_CMD = { BRDctrl_FM415S_SETCLKMODE, 0, 0, 0, NULL };
static CMD_Info GETCLKMODE_CMD = { BRDctrl_FM415S_GETCLKMODE, 1, 0, 0, NULL };
static CMD_Info GETSTATUS_CMD = { BRDctrl_FM415S_GETSTATUS, 1, 0, 0, NULL };
static CMD_Info SETTESTMODE_CMD = { BRDctrl_FM415S_SETTESTMODE, 0, 0, 0, NULL };
static CMD_Info GETTESTMODE_CMD = { BRDctrl_FM415S_GETTESTMODE, 1, 0, 0, NULL };
static CMD_Info START_CMD = { BRDctrl_FM415S_START, 0, 0, 0, NULL };
static CMD_Info STOP_CMD = { BRDctrl_FM415S_STOP, 0, 0, 0, NULL };
static CMD_Info GETTESTRESULT_CMD = { BRDctrl_FM415S_GETTESTRESULT, 1, 0, 0, NULL };
static CMD_Info SETDIR_CMD = { BRDctrl_FM415S_SETDIR, 0, 0, 0, NULL };
static CMD_Info GETDIR_CMD = { BRDctrl_FM415S_GETDIR, 1, 0, 0, NULL };
static CMD_Info GETRXTX_CMD = { BRDctrl_FM415S_GETRXTXSTATUS, 1, 0, 0, NULL };
static CMD_Info SETCLKVAL_CMD = { BRDctrl_FM415S_SETCLKVALUE, 0, 0, 0, NULL };
static CMD_Info GETCLKVAL_CMD = { BRDctrl_FM415S_GETCLKVALUE, 1, 0, 0, NULL };
static CMD_Info PREPARE_CMD = { BRDctrl_FM415S_PREPARE, 0, 0, 0, NULL };
static CMD_Info SETDECIM_CMD = { BRDctrl_FM415S_SETDECIM, 0, 0, 0, NULL };
static CMD_Info GETDECIM_CMD = { BRDctrl_FM415S_GETDECIM, 1, 0, 0, NULL };

int g_isPrintf;

int CFm415sSrv::ReadErrData16(void* pDev, U08 nChan, U32 nReg, U32& nVal)
{
	S32 nStatus = 0;
	CModule* pModule = (CModule*)pDev;
	if (m_AuroraTetrNum[nChan] != 0xFF)
	{
		nStatus = IndRegWrite(pModule, m_AuroraTetrNum[nChan], FM415S_REG_ERR_ADR, nReg);
		nStatus |= IndRegRead(pModule, m_AuroraTetrNum[nChan], FM415S_SPD_REG_ERR_DATA, &nVal);
	}
	else
		return BRDerr_BAD_PARAMETER;
	return nStatus;
}

int CFm415sSrv::ReadErrData32(void* pDev, U08 nChan, U32 nReg, U32& nVal)
{
	U32 tmpVal=0;
	S32 nStatus = 0;
	nStatus = ReadErrData16(pDev, nChan, nReg, tmpVal);
	nVal = tmpVal;
	nStatus |= ReadErrData16(pDev, nChan, nReg+1, tmpVal);
	nVal += (tmpVal << 16);
	return nStatus;
}

int CFm415sSrv::ReadErrData64(void* pDev, U08 nChan, U32 nReg, unsigned long long& nVal)
{
	U32 tmpVal = 0;
	S32 nStatus = 0;
	nStatus = ReadErrData32(pDev, nChan, nReg, tmpVal);
	nVal = tmpVal;
	nStatus |= ReadErrData32(pDev, nChan, nReg + 2, tmpVal);
	nVal += ((unsigned long long)tmpVal << 32);
	return nStatus;
}

CFm415sSrv::CFm415sSrv(int idx, int srv_num, CModule* pModule, PFM415SSRV_CFG pCfg) :
	CService(idx, _BRDC("FM415S"), -1, pModule, pCfg, sizeof(PFM415SSRV_CFG))
{
	m_attribute =
		BRDserv_ATTR_DIRECTION_IN |
		BRDserv_ATTR_DIRECTION_OUT |
		BRDserv_ATTR_STREAMABLE_IN |
		BRDserv_ATTR_STREAMABLE_OUT /* |
		BRDserv_ATTR_SDRAMABLE*/;

	m_SrvNum = srv_num;
	for (int i = 0; i < 4; i++)
		m_AuroraTetrNum[i] = 0xFF;

	Init(&SETCHANMASK_CMD, (CmdEntry)&CFm415sSrv::CtrlChanMask);
	Init(&GETCHANMASK_CMD, (CmdEntry)&CFm415sSrv::CtrlChanMask);
	Init(&SETCLKMODE_CMD, (CmdEntry)&CFm415sSrv::CtrlClkMode);
	Init(&GETCLKMODE_CMD, (CmdEntry)&CFm415sSrv::CtrlClkMode);
	Init(&GETSTATUS_CMD, (CmdEntry)&CFm415sSrv::CtrlStatus);
	Init(&SETTESTMODE_CMD, (CmdEntry)&CFm415sSrv::CtrlTestMode);
	Init(&GETTESTMODE_CMD, (CmdEntry)&CFm415sSrv::CtrlTestMode);
	Init(&START_CMD, (CmdEntry)&CFm415sSrv::CtrlStart);
	Init(&STOP_CMD, (CmdEntry)&CFm415sSrv::CtrlStart);
	Init(&GETTESTRESULT_CMD, (CmdEntry)&CFm415sSrv::CtrlTestResult);
	Init(&SETDIR_CMD, (CmdEntry)&CFm415sSrv::CtrlDir);
	Init(&GETDIR_CMD, (CmdEntry)&CFm415sSrv::CtrlDir);
	Init(&GETRXTX_CMD, (CmdEntry)&CFm415sSrv::CtrlRxTx);
	Init(&SETCLKVAL_CMD, (CmdEntry)&CFm415sSrv::CtrlRate);
	Init(&GETCLKVAL_CMD, (CmdEntry)&CFm415sSrv::CtrlRate);
	Init(&PREPARE_CMD, (CmdEntry)&CFm415sSrv::CtrlPrepare);	
	Init(&SETDECIM_CMD, (CmdEntry)&CFm415sSrv::CtrlDecim);
	Init(&GETDECIM_CMD, (CmdEntry)&CFm415sSrv::CtrlDecim);
}

int CFm415sSrv::CtrlIsAvailable(void* pDev, void* pServData, ULONG cmd, void* args)
{
	CModule* pModule = (CModule*)pDev;
	PSERV_CMD_IsAvailable pServAvailable = (PSERV_CMD_IsAvailable)args;
	pServAvailable->attr = m_attribute;
	m_MainTetrNum = GetTetrNum(pModule, m_index, MAIN_TETR_ID);
	m_Fm415sTetrNum = GetTetrNumEx(pModule, m_index, FM415S_TETR_ID, m_SrvNum+1);
	U32 nVal = 0;
	m_nAuroras = 0;
	for (int i = 0; i < 16; i++)
	{
		IndRegRead(pModule, i, ADM2IFnr_ID, &nVal);
		if (nVal == FM415S_AURORA_TETR_ID)
		{
			if( m_nAuroras < 4 )
			{
				m_AuroraTetrNum[m_nAuroras] = i;
				DBG_printf(stderr, "aurora%d tetr %d\n", m_nAuroras, i);
				m_nAuroras++;
			}
		}
	}
	m_isAvailable = 0;
	if ((m_MainTetrNum != -1) && (m_Fm415sTetrNum != -1))
	{
		m_isAvailable = 1;
		PFM415SSRV_CFG pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
		if (pSrvCfg->isPrintf)
			g_isPrintf = true;
		if (!pSrvCfg->isAlreadyInit)
		{
			pSrvCfg->isAlreadyInit = 1;
			ADM2IF_MODE0 pMode0 = { 0 };
			pMode0.ByBits.Reset = 1;
			pMode0.ByBits.FifoRes = 1;
			IndRegWrite(pModule, m_Fm415sTetrNum, ADM2IFnr_MODE0, pMode0.AsWhole);
			for (int i = 1; i < 32; i++)
				IndRegWrite(pModule, m_Fm415sTetrNum, i, 0);
			pMode0.ByBits.Reset = 0;
			pMode0.ByBits.FifoRes =0;
			IndRegWrite(pModule, m_Fm415sTetrNum, ADM2IFnr_MODE0, pMode0.AsWhole);
			for (int j = 0; j < 4; j++)
			{
				if (m_AuroraTetrNum[j] != 0xFF)
				{
					pMode0.ByBits.Reset = 1;
					pMode0.ByBits.FifoRes = 1;
					IndRegWrite(pModule, m_AuroraTetrNum[j], ADM2IFnr_MODE0, pMode0.AsWhole);
					for (int i = 1; i < 32; i++)
						IndRegWrite(pModule, m_AuroraTetrNum[j], i, 0);
					pMode0.ByBits.Reset = 0;
					pMode0.ByBits.FifoRes = 0;
					IndRegWrite(pModule, m_AuroraTetrNum[j], ADM2IFnr_MODE0, pMode0.AsWhole);
				}
			}
			IndRegWrite(pModule, m_Fm415sTetrNum, FM415S_REG_SLKB_SEL, 1);
			Si571GetFxTal(pModule, &(pSrvCfg->dGenFxtal));
		}
	}
	else
		m_isAvailable = 0;
	pServAvailable->isAvailable = m_isAvailable;
	return 0;
}

int CFm415sSrv::CtrlChanMask(void* pDev, void* pServData, ULONG cmd, void* args)
{
	CModule* pModule = (CModule*)pDev;
	PULONG nChanMask = (PULONG)args;
	PFM415SSRV_CFG pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
	FM415S_SFB_TXDIS rTXDIS = { 0 };
	IndRegRead(pModule, m_Fm415sTetrNum, FM415S_REG_SFB_TXDIS, (U32*)&(rTXDIS.AsWhole));
	if (BRDctrl_FM415S_SETCHANMASK == cmd)
	{
		U16 nMask = 0;
		for (int i = 0; i < pSrvCfg->bSfpCnt; i++)
			nMask |= (1 << i);
		pSrvCfg->nChanMask = *nChanMask & nMask;
		rTXDIS.ByBits.SFB_TXDIS0 = (!(pSrvCfg->nChanMask & 1)) & 1;
		rTXDIS.ByBits.SFB_TXDIS1 = (!((pSrvCfg->nChanMask >> 1) & 1)) & 1;
		rTXDIS.ByBits.SFB_TXDIS2 = (!((pSrvCfg->nChanMask >> 2) & 1)) & 1;
		rTXDIS.ByBits.SFB_TXDIS3 = (!((pSrvCfg->nChanMask >> 3) & 1)) & 1;
		IndRegWrite(pModule, m_Fm415sTetrNum, FM415S_REG_SFB_TXDIS, rTXDIS.AsWhole);
	}
	else
		if (BRDctrl_FM415S_GETCHANMASK == cmd)
		{
			*nChanMask = pSrvCfg->nChanMask;
		}
	return BRDerr_OK;
}

int CFm415sSrv::CtrlClkMode(void* pDev, void* pServData, ULONG cmd, void* args)
{
	CModule* pModule = (CModule*)pDev;
	PULONG nGenType = (PULONG)args;
	PFM415SSRV_CFG pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
	if( cmd == BRDctrl_FM415S_SETCLKMODE )
	{
		*nGenType &= 1;
		IndRegWrite(pModule, m_Fm415sTetrNum, FM415S_REG_SLKB_SEL, *nGenType);
		DBG_printf(stderr, "tetr %d reg 0x%X val=%d\n", m_Fm415sTetrNum, FM415S_REG_SLKB_SEL, *nGenType);
	}
	else
	{
		IndRegRead(pModule, m_Fm415sTetrNum, FM415S_REG_SLKB_SEL, (U32*)nGenType);
	}
	return 0;
}

int CFm415sSrv::CtrlStatus(void* pDev, void* pServData, ULONG cmd, void* args)
{
	IPC_delay(50);
	CModule* pModule = (CModule*)pDev;	
	PFM415S_GETSTATUS rStatus = (PFM415S_GETSTATUS)args;	
	FM415S_AURORA_STATUS pStatusReg = { 0 };
	if (BRDctrl_FM415S_GETSTATUS == cmd)
	{
		for (int i = 0; i < 4; i++)
		{
			if (m_AuroraTetrNum[i] != 0xFF)
			{
				DirRegRead(pModule, m_AuroraTetrNum[i], ADM2IFnr_STATUS, (U32*)&(pStatusReg.AsWhole));
				rStatus->isLaneUp[i] = pStatusReg.ByBits.LaneUp;
				rStatus->isPLL_Lock[i] = pStatusReg.ByBits.GT_PLL_Lock;
			}
		}
	}
	return BRDerr_OK;
}

int CFm415sSrv::CtrlTestMode(void* pDev, void* pServData, ULONG cmd, void* args)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg rParam = {0};
	PFM415S_TESTMODE pTestMode = (PFM415S_TESTMODE)args;	
	FM415S_TEST_SEQUENCE pTestReg = {0};
	if (BRDctrl_FM415S_SETTESTMODE == cmd)
	{
		if (pTestMode->isCntEnable)
			pTestReg.ByBits.Cnt_Enable = 1;
		if (pTestMode->isGenEnable)
			pTestReg.ByBits.Gen_Enable = 1;
		for (int i = 0; i < 4; i++)
			if (m_AuroraTetrNum[i] != 0xFF)
				IndRegWrite(pModule, m_AuroraTetrNum[i], FM415S_REG_TEST_SEQUENCE, pTestReg.AsWhole);
	}
	else
		if (BRDctrl_FM415S_GETTESTMODE == cmd)
		{
			if (m_AuroraTetrNum[0] != 0xFF)
				IndRegRead(pModule, m_AuroraTetrNum[0], FM415S_REG_TEST_SEQUENCE, (U32*)&(pTestReg.AsWhole));
			//*(PULONG)args = rParam.val;
			if (pTestReg.ByBits.Cnt_Enable == 1)
				pTestMode->isCntEnable = 1;
			else
				pTestMode->isCntEnable = 0;
			if (pTestReg.ByBits.Gen_Enable == 1)
				pTestMode->isGenEnable = 1;
			else
				pTestMode->isGenEnable = 0;
		}
	return BRDerr_OK;
}

int CFm415sSrv::CtrlStart(void* pDev, void* pServData, ULONG cmd, void* args)
{
	CModule* pModule = (CModule*)pDev;
	PFM415SSRV_CFG pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
	FM415S_AURORA_MODE1 pMode1 = { 0 };	
	if (BRDctrl_FM415S_START == cmd)
	{		
		for (int i = 0; i < 4; i++)
			if ((pSrvCfg->nChanMask >> i) & 1)
			{
				if (m_AuroraTetrNum[i] != 0xFF)
				{
					IndRegRead(pModule, m_AuroraTetrNum[i], ADM2IFnr_MODE1, (U32*)&(pMode1.AsWhole));
					pMode1.ByBits.StartTx = 1;					
					IndRegWrite(pModule, m_AuroraTetrNum[i], ADM2IFnr_MODE1, pMode1.AsWhole);
				}
			}
	}
	else
	{
		ADM2IF_MODE0 pMode0 = { 0 };
		IndRegRead(pModule, m_Fm415sTetrNum, ADM2IFnr_MODE0, &(pMode0.AsWhole));
		pMode0.ByBits.Start = 0;
		IndRegWrite(pModule, m_Fm415sTetrNum, ADM2IFnr_MODE0, pMode0.AsWhole);
		for (int i = 0; i < 4; i++)
			if ((pSrvCfg->nChanMask >> i) & 1)
			{
				if (m_AuroraTetrNum[i] != 0xFF)
				{
					IndRegRead(pModule, m_AuroraTetrNum[i], ADM2IFnr_MODE0, &(pMode0.AsWhole));
					pMode0.ByBits.Start = 0;
					IndRegWrite(pModule, m_AuroraTetrNum[i], ADM2IFnr_MODE0, pMode0.AsWhole);
					IndRegRead(pModule, m_AuroraTetrNum[i], ADM2IFnr_MODE1, (U32*)&(pMode1.AsWhole));
					pMode1.ByBits.StartTx = 0;
					IndRegWrite(pModule, m_AuroraTetrNum[i], ADM2IFnr_MODE1, pMode1.AsWhole);
				}
			}
	}
	return BRDerr_OK;
}

int CFm415sSrv::CtrlTestResult(void* pDev, void* pServData, ULONG cmd, void* args)
{
	CModule* pModule = (CModule*)pDev;
	PFM415S_GETTESTRESULT pTest = (PFM415S_GETTESTRESULT)args;
	if (m_AuroraTetrNum[pTest->nChan] == 0xFF)
		return BRDerr_BAD_PARAMETER;
	U32 nErrors = 0;
	S32 nStatus = 0;
	nStatus = ReadErrData32(pDev, pTest->nChan, FM415S_ERR_DATA_TOTAL_ERR_L, nErrors);

	if (nErrors > 32)
		nErrors = 32;
	for (U32 i = 0; i < nErrors; i++)
	{
		nStatus = ReadErrData64(pDev, pTest->nChan, FM415S_ERR_DATA_READ_D0 + (i * 0x10), pTest->lReadWord[i]);
		nStatus = ReadErrData64(pDev, pTest->nChan, FM415S_ERR_DATA_EXPECT_D0 + (i * 0x10), pTest->lExpectWord[i]);
		nStatus = ReadErrData16(pDev, pTest->nChan, FM415S_ERR_DATA_INDEX + (i * 0x10), pTest->nIndex[i]);
		nStatus = ReadErrData32(pDev, pTest->nChan, FM415S_ERR_DATA_BLOCK_D0 + (i * 0x10), pTest->nBlock[i]);
	}
	nStatus = ReadErrData32(pDev, pTest->nChan, FM415S_ERR_DATA_TOTAL_ERR_L, pTest->nTotalError);
	
	return BRDerr_OK;
}

int CFm415sSrv::CtrlDir(void* pDev, void* pServData, ULONG cmd, void* args)
{	
	PFM415SSRV_CFG pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
	PFM415S_DIR rParam = (PFM415S_DIR)args;
	if (BRDctrl_FM415S_SETDIR == cmd)
	{
		pSrvCfg->nDirChanMask = rParam->nChan[0] & 1;
		for (int i = 1; i < 4; i++)
			pSrvCfg->nDirChanMask |= ((rParam->nChan[i] & 1) << i);
	}
	return 0;
}

int CFm415sSrv::CtrlRxTx(void* pDev, void* pServData, ULONG cmd, void* args)
{
	CModule* pModule = (CModule*)pDev;
	PFM415S_GETRXTXSTATUS pTest = (PFM415S_GETRXTXSTATUS)args;
	S32 nStatus = 0;
	for (int i = 0; i < 4; i++)
	{
		if (m_AuroraTetrNum[i] != 0xFF)
		{
			U32 nVal = 0;
			//nStatus = ReadErrData32(pDev, i, FM415S_ERR_ADDR_BLOCK_RD_L, pTest->nBlockRead[i]);
			//nStatus |= ReadErrData32(pDev, i, FM415S_ERR_ADDR_BLOCK_WR_L, pTest->nBlockWrite[i]);
			IndRegRead(pDev, m_AuroraTetrNum[i], FM415S_SPD_REG_CNT_TX_L, &nVal);
			pTest->nBlockWrite[i] = nVal;
			IndRegRead(pDev, m_AuroraTetrNum[i], FM415S_SPD_REG_CNT_TX_H, &nVal);
			pTest->nBlockWrite[i] |= (nVal << 16);
			IndRegRead(pDev, m_AuroraTetrNum[i], FM415S_SPD_REG_CNT_RX_L, &nVal);
			pTest->nBlockRead[i] = nVal;
			IndRegRead(pDev, m_AuroraTetrNum[i], FM415S_SPD_REG_CNT_RX_H, &nVal);
			pTest->nBlockRead[i] |= (nVal << 16);
			nStatus = ReadErrData32(pDev, i, FM415S_ERR_ADDR_BLOCK_ERR_L, pTest->nBlockErr[i]);
			nStatus |= ReadErrData32(pDev, i, FM415S_ERR_ADDR_BLOCK_OK_L, pTest->nBlockOk[i]);
		}
	}
	return nStatus;
}

int CFm415sSrv::CtrlRate(void* pDev, void* pServData, ULONG cmd, void* args)
{
	CModule* pModule = (CModule*)pDev;
	PFM415SSRV_CFG pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
	double* dRate = (double*)args;
	if (BRDctrl_FM415S_SETCLKVALUE == cmd)
	{
		switch (pSrvCfg->bGen1Type)
		{
		case 0:
			*dRate = pSrvCfg->nGen1Rate;
			break;
		case 1:
			Si571SetRate(pModule, dRate);
			break;
		case 2:
			Lmk61e2SetRate(pModule, dRate);
			break;
		}
	}
	else
		if (BRDctrl_FM415S_GETCLKVALUE == cmd)
		{
			switch (pSrvCfg->bGen1Type)
			{
			case 0:
				*dRate = pSrvCfg->nGen1Rate;
				break;
			case 1:
				Si571GetRate(pModule, dRate);
				break;
			case 2:
				Lmk61e2GetRate(pModule, dRate);
				break;
			}
		}
	return 0;
}

int CFm415sSrv::CtrlPrepare(void* pDev, void* pServData, ULONG cmd, void* args)
{
	CModule* pModule = (CModule*)pDev;
	S32 nStatus = 0;
	//FM415S_CONTROL rCtrl = { 0 };
	U32 nVal = 0;
	//bool isReady = true, isCont = false;
	PFM415SSRV_CFG pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
	ADM2IF_MODE0 pMode0 = { 0 };
	IndRegRead(pModule, m_Fm415sTetrNum, ADM2IFnr_MODE0, &(pMode0.AsWhole));
	pMode0.ByBits.Start = 1;
	IndRegWrite(pModule, m_Fm415sTetrNum, ADM2IFnr_MODE0, pMode0.AsWhole);
	for (int i = 0; i < 4; i++)
		if ((pSrvCfg->nChanMask >> i) & 1)
		{
			if (m_AuroraTetrNum[i] != 0xFF)
			{
				IndRegRead(pModule, m_AuroraTetrNum[i], ADM2IFnr_MODE0, &(pMode0.AsWhole));
				pMode0.ByBits.Start = 1;
				IndRegWrite(pModule, m_AuroraTetrNum[i], ADM2IFnr_MODE0, pMode0.AsWhole);
			}
		}
	return nStatus;
}

int CFm415sSrv::CtrlDecim(void* pDev, void* pServData, ULONG cmd, void* args)
{
	CModule* pModule = (CModule*)pDev;
	PFM415SSRV_CFG pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
	PU32 nDecim = (PU32)args;
	if (BRDctrl_FM415S_SETDECIM == cmd)
	{		
		for (int i = 0; i < 4; i++)
			if ((pSrvCfg->nChanMask >> i) & 1)
			{
				if (m_AuroraTetrNum[i] != 0xFF)
					IndRegWrite(pModule, m_AuroraTetrNum[i], FM415S_REG_DECIM, *nDecim);
			}
	}
	else
		if (BRDctrl_FM415S_GETDECIM == cmd)
		{
			for (int i = 0; i < 4; i++)
				if ((pSrvCfg->nChanMask >> i) & 1)
				{
					if (m_AuroraTetrNum[i] != 0xFF)
						IndRegRead(pModule, m_AuroraTetrNum[i], FM415S_REG_DECIM, nDecim);
				}
		}
		else
			return BRDerr_ERROR;
	return BRDerr_OK;
}

int CFm415sSrv::CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args)
{
	PFM415SSRV_CFG pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
	pSrvCfg->isAlreadyInit = 0;
	return 0;
}

int CFm415sSrv::SpdRead(CModule* pModule, U32 spdType, U32 regAdr, U32* pRegVal)
{
	FM415SSRV_CFG* pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
	S32 status, spdCtrl = 0x1;
	U32 nVal=0;
	
	for (;;)
	{
		DirRegRead(pModule, m_Fm415sTetrNum, 0, &nVal);
		if (nVal & 0x1)
			break;
	}
	if (spdType == SPDdev_FM415S_GEN) spdCtrl |= (pSrvCfg->bGen1Addr << 4);
	//if (spdType == SPDdev_FM415S_TCA62425) spdCtrl |= 0x220;
	IndRegWrite(pModule, m_Fm415sTetrNum, FM415S_SPD_DEVICE, spdType & 0xFF);
	IndRegWrite(pModule, m_Fm415sTetrNum, FM415S_SPD_ADDR, regAdr);
	IndRegWrite(pModule, m_Fm415sTetrNum, FM415S_SPD_CTRL, spdCtrl);
	for (;;)
	{
		DirRegRead(pModule, m_Fm415sTetrNum, 0, &nVal);
		if (nVal & 0x1)
			break;
	}
	status = IndRegRead(pModule, m_Fm415sTetrNum, FM415S_SPD_DATA, pRegVal);	

	return status;
}

int CFm415sSrv::SpdWrite(CModule* pModule, U32 spdType, U32 regAdr, U32 regVal)
{
	FM415SSRV_CFG* pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
	S32 status, spdCtrl = 0x2;
	U32 nVal = 0;

	for (;;)
	{
		DirRegRead(pModule, m_Fm415sTetrNum, 0, &nVal);
		if (nVal & 0x1)
			break;
	}

	if (spdType == SPDdev_FM415S_GEN) spdCtrl |= (pSrvCfg->bGen1Addr << 4);
	//if (spdType == SPDdev_FM415S_TCA62425) spdCtrl |= 0x220;
	IndRegWrite(pModule, m_Fm415sTetrNum, FM415S_SPD_DEVICE, spdType & 0xFF);
	IndRegWrite(pModule, m_Fm415sTetrNum, FM415S_SPD_ADDR, regAdr);
	IndRegWrite(pModule, m_Fm415sTetrNum, FM415S_SPD_DATA, regVal);
	IndRegWrite(pModule, m_Fm415sTetrNum, FM415S_SPD_CTRL, spdCtrl);


	for (;;)
	{
		status = DirRegRead(pModule, m_Fm415sTetrNum, 0, &nVal);
		if (nVal & 0x1)
			break;
	}


	return status;
}

int CFm415sSrv::Si571SetRate(CModule* pModule, double* pRate)
{
	FM415SSRV_CFG* pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
	U32			regData[20];
	int			regAdr;
	int			status = BRDerr_OK;
	//
	// Скорректировать частоту, если необходимо
	//
	if (*pRate > (double)pSrvCfg->nGen1RefMax)
	{
		*pRate = (double)pSrvCfg->nGen1RefMax;
		status = BRDerr_WARN;
	}

	SI571_SetRate(pRate, pSrvCfg->dGenFxtal, regData);

	//
	// Запрограммировать генератор
	//
	SpdWrite(pModule, SPDdev_FM415S_GEN, 137, 0x10);		// Freeze DCO
	for (regAdr = 7; regAdr <= 18; regAdr++)
		SpdWrite(pModule, SPDdev_FM415S_GEN, regAdr, regData[regAdr]);
	SpdWrite(pModule, SPDdev_FM415S_GEN, 137, 0x0);		// Unfreeze DCO
	SpdWrite(pModule, SPDdev_FM415S_GEN, 135, 0x40);		// Assert the NewFreq bit

	return status;
}

int CFm415sSrv::Si571GetRate(CModule* pModule, double* pRate)
{
	FM415SSRV_CFG* pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
	U32			regData[20];
	//ULONG		clkSrc;
	int			regAdr;

	*pRate = 0.0;

	//
	// Проверить источник частоты
	//
	//GetClkSource( pModule, clkSrc );
	//if( clkSrc != BRDclks_DAC_SUBGEN )
	//	BRDerr_ERROR;
	//
	// Считать регистры Si570/Si571
	//
	for (regAdr = 7; regAdr <= 12; regAdr++)
		SpdRead(pModule, SPDdev_FM415S_GEN, regAdr, &regData[regAdr]);

	SI571_GetRate(pRate, pSrvCfg->dGenFxtal, regData);
	//printf( "Si571GetRate: %x, %x, %x, %x, %x, %x\n", regData[7], regData[8], regData[9], regData[10], regData[11], regData[12] );

	return BRDerr_OK;
}

int CFm415sSrv::Si571GetFxTal(CModule* pModule, double* dGenFxTal)
{
	FM415SSRV_CFG* pSrvCfg = (PFM415SSRV_CFG)m_pConfig;
	U32 regAdr;
	pSrvCfg->dGenFxtal = 0.0;
	if ((pSrvCfg->bGen1Type == 1) && (pSrvCfg->nGen1Ref != 0))
	{
		U32			regData[20];		

		//
		// Подать питание на Si570/Si571
		//		
		SpdWrite(pModule, SPDdev_FM415S_GEN, 135, 0x1);		// Recall
		IPC_delay(10);

		//
		// Считать регистры Si570/Si571
		//
		for (regAdr = 7; regAdr <= 12; regAdr++)
		{
			SpdRead(pModule, SPDdev_FM415S_GEN, regAdr, &regData[regAdr]);
			printf("Si571: reg%02d = 0x%02X\n", regAdr, regData[regAdr]);
		}

		//
		// Рассчитать частоту кварца
		//
		SI571_CalcFxtal(&(pSrvCfg->dGenFxtal), (double)(pSrvCfg->nGen1Ref), regData);
		printf(">> XTAL = %f kHz\n", pSrvCfg->dGenFxtal / 1000.0);
		printf(">> GREF = %f kHz\n", ((double)(pSrvCfg->nGen1Ref)) / 1000.0);
	}
	return 0;
}

int CFm415sSrv::Lmk61e2SetRate(CModule* pModule, double* pRate)
{
	double dDelta = 0.0;
	LMK61E2_REG_DATA rRegs = { 0 };
	LMK61E2_SetRate(pRate, &dDelta, &rRegs);
	BRDC_printf(_BRDC("LMK61e2 set %f Hz (error = %f Hz)\n"), *pRate, dDelta);
	SpdWrite(pModule, SPDdev_FM415S_GEN, OUT_DIV_HIGH, (rRegs.OUT_DIV >> 8) & 0x1);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", OUT_DIV_HIGH, (rRegs.OUT_DIV >> 8) & 0x1);
	SpdWrite(pModule, SPDdev_FM415S_GEN, OUT_DIV_LOW, rRegs.OUT_DIV & 0xFF);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", OUT_DIV_LOW, rRegs.OUT_DIV & 0xFF);
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_NDIV_HIGH, (rRegs.PLL_NDIV >> 8) & 0xF);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_NDIV_HIGH, (rRegs.PLL_NDIV >> 8) & 0xF);
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_NDIV_LOW, rRegs.PLL_NDIV & 0xFF);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_NDIV_LOW, rRegs.PLL_NDIV & 0xFF);
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_NUM_HIGH, (rRegs.PLL_NUM >> 16) & 0x3F);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_NUM_HIGH, (rRegs.PLL_NUM >> 16) & 0x3F);
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_NUM_MED, (rRegs.PLL_NUM >> 8) & 0xFF);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_NUM_MED, (rRegs.PLL_NUM >> 8) & 0xFF);
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_NUM_LOW, rRegs.PLL_NUM & 0xFF);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_NUM_LOW, rRegs.PLL_NUM & 0xFF);
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_DEN_HIGH, (rRegs.PLL_DEN >> 16) & 0x3F);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_DEN_HIGH, (rRegs.PLL_DEN >> 16) & 0x3F);
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_DEN_MED, (rRegs.PLL_DEN >> 8) & 0xFF);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_DEN_MED, (rRegs.PLL_DEN >> 8) & 0xFF);
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_DEN_LOW, rRegs.PLL_DEN & 0xFF);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_DEN_LOW, rRegs.PLL_DEN & 0xFF);
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_DTHRMODE_ORDER, ((rRegs.PLL_DEN & 0x3) << 2) | (rRegs.PLL_ORDER & 3));
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_DTHRMODE_ORDER, ((rRegs.PLL_DEN & 0x3) << 2) | (rRegs.PLL_ORDER & 3));
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_D_CP, (((rRegs.PLL_D - 1) & 0x1) << 5) | (rRegs.PLL_CP & 0xF));
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_D_CP, (((rRegs.PLL_D - 1) & 0x1) << 5) | (rRegs.PLL_CP & 0xF));
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_CP_PHASE_SHIFT_ENABLE_C3, ((rRegs.PLL_CP_PHASE_SHIFT & 0x7) << 4) | ((rRegs.PLL_ENABLE_C3 & 0x1) << 2) | 3);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_CP_PHASE_SHIFT_ENABLE_C3, ((rRegs.PLL_CP_PHASE_SHIFT & 0x7) << 4) | ((rRegs.PLL_ENABLE_C3 & 0x1) << 2) | 3);
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_LF_R2, rRegs.PLL_LF_R2 & 0xFF);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_LF_R2, rRegs.PLL_LF_R2 & 0xFF);
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_LF_C1, rRegs.PLL_LF_C1 & 0x7);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_LF_C1, rRegs.PLL_LF_C1 & 0x7);
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_LF_R3, rRegs.PLL_LF_R3 & 0x7F);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_LF_R3, rRegs.PLL_LF_R3 & 0x7F);
	SpdWrite(pModule, SPDdev_FM415S_GEN, PLL_LF_C3, rRegs.PLL_LF_C3 & 0x7);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", PLL_LF_C3, rRegs.PLL_LF_C3 & 0x7);
	SpdWrite(pModule, SPDdev_FM415S_GEN, 49, 0x10);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", 49, 0x10);
	SpdWrite(pModule, SPDdev_FM415S_GEN, 51, 0);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", 51, 0);
	SpdWrite(pModule, SPDdev_FM415S_GEN, 53, 0);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", 53, 0);
	SpdWrite(pModule, SPDdev_FM415S_GEN, 56, 0);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", 56, 0);
	SpdWrite(pModule, SPDdev_FM415S_GEN, 72, 2);
	DBG_printf(stderr, "LMK61e2 set reg %d = 0x%X\n", 72, 2);
	DBG_printf(stderr, "LMK61e2 PLL_D=%d INT=%d NUM=%d DEN=%d OUT_DIV=%d\n", rRegs.PLL_D, rRegs.PLL_NDIV, rRegs.PLL_NUM, rRegs.PLL_DEN, rRegs.OUT_DIV);
	IPC_delay(100);
	return 0;
}

int CFm415sSrv::Lmk61e2GetRate(CModule* pModule, double* pRate)
{
	double dDelta = 0.0;
	U32 nVal = 0;
	LMK61E2_REG_DATA rRegs = { 0 };
	SpdRead(pModule, SPDdev_FM415S_GEN, OUT_DIV_HIGH, &nVal);
	rRegs.OUT_DIV = (nVal & 1) << 8;
	SpdRead(pModule, SPDdev_FM415S_GEN, OUT_DIV_LOW, &nVal);
	rRegs.OUT_DIV |= (nVal & 0xFF);
	SpdRead(pModule, SPDdev_FM415S_GEN, PLL_NDIV_HIGH, &nVal);
	rRegs.PLL_NDIV = (nVal & 0xF) << 8;
	SpdRead(pModule, SPDdev_FM415S_GEN, PLL_NDIV_LOW, &nVal);
	rRegs.PLL_NDIV |= (nVal & 0xFF);
	SpdRead(pModule, SPDdev_FM415S_GEN, PLL_NUM_HIGH, &nVal);
	rRegs.PLL_NUM = (nVal & 0x3F) << 16;
	SpdRead(pModule, SPDdev_FM415S_GEN, PLL_NUM_MED, &nVal);
	rRegs.PLL_NUM |= (nVal & 0xFF) << 8;
	SpdRead(pModule, SPDdev_FM415S_GEN, PLL_NUM_LOW, &nVal);
	rRegs.PLL_NUM |= (nVal & 0xFF);
	SpdRead(pModule, SPDdev_FM415S_GEN, PLL_DEN_HIGH, &nVal);
	rRegs.PLL_DEN = (nVal & 0x3F) << 16;
	SpdRead(pModule, SPDdev_FM415S_GEN, PLL_DEN_MED, &nVal);
	rRegs.PLL_DEN |= (nVal & 0xFF) << 8;
	SpdRead(pModule, SPDdev_FM415S_GEN, PLL_DEN_LOW, &nVal);
	rRegs.PLL_DEN |= (nVal & 0xFF);
	SpdRead(pModule, SPDdev_FM415S_GEN, PLL_D_CP, &nVal);
	rRegs.PLL_D = 1 + ((nVal >> 5) & 1);
	LMK61E2_GetRate(pRate, &dDelta, &rRegs);
	BRDC_printf(_BRDC("LMK61e2 set %f Hz (error = %f Hz)\n"), *pRate, dDelta);
	return 0;
}