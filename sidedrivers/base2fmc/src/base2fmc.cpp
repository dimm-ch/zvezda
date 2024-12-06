/*
 ***************** File base2fmc.cpp ***********************
 *
 * BRD Driver for ADM interface on Base Module Boards
 *
 * (C) InSys by Dorokhin A. Mar 2012
 *
 ******************************************************
*/

#define	SIDE_API_EXPORTS
#include "base2fmc.h"

#define	CURRFILE _BRDC("BASE2FMC")

//******************************************
CBase2fmc::CBase2fmc()
{
	m_pBasefSrv = NULL;
	m_pPioSrv = NULL;
	m_pDdsSrv = NULL;
	m_pIniData = NULL;
}

//******************************************
CBase2fmc::CBase2fmc(PBRD_InitData pBrdInitData, long sizeInitData)
{
	m_pBasefSrv = NULL;
	m_pPioSrv = NULL;
	m_pDdsSrv = NULL;

	m_pIniData = new BASEF_INI;
	long size = sizeInitData;

	PINIT_Data pInitData = (PINIT_Data)pBrdInitData;

		// конфигурация
	FindKeyWord( _BRDC("switchtype"), m_pIniData->SwitchType, pInitData, size);
	FindKeyWord( _BRDC("adrswitch"), m_pIniData->AdrSwitch, pInitData, size);
	FindKeyWord( _BRDC("gen0type"), m_pIniData->Gen0Type, pInitData, size);
	FindKeyWord( _BRDC("refgen0"), m_pIniData->RefGen0, pInitData, size);
	FindKeyWord( _BRDC("refmaxgen0"), m_pIniData->RefMaxGen0, pInitData, size);
	FindKeyWord( _BRDC("adrgen0"), m_pIniData->AdrGen0, pInitData, size);
	FindKeyWord( _BRDC("sysrefgen"), m_pIniData->SysRefGen, pInitData, size);
	FindKeyWord( _BRDC("extclk"), m_pIniData->ExtClk, pInitData, size);
	FindKeyWord( _BRDC("pldcnt"), m_pIniData->AdmPldCnt, pInitData, size);
	FindKeyWord( _BRDC("piotype"), m_pIniData->PioType, pInitData, size);
	FindKeyWord( _BRDC("ddstype"), m_pIniData->DdsType, pInitData, size);
}

//******************************************
CBase2fmc::~CBase2fmc()
{
	DeleteServices();
	if(m_pIniData)
		delete m_pIniData;
}

//***************************************************************************************
S32 CBase2fmc::Init(PUCHAR pCfgEEPROM, BRDCHAR *pIniString)
{
	S32		errorCode = SUBERR(BRDerr_OK);

	BASE2FMC_ICR baseresICR;
	memset(&baseresICR, 0xff, sizeof(BASE2FMC_ICR));
	if(pCfgEEPROM)
	{
		ULONG CfgSize = *((PUSHORT)pCfgEEPROM + 2);
		GetICR(pCfgEEPROM, CfgSize, &baseresICR);
	}
	// Get Type, Version, PID 
	m_Type = 0;
	m_Version = 0x10;
	m_errcode = 0;
	m_errcnt = 0;
	m_puCnt = 0;

	errorCode = SetConfig(m_pIniData, &baseresICR);
	
	errorCode = SetServices();

	return SUBERR(errorCode);
}

//***************************************************************************************
void CBase2fmc::BaseInfoInit(PVOID pDev, DEVS_API_entry* pEntry, BRDCHAR* pName)
{
	CModule* pMod = (CModule*)pDev;
	m_PID = pMod->GetPID();
	BRDC_sprintf(m_name,  _BRDC("CBASEFRES_%X"), m_PID);
	m_baseUnit.pDev = pDev;
	m_baseUnit.pEntry = pEntry;
	BRDC_strcpy(m_baseUnit.boardName, pName);
}


//***************************************************************************************
void CBase2fmc::GetDeviceInfo(BRD_Info* pInfo)
{
//	pInfo->type	= m_Type;
//	pInfo->>version		= m_Version;
	pInfo->boardType	= (m_Type << 16) | m_Version;	
	pInfo->boardType	= m_Type;
//	pInfo->pid			= m_PID;
	BRDC_strcpy(pInfo->name, _BRDC("FMC-interface"));
	pInfo->verMajor		= VER_MAJOR;	
	pInfo->verMinor		= VER_MINOR;
}

//***************************************************************************************
void CBase2fmc::HardwareInit()
{
//	m_pMainRegs->BrdModeStatus = 0xf0f0;
//	m_pMainRegs->AdcMode = 0x0505;
}

//***************************************************************************************
void CBase2fmc::GetPuList(BRD_PuList* list) const
{
}

//***************************************************************************************
ULONG CBase2fmc::PuFileLoad (
						ULONG id,
						const BRDCHAR *fileName,
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
ULONG CBase2fmc::GetPuState(
					   ULONG id,
					   PUINT pState
					   )
{
	return SUBERR(BRDerr_OK);
}

//***************************************************************************************
void CBase2fmc::GetICR(PVOID pCfgMem, ULONG RealCfgSize, PBASE2FMC_ICR pBaseResIcr)
{
	PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealCfgSize);
	int idxPld = 0;
	int end_flag = 0;
	do
	{
		USHORT sign = *((USHORT*)pCfgMem);
		USHORT size = 0;
		switch(sign)
		{
		case END_TAG:
		case ALT_END_TAG:
			end_flag = 1;
			break;
		case ADM2IF_CFG_TAG:
			{
				PICR_CfgAdm2If pAdmIfCfg = (PICR_CfgAdm2If)pCfgMem;
				memcpy(&(pBaseResIcr->AdmIf), pAdmIfCfg, sizeof(ICR_CfgAdm2If));
				size = sizeof(ICR_CfgAdm2If);
				break;
			}
		//case FMC105P_CFG_TAG:
		//case FMC106P_CFG_TAG:
		//case FMC107P_CFG_TAG:
  //      case FMC113E_CFG_TAG:
		//case FMC110P_CFG_TAG:
  //      case FMC112CP_CFG_TAG:
		//case FMC115CP_CFG_TAG:
		//case FMC117CP_CFG_TAG:
		//case FMC118E_CFG_TAG:
		//case FMC119E_CFG_TAG:
		//case FMC128E_CFG_TAG:
		//case FMC129E_CFG_TAG:
		//case FMC132P_CFG_TAG:
		//{
		//		PICR_CfgFmc105p pBasefCfg = (PICR_CfgFmc105p)pCfgMem;
		//		memcpy(&(pBaseResIcr->Ambfmc), pBasefCfg, sizeof(ICR_CfgFmc105p));
		//		//size += sizeof(ICR_CfgFmc105p);
		//		size += (pBasefCfg->wSize + 4);
		//		break;
		//	}
		case PLD_CFG_TAG:
			{
				PICR_CfgAdmPld pPldCfg = (PICR_CfgAdmPld)pCfgMem;
				memcpy(&(pBaseResIcr->AdmPld[idxPld++]), pPldCfg, sizeof(ICR_CfgAdmPld));
				size = sizeof(ICR_CfgAdmPld);
				break;
			}
		default:
			{
				USHORT btag = sign >> 12;
				if(btag == 5 || btag == 3)
				{
					PICR_CfgFmc105p pBasefCfg = (PICR_CfgFmc105p)pCfgMem;
					memcpy(&(pBaseResIcr->Ambfmc), pBasefCfg, sizeof(ICR_CfgFmc105p));
					size += (pBasefCfg->wSize + 4);
				}
				else
				{
					size = *((USHORT*)pCfgMem + 1);
					size += 4;
				}
				break;
			}
		}
		pCfgMem = (PUCHAR)pCfgMem + size;
	} while(!end_flag && pCfgMem < pEndCfgMem);
}

//***************************************************************************************
ULONG CBase2fmc::SetConfig(PBASEF_INI pIniData, PBASE2FMC_ICR pBaseResICR)
{
		// устанавливаем конфигурацию по умолчанию
	memcpy(&m_FmcIfCfg, &BaseFmcCfg_dflt, sizeof(BASEF_CFG));

	if(pBaseResICR->AdmIf.wTag == ADM2IF_CFG_TAG)
	{	// получаем конфигурацию модуля из ППЗУ
        if(pBaseResICR->Ambfmc.wTag == FMC105P_CFG_TAG || 
			pBaseResICR->Ambfmc.wTag == FMC106P_CFG_TAG ||
			pBaseResICR->Ambfmc.wTag == FMC107P_CFG_TAG ||
			pBaseResICR->Ambfmc.wTag == FMC110P_CFG_TAG ||
			pBaseResICR->Ambfmc.wTag == FMC112CP_CFG_TAG ||
			pBaseResICR->Ambfmc.wTag == FMC115CP_CFG_TAG ||
			pBaseResICR->Ambfmc.wTag == FMC117CP_CFG_TAG ||
			pBaseResICR->Ambfmc.wTag == FMC132P_CFG_TAG ||
			pBaseResICR->Ambfmc.wTag == FMC139P_CFG_TAG ||
			pBaseResICR->Ambfmc.wTag == FMC131VZQ_CFG_TAG ||
			pBaseResICR->Ambfmc.wTag == FMC141VZQ_CFG_TAG ||
			pBaseResICR->Ambfmc.wTag == FMC143VZQ_CFG_TAG ||
			pBaseResICR->Ambfmc.wTag == FMC113E_CFG_TAG ||
            pBaseResICR->Ambfmc.wTag == FMC118E_CFG_TAG ||
            pBaseResICR->Ambfmc.wTag == FMC119E_CFG_TAG ||
            pBaseResICR->Ambfmc.wTag == FMC128E_CFG_TAG ||
            pBaseResICR->Ambfmc.wTag == FMC129E_CFG_TAG ||
            pBaseResICR->Ambfmc.wTag == 0x53B6)
		{
			m_FmcIfCfg.SysRefGen = pBaseResICR->Ambfmc.dSysGen;
			m_FmcIfCfg.AdrSwitch = pBaseResICR->Ambfmc.bAdrSwitch;
			m_FmcIfCfg.DdsType   = pBaseResICR->Ambfmc.bDdsType;
			m_FmcIfCfg.RefGen0	= pBaseResICR->Ambfmc.nRefGen0;
			m_FmcIfCfg.RefMaxGen0	= pBaseResICR->Ambfmc.nRefMaxGen0;
			m_FmcIfCfg.AdrGen0	= pBaseResICR->Ambfmc.bAdrGen0;
			m_FmcIfCfg.Gen0Type = pBaseResICR->Ambfmc.bGen0Type;
			m_FmcIfCfg.SwitchType = pBaseResICR->Ambfmc.bSwitchType;
			m_FmcIfCfg.AdrSwitch = pBaseResICR->Ambfmc.bAdrSwitch;
			m_FmcIfCfg.RefGen5 = pBaseResICR->Ambfmc.dRefGen5;  // clock for DDS (on FMC132P)
		}
		m_FmcIfCfg.AdmPldCnt	= pBaseResICR->AdmIf.bPldCnt;
		m_FmcIfCfg.PioType		= pBaseResICR->AdmIf.bPioType;
		m_Type = pBaseResICR->Ambfmc.wTag;
	}
	if(pIniData)
	{	// уточняем конфигурацию из источника инициализации
		if(pIniData->SwitchType != 0xffffffff)
			m_FmcIfCfg.SwitchType = pIniData->SwitchType;
		if(pIniData->AdrSwitch != 0xffffffff)
			m_FmcIfCfg.AdrSwitch = pIniData->AdrSwitch;
		if(pIniData->Gen0Type != 0xffffffff)
			m_FmcIfCfg.Gen0Type = pIniData->Gen0Type;
		if(pIniData->AdrGen0 != 0xffffffff)
			m_FmcIfCfg.AdrGen0 = pIniData->AdrGen0;
		if(pIniData->RefGen0 != 0xffffffff)
			m_FmcIfCfg.RefGen0 = pIniData->RefGen0;
		if(pIniData->RefMaxGen0 != 0xffffffff)
			m_FmcIfCfg.RefMaxGen0 = pIniData->RefMaxGen0;

		if(pIniData->SysRefGen != 0xffffffff)
			m_FmcIfCfg.SysRefGen = pIniData->SysRefGen;
		if(pIniData->RefGen0 != 0xffffffff)
			m_FmcIfCfg.RefGen0 = pIniData->RefGen0;
		if(pIniData->ExtClk != 0xffffffff)
			m_FmcIfCfg.ExtClk = pIniData->ExtClk;
		if(pIniData->AdmPldCnt != 0xffffffff)
			m_FmcIfCfg.AdmPldCnt = pIniData->AdmPldCnt;
		if(pIniData->PioType != 0xffffffff)
			m_FmcIfCfg.PioType = pIniData->PioType;
		if(pIniData->DdsType != 0xffffffff)
			m_FmcIfCfg.DdsType = pIniData->DdsType;
	}

	return SUBERR(BRDerr_OK);
}

//******************************************
void CBase2fmc::DeleteServices()
{
	if(m_pBasefSrv)
	{
		delete m_pBasefSrv;
		m_pBasefSrv = NULL;
	}
	if(m_pPioSrv) {
		delete m_pPioSrv;
		m_pPioSrv = NULL;
	}
	if(m_pDdsSrv) {
		delete m_pDdsSrv;
		m_pDdsSrv = NULL;
	}
}

//***************************************************************************************
ULONG CBase2fmc::SetServices()
{
	DeleteServices();

	m_pBasefSrv = new CBasefSrv(-1, -1, this, NULL);
	m_SrvList.push_back(m_pBasefSrv);

	if(m_FmcIfCfg.PioType)
	{
		m_pPioSrv = new CPioSrv(-1, -1, this, NULL);
		m_SrvList.push_back(m_pPioSrv);
	}
	if(m_FmcIfCfg.DdsType)
	{
		m_pDdsSrv = new CDdsSrv(-1, -1, this, NULL);
		m_SrvList.push_back(m_pDdsSrv);
	}
	return BRDerr_OK;
}

//***************************************************************************************
void CBase2fmc::GetServ(PSERV_ListItem srv_list)
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
void CBase2fmc::SetSrvList(PSERV_ListItem srv_list)
{
	int srv_num = (int)m_SrvList.size();
	for(int i = 0; i < srv_num; i++)
	{
		m_SrvList[i]->SetName(srv_list[i].name);
		m_SrvList[i]->SetIndex(srv_list[i].idxMain);

		if(BRDC_strstr(srv_list[i].name, _BRDC("BASEFMC")))
		{
			BASEFSRV_CFG basef_srv_cfg;

            basef_srv_cfg.SwitchType = m_FmcIfCfg.SwitchType;
			basef_srv_cfg.AdrSwitch = m_FmcIfCfg.AdrSwitch;
			basef_srv_cfg.Gen0Type = m_FmcIfCfg.Gen0Type;
			basef_srv_cfg.AdrGen0 = m_FmcIfCfg.AdrGen0;
			basef_srv_cfg.RefGen0 = m_FmcIfCfg.RefGen0;
			basef_srv_cfg.RefMaxGen0 = m_FmcIfCfg.RefMaxGen0;
			//lstrcpy(basef_srv_cfg.DlgDllName, _BRDC("BaseFmcSrvDlg.dll"));
			basef_srv_cfg.isAlreadyInit = 0;
			m_SrvList[i]->SetCfgMem(&basef_srv_cfg, sizeof(BASEFSRV_CFG));
			m_SrvList[i]->SetPropDlg((void*)SIDE_propDlg);
		}
		if(BRDC_strstr(srv_list[i].name, _BRDC("PIO")))
		{
			PIOSRV_CFG pio_srv_cfg;
			pio_srv_cfg.isAlreadyInit = 0;
			pio_srv_cfg.PioType = m_FmcIfCfg.PioType;
			m_SrvList[i]->SetCfgMem(&pio_srv_cfg, sizeof(PIOSRV_CFG));
		}
		if(BRDC_strstr(srv_list[i].name, _BRDC("BASEDDS")))
		{
			DDSSRV_CFG dds_srv_cfg;
			lstrcpy(dds_srv_cfg.DlgDllName, _BRDC("BaseDdsSrvDlg.dll"));
			dds_srv_cfg.isAlreadyInit = 0;
			dds_srv_cfg.DdsType = m_FmcIfCfg.DdsType;
			if(m_Type == FMC132P_CFG_TAG)
				dds_srv_cfg.BaseRefClk = m_FmcIfCfg.RefGen5; // clock for DDS
			else
				dds_srv_cfg.BaseRefClk = m_FmcIfCfg.RefGen0; // clock for DDS
			dds_srv_cfg.ExtRefClk = m_FmcIfCfg.ExtClk;
			m_SrvList[i]->SetCfgMem(&dds_srv_cfg, sizeof(DDSSRV_CFG));
			m_SrvList[i]->SetPropDlg((void*)SIDE_propDlg);
		}
	}
}

//***************************************************************************************
LONG CBase2fmc::RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam)
{
	S32  status = (m_baseUnit.pEntry)(m_baseUnit.pDev, NULL, cmd, pRegParam);
	return status;
}

//***************************************************************************************
ULONG CBase2fmc::SrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, PVOID pContext)
{
	ULONG status = CModule::SrvCtrl(handle, cmd, arg, pContext);
	return status;
}

//***************** End of file base2fmc.cpp *****************
