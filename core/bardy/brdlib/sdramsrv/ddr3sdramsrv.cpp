/*
 ***************** File ddr3sdramsrv.cpp ***********************
 *
 * BRD Driver for ADM-interface DDR3 SDRAM service
 *
 * (C) InSys by Dorokhin A. Mar 2012
 *
 ******************************************************
*/

#include "module.h"
#include "ddr3sdramsrv.h"

#define	CURRFILE "DDR3SDRAMSRV"

//***************************************************************************************
CDdr3SdramSrv::CDdr3SdramSrv(int idx, int srv_num, CModule* pModule, PDDR3SDRAMSRV_CFG pCfg) :
	CSdramSrv(idx, _BRDC("BASESDRAM"), srv_num, pModule, pCfg, sizeof(DDR3SDRAMSRV_CFG))
{
}

//***************************************************************************************
void CDdr3SdramSrv::GetSdramTetrNum(CModule* pModule)
{
	PDDR3SDRAMSRV_CFG pSrvCfg = (PDDR3SDRAMSRV_CFG)m_pConfig;
	pSrvCfg->Mode = 1; // DIRECTION_IN
	m_MemTetrNum = GetTetrNum(pModule, m_index, SDRAMFMC106P_TETR_ID);
	if(m_MemTetrNum == -1)
	{
		m_MemTetrNum = GetTetrNum(pModule, m_index, SDRAMDDR3X_TETR_ID);
		if (m_MemTetrNum == -1)
		{
			m_MemTetrNum = GetTetrNum(pModule, m_index, SDRAMRDCONT_TETR_ID);
			pSrvCfg->Mode = 2; // DIRECTION_OUT
			if (m_MemTetrNum == -1)
			{
				m_MemTetrNum = GetTetrNum(pModule, m_index, DDR3X_DAC_TETR_ID);
				if (m_MemTetrNum == -1)
					pSrvCfg->Mode = 0;
			}
		}
		else
			m_MemTetrModif = 3; // SDRAMDDR3X_TETR_ID (0x9B) tetrada of memory
	}
	if(pSrvCfg->TetrNum != -1)
		m_MemTetrNum = pSrvCfg->TetrNum;

	m_attribute = BRDserv_ATTR_EXCLUSIVE_ONLY;

	if (pSrvCfg->Mode & 1) // DIRECTION_IN
		m_attribute |= BRDserv_ATTR_DIRECTION_IN |
		BRDserv_ATTR_STREAMABLE_IN |
		BRDserv_ATTR_EXCLUSIVE_ONLY;

	if (pSrvCfg->Mode & 2) // DIRECTION_OUT
		m_attribute |= BRDserv_ATTR_DIRECTION_OUT |
		BRDserv_ATTR_STREAMABLE_OUT |
		BRDserv_ATTR_EXCLUSIVE_ONLY;

}

//***************************************************************************************
void CDdr3SdramSrv::GetSdramTetrNumEx(CModule* pModule, ULONG TetrInstNum)
{
	PDDR3SDRAMSRV_CFG pSrvCfg = (PDDR3SDRAMSRV_CFG)m_pConfig;
	pSrvCfg->Mode = 1;  // DIRECTION_IN
	m_MemTetrNum = GetTetrNumEx(pModule, m_index, SDRAMFMC106P_TETR_ID, TetrInstNum);
	if(m_MemTetrNum == -1)
	{
		m_MemTetrNum = GetTetrNumEx(pModule, m_index, SDRAMDDR3X_TETR_ID, TetrInstNum);
		pSrvCfg->Mode = 1;
		if(m_MemTetrNum == -1)
		{
			m_MemTetrNum = GetTetrNumEx(pModule, m_index, SDRAMRDCONT_TETR_ID, TetrInstNum);
			pSrvCfg->Mode = 2; // DIRECTION_OUT
			if (m_MemTetrNum == -1)
			{
				m_MemTetrNum = GetTetrNum(pModule, m_index, DDR3X_DAC_TETR_ID);
				if (m_MemTetrNum == -1)
					pSrvCfg->Mode = 0;
			}
		}
		else
			m_MemTetrModif = 3; // SDRAMDDR3X_TETR_ID (0x9B) tetrada of memory
	}
	if(pSrvCfg->TetrNum != -1)
		m_MemTetrNum = pSrvCfg->TetrNum;

	m_attribute = BRDserv_ATTR_EXCLUSIVE_ONLY;

	if (pSrvCfg->Mode & 1) // DIRECTION_IN
		m_attribute |= BRDserv_ATTR_DIRECTION_IN |
		BRDserv_ATTR_STREAMABLE_IN |
		BRDserv_ATTR_EXCLUSIVE_ONLY;

	if (pSrvCfg->Mode & 2) // DIRECTION_OUT
		m_attribute |= BRDserv_ATTR_DIRECTION_OUT |
		BRDserv_ATTR_STREAMABLE_OUT |
		BRDserv_ATTR_EXCLUSIVE_ONLY;

}

//***************************************************************************************
UCHAR CDdr3SdramSrv::ReadSpdByte(CModule* pModule, ULONG OffsetSPD, ULONG CtrlSPD)
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
ULONG CDdr3SdramSrv::GetCfgFromSpd(CModule* pModule, PDDR3SDRAMSRV_CFG pCfgSPD)
{
	SDRAM_SPDCTRL SpdCtrl;
	SpdCtrl.AsWhole = 0;
	SpdCtrl.ByBits.Read = 1;

	//int flg_3x = GetTetrNum(pModule, m_index, SDRAMDDR3X_TETR_ID);
	int flg_106P = GetTetrNum(pModule, m_index, SDRAMFMC106P_TETR_ID);

	UCHAR mem_type[SDRAM_MAXSLOTS];
	SpdCtrl.ByBits.Slot = 0;
	mem_type[0] = ReadSpdByte(pModule, SDRAMspd_MEMTYPE, SpdCtrl.AsWhole);
	//if(flg_3x == -1)
	if (flg_106P == -1)
	{	// не FMC106P (тетрады 0x9B, 0xA5, 0xE5)
		//SpdCtrl.ByBits.Slot = 1;
		//mem_type[1] = ReadSpdByte(pModule, SDRAMspd_MEMTYPE, SpdCtrl.AsWhole);
		mem_type[1] = 0;
	}
	else
	{	// FMC106P (тетрада 0x8F)
		UCHAR val = ReadSpdByte(pModule, DDR3spd_ROWCOLADDR, SpdCtrl.AsWhole);
		if(mem_type[0] == 0 && val != 0)
			mem_type[0] = SDRAMmt_DDR3;
		mem_type[1] = 0;
	}

	for(int i = 0; i < SDRAM_MAXSLOTS; i++)
		if(mem_type[i] == SDRAMmt_DDR3)
			pCfgSPD->ModuleCnt++;
	//BRDCHAR dbg_buf[256];
	//BRDC_sprintf(dbg_buf, _BRDC("Debug INFO: GetCfgFromSpd(): flg = %d, memtype0 = %d, memtype1 = %d, ModuleCnt = %d\n"), flg_106P, mem_type[0], mem_type[1], pCfgSPD->ModuleCnt);
	//OutputDebugString(dbg_buf); // выводим в окно Visual Studio Output
	if(!pCfgSPD->ModuleCnt)
	    return BRDerr_SDRAM_NO_MEMORY;

	ULONG Status = BRDerr_OK;
	UCHAR val;
	UCHAR row_addr[SDRAM_MAXSLOTS], col_addr[SDRAM_MAXSLOTS], module_banks[SDRAM_MAXSLOTS], module_width[SDRAM_MAXSLOTS],
		  chip_width[SDRAM_MAXSLOTS], chip_banks[SDRAM_MAXSLOTS], capacity[SDRAM_MAXSLOTS];
	for(int i = 0; i < pCfgSPD->ModuleCnt; i++)
	{
		SpdCtrl.ByBits.Slot = i;
		val = ReadSpdByte(pModule, DDR3spd_ROWCOLADDR, SpdCtrl.AsWhole);
		row_addr[i] = (val >> 3) & 0x7;
		col_addr[i] = val & 0x7;
		val = ReadSpdByte(pModule, DDR3spd_MODBANKS, SpdCtrl.AsWhole);
		module_banks[i] = (val >> 3) & 0x7;
		chip_width[i] = val & 0x7;
		val = ReadSpdByte(pModule, DDR3spd_CHIPBANKS, SpdCtrl.AsWhole);
		chip_banks[i] = (val >> 4) & 0x7;
		capacity[i] = val & 0xF;
		val = ReadSpdByte(pModule, DDR3spd_WIDTH, SpdCtrl.AsWhole);
		module_width[i] = val & 0x7;


		if(i && ((row_addr[i] != row_addr[i-1]) ||
				 (col_addr[i] != col_addr[i-1]) || 
				 (module_banks[i] != module_banks[i-1]) ||
				 (chip_banks[i] != chip_banks[i-1]) || 
			     (module_width[i] != module_width[i-1]) || 
			     (chip_width[i] != chip_width[i-1]) || 
				 (capacity[i] != capacity[i-1])
				))
		{
		    Status = BRDerr_SDRAM_NO_EQU_SPD;
		}			
	}
	if(BRDerr_SDRAM_NO_EQU_SPD == Status)
		pCfgSPD->ModuleCnt--;

	pCfgSPD->MemType = mem_type[0];
	pCfgSPD->RowAddrBits = row_addr[0] + 12;
	pCfgSPD->ColAddrBits = col_addr[0] + 9;
	pCfgSPD->ModuleBanks = module_banks[0] + 1;
	pCfgSPD->PrimWidth = 8 << module_width[0];
	pCfgSPD->ChipBanks = 8 << chip_banks[0];
	pCfgSPD->ChipWidth = 4 << chip_width[0];
	pCfgSPD->CapacityMbits = (__int64)(1 << capacity[0]) * 256 * 1024 * 1024;
/*
	ULONG PhysMemSize =	(ULONG)(((pCfgSPD->CapacityMbits >> 3) *
		(__int64)pCfgSPD->PrimWidth / pCfgSPD->ChipWidth * pCfgSPD->ModuleBanks * pCfgSPD->ModuleCnt) >> 2); // in 32-bit Words
*/
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CDdr3SdramSrv::CheckCfg(CModule* pModule)
{
	DDR3SDRAMSRV_CFG cfgSPD;
	cfgSPD.ModuleCnt = 0;

	ULONG Status = GetCfgFromSpd(pModule, &cfgSPD);

	PDDR3SDRAMSRV_CFG pMemSrvCfg = (PDDR3SDRAMSRV_CFG)m_pConfig;

	//BRDCHAR dbg_buf[128];
	//BRDC_sprintf(dbg_buf, _BRDC("Debug INFO: CheckCfg(): GetCfgFromSpd()->Status = 0x%08X,  CfgSource = %d\n"), Status, pMemSrvCfg->CfgSource);
	//OutputDebugString(dbg_buf); // выводим в окно Visual Studio Output

	m_dwPhysMemSize = 0;
	if(pMemSrvCfg->CfgSource == 2)
	{ // использовать SPD
		if(BRDerr_OK != Status)
			return Status;
		pMemSrvCfg->MemType = cfgSPD.MemType;
		pMemSrvCfg->ModuleCnt = cfgSPD.ModuleCnt;
		pMemSrvCfg->RowAddrBits = cfgSPD.RowAddrBits;
		pMemSrvCfg->ColAddrBits = cfgSPD.ColAddrBits;
		pMemSrvCfg->ModuleBanks = cfgSPD.ModuleBanks;
		pMemSrvCfg->ChipBanks = cfgSPD.ChipBanks;
		pMemSrvCfg->PrimWidth = cfgSPD.PrimWidth;
		pMemSrvCfg->ChipWidth = cfgSPD.ChipWidth;
		pMemSrvCfg->CapacityMbits = cfgSPD.CapacityMbits;
	}
	if(!pMemSrvCfg->ModuleCnt)
		return BRDerr_SDRAM_NO_MEMORY;
	if(pMemSrvCfg->RowAddrBits < 12 || pMemSrvCfg->RowAddrBits > 16)
	    return BRDerr_SDRAM_BAD_PARAMETER;
	if(pMemSrvCfg->ColAddrBits < 9 || pMemSrvCfg->ColAddrBits > 12)
	    return BRDerr_SDRAM_BAD_PARAMETER;
	if(pMemSrvCfg->ModuleBanks < 1 || pMemSrvCfg->ModuleBanks > 4)
	    return BRDerr_SDRAM_BAD_PARAMETER;
	if(pMemSrvCfg->ChipBanks != 8 &&
		pMemSrvCfg->ChipBanks != 16 && 
		pMemSrvCfg->ChipBanks != 32 && 
		pMemSrvCfg->ChipBanks != 64)
			return BRDerr_SDRAM_BAD_PARAMETER;
	if(pMemSrvCfg->ChipWidth != 4 &&
		pMemSrvCfg->ChipWidth != 8 && 
		pMemSrvCfg->ChipWidth != 16 && 
		pMemSrvCfg->ChipWidth != 32)
			return BRDerr_SDRAM_BAD_PARAMETER;
	if(pMemSrvCfg->PrimWidth != 8 &&
		pMemSrvCfg->PrimWidth != 16 && 
		pMemSrvCfg->PrimWidth != 32 && 
		pMemSrvCfg->PrimWidth != 64)
			return BRDerr_SDRAM_BAD_PARAMETER;

	m_dwPhysMemSize =	(ULONG)(((pMemSrvCfg->CapacityMbits >> 3) * (__int64)pMemSrvCfg->PrimWidth / pMemSrvCfg->ChipWidth * pMemSrvCfg->ModuleBanks *
						pMemSrvCfg->ModuleCnt) >> 2); // in 32-bit Words
//	m_dwPhysMemSize =	(ULONG)(((cfgSPD.CapacityMbits >> 3) * (__int64)cfgSPD.PrimWidth / cfgSPD.ChipWidth * cfgSPD.ModuleBanks *
//						cfgSPD.ModuleCnt) >> 2); // in 32-bit Words

	//DEVS_CMD_Reg RegParam;
	//RegParam.idxMain = m_index;
	//RegParam.tetr = m_MemTetrNum;
	//RegParam.reg = ADM2IFnr_IDMOD;
	//pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
	//m_MemTetrModif = RegParam.val;

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_CNTRID; // CONTROLLER_ID
	param.val = 0xFFFF;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	m_ControllerId = param.val;

	param.reg = SDRAMnr_COUPLE; // COUPLE_TRD_NUMBER
	param.val = 0xFFFF;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	for(int i = 0; i<4; i++)
		m_CoupleNum[i] = (param.val >> (i*4)) & 0xF;

	return BRDerr_OK;
}

//***************************************************************************************
int CDdr3SdramSrv::GetCfgEx(PBRD_SdramCfgEx pCfg)
{
	PDDR3SDRAMSRV_CFG pSrvCfg = (PDDR3SDRAMSRV_CFG)m_pConfig;
	pCfg->Size = sizeof(BRD_SdramCfgEx);
	pCfg->MemType = pSrvCfg->MemType;
	pCfg->Mode = pSrvCfg->Mode;
	pCfg->ModuleCnt	  = pSrvCfg->ModuleCnt;	
	pCfg->ModuleBanks = pSrvCfg->ModuleBanks;
	pCfg->RowAddrBits = pSrvCfg->RowAddrBits;
	pCfg->ColAddrBits = pSrvCfg->ColAddrBits;
	pCfg->ChipBanks = pSrvCfg->ChipBanks;
	pCfg->PrimWidth = pSrvCfg->PrimWidth;
	pCfg->ChipWidth = pSrvCfg->ChipWidth;
	pCfg->CapacityMbits = (U32)(pSrvCfg->CapacityMbits / 1024 / 1024);

	return BRDerr_OK;
}

//***************************************************************************************
int CDdr3SdramSrv::ExtraInit(CModule* pModule)
{
	PDDR3SDRAMSRV_CFG pMemSrvCfg = (PDDR3SDRAMSRV_CFG)m_pConfig;

	if(!m_ControllerId)
		return BRDerr_OK;

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_ADDRMASK;
	param.val = pMemSrvCfg->AdrMask;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = SDRAMnr_ADDRCONST;
	param.val = pMemSrvCfg->AdrConst;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	// определяем размер физической памяти, выделенной для данной терады
	USHORT mask = 1;
	for (int i = 0; i < 10; i++)
	{
		if (pMemSrvCfg->AdrMask & mask)
			break;
		mask <<= 1;
	}
	U64 virt_size = (U64)mask << 23;

	BRDC_printf(_BRDC("SDRAM Cfg Trd%d: virt_size = %d MBytes (mask = 0x%04X, const = 0x%04X)\n"),
                (int)m_MemTetrNum, U32(virt_size / 1024 / 1024), pMemSrvCfg->AdrMask, pMemSrvCfg->AdrConst);

	return BRDerr_OK;
}

//***************************************************************************************
void CDdr3SdramSrv::MemInit(CModule* pModule, ULONG init)
{
	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = m_index;
	RegParam.tetr = m_MemTetrNum;
	RegParam.reg = SDRAMnr_CFG;
	RegParam.val = 0;
	PSDRAM_CFG pMemCfg = (PSDRAM_CFG)&RegParam.val;
	PDDR3SDRAMSRV_CFG pMemSrvCfg = (PDDR3SDRAMSRV_CFG)m_pConfig;
	pMemCfg->ByBits.NumSlots = pMemSrvCfg->ModuleCnt ? (pMemSrvCfg->ModuleCnt - 1) : 0;
	pMemCfg->ByBits.RowAddr = pMemSrvCfg->RowAddrBits - 8;
	pMemCfg->ByBits.ColAddr = pMemSrvCfg->ColAddrBits - 8;
	pMemCfg->ByBits.BankModule = pMemSrvCfg->ModuleBanks - 1;
	pMemCfg->ByBits.BankChip = pMemSrvCfg->ChipBanks >> 3;
	pMemCfg->ByBits.DeviceWidth = 0;
	pMemCfg->ByBits.Registered = 0;

	pMemCfg->ByBits.InitMemCmd = init; //1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
//	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
//	pMemCfg->ByBits.InitMemCmd = 0;
//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
}

//***************************************************************************************
int CDdr3SdramSrv::SetSel(CModule* pModule, ULONG& sel, ULONG cmd)
{
        //PDDR3SDRAMSRV_CFG pSrvCfg = (PDDR3SDRAMSRV_CFG)m_pConfig;
	
	//if( pSrvCfg->Mode != 3 )
	//	return BRDerr_CMD_UNSUPPORTED;

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_MODE3;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PSDRAM_MODE3 pMode3 = (PSDRAM_MODE3)&param.val;
	if(BRDctrl_SDRAM_SETSEL == cmd)
	{
		pMode3->ByBits.SelIn = sel & 0xFF;
		pMode3->ByBits.SelOut = (sel >> 8) & 0xFF;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{
		sel = pMode3->ByBits.SelIn;
		sel += pMode3->ByBits.SelOut << 8;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDdr3SdramSrv::SetTestSeq(CModule* pModule, ULONG mode)
{
	int flg_3x = GetTetrNum(pModule, m_index, SDRAMDDR3X_TETR_ID);
	if(flg_3x == -1)
		return BRDerr_CMD_UNSUPPORTED;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_TESTSEQ;
	param.val = mode;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = SDRAMnr_MODE3;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PSDRAM_MODE3 pMode3 = (PSDRAM_MODE3)&param.val;
	pMode3->ByBits.FullSpeed = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDdr3SdramSrv::GetTestSeq(CModule* pModule, ULONG& mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MemTetrNum;
	param.reg = SDRAMnr_TESTSEQ;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	mode = param.val;
	return BRDerr_OK;
}

// ***************** End of file ddr3sdramsrv.cpp ***********************
