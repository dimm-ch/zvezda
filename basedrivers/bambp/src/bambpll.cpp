//****************** File bambpll.cpp *******************************
// BRD Driver for Data Acquisition Carrier Boards,
// based on PLX905x PCI Controller.
//
//	Copyright (c) 2004, Instrumental Systems,Corp.
//  Written by Dorokhin Andrey
//
//  History:
//  05-10-04 - builded
//
//*******************************************************************

#include "bambpci.h"
#ifdef __linux__
#include <sys/time.h>
#endif

#define	CURRFILE _BRDC("BAMBPLL")

//#include "jamdll.h"
//
//#define BIT_TCK 0x1
//#define BIT_TMS 0x2
//#define BIT_TDI 0x40
//#define BIT_TDO 0x80
//
//PULONG pMemBuf = NULL;
//
////***************************************************************************************
//// Чтение косвенных регистров через адрес памяти
//ULONG ReadIndMem(ULONG adr)
//{
//	pMemBuf[ADM2IFnr_CMDADR] = adr;
//	while(1)
//	{
//		ULONG Status = pMemBuf[ADM2IFnr_STATUS];
//		if(Status & 0x1)
//			break;
//		Sleep(100);
//	}
//	return pMemBuf[ADM2IFnr_CMDDATA];
//}
//
//void WriteIndMem(ULONG adr,ULONG data)
//{
//	pMemBuf[ADM2IFnr_CMDADR] = adr;
//	while(1)
//	{
//		ULONG Status = pMemBuf[ADM2IFnr_STATUS];
//		if(Status & 0x1)
//			break;
//		Sleep(100);
//	}
//	pMemBuf[ADM2IFnr_CMDDATA] = data;
//}
//
////***************************************************************************************
//// Callback функция для процедуры загрузки ПЛИС (вызывается из jamdll.dll)
////***************************************************************************************
//int jtag_hardware_io(int tms, int tdi, int read_tdo)
//{
//	ULONG data = 0;
//	ULONG tdo = 0;
//
//	BOOL alternative_cable_l = FALSE;
//	BOOL alternative_cable_x = FALSE;
//
//	data = (alternative_cable_l ? ((tdi ? 0x01 : 0) | (tms ? 0x04 : 0)) :
//		       (alternative_cable_x ? ((tdi ? 0x01 : 0) | (tms ? 0x04 : 0) | 0x10) :
//		       ((tdi ? 0x40 : 0) | (tms ? 0x02 : 0))));
//
//	WriteIndMem(MAINnr_JTAGOUTDATA1, data);
//
//	if (read_tdo)
//	{
//		tdo = ReadIndMem(MAINnr_JTAGINDATA1);
//		tdo = (alternative_cable_l ? ((tdo & 0x40) ? 1 : 0) :
//		      (alternative_cable_x ? ((tdo & 0x10) ? 1 : 0) :
//		      ((tdo & 0x80) ? 0 : 1)));
//	}
//
//	WriteIndMem(MAINnr_JTAGOUTDATA1, data | (alternative_cable_l ? 0x02 : (alternative_cable_x ? 0x02: 0x01)) );
//	WriteIndMem(MAINnr_JTAGOUTDATA1, data );
//
//	return (tdo);
//}
//
////***************************************************************************************
//ULONG CBambpci::GetMemoryAddress()
//{
//	AMB_CONFIGURATION AmbConfiguration;
//	ULONG status = GetConfiguration(&AmbConfiguration);
//	if(!status)
//	{
//		pMemBuf = (PULONG)AmbConfiguration.VirtAddress[1];
//
//		// for debuging
//		ULONG tetr_id = ReadIndMem(ADM2IFnr_ID);
//		ULONG sig = ReadIndMem(MAINnr_SIG);
//		ULONG base_id = ReadIndMem(MAINnr_BASEID);
//		ULONG base_ver = ReadIndMem(MAINnr_BASEVER);
//		ULONG smod_id = ReadIndMem(MAINnr_SMODID);
//		ULONG smod_ver = ReadIndMem(MAINnr_SMODVER);
//
//	}
//	return status;
//}
//
////***************************************************************************************
//ULONG CBambpci::LoadJamPld(PCTSTR PldFileName) 
//{
//	LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
//	int bHighRes = QueryPerformanceFrequency (&Frequency);
//	QueryPerformanceCounter (&StartPerformCount);
//	
//	JAM_IO io;
//	io.size = sizeof( JAM_IO );
//	io.jtag_io = jtag_hardware_io;
//
//	ULONG status = jam_play((char*)PldFileName, "RUN_XILINX_PROC", &io );
//	if(status != BRDerr_OK)
//		status = BRDerr_HW_ERROR;
//
//	QueryPerformanceCounter (&StopPerformCount);
//	double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
//
//	return status;
//	//return BRDerr_OK;
//}
//

#ifdef __linux__
ULONG GetLastError(void)
{
    //printf("GetLastError(): %s\n", strerror(IPC_sysError()));
    return errno;
}
#endif
//***************************************************************************************
//ULONG CBambpci::OpenDevice(int DevNum)
//{
//	lstrcpy(m_DeviceName, "\\\\.\\");
//	lstrcat(m_DeviceName, AmbDeviceName);
//	TCHAR cNumber[4];
//	sprintf(cNumber, "%d", DevNum);
//	lstrcat(m_DeviceName, cNumber);
//	m_hWDM = CreateFile(m_DeviceName,
//						GENERIC_READ | GENERIC_WRITE,
//						FILE_SHARE_READ | FILE_SHARE_WRITE,
//						NULL, 
//						OPEN_EXISTING, 
//						FILE_FLAG_OVERLAPPED, // 0,
//						NULL);
//	if(m_hWDM == INVALID_HANDLE_VALUE)
//		return BRDerr_NO_KERNEL_SUPPORT;
//	return BRDerr_OK;
//}

//***************************************************************************************
void CBambpci::CloseDevice()
{
    IPC_closeDevice(m_hWDM);
    m_hWDM = 0;
}

#define	MAINnr_JTAGCTRL	0x204
#define	MAINnr_JTAGSTAT 0x204
#define	MAINnr_JTAGDATA	0x205

//***************************************************************************************
ULONG CBambpci::LoadAcePld(UCHAR& PldStatus, PVOID pBuffer, ULONG BufferSize, ULONG PldNum)
{
#ifndef __linux__
	U32 val = 0;
	PMAIN_MRES pMres;
	ReadRegData(0, ADM2IFnt_MAIN, MAINnr_MRES, val);
	pMres = (PMAIN_MRES)&val;
	if(!pMres->ByBits.IsAcePlay)
	{
		PldStatus = PLD_errNOTRDY;
		printf("ACE Player is NOT supported\n");
		return BRDerr_OK;
	}
	printf("LoadAcePld\r");

	LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
	int bHighRes = QueryPerformanceFrequency (&Frequency);
	QueryPerformanceCounter (&StartPerformCount);

	val = 3;
	WriteRegData(0, ADM2IFnt_MAIN, MAINnr_JTAGCTRL, val);
	//ULONG sleep_cnt = 0;
	ULONG cnt = BufferSize / 2; // пишем 2-байтовыми словами
	USHORT* pData = (USHORT*)pBuffer;
	//ULONG index = 0;
	ULONG prev_perc = 0;
	for(ULONG i = 0; i < cnt; i++)
	{
		while(1)
		//for(ULONG j = 0; j < 60000; j++)
		{
			ReadRegData(0, ADM2IFnt_MAIN, MAINnr_JTAGSTAT, val);
			if(val & 0x40)
			{
				if(val & 0x20)
					PldStatus = PLD_errFORMAT;
				else
					PldStatus = PLD_errNOTRDY;
				val = 0;
				WriteRegData(0, ADM2IFnt_MAIN, MAINnr_JTAGCTRL, val);
				return BRDerr_OK;
			}
			if(val & 0x1)
				break;
			//Sleep(1);
			//sleep_cnt++;
		}
		val = pData[i];
		WriteRegData(0, ADM2IFnt_MAIN, MAINnr_JTAGDATA, val);
		ULONG percent = (i * 100 / cnt);
		if(percent != prev_perc)
		{
			printf("LoadAcePld - %d%%     \r", percent);
			prev_perc = percent;
		}
		//if(i < 500)
		//	printf("LoadAcePld - %d\r", i);
		//else
			//if(i > index)
			//{
			//	index += 1000;
			//	printf("LoadAcePld - %d%\r", i);
			//}
	}
	PldStatus = PLD_errNOTRDY;
	ULONG k = 0;
	for(k = 0; k < 5000; k++)
	{
		ReadRegData(0, ADM2IFnt_MAIN, MAINnr_JTAGSTAT, val);
		if(val & 0x20)
		{
			PldStatus = PLD_errOK;
			break;
		}
        IPC_delay(10);
	}
	val = 0;
	WriteRegData(0, ADM2IFnt_MAIN, MAINnr_JTAGCTRL, val);

	QueryPerformanceCounter (&StopPerformCount);
	double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
	printf("Time is %.2f ms\n", msTime);
	//double mcsTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E6;
	//printf("Time is %.2f mcs\n", mcsTime);
	return BRDerr_OK;
#else
    fprintf(stderr, "%s(): Function not implemented\n", __FUNCTION__);
    return BRDerr_ERROR;
#endif
}

//***************************************************************************************
int  CBambpci::DevIoCtl(IPC_handle hDev,
						 DWORD code, 
						 LPVOID InBuf, 
						 DWORD InBufSize, 
						 LPVOID OutBuf, 
						 DWORD OutBufSize, 
						 DWORD *pRet, 
						 LPOVERLAPPED lpOverlapped)
{
	//if(m_hMutex) 
	//	//WaitForSingleObject(m_hMutex, INFINITE);
 //       IPC_captureMutex(m_hMutex, INFINITE);

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

	//if(m_hMutex)
	//	//ReleaseMutex(m_hMutex);
 //       IPC_releaseMutex(m_hMutex);

	return ret;
}

//***************************************************************************************
ULONG CBambpci::LoadPldRom(UCHAR& PldStatus)
{
	ULONG   length;     // the return length from the driver
	UCHAR Status = 0;

    int res = DevIoCtl(
			m_hWDM,
			IOCTL_AMB_LOAD_PLD_EEPROM,
			NULL,
			0,
			&Status,
			sizeof(UCHAR),
			&length,
			NULL);
    if(res < 0){
		return GetLastError();
	}
	PldStatus = Status;

	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::LoadPld(UCHAR& PldStatus, PVOID pBuffer, ULONG BufferSize, ULONG PldNum)
{
	ULONG   length;     // the return length from the driver
	AMB_DATA_BUF PldData = {pBuffer, BufferSize, PldNum};
	UCHAR Status = 0;

    int res = DevIoCtl(
			m_hWDM,
			IOCTL_AMB_LOAD_PLD,
			&PldData,
			sizeof(AMB_DATA_BUF),
			&Status,
			sizeof(UCHAR),
			&length,
			NULL);
    if(res < 0){
		return GetLastError();
	}
	PldStatus = Status;

	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::GetPldStatus(ULONG& PldStatus, ULONG PldNum)
{
	ULONG length;     // the return length from the driver
	ULONG num = PldNum;
	ULONG Status;

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
	if(Status == PLD_errOK)
		PldStatus = 1;
	else
		PldStatus = 0;
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::ReadNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG   length;     // the return length from the driver
    AMB_DATA_BUF NvRAM = {pBuffer, BufferSize, Offset};

    int res = DevIoCtl(
			m_hWDM,
			IOCTL_AMB_READ_NVRAM,
			&NvRAM,
			sizeof(AMB_DATA_BUF),
			pBuffer,
			BufferSize,
			&length,
			NULL);
    if(res < 0){
		return GetLastError();
	}
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::WriteNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG   length;     // the return length from the driver
	if(BufferSize > (512 - 128)) // max size EEPROM = 512 byte, first 128 byte - PCI Configuration Space
		BufferSize = 512 - 128;
	AMB_DATA_BUF NvRAM = {pBuffer, BufferSize, Offset};

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
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::ReadAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	if(/*m_DeviceID != AMBPEX1_DEVID &&*/
		m_DeviceID != SYNCDAC_DEVID &&
		m_DeviceID != SYNCBCO_DEVID)
	{
		ULONG   length;     // the return length from the driver
        AMB_DATA_BUF admIdROM = {pBuffer, BufferSize, Offset};

    int res = DevIoCtl(
				m_hWDM,
				IOCTL_AMB_READ_ADMIDROM,
				&admIdROM,
				sizeof(AMB_DATA_BUF),
				pBuffer,
				BufferSize,
				&length,
				NULL);
    if(res < 0){
			return GetLastError();
		}
	}
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::WriteAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
//	if(m_DeviceID != AMBPEX1_DEVID)
	{
		ULONG   length;     // the return length from the driver
		AMB_DATA_BUF admIdROM = {pBuffer, BufferSize, Offset};

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
	}
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::GetLocation(PAMB_LOCATION pAmbLocation)
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
ULONG CBambpci::GetConfiguration(PAMB_CONFIGURATION pAmbConfiguration)
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
//ULONG CBambpci::GetVersion(PTSTR pVerInfo)
ULONG CBambpci::GetVersion(char* pVerInfo)
{
//    ULONG   length;     // the return length from the driver
//	TCHAR Buf[MAX_STRING_LEN];
	char Buf[MAX_STRING_LEN];

	//LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
	//int bHighRes = QueryPerformanceFrequency (&Frequency);
	//QueryPerformanceCounter (&StartPerformCount);

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
	//QueryPerformanceCounter (&StopPerformCount);
	//double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
	//double mcsTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E6;
	//printf("Time is %.2f mcs\n", mcsTime);
#ifdef _WIN32
    lstrcpyA(pVerInfo, Buf);
#else
    strcpy(pVerInfo, Buf);
#endif
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::GetDeviceID(USHORT& DeviceID)
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
ULONG CBambpci::GetRevisionID(UCHAR& RevisionID)
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
/*
// ***************************************************************************************
ULONG CBplx::AllocMemForMainRegs()
{
    ULONG   length;     // the return length from the driver
	AMB_DATA_BUF data = {NULL, sizeof(HOST_REGISTERS), 0};

    if (!DeviceIoControl(
            m_hWDM,
			IOCTL_AMB_ALLOC_SHARED_MEM,
            &data,
			sizeof(AMB_DATA_BUF),
			&data,
            sizeof(AMB_DATA_BUF),
            &length,
            NULL)) {
        return GetLastError();
    }
	m_pMainRegs = (PHOST_REGISTERS)data.pBuffer;
	return BRDerr_OK;
}

// ***************************************************************************************
ULONG CBplx::FreeMemForMainRegs()
{
    ULONG   length;     // the return length from the driver
	AMB_DATA_BUF data = {(PVOID)m_pMainRegs, sizeof(HOST_REGISTERS), 0};

    if (!DeviceIoControl(
            m_hWDM,
			IOCTL_AMB_FREE_SHARED_MEM,
            &data,
			sizeof(AMB_DATA_BUF),
			&data,
            sizeof(AMB_DATA_BUF),
            &length,
            NULL)) {
        return GetLastError();
    }
	return BRDerr_OK;
}
*/
//***************************************************************************************
ULONG CBambpci::WriteRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal) 
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
ULONG CBambpci::WriteRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal) 
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
ULONG CBambpci::ReadRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal) 
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
ULONG CBambpci::ReadRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal) 
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
ULONG CBambpci::WriteRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
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
ULONG CBambpci::WriteRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
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
ULONG CBambpci::ReadRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
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
ULONG CBambpci::ReadRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize)
{
    ULONG   length;  

	AMB_BUF_REG reg_buf = { AdmNum, TetrNum, RegNum, RegBuf, RegBufSize };

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
ULONG CBambpci::SetMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
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
	AMB_MEM_DMA_CHANNEL *pMemDescr = (AMB_MEM_DMA_CHANNEL *)pParam;
    for(unsigned iBlk = 0; iBlk < pMemDescr->BlockCnt; iBlk++)
    {
                if(IPC_mapPhysAddr(m_hWDM, &pMemDescr->pBlock[iBlk], (size_t)pMemDescr->pBlock[iBlk], pMemDescr->BlockSize))
           return BRDerr_ACCESS_VIOLATION;
    }
    if(pMemDescr->pStub)
	{
                if(IPC_mapPhysAddr(m_hWDM, &pMemDescr->pStub, (size_t)pMemDescr->pStub, sizeof(BRDstrm_Stub)))
           return BRDerr_ACCESS_VIOLATION;
    }

	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::FreeMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	// нужно только для linux (под windows это функции-заглушки)
    AMB_MEM_DMA_CHANNEL *pMemDescr = (AMB_MEM_DMA_CHANNEL *)pParam;
    for(ULONG iBlk = 0; iBlk < pMemDescr->BlockCnt; iBlk++)
		IPC_unmapPhysAddr(m_hWDM, pMemDescr->pBlock[iBlk], pMemDescr->BlockSize);
    if(pMemDescr->pStub)
		IPC_unmapPhysAddr(m_hWDM, pMemDescr->pStub, sizeof(BRDstrm_Stub));

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
ULONG CBambpci::StartMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
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
ULONG CBambpci::StopMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
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
ULONG CBambpci::StateMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
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
ULONG CBambpci::SetDirection(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
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
ULONG CBambpci::SetSource(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
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
ULONG CBambpci::SetDmaRequest(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
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
ULONG CBambpci::SetStatusIrq(ULONG AdmNum, ULONG TetrNum, ULONG IrqMask, ULONG IrqInv, HANDLE hEvent)
{
#ifdef _WIN32
    ULONG   length;  
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
#else
    fprintf(stderr, "%s, %d: - %s not implemented\n", __FILE__, __LINE__, __FUNCTION__);
#endif
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::ClearStatusIrq(ULONG AdmNum, ULONG TetrNum, ULONG IrqMask, ULONG IrqInv, HANDLE hEvent)
{
#ifdef _WIN32
    ULONG   length;  
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
#else
    fprintf(stderr, "%s, %d: - %s not implemented\n", __FILE__, __LINE__, __FUNCTION__);
#endif
	return BRDerr_OK;
}

#ifdef __linux__
// ***************************************************************************************
ULONG CBambpci::WaitDmaBlock(void* pParam, ULONG sizeParam)
{
    int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_WAIT_DMA_BLOCK,
            pParam,
            sizeParam,
            pParam,
            sizeParam);
    if(res < 0){
        return GetLastError();
    }
    return BRDerr_OK;
}

// ***************************************************************************************
ULONG CBambpci::WaitDmaBuffer(void* pParam, ULONG sizeParam)
{
    int res = DevIoCtl(
            m_hWDM, 
            IOCTL_AMB_WAIT_DMA_BUFFER,
            pParam,
            sizeParam,
            pParam,
            sizeParam);
    if(res < 0){
        return GetLastError();
    }
    return BRDerr_OK;
}
#endif
//***************** End of file bambpll.cpp *****************
