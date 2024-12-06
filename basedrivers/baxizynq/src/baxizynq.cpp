
#define	DEVS_API_EXPORTS
#include "baxizynq.h"

#define	CURRFILE _BRDC("BAXIZYNQ")

//-----------------------------------------------------------------------------

CAxizynq::CAxizynq() :
    CBaseModule(MAX_BASE_CFGMEM_SIZE)
{
    for(int i = 0; i < MAX_SYSMON_SRV; i++)
        m_pSysMonSrv[i] = NULL;
    for(int i = 0; i < MAX_MEMORY_SRV; i++)
        m_pMemorySrv[i] = NULL;
    for(int i = 0; i < MAX_STREAM_SRV; i++)
        m_pStreamSrv[i] = NULL;
    m_pIniData = NULL;
    //m_pMemBuf[0] = m_pMemBuf[1] = 0;
    //m_hMutex = NULL;
}

//-----------------------------------------------------------------------------
// BASE_INI* pIniData - данные из ИИ (PID, BusNumber, SlotNumber) при полуавтоматической инициализации и
//						NULL при автоматической инициализации
//-----------------------------------------------------------------------------
CAxizynq::CAxizynq(PBRD_InitData pBrdInitData, long sizeInitData) :
    CBaseModule(MAX_BASE_CFGMEM_SIZE)
{
    for(int i = 0; i < MAX_SYSMON_SRV; i++)
        m_pSysMonSrv[i] = NULL;
    for(int i = 0; i < MAX_MEMORY_SRV; i++)
        m_pMemorySrv[i] = NULL;
    for(int i = 0; i < MAX_STREAM_SRV; i++)
        m_pStreamSrv[i] = NULL;

    m_pIniData = new BASE_INI;
    long size = sizeInitData;

    PINIT_Data pInitData = (PINIT_Data)pBrdInitData;
    // идентификация
    FindKeyWord(_BRDC("pid"), m_pIniData->PID, pInitData, size);

    FindKeyWord(_BRDC("pcibus"), m_pIniData->BusNumber, pInitData, size);
    FindKeyWord(_BRDC("pcidev"), m_pIniData->DevNumber, pInitData, size);
    FindKeyWord(_BRDC("pcislot"), m_pIniData->SlotNumber, pInitData, size);

    // конфигурация

    FindKeyWord(_BRDC("sysgen"), m_pIniData->SysGen, pInitData, size);

    FindKeyWord(_BRDC("slotcnt0"), m_pIniData->MemIni[0].SlotCnt, pInitData, size);
    FindKeyWord(_BRDC("modulecnt0"), m_pIniData->MemIni[0].ModuleCnt, pInitData, size);
    FindKeyWord(_BRDC("modulebanks0"), m_pIniData->MemIni[0].ModuleBanks, pInitData, size);
    FindKeyWord(_BRDC("rowaddrbits0"), m_pIniData->MemIni[0].RowAddrBits, pInitData, size);
    FindKeyWord(_BRDC("columnaddrbits0"), m_pIniData->MemIni[0].ColAddrBits, pInitData, size);
    FindKeyWord(_BRDC("chipbanks0"), m_pIniData->MemIni[0].ChipBanks, pInitData, size);
    FindKeyWord(_BRDC("caslatency0"), m_pIniData->MemIni[0].CasLatency, pInitData, size);
    FindKeyWord(_BRDC("primwidth0"), m_pIniData->MemIni[0].PrimWidth, pInitData, size);
    FindKeyWord(_BRDC("attibutes0"), m_pIniData->MemIni[0].Attributes, pInitData, size);

    FindKeyWord(_BRDC("memtetrnum"), m_pIniData->MemIni[0].TetrNum, pInitData, size);

    // настройка адреса при разделении одной физической памяти между двумя терадами
    FindKeyWord(_BRDC("memadrmask"), m_pIniData->MemIni[0].AdrMask, pInitData, size);
    FindKeyWord(_BRDC("memadrconst"), m_pIniData->MemIni[0].AdrConst, pInitData, size);

    FindKeyWord(_BRDC("slotcnt1"), m_pIniData->MemIni[1].SlotCnt, pInitData, size);
    FindKeyWord(_BRDC("modulecnt1"), m_pIniData->MemIni[1].ModuleCnt, pInitData, size);
    FindKeyWord(_BRDC("modulebanks1"), m_pIniData->MemIni[1].ModuleBanks, pInitData, size);
    FindKeyWord(_BRDC("rowaddrbits1"), m_pIniData->MemIni[1].RowAddrBits, pInitData, size);
    FindKeyWord(_BRDC("columnaddrbits1"), m_pIniData->MemIni[1].ColAddrBits, pInitData, size);
    FindKeyWord(_BRDC("chipbanks1"), m_pIniData->MemIni[1].ChipBanks, pInitData, size);
    FindKeyWord(_BRDC("caslatency1"), m_pIniData->MemIni[1].CasLatency, pInitData, size);
    FindKeyWord(_BRDC("primwidth1"), m_pIniData->MemIni[1].PrimWidth, pInitData, size);
    FindKeyWord(_BRDC("attibutes1"), m_pIniData->MemIni[1].Attributes, pInitData, size);

    FindKeyWord(_BRDC("memtetrnum1"), m_pIniData->MemIni[1].TetrNum, pInitData, size);

    // настройка адреса при разделении одной физической памяти между двумя терадами
    FindKeyWord(_BRDC("memadrmask"), m_pIniData->MemIni[1].AdrMask, pInitData, size);
    FindKeyWord(_BRDC("memadrconst1"), m_pIniData->MemIni[1].AdrConst, pInitData, size);

    // флаг включения питания FMC во время открытия устройства
    FindKeyWord(_BRDC("fmcpower"), m_pIniData->FmcPower, pInitData, size);
    if(m_pIniData->FmcPower == (U32)-1)
        m_pIniData->FmcPower = 1;

    //m_hMutex = NULL;
    //m_pMemBuf[0] = m_pMemBuf[1] = 0;
}

//-----------------------------------------------------------------------------

CAxizynq::~CAxizynq()
{
    //if(m_hMutex)
    //{
    //    IPC_deleteMutex(m_hMutex);
    //    m_hMutex = NULL;
    //}
    if(m_pIniData)
        delete m_pIniData;
    int srv_num = (int)m_SrvList.size();
    if(srv_num)
    {
        for(int i = 0; i < srv_num; i++)
        {
            m_SrvList.pop_back();

        }
    }
    DeleteServices();
    CleanUp();

}

static U16 flagPID = 0;

//-----------------------------------------------------------------------------
// Эта функция выполняет инициализацию (автоматическую и полуавтоматическую)
// U16 CurDevNum - начальный номер экземпляра модуля при поиске нужного базового модуля
// PTSTR pBoardName - сюда записывается оригинальное имя базового модуля
// PTSTR pIniString - NULL при полуавтоматической инициализации и 
//						указатель на строку с инициализационными данными при автоматической инициализации
//-----------------------------------------------------------------------------
S32 CAxizynq::Init(short CurDevNum, BRDCHAR* pBoardName, BRDCHAR* pIniString)
{

    //memset(&m_AmbConfiguration, 0, sizeof(m_AmbConfiguration));

    if(m_pIniData) // NULL при автоматической инициализации
    {
        if(m_pIniData->PID == (U32)-1 && (m_pIniData->BusNumber == (U32)-1 || m_pIniData->DevNumber == (U32)-1) && m_pIniData->SlotNumber == (U32)-1)
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                              CURRFILE, //ErrVitalText);
                              _BRDC("<Init> Undefined vital data (PID, Slot or BUS) into init source"));

        if(!CurDevNum && m_pIniData->PID)
            flagPID = 1;	// если у устройства 0 PID был НЕ равен 0

        if(flagPID == 1 && m_pIniData->PID == 0) // тогда и у других устройств PID НЕ должен быть 0
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                              CURRFILE, //ErrVitalText);
                              _BRDC("<Init> PID=0 by multyboard configuration"));

        if(m_pIniData->PID != 0)
            CurDevNum = 0;
    }
    AMB_LOCATION AmbLocation;
    S32	errorCode = BRDerr_OK;

    //=== Find device
    int DeviceNumber = CurDevNum;
	std::string name{ "/dev/fmc130e" };
	do {
		char strDevNum[8];
		sprintf(strDevNum, "%d", DeviceNumber);
		std::string devname = name + strDevNum;
		m_dev = get_device(devname);
        m_blk_main = get_main_blk(m_dev, 0);
		

        //if(!m_hMutex)
        //{
        //    BRDCHAR nameMutex[MAX_PATH];
        //    BRDC_sprintf(nameMutex, _BRDC("mutex_BAXIZYNQ%d"), DeviceNumber);
        //    m_hMutex = IPC_createMutex(nameMutex, FALSE);
        //}

        // Get DeviceID, RevisionID, PCI Bus, PCI Device, PCI Slot, PID
        if(GetDeviceID(m_DeviceID) != BRDerr_OK)
        {
            //if(m_hMutex)
            //{
            //    IPC_deleteMutex(m_hMutex);
            //    m_hMutex = NULL;
            //}
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                              CURRFILE, //ErrVitalText);
                              _BRDC("<Init> Error by GetDeviceID"));
        }
        if( m_DeviceID != FMC130E_DEVID ) // все типы поддерживаемых модулей
        {
            //if(m_hMutex)
            //{
            //    IPC_deleteMutex(m_hMutex);
            //    m_hMutex = NULL;
            //}
            DeviceNumber++;
            continue;
        }

        if(GetRevisionID(m_RevisionID) != BRDerr_OK)
        {
            //if(m_hMutex)
            //{
            //    IPC_deleteMutex(m_hMutex);
            //    m_hMutex = NULL;
            //}
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                              CURRFILE, //ErrVitalText);
                              _BRDC("<Init> Error by GetRevisionID"));
        }
        if(GetLocation(&AmbLocation) != BRDerr_OK)
        {
            //if(m_hMutex)
            //{
            //    IPC_deleteMutex(m_hMutex);
            //    m_hMutex = NULL;
            //}
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                              CURRFILE, //ErrVitalText);
                              _BRDC("<Init> Error by GetLocation"));
        }

        ICR_IdBase Info;
        if(ReadNvRAM(&Info, sizeof(ICR_IdBase), 0x80) != BRDerr_OK)
        {
            //if(m_hMutex)
            //{
            //    IPC_deleteMutex(m_hMutex);
            //    m_hMutex = NULL;
            //}
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                              CURRFILE, //ErrVitalText);
                              _BRDC("<Init> Error read data from ICR"));
        }

        if(GetBaseIcrDataSize() != BRDerr_OK)
        {
            //if(m_hMutex)
            //{
            //    IPC_deleteMutex(m_hMutex);
            //    m_hMutex = NULL;
            //}
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                              CURRFILE, //ErrVitalText);
                              _BRDC("<Init> Error by GetBaseIcrDataSize"));
        }

        if(GetPid() != BRDerr_OK)
        {
            //if(m_hMutex)
            //{
            //    IPC_deleteMutex(m_hMutex);
            //    m_hMutex = NULL;
            //}
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                              CURRFILE, //ErrVitalText);
                              _BRDC("<Init> Error by GetPid"));
        }

        if(m_pIniData)
        {
            if(m_pIniData->PID == (U32)-1)
            {
                if(m_pIniData->SlotNumber == (U32)-1)
                {
                    if(m_pIniData->BusNumber == AmbLocation.BusNumber && m_pIniData->DevNumber == AmbLocation.DeviceNumber)
                        break;
                }
                else
                {
                    if(m_pIniData->SlotNumber == AmbLocation.SlotNumber)
                        break;
                }
            }
            else
            {
				if (!m_pIniData->PID || m_pIniData->PID == m_PID)
                    break;
            }
            //if(m_hMutex)
            //{
            //    IPC_deleteMutex(m_hMutex);
            //    m_hMutex = NULL;
            //}
            DeviceNumber++;
        }

    } while(m_pIniData);

    m_BusNum = AmbLocation.BusNumber;
    m_DevNum = AmbLocation.DeviceNumber;
    m_SlotNum = AmbLocation.SlotNumber;

    BRDC_sprintf(m_name, _BRDC("BAXIZYNQ_%X"), m_PID);
    BRDC_strcpy(pBoardName, m_name);

    if(pIniString)
    {
        BRDC_sprintf(pIniString, _BRDC("pid=0x%X\npcibus=0x%X\npcidev=0x%X\npcislot=0x%X\n"), m_PID, m_BusNum, m_DevNum, m_SlotNum);
    }

    m_blk_fid = get_fid_blk(m_dev, blk_id::blk_fid_id);

    PUCHAR pBaseCfgMem = new UCHAR[m_sizeICR];
    ULONG BaseCfgSize = m_sizeICR;
    if(GetBaseEEPROM(pBaseCfgMem, BaseCfgSize) != BRDerr_OK)
    {
        delete pBaseCfgMem;
        pBaseCfgMem = NULL;
    }
    ULONG SubCfgSize = 0;
    for(int i = 0; i < m_AdmIfCnt; i++)
    {
        errorCode = SetSubEeprom(i, SubCfgSize);
    }
    errorCode = SetPuInfo(pBaseCfgMem, BaseCfgSize);
    errorCode = SetSdramConfig(m_pIniData, pBaseCfgMem, BaseCfgSize);
    if(pBaseCfgMem)
        delete pBaseCfgMem;

    if(!m_pIniData)
    {
        if(GetAdmIfCntICR(m_AdmIfCnt) == BRDerr_OK)
            errorCode = SetSubunitAuto();
    }
    else
    {
        m_AdmIfCnt = 0;
        m_SubunitCnt = 0;
    }

    errorCode = SetServices();

    return DRVERR(errorCode);
}

//-----------------------------------------------------------------------------

void CAxizynq::CleanUp()
{
}

//-----------------------------------------------------------------------------

void CAxizynq::GetDeviceInfo(BRD_Info* pInfo) const
{
    pInfo->boardType	= ((ULONG)m_DeviceID << 16) | (ULONG)m_RevisionID;
    pInfo->pid			= m_PID;
    switch(m_DeviceID)
    {
    case FMC130E_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC130E"));
        break;
    default:
        BRDC_strcpy(pInfo->name, _BRDC("Unknown"));
        break;
    }
    pInfo->busType		= BRDbus_PCI;
    pInfo->bus			= m_BusNum;
    pInfo->dev			= m_DevNum;
    pInfo->slot			= m_SlotNum;
    pInfo->verMajor		= VER_MAJOR;
    pInfo->verMinor		= VER_MINOR;
    int i = 0;
    for(i = 0; i < MAX_ADMIF; i++)
        pInfo->subunitType[i] = 0xffff;
    for(i = 0; i < m_SubunitCnt; i++)
        pInfo->subunitType[i] = m_Subunit[i].type;
    for(i = 0; i < 8; i++)
        pInfo->base[i] = 0;
//    for(i = 0; i < 2; i++)
//        pInfo->base[i] = m_PhysMem[i];

//    memcpy(&pInfo->base[6], &m_pMemBuf[0], sizeof(m_pMemBuf[0]));
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::HardwareInit()
{
    ULONG Status = BRDerr_OK;
    {
        ULONG pld_status = 1;
        if(GetPldStatus(pld_status, 0) != BRDerr_OK)
            return ErrorPrint(BRDerr_HW_ERROR,
                              CURRFILE,
                              _BRDC("<HardwareInit> Hardware error"));
        if(pld_status)
        {
            //Status = AdmPldStartTest();
            //Status = AdmPldWorkAndCheck();
            //Status = SetAdmPldFileInfo();
        }
    }

    if((!m_pIniData) || (m_pIniData && m_pIniData->FmcPower))
    {
        BRDextn_FMCPOWER pow;
        pow.onOff = 1;
        PowerOnOff(&pow, 0);
    }

    return Status;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetPower(PBRDextn_FMCPOWER pow)
{
    ULONG Status = BRDerr_OK;

	if (m_blk_fid->is_valid())
	{
		ULONG blk_v0 = m_blk_fid->rd(PEFIDadr_CONST_V0);
		uint32_t PowerStat = m_blk_fid->rd(PEFIDadr_POWER_STATUS);
		uint32_t PowerCtrl = m_blk_fid->rd(PEFIDadr_POWER_CTRL);
		PowerCtrl &= 0x8000; // mask POWER_CTRL[power_en]
		if (PowerCtrl)
			pow->onOff = 1;
		else
			pow->onOff = 0;
		pow->value = blk_v0;
		pow->slot = PowerStat;
	}
    else
    {
        pow->value = 0;
        pow->slot = 0;
        Status = BRDerr_INVALID_FUNCTION;
    }
    return Status;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::PowerOnOff(PBRDextn_FMCPOWER pow, int force)
{
    ULONG Status = BRDerr_OK;

	if (m_blk_fid->is_valid())
	{
		uint32_t PowerStat;
		ULONG blk_v0 = m_blk_fid->rd(PEFIDadr_CONST_V0);
		if (pow->onOff == 0)
		{	// выключить питание
			m_blk_fid->wd(PEFIDadr_POWER_CTRL, 0);
			IPC_delay(100);
			PowerStat = m_blk_fid->rd(PEFIDadr_POWER_STATUS);
		}
		else
		{	// включить питание
			uint32_t PowerStat0 = m_blk_fid->rd(PEFIDadr_POWER_STATUS);
			PowerStat = PowerStat0;

			uint32_t PowerCtrl = m_blk_fid->rd(PEFIDadr_POWER_CTRL);
			PowerStat0 = PowerCtrl & 0x8000; // mask POWER_CTRL[power_en]
			if (!PowerStat0 || force)
			{
				m_blk_fid->wd(PEFIDadr_POWER_CTRL, 0x8009);
				IPC_delay(100);
				PowerStat = m_blk_fid->rd(PEFIDadr_POWER_STATUS);
			}
		}
		pow->value = blk_v0;
		pow->slot = PowerStat;
		//BRDC_printf(_BRDC("CAxizynq::PowerOnOff: blk_v0 = %d, PowerStat = 0x%X\n"), pow->value, pow->slot);
	}
	else
	{
		pow->value = 0;
		pow->slot = 0;
		Status = BRDerr_INVALID_FUNCTION;
	}

    return Status;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::ReadFmcExt(PBRDextn_FMCEXT ext)
{
    ULONG Status = BRDerr_OK;

	if (m_blk_fid->is_valid())
	{
		//m_blk_fid->wd(PEFIDadr_SPD_DEVICE, 0);
		//m_blk_fid->wd(PEFIDadr_SPD_CTRL, 0);
		//if (!m_blk_fid->WaitReady())
		//	return -ETIMEDOUT;

		//m_blk_fid->wd(PEFIDadr_SPD_ADDR, ext->addr); // write address
		//uint32_t ctrl_val = (ext->dev << 4) | 0x2001; // SPD_CTRL, type operation - read
		//m_blk_fid->wd(PEFIDadr_SPD_CTRL, ctrl_val);
		//if (!WaitReady())
		//	return -ETIMEDOUT;

		//ext->value = m_blk_fid->rd(PEFIDadr_SPD_DATAL); // read data

		//m_blk_fid->wd(PEFIDadr_SPD_CTRL, 0);
	}
	else
		Status = BRDerr_FUNC_UNIMPLEMENTED;

    return Status;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::WriteFmcExt(PBRDextn_FMCEXT ext)
{
    ULONG Status = BRDerr_OK;

	if (m_blk_fid->is_valid())
	{
		//m_blk_fid->wd(PEFIDadr_SPD_DEVICE, 0);
		//m_blk_fid->wd(PEFIDadr_SPD_CTRL, 0);
		//if (!m_blk_fid->WaitReady())
		//	return -ETIMEDOUT;

		//m_blk_fid->wd(PEFIDadr_SPD_ADDR, ext->addr); // write address
		//m_blk_fid->wd(PEFIDadr_SPD_DATAL, ext->value); // write data
		//uint32_t ctrl_val = (ext->dev << 4) + 0x2002; // SPD_CTRL, type operation - write
		//m_blk_fid->wd(PEFIDadr_SPD_CTRL, ctrl_val);
		//if (!WaitReady())
		//	return -ETIMEDOUT;

		//m_blk_fid->wd(PEFIDadr_SPD_CTRL, 0);
	}
	else
		Status = BRDerr_FUNC_UNIMPLEMENTED;

    return Status;
}

//-----------------------------------------------------------------------------

void CAxizynq::GetPldCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAdmPld pCfgPld)
{
    PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
    int end_flag = 0;
    do
    {
        USHORT sign = *((USHORT*)pCfgMem);
        USHORT size = 0;
        if(sign == PLD_CFG_TAG)
        {
            PICR_CfgAdmPld pPldCfg = (PICR_CfgAdmPld)pCfgMem;
            memcpy(pCfgPld, pPldCfg, sizeof(ICR_CfgAdmPld));
            size += sizeof(ICR_CfgAdmPld);
            end_flag = 1;
        }
        else
        {
            size = *((USHORT*)pCfgMem + 1);
            size += 4;
            //			break;
        }
        pCfgMem = (PUCHAR)pCfgMem + size;
    } while(!end_flag && pCfgMem < pEndCfgMem);
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetPldDescription(BRDCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
    if(BaseEEPROMSize)
    {
        ICR_CfgAdmPld cfgPld;
        GetPldCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &cfgPld);
        m_pldType = cfgPld.bType;
        m_pldSpeedGrade = cfgPld.bSpeedGrade;
        BRDC_sprintf(Description, _BRDC("%d (%d %d %d)"), cfgPld.bType, cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
        switch(cfgPld.bType)
        {
        case 8:
            BRDC_sprintf(Description, _BRDC("XC4VLX%d-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 9:
            BRDC_sprintf(Description, _BRDC("XC4VSX%d-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 10:
            BRDC_sprintf(Description, _BRDC("XC4VFX%d-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 15:
            BRDC_sprintf(Description, _BRDC("XC5VLX%d-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 16:
            BRDC_sprintf(Description, _BRDC("XC5VSX%d-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 17:
            BRDC_sprintf(Description, _BRDC("XC5VFX%d-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 18:
            BRDC_sprintf(Description, _BRDC("XC5VLX%dT-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 19:
            BRDC_sprintf(Description, _BRDC("XC5VSX%dT-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 20:
            BRDC_sprintf(Description, _BRDC("XC5VFX%dT-%dFFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 21:
            BRDC_sprintf(Description, _BRDC("XC6VSX%dT-%dFFG%d"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 22:
            BRDC_sprintf(Description, _BRDC("XC6VLX%dT-%dFFG%d"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 23:
            BRDC_sprintf(Description, _BRDC("XC7K%dT-%dFFG%d"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 24:
            BRDC_sprintf(Description, _BRDC("XC7A%dT-%dFFG%d"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 25:
            if(m_DeviceID == FMC126P_DEVID)
                BRDC_sprintf(Description, _BRDC("XCKU%d-%dFFVA%dE"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            else
                BRDC_sprintf(Description, _BRDC("XCKU%d-%dFLVB%dE"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 26:
            BRDC_sprintf(Description, _BRDC("XCKU%dP-%dFFVA%dE"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 27:
            BRDC_sprintf(Description, _BRDC("XCVU%d-%dFFVA%dE"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 28:
            BRDC_sprintf(Description, _BRDC("XCVU%dP-%dFLVB%dE"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        }
        return 1;
    }
    return 0;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::SetPuInfo(PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
    m_puCnt = 1;
    m_AdmPldInfo.Id = BRDpui_PLD;//0x100;
    m_AdmPldInfo.Type = PLD_CFG_TAG;
	m_AdmPldInfo.Attr = 0;
    BRDCHAR Description0[MAX_DESCRIPLEN] = _BRDC("ADM PLD ");
    BRDCHAR Description[MAX_DESCRIPLEN] = _BRDC("ADM PLD XILINX");// Programmable Unit Description Text
    if(GetPldDescription(Description, pBaseEEPROMMem, BaseEEPROMSize))
    {
        BRDC_strcpy(m_AdmPldInfo.Description, Description0);
        BRDC_strcat(m_AdmPldInfo.Description, Description);
    }
    else
        BRDC_strcpy(m_AdmPldInfo.Description, Description);

    m_PuInfos.push_back(m_AdmPldInfo);

    if(1)
    {
        m_puCnt++;
        m_BaseIcrInfo.Id = BRDpui_BASEICR;
        m_BaseIcrInfo.Type = BASE_ID_TAG;
        m_BaseIcrInfo.Attr = BRDpua_Read | BRDpua_Write | BRDpua_Danger;
        BRDC_strcpy(m_BaseIcrInfo.Description, _BRDC("ID & CFG EEPROM on Base module"));

        m_PuInfos.push_back(m_BaseIcrInfo);
    }


    int subm[2] = { 1, 0 }; // субмодуль FMC1 присутствует, а субмодуль FMC2 отсутствует

    //if(m_BlockFidAddr)
    //{ // FMC субмодули
    //    PULONG BlockMem = m_pMemBuf[0];
    //    ULONG pldver = BlockMem[8];
    //    ULONG coreid = BlockMem[14];
    //    if(coreid != 0x22 || (coreid == 0x22 && pldver > 0x104))
    //    {
    //        BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
    //        ULONG PowerStat = BlockMem[32]; // POWER_STATUS
    //        if (!(PowerStat & 0x100))
    //        {
    //            subm[0] = 0;	// субмодуль FM0 отсутствует
    //            PUSHORT pSubCfgMem = (PUSHORT)(m_Subunit[0].pSubEEPROM); // пытаемся исправить ошибки на субмодуле (в частности, FM416x250M)
    //            if (pSubCfgMem)
    //            {
    //                if (pSubCfgMem[0] == 0x0080)// определяем наличие прописанного ППЗУ на субмодуле
    //                    subm[0] = 1;	// субмодуль FM0 все же присутствует
    //            }
    //        }
    //        if (PowerStat & 0x400)
    //            subm[1] = 1;	// субмодуль FM1 присутствует
    //    }
    //}

    for(int i = 0; i < m_AdmIfCnt; i++)
    {
        if(!subm[i])
            continue; // NO submodule
        PUSHORT pSubCfgMem = (PUSHORT)(m_Subunit[i].pSubEEPROM);
        if(!pSubCfgMem)
            break; // NO submodule EEPROM

        if(( pSubCfgMem[0] != 0xA55A) && (pSubCfgMem[1] != 0x5AA5))// определяет просто наличие ППЗУ на субмодуле
        {
            m_puCnt++;
            m_AdmIcrInfo.Id = BRDpui_SUBICR+i;
            m_AdmIcrInfo.Type = ADM_ID_TAG;
            m_AdmIcrInfo.Attr = BRDpua_Read | BRDpua_Write | BRDpua_Danger;
			BRDC_strcpy(m_AdmIcrInfo.Description, _BRDC("ID & CFG EEPROM on subunit"));
            m_PuInfos.push_back(m_AdmIcrInfo);
        }

        //if (m_blk_fid->is_valid())
        //{
        //    m_puCnt++;
        //    m_AdmFruInfo.Id = BRDpui_FRUID+i;
        //    m_AdmFruInfo.Type = 0;
        //    m_AdmFruInfo.Attr = BRDpua_Read | BRDpua_Write | BRDpua_Danger;
        //    BRDC_strcpy(m_AdmFruInfo.Description, _BRDC("FRU EEPROM on FMC1"));
        //    if(i)
        //        BRDC_strcpy(m_AdmFruInfo.Description, _BRDC("FRU EEPROM on FMC2"));
        //    m_PuInfos.push_back(m_AdmFruInfo);
        //}

    }

    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

void CAxizynq::GetPuList(BRD_PuList* pu_list) const
{
    int pu_num = (int)m_PuInfos.size();
    for(int i = 0; i < pu_num; i++)
    {
        pu_list[i].puId		= m_PuInfos[i].Id;
        pu_list[i].puCode	= m_PuInfos[i].Type;
        pu_list[i].puAttr	= m_PuInfos[i].Attr;
        BRDC_strcpy( (BRDCHAR*)pu_list[i].puDescription, m_PuInfos[i].Description);
    }
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::PuFileLoad(ULONG id, const BRDCHAR* fileName, PUINT pState)
{
    *pState = 0;
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetPuState(ULONG id, PUINT pState)
{
    // Check ADM PLD
    if(id == m_AdmPldInfo.Id)
    {
        ULONG pld_status;
        if(GetPldStatus(pld_status, id - BRDpui_PLD) != BRDerr_OK)
            return ErrorPrint(BRDerr_HW_ERROR,
                              CURRFILE,
                              _BRDC("<GetPuState> Hardware error"));
        *pState = pld_status;
    }
    else
    {
        // Check base module ICR
        if(id == m_BaseIcrInfo.Id)
        {
            ULONG base_status;
            if(GetBaseIcrStatus(base_status) != BRDerr_OK)
                return ErrorPrint(BRDerr_HW_ERROR,
                                  CURRFILE,
                                  _BRDC("<GetPuState> Hardware error"));
            *pState = base_status;
        }
        else
        {
            // Check subunit ICR
            if(id == m_AdmIcrInfo.Id)
            {
                ULONG adm_status;
                if(GetAdmIcrStatus(adm_status, m_AdmIcrInfo.Id - BRDpui_SUBICR) != BRDerr_OK)
                    return ErrorPrint(BRDerr_HW_ERROR,
                                      CURRFILE,
                                      _BRDC("<GetPuState> Hardware error"));
                *pState = adm_status;
            }
            else
            {
                // Check subunit FRU EEPROM
                if(id == m_AdmFruInfo.Id)
                {
                    *pState = 0;
                }
                else
                {
                    return ErrorPrintf( BRDerr_BAD_ID,
                                        CURRFILE,
                                        _BRDC("<GetPuState> Bad Programable Unit ID %d"), id);
                }
            }
        }
    }
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetPid()
{
    {
        if(m_sizeICR)
        {
            ULONG BufferSize = 4;
            ULONG Offset = 128 + 6;
            if(GetNvRAM(&m_PID, BufferSize, Offset) != BRDerr_OK)
            {
                return ErrorPrint(BRDerr_BAD_DEVICE_VITAL_DATA,
                                  CURRFILE,
                                  _BRDC("<GetPid> Undefined vital data into ICR"));
            }
        }
        else
            m_PID = 0;
    }
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::SetAdmPldFileInfo()
{
	main_trd_t trd_main = get_main_trd(m_dev, trd_id::trd_main_id);
	m_pldFileInfo.version = trd_main->ri(MAINnr_FPGAVER);
	m_pldFileInfo.modification = trd_main->ri(MAINnr_FPGAMODE);

	m_pldFileInfo.baseId = trd_main->ri(MAINnr_BASEID);
	m_pldFileInfo.baseVer = trd_main->ri(MAINnr_BASEVER);
	m_pldFileInfo.submodId = trd_main->ri(MAINnr_SMODID);
	m_pldFileInfo.submodVer = trd_main->ri(MAINnr_SMODVER);
	m_pldFileInfo.build = trd_main->ri(MAINnr_FPGABLD);

    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetPldInfo(PBRDextn_PLDINFO pInfo)
{
    SetAdmPldFileInfo();
    if(pInfo->pldId == m_pldFileInfo.pldId)
    {
        pInfo->version = m_pldFileInfo.version;
        pInfo->modification = m_pldFileInfo.modification;
        pInfo->build = m_pldFileInfo.build;
    }
    else
		return ErrorPrint( BRDerr_BAD_PARAMETER,
							CURRFILE,
                            _BRDC("<GetPldInfo> PLD ID is wrong"));
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetPldFileInfo(PBRDextn_PLDFILEINFO pInfo)
{
    SetAdmPldFileInfo();
    if(pInfo->pldId == m_pldFileInfo.pldId)
        GetAdmPldFileInfo(pInfo);
    else
        return ErrorPrint( BRDerr_BAD_PARAMETER,
                           CURRFILE,
                           _BRDC("<GetPldFileInfo> PLD ID is wrong"));
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

void CAxizynq::GetAdmPldFileInfo(PBRDextn_PLDFILEINFO pInfo) const
{
    pInfo->version = m_pldFileInfo.version;
    pInfo->modification = m_pldFileInfo.modification;
    pInfo->build = m_pldFileInfo.build;
    pInfo->baseId = m_pldFileInfo.baseId;
    pInfo->baseVer = m_pldFileInfo.baseVer;
    pInfo->submodId = m_pldFileInfo.submodId;
    pInfo->submodVer = m_pldFileInfo.submodVer;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::ShowDialog(PBRDextn_PropertyDlg pDlg)
{
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::DeleteDialog(PBRDextn_PropertyDlg pDlg)
{
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------
// определение наличия и размера ICR-данных, записанных в ППЗУ базового модуля
//-----------------------------------------------------------------------------

ULONG CAxizynq::GetBaseIcrDataSize()
{
    USHORT buf_read[3];
    buf_read[0] = buf_read[1] = buf_read[2] = 0xffff;
    ULONG ret_status = GetNvRAM(&buf_read, 6, 128);
    if(ret_status == BRDerr_OK)
    {
        if(buf_read[0] == BASE_ID_TAG)
            m_sizeICR = buf_read[2];
        else
            m_sizeICR = 0;
        if(m_sizeICR > MAX_BASE_CFGMEM_SIZE)
            m_sizeICR = 0;
        return ret_status;
    }
    else
        return ret_status;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetBaseIcrStatus(ULONG& Status)
{
    USHORT buf_read[3];
    buf_read[0] = buf_read[1] = buf_read[2] = 0xffff;
    ULONG ret_status = GetNvRAM(&buf_read, 6, 128);
    if(ret_status == BRDerr_OK)
    {
        if(buf_read[0] == BASE_ID_TAG && buf_read[2] <= MAX_BASE_CFGMEM_SIZE)
            Status = 1;
        else
            Status = 0;
        return ret_status;
    }
    else
        return ret_status;
}

// определяет не просто наличие конфигурационного ППЗУ на субмодуле,
// а то что оно прописано корректной информацией
//-----------------------------------------------------------------------------

ULONG CAxizynq::GetAdmIcrStatus(ULONG& Status, int idxSub)
{
    Status = 0;
    if(m_Subunit[idxSub].pSubEEPROM)
    {
        USHORT buf_read = *(PUSHORT)m_Subunit[idxSub].pSubEEPROM;
        if(buf_read == ADM_ID_TAG)
            Status = 1;
    }
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

void CAxizynq::GetAmbpexCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAmbpex pCfgBase)
{
    PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
    int end_flag = 0;
    do
    {
        USHORT sign = *((USHORT*)pCfgMem);
        USHORT size = 0;
        if(sign == BASE_ID_TAG)
        {
            PICR_IdBase pBaseId = (PICR_IdBase)pCfgMem;
            ICR_IdBase	idBase;
            memcpy(&idBase, pBaseId, sizeof(ICR_IdBase));
            PUCHAR pBaseCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdBase);
            size = sizeof(ICR_IdBase);
            PICR_CfgAmbpex pBambpexCfg = (PICR_CfgAmbpex)pBaseCfg;
            memcpy(pCfgBase, pBambpexCfg, sizeof(ICR_CfgAmbpex));
            size += sizeof(ICR_CfgAmbpex);
            end_flag = 1;
        }
        else
        {
            size = *((USHORT*)pCfgMem + 1);
            size += 4;
            //			break;
        }
        pCfgMem = (PUCHAR)pCfgMem + size;
    } while(!end_flag && pCfgMem < pEndCfgMem);
}

//-----------------------------------------------------------------------------

void CAxizynq::GetFmcCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgFmc105p pCfgBase)
{
    PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
    int end_flag = 0;
    do
    {
        USHORT sign = *((USHORT*)pCfgMem);
        USHORT size = 0;
        if(sign == BASE_ID_TAG)
        {
            PICR_IdBase pBaseId = (PICR_IdBase)pCfgMem;
            ICR_IdBase	idBase;
            memcpy(&idBase, pBaseId, sizeof(ICR_IdBase));
            PUCHAR pBaseCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdBase);
            size = sizeof(ICR_IdBase);
            PICR_CfgFmc105p pBfmcCfg = (PICR_CfgFmc105p)pBaseCfg;
            memcpy(pCfgBase, pBfmcCfg, sizeof(ICR_CfgFmc105p));
            size += sizeof(ICR_CfgFmc105p);
            end_flag = 1;
        }
        else
        {
            size = *((USHORT*)pCfgMem + 1);
            size += 4;
        }
        pCfgMem = (PUCHAR)pCfgMem + size;
    } while(!end_flag && pCfgMem < pEndCfgMem);
}

//-----------------------------------------------------------------------------

void CAxizynq::GetDdr3CfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDdr3 pCfgDdr3)
{
    PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
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
        case DDR3_CFG_TAG:
        {
            PICR_CfgDdr3 pSdramCfg = (PICR_CfgDdr3)pCfgMem;
            memcpy(pCfgDdr3, pSdramCfg, sizeof(ICR_CfgDdr3));
            size += sizeof(ICR_CfgDdr3);
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

//-----------------------------------------------------------------------------

ULONG CAxizynq::SetSdramConfig(PBASE_INI pIniData, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
    // устанавливаем конфигурацию по умолчанию
    m_SysGen = 66000000; // 66 МГц
    for(int i = 0; i < MAX_MEMORY_SRV; i++)
    {
        memcpy(&m_MemCfg[i], &MemCfg_dflt, sizeof(MEM_CFG));
        m_MemCfg[i].ModuleCnt |= 0x80;
        m_MemCfg[i].ModuleBanks |= 0x80;
        m_MemCfg[i].RowAddrBits |= 0x80;
        m_MemCfg[i].ColAddrBits |= 0x80;
        m_MemCfg[i].ChipBanks |= 0x80;
        m_MemCfg[i].PrimWidth |= 0x80;
        m_MemCfg[i].CasLat |= 0x80;
        m_MemCfg[i].Attributes |= 0x80;
    }

    if(m_DeviceID == FMC130E_DEVID)
    {
        m_MemCfg[0].SlotCnt = 1;
        m_MemCfg[1].SlotCnt = 0;
    }
    else
    {
        return BRDerr_OK;
    }
    if(BaseEEPROMSize)
    {
        if( m_DeviceID == XM416X250M_DEVID ||
                m_DeviceID == FMC117CP_DEVID ||
                m_DeviceID == FMC121CP_DEVID ||
                (m_DeviceID == PANORAMA_DEVID) ||
                m_DeviceID == FMC125CP_DEVID ||
                m_DeviceID == FMC127P_DEVID ||
                m_DeviceID == FMC112CP_DEVID)
        {
			ICR_CfgFmc105p icr_base;
			GetFmcCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &icr_base);
			m_Ddr3Cfg.CfgSrc = 2;// icr_base.isDDR3; изменял для axizync 
            m_SysGen = icr_base.dSysGen;
            if(!m_Ddr3Cfg.CfgSrc)
            {
                m_Ddr3Cfg.ModuleCnt = 0;
                m_Ddr3Cfg.CapacityMbits = 0;
            }
            if(m_Ddr3Cfg.CfgSrc == 1)
            {
                ICR_CfgDdr3 icr_memory;
                GetDdr3CfgICR(pBaseEEPROMMem, BaseEEPROMSize, &icr_memory);
                m_Ddr3Cfg.ModuleCnt = icr_memory.bModuleCnt;
                m_Ddr3Cfg.ModuleBanks = icr_memory.bModuleBanks;
                m_Ddr3Cfg.RowAddrBits = icr_memory.bRowAddrBits;
                m_Ddr3Cfg.ColAddrBits = icr_memory.bColAddrBits;
                m_Ddr3Cfg.ChipBanks = icr_memory.bChipBanks;
                m_Ddr3Cfg.PrimWidth = icr_memory.bPrimaryWidth;
                m_Ddr3Cfg.ChipWidth = icr_memory.bChipWidth;
                m_Ddr3Cfg.CapacityMbits = (U64)icr_memory.wCapacityMbits * 1024 * 1024;
            }
        }
        else
        {
            ICR_CfgAmbpex icr_base;
            GetAmbpexCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &icr_base);
            m_SysGen = icr_base.dSysGen;
            m_MemCfg[0].SlotCnt = icr_base.bSramCfgCnt;
        }
    }

    if (pIniData)
    {	// уточняем конфигурацию из источника инициализации
        SetSdramfromIniFile(pIniData);
    }
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

void CAxizynq::SetSdramfromIniFile(PBASE_INI pIniData)
{	// уточняем конфигурацию из источника инициализации
    if (pIniData->SysGen != 0xffffffff)
        m_SysGen = pIniData->SysGen;
    for (int iMem = 0; iMem < MAX_MEMORY_SRV; iMem++)
    {
        //if (BRDC_strlen(m_pIniData->MemIni[iMem].DlgDllName))
        //    BRDC_strcpy(m_MemCfg[iMem].DlgDllName, m_pIniData->MemIni[iMem].DlgDllName);
        if (pIniData->MemIni[iMem].SlotCnt != 0xffffffff)
            m_MemCfg[iMem].SlotCnt = pIniData->MemIni[iMem].SlotCnt;
        if (pIniData->MemIni[iMem].ModuleCnt != 0xffffffff)
            m_MemCfg[iMem].ModuleCnt = pIniData->MemIni[iMem].ModuleCnt;
        if (pIniData->MemIni[iMem].ModuleBanks != 0xffffffff)
            m_MemCfg[iMem].ModuleBanks = pIniData->MemIni[iMem].ModuleBanks;
        if (pIniData->MemIni[iMem].RowAddrBits != 0xffffffff)
            m_MemCfg[iMem].RowAddrBits = pIniData->MemIni[iMem].RowAddrBits;
        if (pIniData->MemIni[iMem].ColAddrBits != 0xffffffff)
            m_MemCfg[iMem].ColAddrBits = pIniData->MemIni[iMem].ColAddrBits;
        if (pIniData->MemIni[iMem].ChipBanks != 0xffffffff)
            m_MemCfg[iMem].ChipBanks = pIniData->MemIni[iMem].ChipBanks;
        if (pIniData->MemIni[iMem].PrimWidth != 0xffffffff)
            m_MemCfg[iMem].PrimWidth = pIniData->MemIni[iMem].PrimWidth;
        if (pIniData->MemIni[iMem].CasLatency != 0xffffffff)
            m_MemCfg[iMem].CasLat = pIniData->MemIni[iMem].CasLatency;
        if (pIniData->MemIni[iMem].Attributes != 0xffffffff)
            m_MemCfg[iMem].Attributes = pIniData->MemIni[iMem].Attributes;
        if (pIniData->MemIni[iMem].TetrNum != 0xffffffff)
            m_MemCfg[iMem].TetrNum = pIniData->MemIni[iMem].TetrNum;

        if (pIniData->MemIni[iMem].AdrMask != 0xffffffff)
            m_MemCfg[iMem].AdrMask = pIniData->MemIni[iMem].AdrMask;
        if (pIniData->MemIni[iMem].AdrConst != 0xffffffff)
            m_MemCfg[iMem].AdrConst = pIniData->MemIni[iMem].AdrConst;
    }
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetAdmIfCntICR(int& AdmIfCnt)
{
    // Check base module ICR
    ULONG icr_base_status;
    if(GetBaseIcrStatus(icr_base_status) != BRDerr_OK)
        return ErrorPrint(BRDerr_HW_ERROR,
                          CURRFILE,
                          _BRDC("<GetAdmIfCntICR> Hardware error by get base ICR status"));
    if(icr_base_status)
    {	// получаем конфигурацию из ППЗУ
        PUCHAR pBaseCfgMem = new UCHAR[MAX_BASE_CFGMEM_SIZE];
        ULONG BufferSize = MAX_BASE_CFGMEM_SIZE;
        ULONG Offset = OFFSET_BASE_CFGMEM;
        if(GetNvRAM(pBaseCfgMem, BufferSize, Offset) != BRDerr_OK)
            return ErrorPrint(BRDerr_BAD_DEVICE_VITAL_DATA,
                              CURRFILE,
                              _BRDC("<GetAdmIfCntICR> Bad device vital data by read base ICR"));
        ULONG dRealBaseCfgSize;
        if(*(PUSHORT)pBaseCfgMem == BASE_ID_TAG)
        {
            dRealBaseCfgSize = *((PUSHORT)pBaseCfgMem + 2);
            ICR_CfgAmbpex cfgBase;
            GetAmbpexCfgICR(pBaseCfgMem, dRealBaseCfgSize, &cfgBase);
            AdmIfCnt = cfgBase.bAdmIfCnt;
        }
        else
            return ErrorPrint(BRDerr_BAD_DEVICE_VITAL_DATA,
                              CURRFILE,
                              _BRDC("<GetAdmIfCntICR> Invalid BASE ID tag getted from ICR"));
        delete pBaseCfgMem;
    }
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetBaseEEPROM(PUCHAR pBaseCfgMem, ULONG& BaseCfgSize)
{
    if(BaseCfgSize)
    {	// получаем содержимое ППЗУ базового модуля
        ULONG BufferSize = BaseCfgSize;
        ULONG Offset = OFFSET_BASE_CFGMEM;
        if(GetNvRAM(pBaseCfgMem, BufferSize, Offset) != BRDerr_OK)
            return BRDerr_BAD_DEVICE_VITAL_DATA;
        else
        {
            ULONG dRealBaseCfgSize;
            if(*(PUSHORT)pBaseCfgMem == BASE_ID_TAG)
            {
                dRealBaseCfgSize = *((PUSHORT)pBaseCfgMem + 2);
                ICR_CfgAmbpex cfgBase;
                GetAmbpexCfgICR(pBaseCfgMem, dRealBaseCfgSize, &cfgBase);
                m_AdmIfCnt = cfgBase.bAdmIfCnt;
            }
            else
                return BRDerr_BAD_DEVICE_VITAL_DATA;
        }

    }
    else
        return BRDerr_BAD_DEVICE_VITAL_DATA;
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::SetSubEeprom(int idxSub, ULONG& SubCfgSize)
{
    if(!m_Subunit[idxSub].pSubEEPROM)
    {
        BRDCHAR nameFileMap[MAX_PATH];
        BRDC_sprintf(nameFileMap, _BRDC("subeeprom_%s_%d_%d"), m_name, m_PID, idxSub);
        int isAlreadyCreated = 0;
        m_Subunit[idxSub].hFileMap = IPC_createSharedMemoryEx(nameFileMap, SUBMOD_CFGMEM_SIZE,&isAlreadyCreated);
        m_Subunit[idxSub].pSubEEPROM = (PVOID)IPC_mapSharedMemory(m_Subunit[idxSub].hFileMap);

        if(m_Subunit[idxSub].pSubEEPROM == 0)
            return BRDerr_HW_ERROR;

        PUSHORT pSubCfgMem = (PUSHORT)(m_Subunit[idxSub].pSubEEPROM);
        if(m_Subunit[idxSub].pSubEEPROM && !isAlreadyCreated)
        {	// если 1-й раз, то заполняем
            pSubCfgMem[0] = 0xA55A;
            pSubCfgMem[1] = 0x5AA5;
            // получаем конфигурацию субмодуля из его ППЗУ
            ULONG Status = ReadSubICR(idxSub, m_Subunit[idxSub].pSubEEPROM, SUBMOD_CFGMEM_SIZE, 0);
            if(Status != BRDerr_OK)
                return BRDerr_HW_ERROR;
        }
        // в любом случае получаем размер
        if(pSubCfgMem[0] == ADM_ID_TAG)
        {
            SubCfgSize = pSubCfgMem[1];
            if(!SubCfgSize)
                return BRDerr_BAD_DEVICE_VITAL_DATA;
        }
        else
            return BRDerr_BAD_DEVICE_VITAL_DATA;
    }
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::GetSubEEPROM(int idxSub, PUCHAR pSubCfgMem, ULONG& SubCfgSize)
{
    // Check subunit ICR
    ULONG icr_adm_status;
    if(GetAdmIcrStatus(icr_adm_status, idxSub+m_isFMC2) != BRDerr_OK)
        return BRDerr_HW_ERROR;
    if(icr_adm_status)
    {	// получаем конфигурацию субмодуля из его ППЗУ
        memcpy(pSubCfgMem, m_Subunit[idxSub+m_isFMC2].pSubEEPROM, SubCfgSize);

        // копирование необходимо для C6678 из-за его особенностей выравнивания
        USHORT tag = 0;
        USHORT size = 0;
        memcpy(&tag, pSubCfgMem, 2);
        memcpy(&size, (USHORT*)pSubCfgMem+2, 2);

        if(tag == ADM_ID_TAG)
        {
            SubCfgSize = size;
            if(!SubCfgSize)
                return BRDerr_BAD_DEVICE_VITAL_DATA;
        }
        else
            return BRDerr_BAD_DEVICE_VITAL_DATA;
    }
    else
        return BRDerr_BAD_DEVICE_VITAL_DATA;
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CAxizynq::SetServices()
{
    DeleteServices();
    int iSrv = 0, iMemorySrv = 0, /*iTestSrv = 0, */iStreamSrv = 0, iSysMonSrv = 0;
    for(int iMem = 0; iMem < MAX_MEMORY_SRV; iMem++)
    {
            { // модули с DDR3
                DDR3SDRAMSRV_CFG mem_srv_cfg;
                BRDC_strcpy(mem_srv_cfg.DlgDllName, _BRDC(""));
                mem_srv_cfg.isAlreadyInit = 0;
                mem_srv_cfg.MemType = 0x0B; // DDR3
                mem_srv_cfg.TetrNum = m_MemCfg[iMem].TetrNum;
                mem_srv_cfg.AdrMask = m_MemCfg[iMem].AdrMask;
                mem_srv_cfg.AdrConst = m_MemCfg[iMem].AdrConst;
                if(XM416X250M_DEVID == m_DeviceID ||
                        FMC117CP_DEVID == m_DeviceID ||
                        FMC121CP_DEVID == m_DeviceID ||
                        PANORAMA_DEVID == m_DeviceID ||
                        FMC125CP_DEVID == m_DeviceID ||
                        FMC127P_DEVID == m_DeviceID ||
                        FMC112CP_DEVID == m_DeviceID)
                {
                    mem_srv_cfg.CfgSource = m_Ddr3Cfg.CfgSrc;
                    mem_srv_cfg.SlotCnt = m_Ddr3Cfg.ModuleCnt;
                    mem_srv_cfg.ModuleCnt = m_Ddr3Cfg.ModuleCnt;
                    mem_srv_cfg.ModuleBanks = m_Ddr3Cfg.ModuleBanks;
                    mem_srv_cfg.RowAddrBits = m_Ddr3Cfg.RowAddrBits;
                    mem_srv_cfg.ColAddrBits = m_Ddr3Cfg.ColAddrBits;
                    mem_srv_cfg.ChipBanks = m_Ddr3Cfg.ChipBanks;
                    mem_srv_cfg.PrimWidth = m_Ddr3Cfg.PrimWidth;
                    mem_srv_cfg.ChipWidth = m_Ddr3Cfg.ChipWidth;
                    mem_srv_cfg.CapacityMbits = m_Ddr3Cfg.CapacityMbits;
                }
                else
                {
                    mem_srv_cfg.CfgSource = 2; // from SPD only
                    mem_srv_cfg.SlotCnt = m_MemCfg[iMem].SlotCnt;
                    mem_srv_cfg.ModuleCnt = m_MemCfg[iMem].ModuleCnt;
                    mem_srv_cfg.ModuleBanks = m_MemCfg[iMem].ModuleBanks;
                    mem_srv_cfg.RowAddrBits = m_MemCfg[iMem].RowAddrBits;
                    mem_srv_cfg.ColAddrBits = m_MemCfg[iMem].ColAddrBits;
                    mem_srv_cfg.ChipBanks = m_MemCfg[iMem].ChipBanks;
                    mem_srv_cfg.PrimWidth = m_MemCfg[iMem].PrimWidth;
                    mem_srv_cfg.ChipWidth = 0;
                    mem_srv_cfg.CapacityMbits = 0;
                }
                m_pMemorySrv[iMemorySrv] = new CDdr3SdramSrv(iSrv++, iMemorySrv, this, &mem_srv_cfg);
            }
        m_SrvList.push_back(m_pMemorySrv[iMemorySrv++]);
    }
    for(int iStrm = 0; iStrm < MAX_STREAM_SRV; iStrm++)
    {
        STREAMSRV_CFG strm_srv_cfg;
        m_pStreamSrv[iStreamSrv] = new CStreamSrv(iSrv++, iStreamSrv, this, &strm_srv_cfg);
        m_SrvList.push_back(m_pStreamSrv[iStreamSrv++]);
		//m_mm2s[iStrm] = get_channel(m_dev, iStrm, iStrm);
	}
	//m_mm2s = get_channel(m_dev, DIR_DMA_MM2S, CHAN0);
	//m_s2mm = get_channel(m_dev, DIR_DMA_S2MM, CHAN1);
	m_dma[0] = get_channel(m_dev, DIR_DMA_MM2S, CHAN0);
	m_dma[1] = get_channel(m_dev, DIR_DMA_S2MM, CHAN1);

	m_dma_tetr_num[0] = m_dma_tetr_num[1] = 0;

    for(int iSysMon = 0; iSysMon < MAX_SYSMON_SRV; iSysMon++)
    {
        SYSMONSRV_CFG sysmon_srv_cfg;
        sysmon_srv_cfg.isAlreadyInit = 0;
        sysmon_srv_cfg.DeviceID = m_DeviceID;
        sysmon_srv_cfg.PldType = m_pldType;
        sysmon_srv_cfg.PldSpeedGrade = m_pldSpeedGrade;
        m_pSysMonSrv[iSysMonSrv] = new CSysMonSrv(iSrv++, iSysMonSrv, this, &sysmon_srv_cfg);
        m_SrvList.push_back(m_pSysMonSrv[iSysMonSrv++]);
    }
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

void CAxizynq::DeleteServices()
{
    for(int i = 0; i < MAX_SYSMON_SRV; i++)
    {
        if(m_pSysMonSrv[i]) {
            delete m_pSysMonSrv[i];
            m_pSysMonSrv[i] = NULL;
        }
    }
    for(int i = 0; i < MAX_MEMORY_SRV; i++)
    {
        if(m_pMemorySrv[i]) {
            delete m_pMemorySrv[i];
            m_pMemorySrv[i] = NULL;
        }
    }
    for(int i = 0; i < MAX_STREAM_SRV; i++)
    {
        if(m_pStreamSrv[i]) {
            delete m_pStreamSrv[i];
            m_pStreamSrv[i] = NULL;
        }
    }
}

//-----------------------------------------------------------------------------

void CAxizynq::SetCandidateSrv()
{
    int srv_num = (int)m_ServInfoList.size();
    BRDCHAR* pchar;
    for(int i = 0; i < srv_num; i++)
    {
        ULONG attr = m_ServInfoList[i]->attribute;
        int ius = 0;
        if(attr & BRDserv_ATTR_STREAMABLE_OUT)
        {
            for(int j = 0; j < srv_num; j++)
            {
                if( (m_ServInfoList[j]->attribute & BRDserv_ATTR_STREAM) &&
                        (m_ServInfoList[j]->attribute & BRDserv_ATTR_DIRECTION_OUT))
                    m_ServInfoList[i]->idxCandidSrv[ius++] = j;
            }
        }
        if(attr & BRDserv_ATTR_STREAMABLE_IN)
        {
            for(int j = 0; j < srv_num; j++)
            {
                if( (m_ServInfoList[j]->attribute & BRDserv_ATTR_STREAM) &&
                        (m_ServInfoList[j]->attribute & BRDserv_ATTR_DIRECTION_IN))
                    m_ServInfoList[i]->idxCandidSrv[ius++] = j;
            }
        }
        if(attr & BRDserv_ATTR_SDRAMABLE)
        {
            for(int j = 0; j < srv_num; j++)
            {
                pchar = BRDC_strstr(m_ServInfoList[j]->name, _BRDC("BASESDRAM"));
                if(pchar)
                {
                    if ((attr & BRDserv_ATTR_STREAMABLE_IN) &&
                            (m_ServInfoList[j]->attribute & BRDserv_ATTR_DIRECTION_IN))
                        m_ServInfoList[i]->idxCandidSrv[ius++] = j;
                    if ((attr & BRDserv_ATTR_STREAMABLE_OUT) &&
                            (m_ServInfoList[j]->attribute & BRDserv_ATTR_DIRECTION_OUT))
                        m_ServInfoList[i]->idxCandidSrv[ius++] = j;
                    //break;
                }
            }
        }
        if(attr & BRDserv_ATTR_ADCABLE)
        {
            for(int j = 0; j < srv_num; j++)
            {
                //				if(m_ServInfoList[j]->attribute & BRDserv_ATTR_DSPNODE)
                {
                    pchar = BRDC_strstr(m_ServInfoList[j]->name, _BRDC("ADC"));
                    if(pchar)
                    {
                        m_ServInfoList[i]->idxCandidSrv[ius++] = j;
                        break;
                    }
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------

LONG CAxizynq::RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam)
{
    ULONG admif = 0;
    ULONG status = BRDerr_OK;

    if (cmd <= DEVScmd_REGWRITESDIR && (pRegParam->tetr >= 16 || pRegParam->reg >= 1024))
        return ErrorPrintf(BRDerr_BAD_PARAMETER,
                           CURRFILE,
                           _BRDC("<RegCtrl> Cmd = %d, Tetr = 0x%04X, Reg = 0x%04X, Val = 0x%04X, One or more parameters are bad"),
                           cmd, pRegParam->tetr, pRegParam->reg, pRegParam->val);

    switch(cmd)
    {
    case DEVScmd_REGREADIND:
        status = ReadRegData(admif, pRegParam->tetr, pRegParam->reg, pRegParam->val);
        break;
    case DEVScmd_REGREADSIND:
        status = ReadRegBuf(admif, pRegParam->tetr, pRegParam->reg, pRegParam->pBuf, pRegParam->bytes);
        break;
    case DEVScmd_REGREADDIR:
        status = ReadRegDataDir(admif, pRegParam->tetr, pRegParam->reg, pRegParam->val);
        break;
    case DEVScmd_REGREADSDIR:
        status = ReadRegBufDir(admif, pRegParam->tetr, pRegParam->reg, pRegParam->pBuf, pRegParam->bytes);
        break;
    case DEVScmd_REGWRITEIND:
        status = WriteRegData(admif, pRegParam->tetr, pRegParam->reg, pRegParam->val);
        break;
    case DEVScmd_REGWRITESIND:
        status = WriteRegBuf(admif, pRegParam->tetr, pRegParam->reg, pRegParam->pBuf, pRegParam->bytes);
        break;
    case DEVScmd_REGWRITEDIR:
        status = WriteRegDataDir(admif, pRegParam->tetr, pRegParam->reg, pRegParam->val);
        break;
    case DEVScmd_REGWRITESDIR:
        status = WriteRegBufDir(admif, pRegParam->tetr, pRegParam->reg, pRegParam->pBuf, pRegParam->bytes);
        break;
    case DEVScmd_SETSTATIRQ:
		status = BRDerr_CMD_UNSUPPORTED;
        break;
    case DEVScmd_CLEARSTATIRQ:
		status = BRDerr_CMD_UNSUPPORTED;
        break;
#ifdef __linux__
    case DEVScmd_WAITSTATIRQ:
		status = BRDerr_CMD_UNSUPPORTED;
        break;
#endif
    case DEVScmd_GETBASEADR:
		status = BRDerr_CMD_UNSUPPORTED;
        break;
    case DEVScmd_PACKEXECUTE:
        return BRDerr_CMD_UNSUPPORTED;
    default:
    {
        BRDCHAR msg[MAX_PATH];
        BRDC_sprintf(msg, _BRDC("<RegCtrl> Command (%d)not supported"), cmd);
        return ErrorPrint(BRDerr_CMD_UNSUPPORTED,
                          CURRFILE,
                          msg);
    }
    }
    return status;
}

//-----------------------------------------------------------------------------
//                              управление стримами
//-----------------------------------------------------------------------------

ULONG CAxizynq::StreamCtrl(ULONG cmd, PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
    ULONG status = BRDerr_OK;

    switch(cmd)
    {
    case STRMcmd_SETMEMORY:
    {
        ULONG addr_data = 0;
        if(m_curHandle)
        {
            int idx	 = (m_curHandle >> 16) & 0x3F;
            if(idx < (long)m_SrvList.size())
				status = m_SrvList[idx]->DoCmd(this, SERVcmd_SYS_GETADDRDATA, 0, &addr_data, NULL);
            else
            {
                DEV_CMD_Ctrl param;
                param.handle = m_curHandle;
                param.cmd = SERVcmd_SYS_GETADDRDATA;
                param.arg = &addr_data;
                if(m_ServInfoList[idx]->baseORsideDLL)
                {
                    SIDE_API_entry* pSIDE_Entry = (SIDE_API_entry*)m_ServInfoList[idx]->pSideEntry;
                    status = pSIDE_Entry(m_ServInfoList[idx]->pSideDll, NULL, SIDEcmd_CTRL, &param);
                }
                else
                {
                    DEV_API_entry* pDEV_Entry = (DEV_API_entry*)m_ServInfoList[idx]->pSideEntry;
                    status = pDEV_Entry(m_ServInfoList[idx]->pSideDll, DEVcmd_CTRL, &param);
                }
            }
            if(status == BRDerr_OK)
                ((PAMB_MEM_DMA_CHANNEL)pParam)->LocalAddr = addr_data;
            else
                ((PAMB_MEM_DMA_CHANNEL)pParam)->LocalAddr = 0;
        }
        else
            ((PAMB_MEM_DMA_CHANNEL)pParam)->LocalAddr = 0;
        status = SetMemory(pParam, sizeParam);
    }
        break;
    case STRMcmd_FREEMEMORY:
        status = FreeMemory(pParam, sizeParam);
        break;
    case STRMcmd_STARTMEMORY:
    {
        status = StartMemory(pParam, sizeParam, pOverlap);
    }
        break;
    case STRMcmd_STOPMEMORY:
        status = StopMemory(pParam, sizeParam);
        break;
    case STRMcmd_STATEMEMORY:
        status = StateMemory(pParam, sizeParam);
        break;
    case STRMcmd_SETDIR:
        status = SetDirection(pParam, sizeParam);
        break;
    case STRMcmd_SETSRC:
        status = SetSource(pParam, sizeParam);
        break;
    case STRMcmd_SETDRQ:
		status = BRDerr_CMD_UNSUPPORTED;
        break;
    case STRMcmd_RESETFIFO:
        status = ResetFifo(pParam, sizeParam);
        break;
    case STRMcmd_ADJUST:
		status = BRDerr_CMD_UNSUPPORTED;
        break;
    case STRMcmd_DONE:
		status = BRDerr_CMD_UNSUPPORTED;
        break;
    case STRMcmd_GETDMACHANINFO:
        status = GetDmaChanInfo(pParam, sizeParam);
        break;
    case STRMcmd_SETIRQMODE:
		status = BRDerr_CMD_UNSUPPORTED;
        break;

    case STRMcmd_GETIRQCNT:
    {	// в axizynq не может быть расширенного режима прерываний, 
		// поскольку вообще лругой контроллер DMA :))
		status = BRDerr_CMD_UNSUPPORTED;
        break;
    }
#ifdef __linux__
    case STRMcmd_WAITDMABUFFER:
        status = WaitDmaBuffer(pParam, sizeParam);
        break;
    case STRMcmd_WAITDMABLOCK:
        status = WaitDmaBlock(pParam, sizeParam);
        break;
#endif
    }
    return status;
}

//***************** End of file baxizynq.cpp *****************
