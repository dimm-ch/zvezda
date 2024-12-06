/*
 ***************** File sysmonsrv.cpp ***********************
 *
 * CTRL-command for BRD Driver for SYSMON service
 *
 * (C) InSys by Dorokhin A. Mar 2012
 *
 ******************************************************
*/

#include "module.h"
#include "sysmonsrv.h"

#define	CURRFILE _BRDC("SYSMONSRV")

static CMD_Info GETTEMP_CMD			= { BRDctrl_SYSMON_GETTEMP,			1, 0, 0, NULL};
static CMD_Info GETVCCINT_CMD		= { BRDctrl_SYSMON_GETVCCINT,		1, 0, 0, NULL};
static CMD_Info GETVCCAUX_CMD		= { BRDctrl_SYSMON_GETVCCAUX,		1, 0, 0, NULL};
static CMD_Info GETVREFP_CMD		= { BRDctrl_SYSMON_GETVREFP,		1, 0, 0, NULL};
static CMD_Info GETVREFN_CMD		= { BRDctrl_SYSMON_GETVREFN,		1, 0, 0, NULL};
static CMD_Info GETSTATUS_CMD		= { BRDctrl_SYSMON_GETSTATUS,		1, 0, 0, NULL};
static CMD_Info GETVNOMINALS_CMD	= { BRDctrl_SYSMON_GETVNOMINALS,	1, 0, 0, NULL};
static CMD_Info GETVN7S_CMD			= { BRDctrl_SYSMON_GETVN7S,			1, 0, 0, NULL};
static CMD_Info GETVCCBRAM_CMD		= { BRDctrl_SYSMON_GETVCCBRAM,		1, 0, 0, NULL};

//***************************************************************************************
CSysMonSrv::CSysMonSrv(int idx, int srv_num, CModule* pModule, PSYSMONSRV_CFG pCfg) :
	CService(idx, _BRDC("SYSMON"), srv_num, pModule, pCfg, sizeof(SYSMONSRV_CFG))
{
	m_attribute = 
			BRDserv_ATTR_DIRECTION_IN |
			BRDserv_ATTR_STREAMABLE_IN |
			BRDserv_ATTR_DIRECTION_OUT |
			BRDserv_ATTR_STREAMABLE_OUT;// |
			//BRDserv_ATTR_EXCLUSIVE_ONLY;

	Init(&GETTEMP_CMD, (CmdEntry)&CSysMonSrv::CtrlTemp);
	Init(&GETVCCINT_CMD, (CmdEntry)&CSysMonSrv::CtrlVccint);
	Init(&GETVCCAUX_CMD, (CmdEntry)&CSysMonSrv::CtrlVccaux);
	Init(&GETVREFP_CMD, (CmdEntry)&CSysMonSrv::CtrlVrefp);
	Init(&GETVREFN_CMD, (CmdEntry)&CSysMonSrv::CtrlVrefn);
	Init(&GETSTATUS_CMD, (CmdEntry)&CSysMonSrv::CtrlStatus);
	Init(&GETVNOMINALS_CMD, (CmdEntry)&CSysMonSrv::CtrlVoltNominal);
	Init(&GETVN7S_CMD, (CmdEntry)&CSysMonSrv::CtrlVoltNominal7s);
	Init(&GETVCCBRAM_CMD, (CmdEntry)&CSysMonSrv::CtrlVccbram);
}

//***************************************************************************************
int CSysMonSrv::CtrlIsAvailable(
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
	if(m_MainTetrNum != -1)
	{
		DEVS_CMD_Reg param;
		param.idxMain = m_index;
		param.tetr = m_MainTetrNum;
		PMAIN_MRES pMres;
		param.reg = MAINnr_MRES;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pMres = (PMAIN_MRES)&param.val;
		if(pMres->ByBits.SysMonEn)
		{
			m_isAvailable = 1;
			PSYSMONSRV_CFG pSrvCfg = (PSYSMONSRV_CFG)m_pConfig;
			if(!pSrvCfg->isAlreadyInit)
				pSrvCfg->isAlreadyInit = 1;
		}
	}
	else
		m_isAvailable = 0;

	pServAvailable->isAvailable = m_isAvailable;
	return BRDerr_OK;
}

//***************************************************************************************
U32 CSysMonSrv::readData(CModule* pModule, int reg)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MainTetrNum;
	param.reg = MAIN17nr_SMONSTAT; // 0x212
	do {
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	} while(!(param.val&1));
	param.reg = MAIN17nr_SMONADDR; // 0x210
	param.val = reg;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = MAIN17nr_SMONDATA; // 0x211;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//	if(g_pldType >= 15 && g_pldType <= 22) // заготовка на будущее (vertex7)
		param.val >>= 6; // vertex5 и vertex6
//	else
//		param.val >>= 4; // vertex7
	return param.val;
}

//***************************************************************************************
// функция не доделана
U32 CSysMonSrv::writeData(CModule* pModule, int reg, U32 val)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MainTetrNum;
	param.reg = MAIN17nr_SMONSTAT; // 0x212
	do {
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	} while (!(param.val & 1));
	param.reg = MAIN17nr_SMONADDR; // 0x210
	param.val = reg;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = MAIN17nr_SMONDATA; // 0x211;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//	if(g_pldType >= 15 && g_pldType <= 22) // заготовка на будущее (vertex7)
	param.val >>= 6; // vertex5 и vertex6
					 //	else
					 //		param.val >>= 4; // vertex7
	return param.val;
}

//***************************************************************************************
int CSysMonSrv::CtrlTemp(
	void		*pDev,		// InfoSM or InfoBM
	void		*pServData,	// Specific Service Data
	ULONG		cmd,		// Command Code (from BRD_ctrl())
	void		*args 		// Command Arguments (from BRD_ctrl())
)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_SysMonVal pMonVal = (PBRD_SysMonVal)args;
	PSYSMONSRV_CFG pSrvCfg = (PSYSMONSRV_CFG)m_pConfig;

	U32 cur_val = readData(pModule, 0);
	U32 max_val = readData(pModule, 0x20);
	U32 min_val = readData(pModule, 0x24);

	if (pSrvCfg->DeviceID == 0x5522 ||			// FMC126P
		pSrvCfg->DeviceID == 0x5523)			// FMC132P
	{ // UltraScale, UltraScale+
		if (pSrvCfg->PldType == 26 ||
			pSrvCfg->PldType == 28)
		{ // UltraScale+
			pMonVal->curv = ((cur_val * 509.3140064) / 1024 - 280.2308787);
			pMonVal->maxv = ((max_val * 509.3140064) / 1024 - 280.2308787);
			pMonVal->minv = ((min_val * 509.3140064) / 1024 - 280.2308787);
		}
		else
		{ // UltraScale
			pMonVal->curv = ((cur_val * 501.3743) / 1024 - 273.6777);
			pMonVal->maxv = ((max_val * 501.3743) / 1024 - 273.6777);
			pMonVal->minv = ((min_val * 501.3743) / 1024 - 273.6777);
		}
	}
	else
	{
		pMonVal->curv = ((cur_val * 503.975) / 1024 - 273.15);
		pMonVal->maxv = ((max_val * 503.975) / 1024 - 273.15);
		pMonVal->minv = ((min_val * 503.975) / 1024 - 273.15);
	}

	return BRDerr_OK;
}

//***************************************************************************************
int CSysMonSrv::CtrlVccint(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_SysMonVal pMonVal = (PBRD_SysMonVal)args;
	U32 val = readData(pModule, 1);
	pMonVal->curv = ((val * 3.) / 1024);
	val = readData(pModule, 0x21);
	pMonVal->maxv = ((val * 3.) / 1024);
	val = readData(pModule, 0x25);
	pMonVal->minv = ((val * 3.) / 1024);

	return BRDerr_OK;
}

//***************************************************************************************
int CSysMonSrv::CtrlVccaux(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_SysMonVal pMonVal = (PBRD_SysMonVal)args;
	U32 val = readData(pModule, 2);
	pMonVal->curv = ((val * 3.) / 1024);
	val = readData(pModule, 0x22);
	pMonVal->maxv = ((val * 3.) / 1024);
	val = readData(pModule, 0x26);
	pMonVal->minv = ((val * 3.) / 1024);

	return BRDerr_OK;
}

//***************************************************************************************
int CSysMonSrv::CtrlVrefp(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	REAL64* pVal = (REAL64*)args;
	U32 val = readData(pModule, 4);
	*pVal = ((val * 3.) / 1024);

	return BRDerr_OK;
}

//***************************************************************************************
int CSysMonSrv::CtrlVrefn(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	REAL64* pVal = (REAL64*)args;
	U32 val = readData(pModule, 5);
	if(val == 0x3FF)
		val = 0;
	*pVal = ((val * 3.) / 1024);

	return BRDerr_OK;
}

//***************************************************************************************
int CSysMonSrv::CtrlStatus(
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
	param.reg = MAIN17nr_SMONSTAT; // 0x212
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	U32* pVal = (U32*)args;
	*pVal = param.val;
	return BRDerr_OK;
}

//***************************************************************************************
int CSysMonSrv::CtrlVoltNominal(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
    //CModule* pModule = (CModule*)pDev;

	PBRD_VoltNominals pNominals = (PBRD_VoltNominals)args;
	PSYSMONSRV_CFG pSrvCfg = (PSYSMONSRV_CFG)m_pConfig;
	if(pSrvCfg->DeviceID == 0x5509 ||		// FMC105
		pSrvCfg->DeviceID == 0x5507)		// AMBPEX5
	{ // Virtex5
		pNominals->vccint = 1.;
		pNominals->vccaux = 2.5;
		pNominals->vrefp = 2.5;
		pNominals->vrefn = 0.0;
	}
	if(pSrvCfg->DeviceID == 0x550A || 		// FMC106
		pSrvCfg->DeviceID == 0x550E ||		// FMC113
		pSrvCfg->DeviceID == 0x550F ||		// FMC108
		pSrvCfg->DeviceID == 0x53B1 ||		// FMC115
		pSrvCfg->DeviceID == 0x550C ||		// FMC114
		pSrvCfg->DeviceID == 0x550D ||		// FMC110
		pSrvCfg->DeviceID == 0x53B2)		// FMC112
	{ // Virtex6
		pNominals->vccint = 1.;
		pNominals->vccaux = 2.5;
		pNominals->vrefp = 1.25;
		pNominals->vrefn = 0.0;
	}
	if(pSrvCfg->DeviceID == 0x5511 ||			// FMC107P
		pSrvCfg->DeviceID == 0x551E ||			// FMC124P
		pSrvCfg->DeviceID == 0x551F ||			// FMC127P
		pSrvCfg->DeviceID == 0x53B3 ||			// FMC117cP
		pSrvCfg->DeviceID == 0x53B4	||			// RFDR4
		pSrvCfg->DeviceID == 0x3018 ||			// FMC118E
		pSrvCfg->DeviceID == 0x3019)			// FMC119E
	{ // Kintex7, Virtex7, Artix7
		pNominals->vccint = 1.;
		pNominals->vccaux = 1.8;
		pNominals->vrefp = 1.25;
		pNominals->vrefn = 0.0;
	}

	if (pSrvCfg->DeviceID == 0x5522 ||			// FMC126P
		pSrvCfg->DeviceID == 0x5523)			// FMC132P
	{ // UltraScale, UltraScale+
		pNominals->vccaux = 1.8;
		pNominals->vrefp = 1.25;
		pNominals->vrefn = 0.0;
		if (pSrvCfg->PldType == 26 ||
			pSrvCfg->PldType == 28)
		{ // UltraScale+
			if (pSrvCfg->PldSpeedGrade == 3)
				pNominals->vccint = 0.9;
			else
				pNominals->vccint = 0.85;
		}
		else
		{ // UltraScale
			if (pSrvCfg->PldSpeedGrade == 3)
				pNominals->vccint = 1.0;
			else
				pNominals->vccint = 0.95;
		}
	}

	return BRDerr_OK;
}

//***************************************************************************************
int CSysMonSrv::CtrlVoltNominal7s(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
    //CModule* pModule = (CModule*)pDev;

	PBRD_VoltNominals7s pNominals = (PBRD_VoltNominals7s)args;
	PSYSMONSRV_CFG pSrvCfg = (PSYSMONSRV_CFG)m_pConfig;
	if (pSrvCfg->DeviceID == 0x5511 ||			// FMC107P
		pSrvCfg->DeviceID == 0x551E ||			// FMC124P
		pSrvCfg->DeviceID == 0x551F ||			// FMC127P
		pSrvCfg->DeviceID == 0x53B4 ||			// RFDR4
		pSrvCfg->DeviceID == 0x3018 ||			// FMC118E
		pSrvCfg->DeviceID == 0x3019)			// FMC119E
	{ // Kintex7, Virtex7, Artix7
		pNominals->vccint = 1.0;
		pNominals->vccaux = 1.8;
		pNominals->vrefp = 1.25;
		pNominals->vrefn = 0.0;
		pNominals->vccbram = 1.0;
	}
	else
		if (pSrvCfg->DeviceID == 0x5522 ||			// FMC126P
			pSrvCfg->DeviceID == 0x5523)			// FMC132P
		{ // UltraScale, UltraScale+
			pNominals->vccaux = 1.8;
			pNominals->vrefp = 1.25;
			pNominals->vrefn = 0.0;
			if (pSrvCfg->PldType == 26 ||
				pSrvCfg->PldType == 28)
			{ // UltraScale+
				if (pSrvCfg->PldSpeedGrade == 3)
				{
					pNominals->vccint = 0.9;
					pNominals->vccbram = 0.9;
				}
				else
				{
					pNominals->vccint = 0.85;
					pNominals->vccbram = 0.85;
				}
			}
			else
			{ // UltraScale
				if (pSrvCfg->PldSpeedGrade == 3)
				{
					pNominals->vccint = 1.0;
					pNominals->vccbram = 1.0;
				}
				else
				{
					pNominals->vccint = 0.95;
					pNominals->vccbram = 0.95;
				}
			}
		}
		else
			return BRDerr_CMD_UNSUPPORTED;
	return BRDerr_OK;
}

//***************************************************************************************
int CSysMonSrv::CtrlVccbram(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_SysMonVal pMonVal = (PBRD_SysMonVal)args;
	U32 val = readData(pModule, 6);
	pMonVal->curv = ((val * 3.) / 1024);
	val = readData(pModule, 0x23);
	pMonVal->maxv = ((val * 3.) / 1024);
	val = readData(pModule, 0x27);
	pMonVal->minv = ((val * 3.) / 1024);

	return BRDerr_OK;
}

// ***************** End of file sysmonsrv.cpp ***********************
