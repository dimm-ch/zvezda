//****************** File bambpexll.cpp *******************************
// BRD Driver for Data Acquisition Carrier Boards,
// based on Xilinx Core PCI-E Controller.
//
//	Copyright (c) 2007, Instrumental Systems,Corp.
//  Written by Dorokhin Andrey
//
//  History:
//  20-04-07 - builded
//
//*******************************************************************

#include "bambpex.h"
#include "rw_i2c.h"
#ifdef __linux__
#include <sys/time.h>
#endif

#define	CURRFILE _BRDC("BAMBPEXLL")

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

PULONG pMemBuf = NULL;

//-----------------------------------------------------------------

#ifdef __linux__
ULONG GetLastError(void)
{
	return errno;
}
#endif

//***************************************************************************************
//ULONG CBambpex::GetMemoryAddress(PULONG& MemoryAddress)
ULONG CBambpex::GetMemoryAddress()
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
		if(!m_pMemBuf[2])
		{
			S32 res = MapDeviceMemory(m_AmbConfiguration, 2);
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
			m_pMemBuf[2] = (PULONG)m_AmbConfiguration.VirtAddress[2];
			m_PhysMem[0] = (U32)m_AmbConfiguration.PhysAddress[0];
			m_PhysMem[1] = (U32)m_AmbConfiguration.PhysAddress[1];
			m_PhysMem[2] = (U32)m_AmbConfiguration.PhysAddress[2];
		}
		else
			status = BRDerr_ACCESS_VIOLATION;
	}
	return status;
}

typedef struct
{
	int size;
	int (*jtag_io)(int tms, int tdi, int read_tdo);
} JAM_IO;

//typedef int __cdecl JAM_ENTRY( char* filename, char* action, JAM_IO *io );
typedef int JAM_ENTRY( char* filename, const char* action, JAM_IO *io );

//***************************************************************************************
ULONG CBambpex::LoadPld(const BRDCHAR* PldFileName, UCHAR& PldStatus)
{
    return BRDerr_FUNC_UNIMPLEMENTED;
}

//***************************************************************************************
int  CBambpex::DevIoCtl(IPC_handle hDev,
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

//***************************************************************************************
ULONG CBambpex::SetPldStatus(ULONG PldStatus, ULONG PldNum)
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

//***************************************************************************************
ULONG CBambpex::GetPldStatus(ULONG& PldStatus, ULONG PldNum)
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

//***************************************************************************************
ULONG CBambpex::ReadMemDataDir(ULONG* pDataReg, ULONG* pStatusReg, ULONG& Value)
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

//***************************************************************************************
ULONG CBambpex::ReadMemData(ULONG* pCmdReg, ULONG* pDataReg, ULONG* pStatusReg, ULONG RegNumber, ULONG& Value)
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

//***************************************************************************************
ULONG CBambpex::WriteMemDataDir(ULONG* pDataReg, ULONG* pStatusReg, ULONG Value)
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

//***************************************************************************************
ULONG CBambpex::WriteMemData(ULONG* pCmdReg, ULONG* pDataReg, ULONG* pStatusReg, ULONG RegNumber, ULONG Value)
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

//***************************************************************************************
ULONG CBambpex::LoadBitPld(UCHAR& PldStatus, PVOID pBuffer, ULONG BufferSize)
{
#ifdef _WIN32
	LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
	int bHighRes = QueryPerformanceFrequency (&Frequency);
	QueryPerformanceCounter (&StartPerformCount);
#endif
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

#ifdef _WIN32
	QueryPerformanceCounter (&StopPerformCount);
	double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
	BRDC_printf(_BRDC("Load Time is %.2f ms\n"), msTime);
	//double mcsTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E6;
	//printf("Time is %.2f mcs\n", mcsTime);
#endif
	return BRDerr_OK;
}

//***************************************************************************************
// тетрада MAIN под сброс и все её регистры в ноль
ULONG CBambpex::GenReset()
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

//***************************************************************************************
// все тетрады, кроме MAIN под сброс и все их регистры в ноль
ULONG CBambpex::AllReset()
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

//***************************************************************************************
// отмена сброса всех тетрад
ULONG CBambpex::Unreset()
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

//***************************************************************************************
ULONG CBambpex::LoadPartPld(UCHAR PartPldNum, UCHAR& PldStatus, UCHAR* pBuffer, ULONG BufferSize)
{
#ifdef _WIN32
	LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
	int bHighRes = QueryPerformanceFrequency (&Frequency);
	QueryPerformanceCounter (&StartPerformCount);
#endif
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
#ifdef _WIN32
	QueryPerformanceCounter (&StopPerformCount);
	double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
	BRDC_printf(_BRDC("Load Time is %.2f ms\n"), msTime);
	//double mcsTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E6;
	//printf("Time is %.2f mcs\n", mcsTime);
#endif
	return BRDerr_OK;
}

static UCHAR EepromBase[512];

//***************************************************************************************
ULONG CBambpex::ReadNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	memset(pBuffer, 0xff, BufferSize);
	ULONG   length;     // the return length from the driver
	AMB_DATA_BUF NvRAM = {NULL, MAX_BASE_CFGMEM_SIZE, OFFSET_BASE_CFGMEM};

	//for(int i=0; i<128; i++) {
	//	EepromBase[OFFSET_BASE_CFGMEM+i] = i;
	//}
	//FreqHz = GetRegCpuSpeed(0);// / 1000000000;
	//__int64 time0 = GetCpuTicks();
	if (m_DeviceID == DSP134V_DEVID || 
		m_DeviceID == FMC143VKUS_DEVID)
	{ // на DSP134V (и на FMC143VKUSlave) нет ППЗУ для ICR, поэтому заполняем необходимые структуры некими значениями
		memset(EepromBase + Offset, 0, MAX_BASE_CFGMEM_SIZE);
		ICR_IdBase* info = (ICR_IdBase*)(EepromBase + Offset);
		info->wTag = 0x4953;
		info->wSize = sizeof(ICR_IdBase) - 4;
		info->wSizeAll = sizeof(ICR_IdBase) + sizeof(ICR_CfgAmbpex);
		info->dSerialNum = 1;
		if (m_DeviceID == FMC143VKUS_DEVID)
			info->dSerialNum = m_BusNum;
		info->wDeviceId = m_DeviceID;
		info->bVersion = 10;
		info->bDay = 31;
		info->bMon = 6;
		info->wYear = 2018;

		PICR_CfgAmbpex pBaseCfg = (PICR_CfgAmbpex)((PUCHAR)info + sizeof(ICR_IdBase));
		pBaseCfg->wTag = DSP134V_DEVID;
		if (m_DeviceID == FMC143VKUS_DEVID)
			pBaseCfg->wTag = FMC143VKUS_DEVID;
		pBaseCfg->wSize = sizeof(ICR_CfgAmbpex) - 4;
		pBaseCfg->bAdmIfCnt = 0;
		pBaseCfg->dSysGen = 66000000;
		pBaseCfg->bDdsType = 0;
		pBaseCfg->bSramCfgCnt = 0;

	}
	else
	{
		int res = DevIoCtl(
			m_hWDM,
			IOCTL_AMB_READ_NVRAM,
			&NvRAM,
			sizeof(AMB_DATA_BUF),
			EepromBase + OFFSET_BASE_CFGMEM,
			MAX_BASE_CFGMEM_SIZE,
			&length,
			NULL);
		//if(res < 0)
		//{
		//	memset(EepromBase + Offset, 0, MAX_BASE_CFGMEM_SIZE);
		//	ICR_IdBase* info = (ICR_IdBase*)(EepromBase + Offset);
		//	info->wTag = 0x4953;
		//	info->wSize = sizeof(ICR_IdBase);
		//	info->wDeviceId = m_DeviceID;
		//	info->dSerialNum = 59044;
		//	info->bVersion = 10;
		//	info->wSizeAll = sizeof(ICR_IdBase);
		//}
		if (res < 0) {
			return GetLastError();
		}
	}
	//__int64 time1 = GetCpuTicks();
    //__int64 wait_time = ULONG((time1 - time0) * 1000000 / FreqHz);

	//ReadMainSpd(1, (USHORT*) (EepromBase + OFFSET_BASE_CFGMEM), 64, 128);


	memcpy(pBuffer, EepromBase + Offset, BufferSize);
/*
    fprintf(stderr, "IOCTL_AMB_READ_NVRAM\n");
    u8 *ptr = (u8*)pBuffer;
    for(int i=0; i<32; i++) {
        fprintf(stderr, "0x%x ", ptr[i]);
    }
    fprintf(stderr, "\n");
*/
	return BRDerr_OK;
}

//***************************************************************************************
// исправление результатов старой (уже исправленной) ошибки драйвера при записи нечётного числа байт в ППЗУ
ULONG CBambpex::BaseIcrCorrectError()
{
	EepromBase[128 + m_sizeICR - 1] = 0;
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::GetNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	memcpy(pBuffer, EepromBase + Offset, BufferSize);
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::WriteNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	if(!BufferSize)
		return BRDerr_OK;

	memcpy(EepromBase + Offset, pBuffer, BufferSize);

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

//***************************************************************************************
ULONG CBambpex::ReadAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
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
    for(int i=0; i<32; i++) {
        fprintf(stderr, "0x%x ", ptr[i]);
    }
    fprintf(stderr, "\n");
*/
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::WriteAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	if(!BufferSize)
		return BRDerr_OK;
	memcpy(EepromSubmod + Offset, pBuffer, BufferSize);

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

//***************************************************************************************
ULONG CBambpex::ReadSubICR(int submod, PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG Status = BRDerr_OK;
	if(m_BlockFidAddr)
	{
		ULONG* BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
		memset(EepromSubmod, 0xaa, 256);
		ULONG signature = BlockMem[52] >> 16;
		if (0x12C0 == signature)
		{
			int busNum = submod ? 2 : 0; // FMC2 : FMC1
			U32 bitAck = getACK(BlockMem, busNum, 0xA0);

			if (!bitAck) // если бит ACK сброшен в 0, то ПЗУ откликнулось, следовательно это 16-битное ПЗУ
				Status = readI2C(BlockMem, busNum, 0xA0, EepromSubmod, 0x400, 256, !bitAck);
			else
				Status = readI2C(BlockMem, busNum, 0xA8, EepromSubmod, 0x400, 256, !bitAck);
		}
		else
			if (submod)
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

//***************************************************************************************
ULONG CBambpex::WriteSubICR(int submod, PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG Status = BRDerr_OK;
	if(m_BlockFidAddr)
	{
		ULONG* BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
		memcpy(EepromSubmod + Offset, pBuffer, BufferSize);
		ULONG signature = BlockMem[52] >> 16;
		if (0x12C0 == signature)
		{
			int busNum = submod ? 2 : 0; // FMC2 : FMC1
			U32 bitAck = getACK(BlockMem, busNum, 0xA0);

			if (!bitAck) // если бит ACK сброшен в 0, то ПЗУ откликнулось, следовательно это 16-битное ПЗУ
				Status = writeI2C(BlockMem, busNum, 0xA0, EepromSubmod, 0x400, 256, !bitAck);
			else
				Status = writeI2C(BlockMem, busNum, 0xA8, EepromSubmod, 0x400, 256, !bitAck);
		}
		else
			if (submod)
				Status = WriteFidSpd(2, 0x50, EepromSubmod, 0x400, 256); // FMC2
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

//***************************************************************************************
ULONG CBambpex::ReadFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
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

//***************************************************************************************
ULONG CBambpex::WriteFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
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
ULONG CBambpex::ReadAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
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
ULONG CBambpex::WriteAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
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
//***************************************************************************************
ULONG CBambpex::GetLocation(PAMB_LOCATION pAmbLocation)
{
    ULONG   length;     // the return length from the driver

	int res = DevIoCtl(
            m_hWDM,
            IOCTL_AMB_GET_LOCATION,
            NULL,
            0,
            pAmbLocation,
            sizeof(AMB_LOCATION),
            &length,
			NULL);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::GetConfiguration(PAMB_CONFIGURATION pAmbConfiguration)
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

//***************************************************************************************
//ULONG CBambpex::GetVersion(PTSTR pVerInfo)
ULONG CBambpex::GetVersion(char* pVerInfo)
{
//    ULONG   length;     // the return length from the driver
//	BRDCHAR Buf[MAX_STRING_LEN];
	char Buf[MAX_STRING_LEN];

//	LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
//	int bHighRes = QueryPerformanceFrequency (&Frequency);
//	QueryPerformanceCounter (&StartPerformCount);
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
//	QueryPerformanceCounter (&StopPerformCount);

//	double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
#ifdef _WIN32
    lstrcpyA(pVerInfo, Buf);
#else
    strcpy(pVerInfo, Buf);
#endif
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::GetDeviceID(USHORT& DeviceID)
{
    ULONG   length;     // the return length from the driver
	AMB_DATA_BUF PciConfigPtr =
//							{NULL, sizeof(USHORT), FIELD_OFFSET(PCI_COMMON_CONFIG, DeviceID)};
							{NULL, sizeof(USHORT), 2};

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
//	m_DeviceID = DeviceID;
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::GetRevisionID(UCHAR& RevisionID)
{
    ULONG   length;     // the return length from the driver
	AMB_DATA_BUF PciConfigPtr =
//							{NULL, sizeof(UCHAR), FIELD_OFFSET(PCI_COMMON_CONFIG, RevisionID)};
							{NULL, sizeof(UCHAR), 8};

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
//	m_RevisionID = RevisionID;
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::WriteRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
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

//***************************************************************************************
ULONG CBambpex::WriteRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
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

//***************************************************************************************
ULONG CBambpex::ReadRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
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

//***************************************************************************************
ULONG CBambpex::ReadRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal)
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

//***************************************************************************************
ULONG CBambpex::WriteRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
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

//***************************************************************************************
ULONG CBambpex::WriteRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
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

//***************************************************************************************
ULONG CBambpex::ReadRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
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

//***************************************************************************************
ULONG CBambpex::ReadRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
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

//***************************************************************************************
ULONG CBambpex::SetMemory(PVOID pParam, ULONG sizeParam)
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
			if(IPC_mapPhysAddr(m_devMem ? m_devMem : m_hWDM, &pMemDescr->pBlock[iBlk], (size_t)pMemDescr->pBlock[iBlk], pMemDescr->BlockSize))
				return BRDerr_ACCESS_VIOLATION;
		}
	}
    if(pMemDescr->pStub)
	{
		if(IPC_mapPhysAddr(m_devMem ? m_devMem : m_hWDM, &pMemDescr->pStub, (size_t)pMemDescr->pStub, sizeof(BRDstrm_Stub)))
			return BRDerr_ACCESS_VIOLATION;
    }

	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::FreeMemory(PVOID pParam, ULONG sizeParam)
{
	// нужно только для linux (под windows это функции-заглушки)
    AMB_MEM_DMA_CHANNEL *pMemDescr = (AMB_MEM_DMA_CHANNEL *)pParam;
    if(pMemDescr->MemType == 1	||	// для системной памяти
        pMemDescr->MemType == 8) {	// для физических адресов
        for(ULONG iBlk = 0; iBlk < pMemDescr->BlockCnt; iBlk++) {
            IPC_unmapPhysAddr(m_devMem ? m_devMem : m_hWDM, pMemDescr->pBlock[iBlk], pMemDescr->BlockSize);
        }
    }
    if(pMemDescr->pStub) {
        IPC_unmapPhysAddr(m_devMem ? m_devMem : m_hWDM, pMemDescr->pStub, sizeof(BRDstrm_Stub));
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

//***************************************************************************************
ULONG CBambpex::StartMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
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

//***************************************************************************************
ULONG CBambpex::StopMemory(PVOID pParam, ULONG sizeParam)
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

//***************************************************************************************
ULONG CBambpex::StateMemory(PVOID pParam, ULONG sizeParam)
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

//***************************************************************************************
ULONG CBambpex::SetDirection(PVOID pParam, ULONG sizeParam)
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

//***************************************************************************************
ULONG CBambpex::SetSource(PVOID pParam, ULONG sizeParam)
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

//***************************************************************************************
ULONG CBambpex::SetDmaRequest(PVOID pParam, ULONG sizeParam)
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

//***************************************************************************************
ULONG CBambpex::SetStatusIrq(ULONG AdmNum, ULONG TetrNum, ULONG IrqMask, ULONG IrqInv, HANDLE hEvent)
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

//***************************************************************************************
ULONG CBambpex::ClearStatusIrq(ULONG AdmNum, ULONG TetrNum, ULONG IrqMask, ULONG IrqInv, HANDLE hEvent)
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

//***************************************************************************************
ULONG CBambpex::ResetFifo(PVOID pParam, ULONG sizeParam)
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

//***************************************************************************************
ULONG CBambpex::Adjust(PVOID pParam, ULONG sizeParam)
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

//***************************************************************************************
ULONG CBambpex::Done(PVOID pParam, ULONG sizeParam)
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

//***************************************************************************************
ULONG CBambpex::GetDmaChanInfo(PVOID pParam, ULONG sizeParam)
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

//***************************************************************************************
ULONG CBambpex::SetIrqMode(PVOID pParam, ULONG sizeParam)
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
ULONG CBambpex::WaitDmaBlock(void* pParam, ULONG sizeParam)
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
ULONG CBambpex::WaitDmaBuffer(void* pParam, ULONG sizeParam)
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

//***************************************************************************************
ULONG CBambpex::WaitStatusIrq(ULONG AdmNum, ULONG TetrNum, ULONG IrqMask, ULONG IrqInv, HANDLE hEvent)
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

//***************************************************************************************
ULONG CBambpex::MapDeviceMemory(AMB_CONFIGURATION& AmbConfiguration, int bar)
{
	if(!AmbConfiguration.Size[bar])
		return BRDerr_OK;

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

	int res = IPC_mapPhysAddr(m_devMem ? m_devMem : m_hWDM, &AmbConfiguration.VirtAddress[bar], Addr, area_size);
	if (res != IPC_OK) {
        BRDC_printf(_BRDC("%s(): IPC_mapPhysAddr failed - %s\n"), __FUNCTION__, strerror(errno));
		return BRDerr_FATAL;
	}

	// add offset to virtual address
	AmbConfiguration.VirtAddress[bar] = ((U08*)AmbConfiguration.VirtAddress[bar] + off);

    //BRDC_printf(_BRDC("%s(): Mapped Address - %p\n"), __FUNCTION__, AmbConfiguration.VirtAddress[bar]);

	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::UnmapDeviceMemory(AMB_CONFIGURATION& AmbConfiguration, int bar)
{
	if (!AmbConfiguration.Size[bar])
		return BRDerr_OK;

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

	int res = IPC_unmapPhysAddr(m_devMem ? m_devMem : m_hWDM, AmbConfiguration.VirtAddress[bar], area_size);
	if (res != IPC_OK) {
        BRDC_printf(_BRDC("%s(): IPC_unmapPhysAddr failed - %s\n"), __FUNCTION__, strerror(errno));
		return BRDerr_FATAL;
	}

    //BRDC_printf(_BRDC("%s(): Unmap Address - %p\n"), __FUNCTION__, AmbConfiguration.VirtAddress[bar]);

	return BRDerr_OK;
}
#else
//***************************************************************************************
ULONG CBambpex::MapDeviceMemory(AMB_CONFIGURATION& AmbConfiguration, int bar)
{
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::UnmapDeviceMemory(AMB_CONFIGURATION& AmbConfiguration, int bar)
{
	return BRDerr_OK;
}

#endif
//***************** End of file bambpll.cpp *****************
