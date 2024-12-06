#define SIDE_API_EXPORTS
#include "mfm415s.h"

#define CURRFILE "MFM415S"

void Cmfm415s::GetICR(PVOID pCfgMem, ULONG RealCfgSize, PICR_CfgAdm pCfgAdm)
{
	PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealCfgSize);
	int end_flag = 0;
	do
	{
		USHORT sign = *((USHORT*)pCfgMem);
		USHORT size = 0;
		switch (sign)
		{
		case END_TAG:
		case ALT_END_TAG:
			size = 2;
			end_flag = 1;
			break;
		case ADM_ID_TAG:
		{
			PICR_IdAdm pAdmId = (PICR_IdAdm)pCfgMem;
			RealCfgSize = pAdmId->wSizeAll;
			PUCHAR pSubCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdAdm);
			size = sizeof(ICR_IdAdm);
			PICR_Cfg00A5 pAdmCfg = (PICR_Cfg00A5)pSubCfg;
			memcpy(pCfgAdm, pAdmCfg, sizeof(ICR_Cfg00A5));
			size += sizeof(ICR_Cfg00A5);
			break;
		}
		default:
			size = *((USHORT*)pCfgMem + 1);
			size += 4;
			break;
		}
		pCfgMem = (PUCHAR)pCfgMem + size;
	} while (!end_flag && pCfgMem < pEndCfgMem);
	if (pCfgMem < pEndCfgMem)
	{
		end_flag = 0;
		do
		{
			USHORT sign = *((USHORT*)pCfgMem);
			USHORT size = 0;
			switch (sign)
			{
			case END_TAG:
			case ALT_END_TAG:
				size = 2;
				end_flag = 1;
				break;
			case ADM_ID_TAG:
			{
				PICR_IdAdm pAdmId = (PICR_IdAdm)pCfgMem;
				RealCfgSize = pAdmId->wSizeAll;
				PUCHAR pSubCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdAdm);
				size = sizeof(ICR_IdAdm);
				PICR_Cfg00A5 pAdmCfg = (PICR_Cfg00A5)pSubCfg;
				memcpy(pCfgAdm, pAdmCfg, sizeof(ICR_Cfg00A5));
				size += sizeof(ICR_Cfg00A5);
				break;
			}
			default:
				size = *((USHORT*)pCfgMem + 1);
				size += 4;
				break;
			}
			pCfgMem = (PUCHAR)pCfgMem + size;
		} while (!end_flag && pCfgMem < pEndCfgMem);
	}
}

ULONG Cmfm415s::SetConfig(PICR_CfgAdm pCfgAdm)
{
	memcpy(&m_SubCfg, &rFM415SCfg_dflt, sizeof(FM415S_CFG));
	if (pCfgAdm->wTag == ADM_CFG_TAG)
	{
		m_SubCfg.bGen1Addr = pCfgAdm->bGen1Adr;
		m_SubCfg.bGen1Type = pCfgAdm->bGen1Type;
		m_SubCfg.bSfpCnt = pCfgAdm->bSfpCnt;
		m_SubCfg.nGen1Ref = pCfgAdm->nGen1Ref;
		m_SubCfg.nGen1RefMax = pCfgAdm->nGen1RefMax;
	}
	if (m_pIniData)
	{
		if (m_pIniData->nGenRef != 0xFFFFFFFF)
			m_SubCfg.nGen1Ref = m_pIniData->nGenRef;
		if (m_pIniData->nGenAdr != 0xFFFFFFFF)
			m_SubCfg.bGen1Addr = m_pIniData->nGenAdr;
		if (m_pIniData->nGenMax != 0xFFFFFFFF)
			m_SubCfg.nGen1RefMax = m_pIniData->nGenMax;
		if (m_pIniData->nGenType != 0xFFFFFFFF)
			m_SubCfg.bGen1Type = m_pIniData->nGenType;
		if (m_pIniData->nDecim)
			m_SubCfg.nDecim = m_pIniData->nDecim;
		if ((m_pIniData->isPrintf != 0xFFFFFFFF) && (m_pIniData->isPrintf != 0))
			m_SubCfg.isPrintf = true;
	}
	return BRDerr_OK;
}

Cmfm415s::Cmfm415s() :
	CSubModule()
{
	for (int i = 0; i < MAX_SERVICES; i++)
		m_pFm415sSrv[i] = NULL;
	m_pIniData = NULL;
}

Cmfm415s::Cmfm415s(PBRD_InitData pInitData, long sizeInitData) :
	CSubModule(pInitData, sizeInitData)
{
	for (int i = 0; i < MAX_SERVICES; i++)
		m_pFm415sSrv[i] = NULL;
	m_pIniData = new FM415S_INI;
	long nSize = sizeInitData;
	PINIT_Data pData = (PINIT_Data)pInitData;
	FindKeyWord(_BRDC("genadr"), m_pIniData->nGenAdr, pData, nSize);
	FindKeyWord(_BRDC("genref"), m_pIniData->nGenRef, pData, nSize);
	FindKeyWord(_BRDC("genmax"), m_pIniData->nGenMax, pData, nSize);
	FindKeyWord(_BRDC("gentype"), m_pIniData->nGenType, pData, nSize);
	FindKeyWord(_BRDC("isprintf"), m_pIniData->isPrintf, pData, nSize);
}

Cmfm415s::~Cmfm415s()
{
	for (int i = 0; i < MAX_SERVICES; i++)	
		if (m_pFm415sSrv[i])
			delete m_pFm415sSrv[i];
	if (m_pIniData)
		delete m_pIniData;
}

S32 Cmfm415s::Init(PUCHAR pSubmodEEPROM, BRDCHAR* pIniString)
{
	S32 nStatus = 0;
	nStatus = CSubModule::Init(pSubmodEEPROM, pIniString);

	ICR_CfgAdm cfgAdm;
	memset(&cfgAdm, 0xff, sizeof(ICR_CfgAdm));
	if (pSubmodEEPROM)
	{
		ULONG AdmCfgSize = *((PUSHORT)pSubmodEEPROM + 2);
		GetICR(pSubmodEEPROM, AdmCfgSize, &cfgAdm);
	}
	SetConfig(&cfgAdm);
	SetServices();
	return SUBERR(nStatus);
}

void Cmfm415s::GetDeviceInfo(BRD_Info* pInfo)
{
	pInfo->boardType = (m_Type << 16) | m_Version;
	pInfo->boardType = m_Type;
	pInfo->pid = m_PID;
	BRDC_strcpy(pInfo->name, _BRDC("FM401S"));
	pInfo->verMajor = VER_MAJOR;
	pInfo->verMinor = VER_MINOR;
}

void Cmfm415s::GetServ(PSERV_ListItem srv_list)
{
	//return;
	int srv_num = (int)m_SrvList.size();
	for (int i = 0; i < srv_num; i++)
	{
		BRDC_strcpy(srv_list[i].name, m_SrvList[i]->GetName());
		srv_list[i].attr = m_SrvList[i]->GetAttribute();
		srv_list[i].idxMain = m_SrvList[i]->GetIndex();
	}
}

void Cmfm415s::DeleteServices()
{
	//return;
	U32 nSrvIndex = 0;
	for (int i = 0; i < MAX_SERVICES; i++)
	{
		delete m_pFm415sSrv[nSrvIndex];
		m_pFm415sSrv[nSrvIndex] = NULL;
		nSrvIndex++;
	}
}

ULONG Cmfm415s::SetServices()
{
	FM415SSRV_CFG rCfg = { 0 };
	U32 nSrvIndex = 0;
	for(int i = 0; i < MAX_SERVICES; i++)
	{
		m_pFm415sSrv[nSrvIndex] = new CFm415sSrv(-1, i, this, &rCfg);
		m_SrvList.push_back(m_pFm415sSrv[nSrvIndex]);
		nSrvIndex++;
	}
	return BRDerr_OK;
}

void Cmfm415s::SetSrvList(PSERV_ListItem srv_list)
{
	int srv_num = (int)m_SrvList.size();
	for (int i = 0; i < srv_num; i++)
	{
		m_SrvList[i]->SetName(srv_list[i].name);
		m_SrvList[i]->SetIndex(srv_list[i].idxMain);
		FM415SSRV_CFG rFm415sSrvCfg = { 0 };
		rFm415sSrvCfg.bGen1Addr = m_SubCfg.bGen1Addr;
		rFm415sSrvCfg.bGen1Type = m_SubCfg.bGen1Type;
		rFm415sSrvCfg.bSfpCnt = m_SubCfg.bSfpCnt;
		rFm415sSrvCfg.nGen1Rate = m_SubCfg.nGen1Rate;
		rFm415sSrvCfg.nGen1Ref = m_SubCfg.nGen1Ref;
		rFm415sSrvCfg.nGen1RefMax = m_SubCfg.nGen1RefMax;
		rFm415sSrvCfg.nDecim = m_SubCfg.nDecim;
		rFm415sSrvCfg.isPrintf = m_SubCfg.isPrintf;
		m_SrvList[i]->SetCfgMem(&rFm415sSrvCfg, sizeof(FM415SSRV_CFG));
	}
}