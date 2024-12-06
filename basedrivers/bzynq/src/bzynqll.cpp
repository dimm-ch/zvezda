
#include "bzynq.h"
#ifdef __linux__
#include <sys/time.h>
#endif

#define	CURRFILE _BRDC("BZYNQLL")

#define BIT_TCK 0x1000
#define BIT_TMS 0x2000
#define BIT_TDI 0x4000

#pragma pack(push,1)

// Board Control register 0x40 (PE_MAIN)
typedef union _BRD_MODE {
	USHORT AsWhole; // Board Control Register as a Whole Word
	struct { // Board Control Register as Bit Pattern
		USHORT	RstClkOut	: 1, // Output Clock Reset for ADMPLD
				Sleep		: 1, // Sleep mode for ADMPLD
				RstClkIn	: 1, // Input Clock Reset for ADMPLD
				Reset		: 1, // Reset for ADMPLD
				RegLoop		: 1, // Register operation loopback mode
				Res			: 3, // Reserved
				OutFlags	: 8; // Output Flags
	} ByBits;
} BRD_MODE, *PBRD_MODE;

// Board Status register 0x80 (PE_MAIN)
typedef union _BRD_STATUS {
	USHORT AsWhole; // Board Status Register as a Whole Word
	struct { // Board Status Register as Bit Pattern
		USHORT	SlvDcm		: 1, // Capture Input Clock from ADMPLD
				Res			: 3, // 
			    Tdo			: 1, // pin TDO
				Res1		: 3, // 
				InFlags		: 8; // Input Flags
	} ByBits;
} BRD_STATUS, *PBRD_STATUS;

#pragma pack(pop)

static PULONG pMemBuf = NULL;

//-----------------------------------------------------------------
#ifdef __linux__
ULONG GetLastError(void)
{
	return errno;
}
#endif

//-----------------------------------------------------------------------------

ULONG CBzynq::GetMemoryAddress()
{
	ULONG status = GetConfiguration(&m_AmbConfiguration);
    if(status == BRDerr_OK)
    {
		// маппировать на 3-м кольце нужно только под linux (под windows адреса маппируются на 0-м кольце)
        if(!m_pMemBuf[0])
		{
			S32 res = MapDeviceMemory(m_AmbConfiguration, 0);
			if (res != BRDerr_OK)
                return BRDerr_ACCESS_VIOLATION;
        }
        if(!m_pMemBuf[1])
		{
			S32 res = MapDeviceMemory(m_AmbConfiguration, 1);
			if (res != BRDerr_OK)
				return BRDerr_ACCESS_VIOLATION;
        }

		pMemBuf = (PULONG)m_AmbConfiguration.VirtAddress[0];
		if(pMemBuf)
		{
			ULONG blk_id = pMemBuf[0];
			blk_id &= 0xFFF;
            //ULONG blk_ver = pMemBuf[2];
			//ULONG dev_id = pMemBuf[4];
			//ULONG dev_rev = pMemBuf[6];
			//ULONG pld_ver = pMemBuf[8];
			//ULONG blk_cnt = pMemBuf[10];
			if(0x013 != blk_id) // идентификатор блока управления PE_MAIN
				status = BRDerr_BAD_ID;
			m_pMemBuf[0] = pMemBuf;
			m_pMemBuf[1] = (PULONG)m_AmbConfiguration.VirtAddress[1];
			m_PhysMem[0] = (U32)m_AmbConfiguration.PhysAddress[0];
			m_PhysMem[1] = (U32)m_AmbConfiguration.PhysAddress[1];
		}
		else
			status = BRDerr_ACCESS_VIOLATION;
	}
	return status;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::LoadPld(const BRDCHAR* PldFileName, UCHAR& PldStatus)
{
    return BRDerr_FUNC_UNIMPLEMENTED;
}

//-----------------------------------------------------------------------------

int  CBzynq::DevIoCtl(IPC_handle hDev,
						 DWORD code, 
						 LPVOID InBuf, 
						 DWORD InBufSize, 
						 LPVOID OutBuf, 
						 DWORD OutBufSize, 
						 DWORD *pRet, 
						 LPOVERLAPPED lpOverlapped)
{
	if(m_hMutex) 
		//WaitForSingleObject(m_hMutex, INFINITE);
        IPC_captureMutex(m_hMutex, INFINITE);

	int ret = 0;

    if(lpOverlapped)
    {
        ret = IPC_ioctlDeviceOvl(
								hDev,
								code,
								InBuf,
								InBufSize,
								OutBuf,
								OutBufSize,
								(void*)lpOverlapped);
	}
	else
    {
        //BOOL ret = DeviceIoControl(
        ret = IPC_ioctlDevice(
							hDev,
							code,
							InBuf,
							InBufSize,
							OutBuf,
							OutBufSize);
    }

	if(m_hMutex)
		//ReleaseMutex(m_hMutex);
        IPC_releaseMutex(m_hMutex);

	return ret;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::SetPldStatus(ULONG PldStatus, ULONG PldNum)
{
	ULONG length;     // the return length from the driver
	ULONG param = (PldNum << 16) + PldStatus;

    int res = DevIoCtl(
			m_hWDM,
            IOCTL_AMB_SET_PLD_STATUS,
            &param,
            sizeof(ULONG),
            NULL,
            0,
            &length,
            NULL);
    if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::GetPldStatus(ULONG& PldStatus, ULONG PldNum)
{
	ULONG length;     // the return length from the driver
	ULONG Status;

	if(PldNum)
	{
		if(PldNum == 1)
		{
			DEVS_CMD_Reg RegParam;
			RegParam.idxMain = 0;
			RegParam.tetr = m_FdspTetrNum;
			RegParam.reg = 0x200;
			Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
			if(RegParam.val & 1)
				PldStatus = 1;
			else
				PldStatus = 0;
		}
		else
		{
			PULONG BlockMem = m_pMemBuf[0];
			ULONG PldId = PldNum - 0x20;
			ULONG icap_ctrl = (PldId << 8) + 1;
			//ULONG icap_ctrl = (PldNum << 8) + 1;
			BlockMem[58] = icap_ctrl; //ICAP_CTRL = 58
			ULONG val = BlockMem[58]; //ICAP_CTRL = 58
			ULONG rp_done = (val >> 8) & 0xFF;
			BlockMem[58] = 0;
			PldStatus = rp_done;
		}
	}
	else
	{
		ULONG num = PldNum;
	    int res = DevIoCtl(
				m_hWDM,
				IOCTL_AMB_GET_PLD_STATUS,
				&num,
				sizeof(ULONG),
				&Status,
				sizeof(ULONG),
				&length,
				NULL);
	    if(res < 0){
			return GetLastError();
		}
		PldStatus = Status;
	}

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::ReadMemDataDir(ULONG* pDataReg, ULONG* pStatusReg, ULONG& Value)
{
	ULONG cmd_rdy;
	for(int i = 0; i < 5; i++)
	{
		cmd_rdy = *pStatusReg;
		if(cmd_rdy & 1)
			break;
		IPC_delay(2);
	}
	if(!(cmd_rdy & 1))
		return 1;

	Value = *pDataReg;
	return 0;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::ReadMemData(ULONG* pCmdReg, ULONG* pDataReg, ULONG* pStatusReg, ULONG RegNumber, ULONG& Value)
{
	*pCmdReg = RegNumber;
	ULONG cmd_rdy;
	for(int i = 0; i < 5; i++)
	{
		cmd_rdy = *pStatusReg;
		if(cmd_rdy & 1)
			break;
		IPC_delay(2);
	}
	if(!(cmd_rdy & 1))
		return 1;

	Value = *pDataReg & 0xFFFF;
	return 0;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::WriteMemDataDir(ULONG* pDataReg, ULONG* pStatusReg, ULONG Value)
{
	ULONG cmd_rdy;
	for(int i = 0; i < 5; i++)
	{
		cmd_rdy = *pStatusReg;
		if(cmd_rdy & 1)
			break;
		IPC_delay(2);
	}
	if(!(cmd_rdy & 1))
		return 1;

	*pDataReg = Value;
	return 0;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::WriteMemData(ULONG* pCmdReg, ULONG* pDataReg, ULONG* pStatusReg, ULONG RegNumber, ULONG Value)
{
	*pCmdReg = RegNumber;
	ULONG cmd_rdy;
	for(int i = 0; i < 5; i++)
	{
		cmd_rdy = *pStatusReg;
		if(cmd_rdy & 1)
			break;
		IPC_delay(2);
	}
	if(!(cmd_rdy & 1))
		return 1;

	*pDataReg = Value;
	return 0;
}

#define REG_SIZE	1024			// register size
#define TETRAD_SIZE	4096			// tetrad size

//-----------------------------------------------------------------------------

ULONG CBzynq::LoadBitPld(UCHAR& PldStatus, PVOID pBuffer, ULONG BufferSize)
{
	ULONG Address = m_FdspTetrNum * TETRAD_SIZE;
	PULONG adrStatusReg = m_pMemBuf[1] + Address;
	PULONG adrDataReg = m_pMemBuf[1] + Address + REG_SIZE;
	PULONG adrCmdAdrReg = m_pMemBuf[1] + Address + 2 * REG_SIZE;
	PULONG adrCmdDataReg = m_pMemBuf[1] + Address + 3 * REG_SIZE;

	ULONG val = 3;
	//WriteRegData(0, 8, 0x0D, val);
	WriteMemData(adrCmdAdrReg, adrCmdDataReg, adrStatusReg, 0x0D, val);
	IPC_delay(10);
	ULONG i;
	for( i = 0; i < 100; i++)
	{
		ReadMemData(adrCmdAdrReg, adrCmdDataReg, adrStatusReg, 0x200, val);
		if(!(val & 2))
			break;
		IPC_delay(1);
	}
	val = 1;
	//WriteRegData(0, 8, 0x0D, val);
	WriteMemData(adrCmdAdrReg, adrCmdDataReg, adrStatusReg, 0x0D, val);
	IPC_delay(10);
	for(i = 0; i < 100; i++)
	{
		ReadMemData(adrCmdAdrReg, adrCmdDataReg, adrStatusReg, 0x200, val);
		if(val & 2)
			break;
		IPC_delay(1);
	}
	ULONG cnt = BufferSize >> 2; // пишем 4-байтовыми словами
	ULONG* pData = (ULONG*)pBuffer;

	// volatile - чтобы код был не выкинут при оптимизации
	// но оптимизацию все равно лучше отключить
	volatile ULONG _val;
	for(ULONG i = 0; i < cnt; i++)
	{
		// бессмысленные операторы добавлены только для временной задержки
		_val = pData[i];
		*adrDataReg = _val;
		_val++;
	}
	IPC_delay(10); // для временной задержки при включенной оптимизации
	PldStatus = 0;
	ULONG k = 0;
	for(k = 0; k < 1000; k++)
	{
		// бессмысленные операторы добавлены только для временной задержки
		_val = 0;
		*adrDataReg = _val;
		_val = *adrDataReg;
		
	}
	//ReadRegData(0, 8, 0x200, val);
	ReadMemData(adrCmdAdrReg, adrCmdDataReg, adrStatusReg, 0x200, val);
	if(val & 1)
		PldStatus = 1;

	val = 0;
	//WriteRegData(0, 8, 0x0D, val);
	WriteMemData(adrCmdAdrReg, adrCmdDataReg, adrStatusReg, 0x0D, val);

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

// тетрада MAIN под сброс и все её регистры в ноль
ULONG CBzynq::GenReset()
{
	ULONG Status;
	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = 0;
	RegParam.tetr = ADM2IFnt_MAIN;
	RegParam.reg = ADM2IFnr_MODE0;
	RegParam.val = 1;
	Status = RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
	for(int i = 1; i < 32; i++)
	{
		RegParam.reg = i;
		RegParam.val = 0;
		Status = RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
		if(Status != BRDerr_OK)
			return Status;
	}
	return Status;
}

//-----------------------------------------------------------------------------

// все тетрады, кроме MAIN под сброс и все их регистры в ноль
ULONG CBzynq::AllReset()
{
	ULONG Status;
	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = 0;
	for(int iTetr = 1; iTetr < 16; iTetr++)
	{
		RegParam.tetr = iTetr;
		RegParam.reg = 0;
		RegParam.val = 1;
		Status = RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
		if(Status != BRDerr_OK)
			return Status;
		for(int i = 1; i < 32; i++)
		{
			RegParam.reg = i;
			RegParam.val = 0;
			Status = RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			if(Status != BRDerr_OK)
				return Status;
		}
	}
	return Status;
}

//-----------------------------------------------------------------------------

// отмена сброса всех тетрад
ULONG CBzynq::Unreset()
{
	ULONG Status;
	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = 0;
	for(int iTetr = 0; iTetr < 16; iTetr++)
	{
		RegParam.tetr = iTetr;
		RegParam.reg = 0;
		RegParam.val = 0;
		Status = RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
		if(Status != BRDerr_OK)
			return Status;
	}
	return Status;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::LoadPartPld(UCHAR PartPldNum, UCHAR& PldStatus, UCHAR* pBuffer, ULONG BufferSize)
{
	GenReset();
	PULONG BlockMem = m_pMemBuf[0];
	ULONG val = BlockMem[58]; //ICAP_CTRL = 58
	ULONG icap_sig = val >> 16;
	if(ICAP_SIG != icap_sig)
	{
		PldStatus = 0;
		return BRDerr_OK;
	}
    //ULONG part_puId = BlockMem[60]; //ICAP_DATA = 60
	ULONG icap_ctrl = (PartPldNum << 8) + 3;
	BlockMem[58] = icap_ctrl; //ICAP_CTRL = 58
	val = BlockMem[58]; //ICAP_CTRL = 58
	ULONG rp_done = (val >> 8) & 0xFF;

    //ULONG* pData = (ULONG*)pBuffer;
	for(ULONG i = 0; i < BufferSize; i++)
	{
		BlockMem[60] = pBuffer[i];
		BlockMem[58] = icap_ctrl; //ICAP_CTRL = 58
	}
	val = BlockMem[58]; //ICAP_CTRL = 58
	rp_done = (val >> 8) & 0xFF;
	BlockMem[58] = 0;
	PldStatus = (UCHAR)rp_done;

	AllReset();
	Unreset();

	return BRDerr_OK;
}

static UCHAR EepromBase[512];

//-----------------------------------------------------------------------------

ULONG CBzynq::ReadNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	memset(pBuffer, 0xff, BufferSize);
	ULONG   length;     // the return length from the driver
	AMB_DATA_BUF NvRAM = {NULL, MAX_BASE_CFGMEM_SIZE, OFFSET_BASE_CFGMEM};

    int res = DevIoCtl(
			m_hWDM,
			IOCTL_AMB_READ_NVRAM,
			&NvRAM,
			sizeof(AMB_DATA_BUF),
			EepromBase + OFFSET_BASE_CFGMEM,
			MAX_BASE_CFGMEM_SIZE,
			&length,
			NULL);
    if(res < 0){
		return GetLastError();
	}

	memcpy(pBuffer, EepromBase + Offset, BufferSize);
/*
    fprintf(stderr, "IOCTL_AMB_READ_NVRAM\n");
    u8 *ptr = (u8*)pBuffer;
    for(unsigned i=0; i<BufferSize; i++) {
        if((i == 0) || ((i % 16) == 0))
            fprintf(stderr, "\n");
        fprintf(stderr, "%x ", ptr[i]);
    }
    fprintf(stderr, "\n");
*/
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

// исправление результатов старой (уже исправленной) ошибки драйвера при записи нечётного числа байт в ППЗУ
ULONG CBzynq::BaseIcrCorrectError()
{
	EepromBase[128 + m_sizeICR - 1] = 0;
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::GetNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	memcpy(pBuffer, EepromBase + Offset, BufferSize);
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::WriteNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	if(!BufferSize)
		return BRDerr_OK;

	memcpy(EepromBase + Offset, pBuffer, BufferSize);
/*
    fprintf(stderr, "IOCTL_AMB_WRITE_NVRAM\n");
    u8 *ptr = (u8*)pBuffer;
    for(unsigned i=0; i<BufferSize; i++) {
        if((i == 0) || ((i % 16) == 0))
            fprintf(stderr, "\n");
        fprintf(stderr, "%x ", ptr[i]);
    }
    fprintf(stderr, "\n");
*/
	ULONG   length;     // the return length from the driver
	AMB_DATA_BUF NvRAM = {EepromBase + OFFSET_BASE_CFGMEM, MAX_BASE_CFGMEM_SIZE, OFFSET_BASE_CFGMEM};

    int res = DevIoCtl(
			m_hWDM,
			IOCTL_AMB_WRITE_NVRAM,
			&NvRAM,
			sizeof(AMB_DATA_BUF),
            NULL,
            0,
			&length,
			NULL);
    if(res < 0){
		return GetLastError();
	}
    IPC_delay(10);
	return BRDerr_OK;
}

static UCHAR EepromSubmod[512];

//-----------------------------------------------------------------------------

ULONG CBzynq::ReadAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	memset(EepromSubmod, 0xff, 256);

	ULONG   length;     // the return length from the driver
	AMB_DATA_BUF admIdROM = {NULL, 256, 0};

	if(m_BlockFidAddr)
		admIdROM.Offset = 0x400;

    int res = DevIoCtl(
			m_hWDM,
			IOCTL_AMB_READ_ADMIDROM,
			&admIdROM,
			sizeof(AMB_DATA_BUF),
			EepromSubmod,
			256, // bytes
			&length,
			NULL);
    if(res < 0){
		return GetLastError();
	}
    memcpy(pBuffer, EepromSubmod + Offset, BufferSize);
/*
    fprintf(stderr, "IOCTL_AMB_READ_ADMIDROM\n");
    u8 *ptr = (u8*)pBuffer;
    for(unsigned i=0; i<BufferSize; i++) {
        if((i == 0) || ((i % 16) == 0))
            fprintf(stderr, "\n");
        fprintf(stderr, "%x ", ptr[i]);
    }
    fprintf(stderr, "\n");
*/
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::WriteAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	if(!BufferSize)
		return BRDerr_OK;

	memcpy(EepromSubmod + Offset, pBuffer, BufferSize);
/*
    fprintf(stderr, "IOCTL_AMB_WRITE_ADMIDROM\n");
    u8 *ptr = (u8*)pBuffer;
    for(unsigned i=0; i<BufferSize; i++) {
        if((i == 0) || ((i % 16) == 0))
            fprintf(stderr, "\n");
        fprintf(stderr, "%x ", ptr[i]);
    }
    fprintf(stderr, "\n");
*/
    ULONG   length;     // the return length from the driver
	AMB_DATA_BUF admIdROM = {EepromSubmod, 256, 0};

	if(m_BlockFidAddr)
		admIdROM.Offset = 0x400;
    int res = DevIoCtl(
          m_hWDM,
			IOCTL_AMB_WRITE_ADMIDROM,
            &admIdROM,
			sizeof(AMB_DATA_BUF),
            NULL,
            0,
            &length,
			NULL);
    if(res < 0){
        return GetLastError();
    }
    IPC_delay(10);
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::ReadSubICR(int submod, PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG Status = BRDerr_OK;
	if(m_BlockFidAddr)
	{
		memset(EepromSubmod, 0xaa, 256);
		if(submod)
			Status = ReadFidSpd(2, 0x50, EepromSubmod, 0x400, 256); // FMC2
		else
			Status = ReadFidSpd(0, 0x50, EepromSubmod, 0x400, 256); // FMC1

		memcpy(pBuffer, EepromSubmod + Offset, BufferSize);
	}
	else
	{
		if(!submod)
			Status = ReadAdmIdROM(pBuffer, BufferSize, Offset);
		else
			Status = BRDerr_INVALID_FUNCTION;
	}
	return Status;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::WriteSubICR(int submod, PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG Status = BRDerr_OK;
	if(m_BlockFidAddr)
	{
		memcpy(EepromSubmod + Offset, pBuffer, BufferSize);
		if(submod)
		{
			Status = WriteFidSpd(2, 0x50, EepromSubmod, 0x400, 256); // FMC2
			//Status = ReadFidSpd(2, 0x50, EepromSubmod, 0x400, 256); // FMC2
		}
		else
			Status = WriteFidSpd(0, 0x50, EepromSubmod, 0x400, 256); // FMC1
	}
	else
	{
		if(!submod)
			Status = WriteAdmIdROM(pBuffer, BufferSize, Offset);
		else
			Status = BRDerr_INVALID_FUNCTION;
	}
	return Status;
}

static UCHAR FruidEeprom[1024];

//-----------------------------------------------------------------------------

ULONG CBzynq::ReadFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	memset(FruidEeprom, 0xff, 256);
	ULONG   length;     // the return length from the driver
	AMB_DATA_BUF admFruROM = {NULL, 256, 0};

	if(m_BlockFidAddr)
	{
		int res = DevIoCtl(
				m_hWDM,
				IOCTL_AMB_READ_ADMIDROM,
				&admFruROM,
				sizeof(AMB_DATA_BUF),
				FruidEeprom,
				256, // bytes
				&length,
				NULL);
		if(res < 0){
			return GetLastError();
		}
		memcpy(pBuffer, FruidEeprom + Offset, BufferSize);
	}
	else
		return BRDerr_NO_DATA;
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::WriteFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	if(!BufferSize)
		return BRDerr_OK;
	memcpy(FruidEeprom + Offset, pBuffer, BufferSize);

    ULONG   length;     // the return length from the driver
	AMB_DATA_BUF admFruROM = {FruidEeprom, 256, 0};

	if(m_BlockFidAddr)
	{
	    int res = DevIoCtl(
			  m_hWDM,
				IOCTL_AMB_WRITE_ADMIDROM,
				&admFruROM,
				sizeof(AMB_DATA_BUF),
				NULL,
				0,
				&length,
				NULL);
		if(res < 0){
			return GetLastError();
		}
	    IPC_delay(10);
	}
	else
		return BRDerr_NO_DATA;

	return BRDerr_OK;
}
/*
// ***************************************************************************************
ULONG CBzynq::ReadAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	BufferSize = (((BufferSize + 1) >> 1) << 1);
    ULONG   length;     // the return length from the driver
	AMB_DATA_BUF admIdROM = {NULL, BufferSize, Offset};

//    if (!DeviceIoControl(
	if (!DevIoCtl(
            m_hWDM,
			IOCTL_AMB_READ_ADMIDROM,
            &admIdROM,
			sizeof(AMB_DATA_BUF),
			pBuffer,
            BufferSize,
            &length,
            NULL)) {
        return GetLastError();
    }
	return BRDerr_OK;
}

// ***************************************************************************************
ULONG CBzynq::WriteAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
    BufferSize = (((BufferSize + 1) >> 1) << 1);
	ULONG   length;     // the return length from the driver
	AMB_DATA_BUF admIdROM = {pBuffer, BufferSize, Offset};

//    if (!DeviceIoControl(
  	if (!DevIoCtl(
          m_hWDM,
			IOCTL_AMB_WRITE_ADMIDROM,
            &admIdROM,
			sizeof(AMB_DATA_BUF),
            NULL,
            0,
            &length,
            NULL)) {
        return GetLastError();
    }
	Sleep(10);
	return BRDerr_OK;
}
*/
//-----------------------------------------------------------------------------

ULONG CBzynq::GetLocation(PAMB_LOCATION pAmbLocation)
{
    pAmbLocation->BusNumber = -1;
    pAmbLocation->DeviceNumber = -1;
    pAmbLocation->SlotNumber = -1;
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::GetConfiguration(PAMB_CONFIGURATION pAmbConfiguration)
{
    ULONG   length;     // the return length from the driver

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_GET_CONFIGURATION,
            NULL,
            0,
            pAmbConfiguration,
            sizeof(AMB_CONFIGURATION),
            &length,
			NULL);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

//ULONG CBzynq::GetVersion(PTSTR pVerInfo)
ULONG CBzynq::GetVersion(char* pVerInfo)
{
	char Buf[MAX_STRING_LEN];

	int res = DevIoCtl(
            m_hWDM, 
			IOCTL_AMB_GET_VERSION,
            NULL,
            0,
            Buf,
            MAX_STRING_LEN);
	if(res < 0){
        return GetLastError();
    }

    strcpy(pVerInfo, Buf);

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::GetDeviceID(USHORT& DeviceID)
{
    ULONG   length;
    AMB_DATA_BUF PciConfigPtr = {NULL, sizeof(USHORT), 2};

    int res = DevIoCtl(
            m_hWDM,
            IOCTL_AMB_GET_BUS_CONFIG,
            &PciConfigPtr,
            sizeof(AMB_DATA_BUF),
            &DeviceID,
            sizeof(USHORT),
            &length,
            NULL);
    if(res < 0){
        return GetLastError();
    }
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::GetRevisionID(UCHAR& RevisionID)
{
    ULONG   length;
    AMB_DATA_BUF PciConfigPtr = {NULL, sizeof(UCHAR), 8};

    int res = DevIoCtl(
            m_hWDM,
            IOCTL_AMB_GET_BUS_CONFIG,
            &PciConfigPtr,
            sizeof(AMB_DATA_BUF),
            &RevisionID,
            sizeof(UCHAR),
            &length,
            NULL);
    if(res < 0){
        return GetLastError();
    }
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::WriteRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
{
    ULONG   length;  

	AMB_DATA_REG reg_data = { AdmNum, TetrNum, RegNum, RegVal};

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_WRITE_REG_DATA,
            &reg_data,
            sizeof(AMB_DATA_REG),
            NULL,
            0,
            &length,
            NULL);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::WriteRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
{
    ULONG   length;  

	AMB_DATA_REG reg_data = { AdmNum, TetrNum, RegNum, RegVal};

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_WRITE_REG_DATA_DIR,
            &reg_data,
            sizeof(AMB_DATA_REG),
            NULL,
            0,
            &length,
            NULL);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::ReadRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
{
    ULONG   length;  

	AMB_DATA_REG reg_data = { AdmNum, TetrNum, RegNum, 0};

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_READ_REG_DATA,
            &reg_data,
            sizeof(AMB_DATA_REG),
            &reg_data,
            sizeof(AMB_DATA_REG),
            &length,
            NULL);
	if(res < 0){
        return GetLastError();
    }
	RegVal = reg_data.Value;
	
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::ReadRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
{
    ULONG   length;  

	AMB_DATA_REG reg_data = { AdmNum, TetrNum, RegNum, 0};

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_READ_REG_DATA_DIR,
            &reg_data,
            sizeof(AMB_DATA_REG),
            &reg_data,
            sizeof(AMB_DATA_REG),
            &length,
            NULL);
	if(res < 0){
        return GetLastError();
    }
	RegVal = reg_data.Value;
	
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::WriteRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
    ULONG   length;  

	AMB_BUF_REG reg_buf = { AdmNum, TetrNum, RegNum, RegBuf, RegBufSize };

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_WRITE_REG_BUF,
            &reg_buf,
            sizeof(AMB_BUF_REG),
            NULL,
            0,
            &length,
            NULL);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::WriteRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
    ULONG   length;  

	AMB_BUF_REG reg_buf = { AdmNum, TetrNum, RegNum, RegBuf, RegBufSize };

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_WRITE_REG_BUF_DIR,
            &reg_buf,
            sizeof(AMB_BUF_REG),
            NULL,
            0,
            &length,
            NULL);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::ReadRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
    ULONG   length;  

	AMB_BUF_REG reg_buf = { AdmNum, TetrNum, RegNum, RegBuf, RegBufSize };

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_READ_REG_BUF,
            &reg_buf,
            sizeof(AMB_BUF_REG),
            &reg_buf,
            sizeof(AMB_BUF_REG),
            &length,
            NULL);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::ReadRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
    ULONG   length;  

    AMB_BUF_REG reg_buf = { 0, 0, 0, 0, 0 };

    reg_buf.AdmNumber = AdmNum;
    reg_buf.TetrNumber = TetrNum;
    reg_buf.RegNumber = RegNum;
    reg_buf.pBuffer = RegBuf;
    reg_buf.BufferSize = RegBufSize;

    //fprintf(stderr, "%s, %d, %s(): sizeof(AMB_BUF_REG) = %d\n", __FILE__, __LINE__, __func__, sizeof(AMB_BUF_REG));

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_READ_REG_BUF_DIR,
            &reg_buf,
            sizeof(AMB_BUF_REG),
            &reg_buf,
            sizeof(AMB_BUF_REG),
            &length,
            NULL);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::SetMemory(PVOID pParam, ULONG sizeParam)
{
    AMB_MEM_DMA_CHANNEL *pMemDescr = (AMB_MEM_DMA_CHANNEL *)pParam;
#ifdef __linux__
	if((m_DeviceID == PANORAMA_DEVID || m_DeviceID == FMC121CP_DEVID) &&
		(pMemDescr->MemType == 0))
		return BRDerr_BAD_PARAMETER;
#endif

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_SET_MEMIO,
            pParam,
            sizeParam,
            pParam,
            sizeParam);
	if(res < 0){
        return GetLastError();
    }

    // нужно только для linux (под windows это функции-заглушки)
    if(pMemDescr->MemType == 1	||	// для системной памяти
			pMemDescr->MemType == 8)	// для физических адресов
	{
		for(unsigned iBlk = 0; iBlk < pMemDescr->BlockCnt; iBlk++)
		{
            if(IPC_mapPhysAddr(m_hWDM, &pMemDescr->pBlock[iBlk], (size_t)pMemDescr->pBlock[iBlk], pMemDescr->BlockSize))
				return BRDerr_ACCESS_VIOLATION;
		}
	}
    if(pMemDescr->pStub)
	{
        if(IPC_mapPhysAddr(m_hWDM, &pMemDescr->pStub, (size_t)pMemDescr->pStub, sizeof(BRDstrm_Stub)))
			return BRDerr_ACCESS_VIOLATION;
    }

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::FreeMemory(PVOID pParam, ULONG sizeParam)
{
	// нужно только для linux (под windows это функции-заглушки)
    AMB_MEM_DMA_CHANNEL *pMemDescr = (AMB_MEM_DMA_CHANNEL *)pParam;
    if(pMemDescr->MemType == 1	||	// для системной памяти
        pMemDescr->MemType == 8) {	// для физических адресов
        for(ULONG iBlk = 0; iBlk < pMemDescr->BlockCnt; iBlk++) {
            IPC_unmapPhysAddr(m_hWDM, pMemDescr->pBlock[iBlk], pMemDescr->BlockSize);
        }
    }
    if(pMemDescr->pStub) {
        IPC_unmapPhysAddr(m_hWDM, pMemDescr->pStub, sizeof(BRDstrm_Stub));
    }

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_FREE_MEMIO,
            pParam,
            sizeParam,
            pParam,
            sizeParam);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::StartMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
    ULONG   length;  

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_START_MEMIO,
            pParam,
            sizeParam,
            NULL,
            0,
            &length,
            pOverlap);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::StopMemory(PVOID pParam, ULONG sizeParam)
{
	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_STOP_MEMIO,
            pParam,
            sizeParam,
            pParam,
            sizeParam);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::StateMemory(PVOID pParam, ULONG sizeParam)
{
	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_STATE_MEMIO,
            pParam,
            sizeParam,
            pParam,
            sizeParam);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::SetDirection(PVOID pParam, ULONG sizeParam)
{
	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_SET_DIR_MEMIO,
            pParam,
            sizeParam,
            NULL,
            0);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::SetSource(PVOID pParam, ULONG sizeParam)
{
	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_SET_SRC_MEMIO,
            pParam,
            sizeParam,
            NULL,
            0);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::SetDmaRequest(PVOID pParam, ULONG sizeParam)
{
	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_SET_DRQ_MEMIO,
            pParam,
            sizeParam,
            NULL,
            0);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::SetStatusIrq(ULONG AdmNum, ULONG TetrNum, ULONG IrqMask, ULONG IrqInv, HANDLE hEvent)
{
    ULONG   length = 0;
	AMB_TETR_IRQ tetr_irq = { AdmNum, TetrNum, IrqMask, IrqInv, hEvent };

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_SET_TETRIRQ,
            &tetr_irq,
            sizeof(AMB_TETR_IRQ),
            NULL,
            0,
            &length,
            NULL);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::ClearStatusIrq(ULONG AdmNum, ULONG TetrNum, ULONG IrqMask, ULONG IrqInv, HANDLE hEvent)
{
    ULONG   length = 0;
	AMB_TETR_IRQ tetr_irq = { AdmNum, TetrNum, IrqMask, IrqInv, hEvent };

	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_CLEAR_TETRIRQ,
            &tetr_irq,
            sizeof(AMB_TETR_IRQ),
            NULL,
            0,
            &length,
            NULL);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::ResetFifo(PVOID pParam, ULONG sizeParam)
{
	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_RESET_FIFO,
            pParam,
            sizeParam,
            NULL,
            0);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::Adjust(PVOID pParam, ULONG sizeParam)
{
	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_ADJUST,
            pParam,
            sizeParam,
            NULL,
            0);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::Done(PVOID pParam, ULONG sizeParam)
{
	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_DONE,
            pParam,
            sizeParam,
            NULL,
            0);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::GetDmaChanInfo(PVOID pParam, ULONG sizeParam)
{
	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_GET_INFOIO,
            pParam,
            sizeParam,
            pParam,
            sizeParam);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::SetIrqMode(PVOID pParam, ULONG sizeParam)
{
	int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_SET_IRQ_TABLE,
            pParam,
            sizeParam,
            NULL,
            0);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

#ifdef __linux__
// ***************************************************************************************
ULONG CBzynq::WaitDmaBlock(void* pParam, ULONG sizeParam)
{
    int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_WAIT_DMA_BLOCK,
            pParam,
            sizeParam,
            pParam,
            sizeParam);
    if(res < 0){
        return IPC_sysError();
    }
    return BRDerr_OK;
}

// ***************************************************************************************
ULONG CBzynq::WaitDmaBuffer(void* pParam, ULONG sizeParam)
{
    int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_WAIT_DMA_BUFFER,
            pParam,
            sizeParam,
            pParam,
            sizeParam);
    if(res < 0){
        return IPC_sysError();
    }
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::WaitStatusIrq(ULONG AdmNum, ULONG TetrNum, ULONG IrqMask, ULONG IrqInv, HANDLE hEvent)
{
    ULONG   length = 0;
    AMB_TETR_IRQ tetr_irq = { AdmNum, TetrNum, IrqMask, IrqInv /* as Timeout */, hEvent };

    int res = DevIoCtl(
            m_hWDM,
            IOCTL_AMB_WAIT_TETRIRQ,
            &tetr_irq,
            sizeof(AMB_TETR_IRQ),
            NULL,
            0,
            &length,
            NULL);
    if(res < 0){
        return GetLastError();
    }
    return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::MapDeviceMemory(AMB_CONFIGURATION& AmbConfiguration, int bar)
{
	unsigned page_size = sysconf(_SC_PAGESIZE);
	unsigned page_mask = ~(page_size - 1);

    //BRDC_printf(_BRDC("%s(): PAGE_SIZE = %x, \t PAGE_MASK = %x\n"), __FUNCTION__, page_size, page_mask);
    //BRDC_printf(_BRDC("%s(): BAR%d = 0x%lx, \t SIZE = 0x%x\n"), __FUNCTION__, bar, AmbConfiguration.PhysAddress[bar], AmbConfiguration.Size[bar]);

	unsigned area_size = BRD_ALIGN_UP(AmbConfiguration.Size[bar], page_size);

	// адреса BARx могут быть не выровнены на страницу и даже
	// расположены внутри одной страницы (на контроллере PLX)
	// такое происходит на относительно старых чипсетах где процессор не имеет IOMMU
	// или на каких-нибудь SoC.
	size_t Addr = (AmbConfiguration.PhysAddress[bar] & page_mask);
	size_t off = (AmbConfiguration.PhysAddress[bar] & ~page_mask);

    //BRDC_printf(_BRDC("%s(): Addr - 0x%lx\n"), __FUNCTION__, Addr);
    //BRDC_printf(_BRDC("%s(): off  - 0x%lx\n"), __FUNCTION__, off);
    //BRDC_printf(_BRDC("%s(): try to map %s - 0x%lx\n"), __FUNCTION__, off ? _BRDC("(not aligned)") : _BRDC("(aligned)"), Addr);

    int res = IPC_mapPhysAddr(m_hWDM, &AmbConfiguration.VirtAddress[bar], Addr, area_size);
	if (res != IPC_OK) {
        BRDC_printf(_BRDC("%s(): IPC_mapPhysAddr failed - %s\n"), __FUNCTION__, strerror(errno));
		return BRDerr_FATAL;
	}

	// add offset to virtual address
	AmbConfiguration.VirtAddress[bar] = ((U08*)AmbConfiguration.VirtAddress[bar] + off);

    //BRDC_printf(_BRDC("%s(): Mapped Address - %p\n"), __FUNCTION__, AmbConfiguration.VirtAddress[bar]);

	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::UnmapDeviceMemory(AMB_CONFIGURATION& AmbConfiguration, int bar)
{
	unsigned page_size = sysconf(_SC_PAGESIZE);
	unsigned page_mask = ~(page_size - 1);
	unsigned area_size = BRD_ALIGN_UP(AmbConfiguration.Size[bar], page_size);

	//size_t Addr = (AmbConfiguration.PhysAddress[bar] & page_mask);
	size_t off = (AmbConfiguration.PhysAddress[bar] & ~page_mask);

    //BRDC_printf(_BRDC("%s(): Addr - 0x%lx\n"), __FUNCTION__, Addr);
    //BRDC_printf(_BRDC("%s(): off  - 0x%lx\n"), __FUNCTION__, off);
    //BRDC_printf(_BRDC("%s(): try to unmap %s - 0x%lx\n"), __FUNCTION__, off ? _BRDC("(not aligned)") : _BRDC("(aligned)"), Addr);

	// remove added offset from virtual address
	AmbConfiguration.VirtAddress[bar] = ((U08*)AmbConfiguration.VirtAddress[bar] - off);

    int res = IPC_unmapPhysAddr(m_hWDM, AmbConfiguration.VirtAddress[bar], area_size);
	if (res != IPC_OK) {
        BRDC_printf(_BRDC("%s(): IPC_unmapPhysAddr failed - %s\n"), __FUNCTION__, strerror(errno));
		return BRDerr_FATAL;
	}

    //BRDC_printf(_BRDC("%s(): Unmap Address - %p\n"), __FUNCTION__, AmbConfiguration.VirtAddress[bar]);

	return BRDerr_OK;
}
#else
//-----------------------------------------------------------------------------

ULONG CBzynq::MapDeviceMemory(AMB_CONFIGURATION& AmbConfiguration, int bar)
{
	return BRDerr_OK;
}

//-----------------------------------------------------------------------------

ULONG CBzynq::UnmapDeviceMemory(AMB_CONFIGURATION& AmbConfiguration, int bar)
{
	return BRDerr_OK;
}

#endif
//***************** End of file bambpll.cpp *****************
