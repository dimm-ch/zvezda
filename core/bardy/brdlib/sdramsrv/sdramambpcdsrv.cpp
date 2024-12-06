/*
 ***************** File SdramAmbpcdSrv.cpp ***********************
 *
 * BRD Driver for ADM-interface SdramAmbpcd service
 *
 * (C) InSys by Dorokhin A. Feb 2005
 *
 ******************************************************
*/

#include "module.h"
#include "sdramambpcdsrv.h"

#define	CURRFILE "SDRAMAMBPCDSRV"

//***************************************************************************************
CSdramAmbpcdSrv::CSdramAmbpcdSrv(int idx, int srv_num, CModule* pModule, PSDRAMAMBPCDSRV_CFG pCfg) :
	CSdramSrv(idx, _BRDC("BASESDRAM"), srv_num, pModule, pCfg, sizeof(SDRAMAMBPCDSRV_CFG))
{
}

//***************************************************************************************
void CSdramAmbpcdSrv::GetSdramTetrNum(CModule* pModule)
{
	PSDRAMAMBPCDSRV_CFG pSrvCfg = (PSDRAMAMBPCDSRV_CFG)m_pConfig;
	pSrvCfg->SdramCfg.Mode = 0;
	m_MemTetrNum = GetTetrNum(pModule, m_index, SDRAMAMBPCD_TETR_ID);
	if(m_MemTetrNum == -1)
	{
//		m_MemTetrNum = GetTetrNum(pModule, m_index, SDRAMFMC106P_TETR_ID);
//		if(m_MemTetrNum == -1)
//		{
		//m_MemTetrNum = GetTetrNum(pModule, m_index, SDRAMDDR3X_TETR_ID);
		//if(m_MemTetrNum == -1)
		//{
			m_MemTetrNum = GetTetrNum(pModule, m_index, SDRAMAMBPEX2_TETR_ID);
			if(m_MemTetrNum == -1)
			{
				m_MemTetrNum = GetTetrNum(pModule, m_index, SDRAMAMBPEX5_TETR_ID);
				if(m_MemTetrNum == -1)
				{
					pSrvCfg->SdramCfg.Mode |= 0x02;
					m_MemTetrNum = GetTetrNum(pModule, m_index, SDRAMAMBPCDRD_TETR_ID);
					if(m_MemTetrNum == -1)
					{
						m_MemTetrNum = GetTetrNum(pModule, m_index, SDRAMVK3RD_TETR_ID);	
						if(m_MemTetrNum == -1)
							m_MemTetrNum = GetTetrNum(pModule, m_index, SDRAMAMBPEX2RD_TETR_ID);	
					}
				}
				else
					pSrvCfg->SdramCfg.Mode |= 0x01;
			}
			else
				pSrvCfg->SdramCfg.Mode |= 0x01;
		//}
		//else
		//	pSrvCfg->SdramCfg.Mode |= 0x01;
	}
	else
		pSrvCfg->SdramCfg.Mode |= 0x01;
}

//***************************************************************************************
void CSdramAmbpcdSrv::FreeInfoForDialog(PVOID pInfo)
{
	PSDRAMAMBPCDSRV_INFO pSrvInfo = (PSDRAMAMBPCDSRV_INFO)pInfo;
	CSdramSrv::FreeInfoForDialog(pSrvInfo->pSdramInfo);
	delete pSrvInfo;
}

//***************************************************************************************
PVOID CSdramAmbpcdSrv::GetInfoForDialog(CModule* pDev)
{
	PSDRAMAMBPCDSRV_CFG pSrvCfg = (PSDRAMAMBPCDSRV_CFG)m_pConfig;
	PSDRAMAMBPCDSRV_INFO pSrvInfo = new SDRAMAMBPCDSRV_INFO;
	pSrvInfo->Size = sizeof(SDRAMSRV_INFO);
	pSrvInfo->pSdramInfo = (PSDRAMSRV_INFO)CSdramSrv::GetInfoForDialog(pDev);
	pSrvInfo->PrimWidth = pSrvCfg->PrimWidth;
	pSrvInfo->CasLatency = pSrvCfg->CasLatency;
	pSrvInfo->Attributes = pSrvCfg->Attributes;
	return pSrvInfo;
}

//***************************************************************************************
int CSdramAmbpcdSrv::SetPropertyFromDialog(void *pDev, void	*args)
{
	CModule* pModule = (CModule*)pDev;
	PSDRAMAMBPCDSRV_INFO pInfo = (PSDRAMAMBPCDSRV_INFO)args;
	CSdramSrv::SetReadMode(pModule, pInfo->pSdramInfo->ReadMode);
	return BRDerr_OK;
}

//***************************************************************************************
UCHAR CSdramAmbpcdSrv::ReadSpdByte(CModule* pModule, ULONG OffsetSPD, ULONG CtrlSPD)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_SPDADDR;
	param.val = OffsetSPD;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = SDRAMnr_SPDCTRL;
	param.val = CtrlSPD;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = SDRAMnr_SPDDATAL;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	return (UCHAR)param.val;
}

//***************************************************************************************
ULONG CSdramAmbpcdSrv::GetCfgFromSpd(CModule* pModule, PSDRAMAMBPCDSRV_CFG pCfgSPD)
{
	SDRAM_SPDCTRL SpdCtrl;
	SpdCtrl.AsWhole = 0;
	SpdCtrl.ByBits.Read = 1;

	int flg_pex5 = GetTetrNum(pModule, m_index, SDRAMAMBPEX5_TETR_ID);

	UCHAR mem_type[SDRAM_MAXSLOTS];
	SpdCtrl.ByBits.Slot = 0;
	mem_type[0] = ReadSpdByte(pModule, SDRAMspd_MEMTYPE, SpdCtrl.AsWhole);
	if(flg_pex5 == -1)
	{
		SpdCtrl.ByBits.Slot = 1;
		mem_type[1] = ReadSpdByte(pModule, SDRAMspd_MEMTYPE, SpdCtrl.AsWhole);
	}
	else
		mem_type[1] = 0;
	for(int i = 0; i < SDRAM_MAXSLOTS; i++)
		if(mem_type[i] == SDRAMmt_DDR || mem_type[i] == SDRAMmt_DDR2 || mem_type[i] == SDRAMmt_DDR3)
			pCfgSPD->SdramCfg.ModuleCnt++;
	if(!pCfgSPD->SdramCfg.ModuleCnt)
	    return BRDerr_SDRAM_NO_MEMORY;

	ULONG Status = BRDerr_OK;
	UCHAR row_addr[SDRAM_MAXSLOTS], col_addr[SDRAM_MAXSLOTS], module_banks[SDRAM_MAXSLOTS],
		  width[SDRAM_MAXSLOTS], chip_banks[SDRAM_MAXSLOTS], cas_lat[SDRAM_MAXSLOTS], attrib[SDRAM_MAXSLOTS];
	for(int i = 0; i < pCfgSPD->SdramCfg.ModuleCnt; i++)
	{
		SpdCtrl.ByBits.Slot = i;
		row_addr[i] = ReadSpdByte(pModule, SDRAMspd_ROWADDR, SpdCtrl.AsWhole);
		col_addr[i] = ReadSpdByte(pModule, SDRAMspd_COLADDR, SpdCtrl.AsWhole);
		module_banks[i] = ReadSpdByte(pModule, SDRAMspd_MODBANKS, SpdCtrl.AsWhole);
		if(mem_type[i] == SDRAMmt_DDR2 || mem_type[i] == SDRAMmt_DDR3)
			module_banks[i] = (module_banks[i] & 0x07) + 1;
		width[i] = ReadSpdByte(pModule, SDRAMspd_WIDTH, SpdCtrl.AsWhole);
		chip_banks[i] = ReadSpdByte(pModule, SDRAMspd_CHIPBANKS, SpdCtrl.AsWhole);
//		if(mem_type[i] == SDRAMmt_DDR2)
//			chip_banks[i] >>= 1; 
		cas_lat[i] = ReadSpdByte(pModule, SDRAMspd_CASLAT, SpdCtrl.AsWhole);
		attrib[i] = ReadSpdByte(pModule, SDRAMspd_ATTRIB, SpdCtrl.AsWhole);

		if(i && ((row_addr[i] != row_addr[i-1]) ||
				 (col_addr[i] != col_addr[i-1]) || 
				 (module_banks[i] != module_banks[i-1]) ||
			     (width[i] != width[i-1]) || 
				 (chip_banks[i] != chip_banks[i-1]) || 
				 (cas_lat[i] != cas_lat[i-1]) || 
				 (attrib[i] != attrib[i-1])
				))
		{
		    Status = BRDerr_SDRAM_NO_EQU_SPD;
		}			
	}
	if(BRDerr_SDRAM_NO_EQU_SPD == Status)
		pCfgSPD->SdramCfg.ModuleCnt--;

	pCfgSPD->SdramCfg.MemType = mem_type[0];
	pCfgSPD->SdramCfg.RowAddrBits = row_addr[0];
	pCfgSPD->SdramCfg.ColAddrBits = col_addr[0];
	pCfgSPD->SdramCfg.ModuleBanks = module_banks[0];
	pCfgSPD->SdramCfg.ChipBanks = chip_banks[0];
	pCfgSPD->PrimWidth = width[0];
	pCfgSPD->CasLatency = cas_lat[0];
	pCfgSPD->Attributes = attrib[0];
/*
	ULONG PhysMemSize =	(1 << row_addr[0]) *
						(1 << col_addr[0]) * 
						module_banks[0] * 
						chip_banks[0] *
						pCfgSPD->SdramCfg.ModuleCnt * 2; // in 32-bit Words
*/
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CSdramAmbpcdSrv::CheckCfg(CModule* pModule)
{
	SDRAMAMBPCDSRV_CFG cfgSPD;
	cfgSPD.SdramCfg.ModuleCnt = 0;

	ULONG Status = GetCfgFromSpd(pModule, &cfgSPD);

	PSDRAMAMBPCDSRV_CFG pMemSrvCfg = (PSDRAMAMBPCDSRV_CFG)m_pConfig;

	pMemSrvCfg->SdramCfg.MemType = cfgSPD.SdramCfg.MemType;

	if(BRDerr_OK != Status)
	{
		pMemSrvCfg->SdramCfg.ModuleCnt &= 0x7F;
		pMemSrvCfg->SdramCfg.RowAddrBits &= 0x7F;
		pMemSrvCfg->SdramCfg.ColAddrBits &= 0x7F;
		pMemSrvCfg->SdramCfg.ModuleBanks &= 0x7F;
		pMemSrvCfg->SdramCfg.ChipBanks &= 0x7F;
		pMemSrvCfg->PrimWidth &= 0x7F; 
		pMemSrvCfg->CasLatency &= 0x7F;
		pMemSrvCfg->Attributes &= 0x7F;
	}
	else
	{
		if(pMemSrvCfg->SdramCfg.ModuleCnt & 0x80)
			pMemSrvCfg->SdramCfg.ModuleCnt = cfgSPD.SdramCfg.ModuleCnt;
		if(pMemSrvCfg->SdramCfg.RowAddrBits & 0x80)
			pMemSrvCfg->SdramCfg.RowAddrBits = cfgSPD.SdramCfg.RowAddrBits;
		if(pMemSrvCfg->SdramCfg.ColAddrBits & 0x80)
			pMemSrvCfg->SdramCfg.ColAddrBits = cfgSPD.SdramCfg.ColAddrBits;
		if(pMemSrvCfg->SdramCfg.ModuleBanks & 0x80)
			pMemSrvCfg->SdramCfg.ModuleBanks = cfgSPD.SdramCfg.ModuleBanks;
		if(pMemSrvCfg->SdramCfg.ChipBanks & 0x80)
			pMemSrvCfg->SdramCfg.ChipBanks = cfgSPD.SdramCfg.ChipBanks;
		if(pMemSrvCfg->PrimWidth & 0x80)
			pMemSrvCfg->PrimWidth = cfgSPD.PrimWidth;
		if(pMemSrvCfg->CasLatency & 0x80)
			pMemSrvCfg->CasLatency = cfgSPD.CasLatency;
		if(pMemSrvCfg->Attributes & 0x80)
			pMemSrvCfg->Attributes = cfgSPD.Attributes;
	}

	if(!pMemSrvCfg->SdramCfg.ModuleCnt)
		return BRDerr_SDRAM_NO_MEMORY;
	if(pMemSrvCfg->SdramCfg.RowAddrBits < 11 || pMemSrvCfg->SdramCfg.RowAddrBits > 16)
	    return BRDerr_SDRAM_BAD_PARAMETER;
	if(pMemSrvCfg->SdramCfg.ColAddrBits < 8 || pMemSrvCfg->SdramCfg.ColAddrBits > 13)
	    return BRDerr_SDRAM_BAD_PARAMETER;
	if(pMemSrvCfg->SdramCfg.ModuleBanks < 1 || pMemSrvCfg->SdramCfg.ModuleBanks > 2)
	    return BRDerr_SDRAM_BAD_PARAMETER;
	//if(pMemSrvCfg->SdramCfg.ChipBanks != 2 &&
	//	pMemSrvCfg->SdramCfg.ChipBanks != 4)
	//		return BRDerr_SDRAM_BAD_PARAMETER;
	if(pMemSrvCfg->SdramCfg.ChipBanks != 2 &&
		pMemSrvCfg->SdramCfg.ChipBanks != 4 && 
		pMemSrvCfg->SdramCfg.ChipBanks != 8)
			return BRDerr_SDRAM_BAD_PARAMETER;

	m_dwPhysMemSize =	(1 << pMemSrvCfg->SdramCfg.RowAddrBits) *
						(1 << pMemSrvCfg->SdramCfg.ColAddrBits) * 
						pMemSrvCfg->SdramCfg.ModuleBanks * 
						pMemSrvCfg->SdramCfg.ChipBanks *
						pMemSrvCfg->SdramCfg.ModuleCnt * 2; // in 32-bit Words

	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = m_index;
	RegParam.tetr = m_MemTetrNum;
	RegParam.reg = ADM2IFnr_IDMOD;
	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
	m_MemTetrModif = RegParam.val;

	//if(2 == m_MemTetrModif) // ADS10x2G
	//	if(pMemSrvCfg->PrimWidth != 0x4)
	//	    return BRDerr_SDRAM_BAD_PARAMETER;

	return BRDerr_OK;
}

//***************************************************************************************
void CSdramAmbpcdSrv::MemInit(CModule* pModule, ULONG init)
{
	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = m_index;
	RegParam.tetr = m_MemTetrNum;
	RegParam.reg = SDRAMnr_CFG;
	RegParam.val = 0;
	PSDRAM_CFG pMemCfg = (PSDRAM_CFG)&RegParam.val;
	PSDRAMAMBPCDSRV_CFG pMemSrvCfg = (PSDRAMAMBPCDSRV_CFG)m_pConfig;
	pMemCfg->ByBits.NumSlots = pMemSrvCfg->SdramCfg.ModuleCnt ? (pMemSrvCfg->SdramCfg.ModuleCnt - 1) : 0;
//	pMemCfg->ByBits.NumSlots = pMemSrvCfg->SdramCfg.ModuleCnt;
	pMemCfg->ByBits.RowAddr = pMemSrvCfg->SdramCfg.RowAddrBits - 8;
	pMemCfg->ByBits.ColAddr = pMemSrvCfg->SdramCfg.ColAddrBits - 8;
	pMemCfg->ByBits.BankModule = pMemSrvCfg->SdramCfg.ModuleBanks - 1;
	if(pMemSrvCfg->SdramCfg.MemType == SDRAMmt_DDR2)
		pMemCfg->ByBits.BankChip = pMemSrvCfg->SdramCfg.ChipBanks >> 3;
	else
		pMemCfg->ByBits.BankChip = pMemSrvCfg->SdramCfg.ChipBanks >> 2;
	pMemCfg->ByBits.DeviceWidth = 0;
	if(pMemSrvCfg->PrimWidth == 0x04)
		pMemCfg->ByBits.DeviceWidth = 1;
	pMemCfg->ByBits.Registered = 0;
//	if(pMemSrvCfg->Attributes == 0x26)
	if(pMemSrvCfg->Attributes & 0x02)
		pMemCfg->ByBits.Registered = 1;

	pMemCfg->ByBits.InitMemCmd = init; //1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
//	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
//	pMemCfg->ByBits.InitMemCmd = 0;
//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
}

// ***************** End of file SdramAmbpcdSrv.cpp ***********************
