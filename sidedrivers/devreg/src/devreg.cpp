/*
 ***************** File devreg.cpp ***********************
 *
 * BRD Driver for ADM interface on Base Module Boards
 *
 * (C) InSys by Dorokhin A. Nov 2007
 *
 ******************************************************
*/

#define	SIDE_API_EXPORTS
#include "devreg.h"

#define	CURRFILE _BRDC("DEVREG")

//******************************************
CDevReg::CDevReg()
{
	m_pRegSrv = NULL;
}

//******************************************
CDevReg::CDevReg(PBRD_InitData pBrdInitData, long sizeInitData)
{
	m_pRegSrv = NULL;

	PINIT_Data pInitData = (PINIT_Data)pBrdInitData;

		// конфигурация
	FindKeyWord( _BRDC("devbaseadr"), m_DevBaseAdr, pInitData, sizeInitData);
	if(m_DevBaseAdr == 0xffffffff)
		m_DevBaseAdr = 0x203;
}

//******************************************
CDevReg::~CDevReg()
{
	DeleteServices();
}

//***************************************************************************************
S32 CDevReg::Init(PUCHAR pCfgEEPROM, BRDCHAR *pIniString)
{
	S32		errorCode = SUBERR(BRDerr_OK);

	// Get Type, Version, PID
	m_Type = 0;
	m_Version = 0x10;
	m_errcode = 0;
	m_errcnt = 0;
	m_puCnt = 0;

	errorCode = SetServices();

	return SUBERR(errorCode);
}
/*
#define REG_SIZE	1024			// register size
#define TETRAD_SIZE	4096			// tetrad size
#define TRDADR_CMD_ADR  2
#define TRDADR_CMD_DATA 3

// ***************************************************************************************
ULONG readRegdata(ULONG	*pBaseAddr, ULONG TetrNumber, ULONG RegNumber, ULONG& Value)
{
	//ULONG status = 0;

	ULONG Address = TetrNumber * TETRAD_SIZE;
	ULONG CmdAddress = Address + TRDADR_CMD_ADR * REG_SIZE;
	ULONG DataAddress = Address + TRDADR_CMD_DATA * REG_SIZE;
	ULONG StatusAddress = Address;

	pBaseAddr[CmdAddress] = RegNumber;

	ULONG cmd_rdy;
	for(int i = 0; i < 5; i++)
	{
		IPC_delay(2);
		cmd_rdy = pBaseAddr[StatusAddress];
		if(cmd_rdy & 1)
			break;
	}
	if(!(cmd_rdy & 1))
		return 1;

	Value = pBaseAddr[DataAddress] & 0xFFFF;
	return 0;
}
*/
//***************************************************************************************
void CDevReg::BaseInfoInit(PVOID pDev, DEVS_API_entry* pEntry, BRDCHAR* pName)
{
	CModule* pMod = (CModule*)pDev;
	m_PID = pMod->GetPID();
	BRDC_sprintf(m_name, _BRDC("CDEVREG_%X"), m_PID);
	m_baseUnit.pDev = pDev;
	m_baseUnit.pEntry = pEntry;
	BRDC_strcpy(m_baseUnit.boardName, pName);

/*	m_BaseAdrOper = (PULONG)param.idxMain;
	m_BaseAdrTetr = (PULONG)param.reg;

	ULONG baseTag = 0;
	ULONG status = 0;
	status = readRegdata(m_BaseAdrTetr, 0, 0x108, baseTag);
	if(baseTag == 0x4953)
	{
		ULONG baseid = 0, tmask = 0;
		status = readRegdata(m_BaseAdrTetr, 0, 0x110, baseid);
		//printf("Base Module ID = 0x%X\n", baseid);
		status = readRegdata(m_BaseAdrTetr, 0, 0x10C, tmask);
		// получить список тетрад
		ULONG idTetr = 0, modTetr = 0, verTetr = 0;
		for(ULONG iTetr = 0; iTetr < 16; iTetr++)
		{
			if((tmask >> iTetr) & 1)
			{
				readRegdata(m_BaseAdrTetr, iTetr, 0x100, idTetr);
				readRegdata(m_BaseAdrTetr, iTetr, 0x101, modTetr);
				readRegdata(m_BaseAdrTetr, iTetr, 0x102, verTetr);
				//printf("Tetrad[%d] ID = 0x%04X, ID_MOD = 0x%04X, VER = 0x%04X\n", iTetr, idTetr, modTetr, verTetr);
			}
		}
	}*/

}


//***************************************************************************************
void CDevReg::GetDeviceInfo(BRD_Info* pInfo)
{
//	pInfo->type	= m_Type;
//	pInfo->>version		= m_Version;
	pInfo->boardType	= (m_Type << 16) | m_Version;
	pInfo->boardType	= m_Type;
//	pInfo->pid			= m_PID;
	lstrcpy(pInfo->name, _BRDC("REG-interface"));
	pInfo->verMajor		= VER_MAJOR;
	pInfo->verMinor		= VER_MINOR;
}

//***************************************************************************************
void CDevReg::HardwareInit()
{
}

//***************************************************************************************
void CDevReg::GetPuList(BRD_PuList* list) const
{
}

//***************************************************************************************
ULONG CDevReg::PuFileLoad (
						ULONG id,
						const BRDCHAR * fileName,
						PUINT pState
						)
{
	if(BRDC_strlen(fileName) == 0)
	{
		return SUBERR(BRDerr_BAD_FILE);
	}
	return SUBERR(BRDerr_OK);
}

//***************************************************************************************
ULONG CDevReg::GetPuState(
					   ULONG id,
					   PUINT pState
					   )
{
	return SUBERR(BRDerr_OK);
}

//******************************************
void CDevReg::DeleteServices()
{
	if(m_pRegSrv)
	{
		delete m_pRegSrv;
		m_pRegSrv = NULL;
	}
}

//***************************************************************************************
ULONG CDevReg::SetServices()
{
	DeleteServices();

	m_pRegSrv= new CRegSrv(-1, -1, this, NULL);
	m_SrvList.push_back(m_pRegSrv);

	return BRDerr_OK;
}

//***************************************************************************************
void CDevReg::GetServ(PSERV_ListItem srv_list)
{
	int srv_num = (int)m_SrvList.size();
	for(int i = 0; i < srv_num; i++)
	{
		BRDC_strcpy(srv_list[i].name, m_SrvList[i]->GetName());
		srv_list[i].attr = m_SrvList[i]->GetAttribute();
		srv_list[i].idxMain = m_SrvList[i]->GetIndex();
	}
}

//***************************************************************************************
void CDevReg::SetSrvList(PSERV_ListItem srv_list)
{
	int srv_num = (int)m_SrvList.size();
	for(int i = 0; i < srv_num; i++)
	{
		m_SrvList[i]->SetName(srv_list[i].name);
		m_SrvList[i]->SetIndex(srv_list[i].idxMain);

		if(BRDC_strstr(srv_list[i].name, _BRDC("REG")))
		{
			REGSRV_CFG reg_srv_cfg;
			//lstrcpy(reg_srv_cfg.DlgDllName, _BRDC("RegSrvDlg.dll"));
			reg_srv_cfg.isAlreadyInit = 0;
			reg_srv_cfg.baseAdr = m_DevBaseAdr;
			m_SrvList[i]->SetCfgMem(&reg_srv_cfg, sizeof(REGSRV_CFG));
			m_SrvList[i]->SetPropDlg((void*)SIDE_propDlg);
		}
	}
}

//***************************************************************************************
LONG CDevReg::RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam)
{
	S32  status = (m_baseUnit.pEntry)(m_baseUnit.pDev, NULL, cmd, pRegParam);
	return status;
}

//***************************************************************************************
ULONG CDevReg::SrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, PVOID pContext)
{
	ULONG status = CModule::SrvCtrl(handle, cmd, arg, pContext);
	return status;
}

//***************** End of file devreg.cpp *****************
