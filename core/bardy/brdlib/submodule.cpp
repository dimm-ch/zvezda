/*
 ***************** File submodule.cpp ***********************
 *
 * BRD Driver for ...
 *
 * (C) InSys by Dorokhin A. Mar 2004
 *
 ******************************************************
*/

#include "submodule.h"

#define	CURRFILE _BRDC("SUBMODULE")

//********************** FindDlgDLLName *****************
// PTSTR* pLibName - указатель на название библиотеки (OUT)
// PBRD_InitData pInitData - указатель на массив данных инициализации вида "ключ-значение" (IN)
// long initDataSize - число оставшихся элементов массива pInitData (IN)
//*******************************************************
void FindDlgDLLName(BRDCHAR **pLibName, PBRD_InitData pInitData, long initDataSize)
{
//	pLibName = NULL;
	// Search SubSections
	for(long i = 0; i < initDataSize; i++)
	{
		if(!BRDC_stricmp(pInitData[i].key, _BRDC("dlgtype")))	// _stricmp
		{
			*pLibName = pInitData[i].val;
			break;
		}
	}
}

//*******************************************************
void SetDlgDLLName(BRDCHAR* DlgDllName, PBRD_InitData pInitData, long initDataSize)
{
	BRDCHAR *pDlgLibName = NULL;
	// Search SubSections
	for(long i = 0; i < initDataSize; i++)
	{
		if(!BRDC_stricmp(pInitData[i].key, _BRDC("dlgtype")))	// _stricmp
		{
			pDlgLibName = pInitData[i].val;
			break;
		}
	}
	if(pDlgLibName)
//		_tcscpy_s(*DlgDllName, pDlgLibName);
		BRDC_strcpy(DlgDllName, pDlgLibName);
	else
//		_tcscpy_s(*DlgDllName, "");
		BRDC_strcpy(DlgDllName, _BRDC(""));
}

//******************************************
CSubModule::CSubModule()
{
	m_pSubIniData = NULL;
}

//******************************************
CSubModule::CSubModule(PBRD_InitData pBrdInitData, long sizeInitData)
{
	m_pSubIniData = new SUB_INI;
	long size = sizeInitData;

	PINIT_Data pInitData = (PINIT_Data)pBrdInitData;
		// идентификация
	FindKeyWord(_BRDC("adm"), m_pSubIniData->Type, pInitData, size);
	FindKeyWord(_BRDC("ver"), m_pSubIniData->Version, pInitData, size);
	FindKeyWord(_BRDC("pid"), m_pSubIniData->PID, pInitData, size);

		// конфигурация
	FindKeyWord(_BRDC("sysrefgen"), m_pSubIniData->SysRefGen, pInitData, size);
	FindKeyWord(_BRDC("refgen1"), m_pSubIniData->BaseRefGen[0], pInitData, size);
	FindKeyWord(_BRDC("refgen2"), m_pSubIniData->BaseRefGen[1], pInitData, size);
	FindKeyWord(_BRDC("extclk"), m_pSubIniData->BaseExtClk, pInitData, size);
	FindKeyWord(_BRDC("refpvs"), m_pSubIniData->RefPVS, pInitData, size);
	
}

//******************************************
CSubModule::~CSubModule()
{
	if(m_pSubIniData)
		delete m_pSubIniData;
}

//***************************************************************************************
S32 CSubModule::Init(PUCHAR pSubmodEEPROM, BRDCHAR *pIniString)
{
	S32		errorCode = BRDerr_OK;

	ICR_IdAdm	idAdm;
	ICR_CfgAdm2If AdmIf;
	memset(&idAdm, 0xff, sizeof(ICR_IdAdm));
	memset(&AdmIf, 0xff, sizeof(ICR_CfgAdm2If));

	m_SysGen = 125000000;

	if(pSubmodEEPROM)
	{
		ULONG AdmCfgSize = *((PUSHORT)pSubmodEEPROM + 2);
		GetICR(pSubmodEEPROM, AdmCfgSize, &idAdm, &AdmIf);
	}
	// Get Type, Version, PID 
	m_Type = idAdm.wType; // From ICR or 0xff
	m_Version = idAdm.bVersion; // From ICR or 0xff
	m_PID = idAdm.dSerialNum; // From ICR or 0xff

	if(m_pSubIniData && m_Type != (U32)-1 && m_Version != (U32)-1 && m_PID != (U32)-1)
	{
		if(m_pSubIniData->PID == (U32)-1)
		{
			//if(m_pSubIniData->Type == -1)
			//{
			//	if(m_pSubIniData->Version != -1 && m_pSubIniData->Version != m_Version)
			//		return BRDerr_BAD_DEVICE_VITAL_DATA;
			//}
			//else
			//	if(m_pSubIniData->Type != m_Type)
			//		return BRDerr_BAD_DEVICE_VITAL_DATA;
		}
		else
		{	// если PID не нулевой, то он должен быть равен считанному из ППЗУ
			// (конечно, если в ППЗУ он был записан)
			if(m_pSubIniData->PID && m_pSubIniData->PID != m_PID)
				return BRDerr_BAD_DEVICE_VITAL_DATA;
		}
	}
	
	BRDC_sprintf(m_name, _BRDC("CADM_%X"), m_PID);

	if(pIniString)
	{
		BRDC_sprintf(pIniString, _BRDC("type=0x%X\nversion=0x%X\npid=0x%X\n"), m_Type, m_Version, m_PID);
	}

	errorCode = SetAdmIfConfig(&AdmIf);
	
	return errorCode;
}
/*
// ***************************************************************************************
void CSubModule::HardwareInit()
{
//	m_pMainRegs->BrdModeStatus = 0xf0f0;
//	m_pMainRegs->AdcMode = 0x0505;
}
*/
//***************************************************************************************
void CSubModule::GetPuList(BRD_PuList* list) const
{
}

//***************************************************************************************
ULONG CSubModule::PuFileLoad (
						ULONG id,
						const BRDCHAR *fileName,
						PUINT pState
						)
{
	if(BRDC_strlen(fileName) == 0)
	{
//		return Dev_ErrorPrintf( SUBERR(BRDerr_BAD_FILE), 
//								pDev, CURRFILE, "<CMD_puLoad> Bad File Name = %s", fileName);
		return SUBERR(BRDerr_BAD_FILE);
	}
	return SUBERR(BRDerr_OK);
}

//***************************************************************************************
ULONG CSubModule::GetPuState(
					   ULONG id,
					   PUINT pState
					   )
{
	return SUBERR(BRDerr_OK);
}

//***************************************************************************************
void CSubModule::GetICR(PVOID pCfgMem, ULONG RealCfgSize, PICR_IdAdm pIdAdm, PICR_CfgAdm2If pAdmIf)
{
//	PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealAdmCfgSize);
	PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealCfgSize);
	int end_flag = 0;
	do
	{
		USHORT sign = *((USHORT*)pCfgMem);
		USHORT size = 0;
		switch(sign)
		{
		case END_TAG:
		case ALT_END_TAG:
				size = 2;
				end_flag = 1;
				break;
		case ADM2IF_CFG_TAG:
			{
				PICR_CfgAdm2If pAdmIfCfg = (PICR_CfgAdm2If)pCfgMem;
				memcpy(pAdmIf, pAdmIfCfg, sizeof(ICR_CfgAdm2If));
				size = sizeof(ICR_CfgAdm2If);
				break;
			}
		case AMB3UV_CFG_TAG:
			{
				PICR_CfgAmb3uv pBambpCfg = (PICR_CfgAmb3uv)pCfgMem;
				m_SysGen = pBambpCfg->dSysGen;
				size += sizeof(ICR_CfgAmb3uv);
				break;
			}
		case AMBPCX_CFG_TAG:
		case AMBPCD_CFG_TAG:
		case AMBPEX2_CFG_TAG:
			{
				PICR_CfgAmbp pBambpCfg = (PICR_CfgAmbp)pCfgMem;
				m_SysGen = pBambpCfg->dSysGen;
				size += sizeof(ICR_CfgAmbp);
				break;
			}
		case AMBPEX1_CFG_TAG:
		case AMBPEX8_CFG_TAG:
			{
				PICR_CfgAmbpex pBambpexCfg = (PICR_CfgAmbpex)pCfgMem;
				m_SysGen = pBambpexCfg->dSysGen;
				size += sizeof(ICR_CfgAmbpex);
				break;
			}
		//case FMC105P_CFG_TAG:
		//case FMC106P_CFG_TAG:
  //      case FMC113E_CFG_TAG:
		//	{
		//		PICR_CfgFmc105p pBasefCfg = (PICR_CfgFmc105p)pCfgMem;
		//		m_SysGen = pBasefCfg->dSysGen;
		//		size += sizeof(ICR_CfgFmc105p);
		//		break;
		//	}
		default:
				size = *((USHORT*)pCfgMem + 1);
				size += 4;
				break;
		}
		pCfgMem = (PUCHAR)pCfgMem + size;
	} while(!end_flag && pCfgMem < pEndCfgMem);
	if(pCfgMem < pEndCfgMem)
	{
		end_flag = 0;
		do
		{
			USHORT sign = *((USHORT*)pCfgMem);
			USHORT size = 0;
			switch(sign)
			{
			case END_TAG:
			case ALT_END_TAG:
				{
					size = 2;
					end_flag = 1;
					break;
				}
			case ADM_ID_TAG:
				{
					PICR_IdAdm pAdmId = (PICR_IdAdm)pCfgMem;
					memcpy(pIdAdm, pAdmId, sizeof(ICR_IdAdm));
					RealCfgSize = pIdAdm->wSizeAll;
					//PUCHAR pSubCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdAdm);
					size = sizeof(ICR_IdAdm);
	//				PICR_Cfg0083 pAdmCfg = (PICR_Cfg0083)pSubCfg;
	//				memcpy(&(pIcr->cfgAdm), pAdmCfg, sizeof(ICR_Cfg0083));
	//				size += sizeof(ICR_Cfg0083);
					break;
				}
			default:
					size = *((USHORT*)pCfgMem + 1);
					size += 4;
					break;
			}
			pCfgMem = (PUCHAR)pCfgMem + size;
		} while(!end_flag && pCfgMem < pEndCfgMem);
	}
}

//***************************************************************************************
ULONG CSubModule::SetAdmIfConfig(PICR_CfgAdm2If pAdmIf)
{
		// устанавливаем конфигурацию по умолчанию
	memcpy(&m_AdmIfCfg, &AdmIfCfg_dflt, sizeof(ADMIF_CFG));

	if(pAdmIf->wTag == ADM2IF_CFG_TAG)
	{	// получаем конфигурацию субмодуля из ППЗУ
		m_AdmIfCfg.RefGen[0] = pAdmIf->dRefGen[0];
		m_AdmIfCfg.RefGen[1] = pAdmIf->dRefGen[1];
		m_AdmIfCfg.RefPVS = pAdmIf->wRefPVS;
		if(!pAdmIf->bAdcFifoCnt)
			m_AdmIfCfg.AdcFifoSize[0] = 0;
//		m_SubCfg.BusClk = pAdmIf->dBusClk;
//		m_SubCfg.ExtClk = pAdmIf->dExtClk;
		m_AdmIfCfg.SysRefGen = m_SysGen;
	}
	if(m_pSubIniData)
	{
		// уточняем параметры идентификации из источника инициализации
		if(m_pSubIniData->Type != 0xffffffff)
			m_Type = m_pSubIniData->Type;
		if(m_pSubIniData->Version != 0xffffffff)
			m_Version = m_pSubIniData->Version;
		//FindKeyWord("pid", m_pSubIniData->PID, pInitData, size);
		// уточняем конфигурацию из источника инициализации
		if(m_pSubIniData->BaseRefGen[0] != 0xffffffff)
			m_AdmIfCfg.RefGen[0] = m_pSubIniData->BaseRefGen[0];
		if(m_pSubIniData->BaseRefGen[1] != 0xffffffff)
			m_AdmIfCfg.RefGen[1] = m_pSubIniData->BaseRefGen[1];
		if(m_pSubIniData->SysRefGen != 0xffffffff)
			m_AdmIfCfg.SysRefGen = m_pSubIniData->SysRefGen;
		if(m_pSubIniData->BaseExtClk != 0xffffffff)
			m_AdmIfCfg.ExtClk = m_pSubIniData->BaseExtClk;
		if(m_pSubIniData->RefPVS != 0xffffffff)
			m_AdmIfCfg.RefPVS = m_pSubIniData->RefPVS;
	}

	return BRDerr_OK;
}

//***************************************************************************************
void CSubModule::BaseInfoInit(PVOID pDev, DEVS_API_entry* pEntry, BRDCHAR* pName)
{
	if(m_PID == (U32)-1)
	{
		CModule* pMod = (CModule*)pDev;
		m_PID = pMod->GetPID();
	}
	m_baseUnit.pDev = pDev;
	m_baseUnit.pEntry = pEntry;
	BRDC_strcpy(m_baseUnit.boardName, pName);
	BRDC_strcpy(m_name, pName);
}

//***************************************************************************************
//void CSubModule::GetServ(PSIDE_SRV_ListItem srv_list)
void CSubModule::GetServ(PSERV_ListItem srv_list)
{
	int srv_num = (int)m_SrvList.size();
	for(int i = 0; i < srv_num; i++)
	{
		BRDC_strcpy(srv_list[i].name, m_SrvList[i]->GetName());
		srv_list[i].attr = m_SrvList[i]->GetAttribute();
		srv_list[i].idxMain = m_SrvList[i]->GetIndex();
//		srv_list[i].srv = m_SrvList[i];
	}
}

//***************************************************************************************
LONG CSubModule::RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam)
{
//	S32  status = DEVS_entry(m_baseUnit.pDev, 0, cmd, pRegParam);
	S32  status = (m_baseUnit.pEntry)(m_baseUnit.pDev, NULL, cmd, pRegParam);
	return status;
}

//***************************************************************************************
ULONG CSubModule::SrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, PVOID pContext)
{
	ULONG status = CModule::SrvCtrl(handle, cmd, arg, pContext);
	return status;
}

//***************** End of file submodule.cpp *****************
