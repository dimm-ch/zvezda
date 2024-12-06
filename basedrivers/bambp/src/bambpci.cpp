//****************** File bambpci.cpp *******************************
// BRD Driver for Data Acquisition Carrier Boards,
// based on PLX9x56 PCI Controller.
//
//	Copyright (c) 2004, Instrumental Systems,Corp.
//  Written by Dorokhin Andrey
//
//  History:
//  05-10-04 - builded
//
//*******************************************************************

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

//#pragma warning(disable: 4786)

//#include "gipcy.h"
#define	DEVS_API_EXPORTS
#include "bambpci.h"

#define	CURRFILE _BRDC("BAMBP")

//******************************************
CBambpci::CBambpci() :
	CBaseModule(MAX_BASE_CFGMEM_SIZE)
{
	for(int i = 0; i < MAX_SYSMON_SRV; i++)
		m_pSysMonSrv[i] = NULL;
	for(int i = 0; i < MAX_TEST_SRV; i++)
		m_pTestSrv[i] = NULL;
	for(int i = 0; i < MAX_MEMORY_SRV; i++)
		m_pMemorySrv[i] = NULL;
	for(int i = 0; i < MAX_STREAM_SRV; i++)
		m_pStreamSrv[i] = NULL;
	m_pIniData = NULL;
	BRDC_strcpy(m_pDlgDllName, _BRDC("BambpDlg.dll"));

	m_pSDRAM = 0;
	m_hWDM = 0;
}

//********************** FindDlgDLLName *****************
// PTSTR* pLibName - указатель на название библиотеки (OUT)
// PBRD_InitData pInitData - указатель на массив данных инициализации вида "ключ-значение" (IN)
// long initDataSize - число оставшихся элементов массива pInitData (IN)
//*******************************************************
static void FindDlgDLLName(BRDCHAR **pLibName, PBRD_InitData pInitData, long initDataSize, const BRDCHAR* pKeyword)
{
//	pLibName = NULL;
	// Search SubSections
	for(long i = 0; i < initDataSize; i++)
	{
		if(!BRDC_stricmp(pInitData[i].key, pKeyword))	// Keyword
		{
			*pLibName = pInitData[i].val;
			break;
		}
	}
}

//******************************************
// BASE_INI* pIniData - данные из ИИ (PID, BusNumber, SlotNumber) при полуавтоматической инициализации и
//						NULL при автоматической инициализации
//******************************************
CBambpci::CBambpci(PBRD_InitData pBrdInitData, long sizeInitData) :
	CBaseModule(MAX_BASE_CFGMEM_SIZE)
{
	for(int i = 0; i < MAX_SYSMON_SRV; i++)
		m_pSysMonSrv[i] = NULL;
	for(int i = 0; i < MAX_TEST_SRV; i++)
		m_pTestSrv[i] = NULL;
	for(int i = 0; i < MAX_MEMORY_SRV; i++)
		m_pMemorySrv[i] = NULL;
	for(int i = 0; i < MAX_STREAM_SRV; i++)
		m_pStreamSrv[i] = NULL;

	m_pIniData = new BASE_INI;
	//m_pIniData = (PBASE_INI)malloc(sizeof(BASE_INI));
	long size = sizeInitData;

	PINIT_Data pInitData = (PINIT_Data)pBrdInitData;
		// идентификация
	FindKeyWord(_BRDC("pid"), m_pIniData->PID, pInitData, size);
	FindKeyWord(_BRDC("pcibus"), m_pIniData->BusNumber, pInitData, size);
	FindKeyWord(_BRDC("pcidev"), m_pIniData->DevNumber, pInitData, size);
	FindKeyWord(_BRDC("pcislot"), m_pIniData->SlotNumber, pInitData, size);

		// конфигурация
//	FindKeyWord("dlgtype", m_pIniData->DlgDllName, pInitData, size);
	BRDCHAR *pDlgLibName = NULL;
	FindDlgDLLName(&pDlgLibName, pBrdInitData, size, _BRDC("bambpdlgdll"));
	if(pDlgLibName)
		BRDC_strcpy(m_pDlgDllName, pDlgLibName);
	else
		BRDC_strcpy(m_pDlgDllName, _BRDC("BambpDlg.dll"));

	pDlgLibName = NULL;
	FindDlgDLLName(&pDlgLibName, pBrdInitData, size, _BRDC("sdramdlgdll"));
//	FindDlgDLLName(&pDlgLibName, pBrdInitData, size, _BRDC("SdramAmbpcdSrvDlg"));
	if(pDlgLibName)
	{
		BRDC_strcpy(m_pIniData->MemIni[0].DlgDllName, pDlgLibName);
		BRDC_strcpy(m_pIniData->MemIni[1].DlgDllName, pDlgLibName);
	}
	else
	{
		BRDC_strcpy(m_pIniData->MemIni[0].DlgDllName, _BRDC(""));
		BRDC_strcpy(m_pIniData->MemIni[1].DlgDllName, _BRDC(""));
	}

	//PTSTR pFotrJamPldFileName = NULL;
	//FindDlgDLLName(&pFotrJamPldFileName, pBrdInitData, size, _BRDC("FotrJamPldFile"));
	//if(pFotrJamPldFileName)
	//	_tcscpy_s(m_FotrJamPldFileName, pFotrJamPldFileName);
	//else
	//	_tcscpy_s(m_FotrJamPldFileName, "admfotr3g_v10.stapl");

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

	FindKeyWord(_BRDC("dsppldtype0"), m_pIniData->DspIni[0].PldType, pInitData, size);
	FindKeyWord(_BRDC("dsppldvolume0"), m_pIniData->DspIni[0].PldVolume, pInitData, size);
	FindKeyWord(_BRDC("dsppldpins0"), m_pIniData->DspIni[0].PldPins, pInitData, size);
	FindKeyWord(_BRDC("dsppldrate0"), m_pIniData->DspIni[0].PldSpeedGrade, pInitData, size);
	FindKeyWord(_BRDC("dsploadrom0"), m_pIniData->DspIni[0].LoadRom, pInitData, size);
	FindKeyWord(_BRDC("dsppiotype0"), m_pIniData->DspIni[0].PioType, pInitData, size);
	FindKeyWord(_BRDC("dspsramchipcnt0"), m_pIniData->DspIni[0].SramChipCnt, pInitData, size);
	FindKeyWord(_BRDC("dspsramchipdepth0"), m_pIniData->DspIni[0].SramChipDepth, pInitData, size);
	FindKeyWord(_BRDC("dspsramchipbitswidth0"), m_pIniData->DspIni[0].SramChipBitsWidth, pInitData, size);

	FindKeyWord(_BRDC("slotcnt1"), m_pIniData->MemIni[1].SlotCnt, pInitData, size);
	FindKeyWord(_BRDC("modulecnt1"), m_pIniData->MemIni[1].ModuleCnt, pInitData, size);
	FindKeyWord(_BRDC("modulebanks1"), m_pIniData->MemIni[1].ModuleBanks, pInitData, size);
	FindKeyWord(_BRDC("rowaddrbits1"), m_pIniData->MemIni[1].RowAddrBits, pInitData, size);
	FindKeyWord(_BRDC("columnaddrbits1"), m_pIniData->MemIni[1].ColAddrBits, pInitData, size);
	FindKeyWord(_BRDC("chipbanks1"), m_pIniData->MemIni[1].ChipBanks, pInitData, size);
	FindKeyWord(_BRDC("caslatency1"), m_pIniData->MemIni[1].CasLatency, pInitData, size);
	FindKeyWord(_BRDC("primwidth1"), m_pIniData->MemIni[1].PrimWidth, pInitData, size);
	FindKeyWord(_BRDC("attibutes1"), m_pIniData->MemIni[1].Attributes, pInitData, size);

	FindKeyWord(_BRDC("dsppldtype1"), m_pIniData->DspIni[1].PldType, pInitData, size);
	FindKeyWord(_BRDC("dsppldvolume1"), m_pIniData->DspIni[1].PldVolume, pInitData, size);
	FindKeyWord(_BRDC("dsppldpins1"), m_pIniData->DspIni[1].PldPins, pInitData, size);
	FindKeyWord(_BRDC("dsppldrate1"), m_pIniData->DspIni[1].PldSpeedGrade, pInitData, size);
	FindKeyWord(_BRDC("dsploadrom1"), m_pIniData->DspIni[1].LoadRom, pInitData, size);
	FindKeyWord(_BRDC("dsppiotype1"), m_pIniData->DspIni[1].PioType, pInitData, size);
	FindKeyWord(_BRDC("dspsramchipcnt1"), m_pIniData->DspIni[1].SramChipCnt, pInitData, size);
	FindKeyWord(_BRDC("dspsramchipdepth1"), m_pIniData->DspIni[1].SramChipDepth, pInitData, size);
	FindKeyWord(_BRDC("dspsramchipbitswidth1"), m_pIniData->DspIni[1].SramChipBitsWidth, pInitData, size);

	m_pSDRAM = 0;
	m_hWDM = 0;
}

//******************************************
CBambpci::~CBambpci()
{
	if(m_pIniData)
		delete m_pIniData;
		//free(m_pIniData);
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

//	_CrtDumpMemoryLeaks();

}

static U16 flagPID = 0;

//******************** Init **********************
// Эта функция выполняет инициализацию (автоматическую и полуавтоматическую)
// U16 CurDevNum - начальный номер экземпляра модуля при поиске нужного базового модуля
// PTSTR pBoardName - сюда записывается оригинальное имя базового модуля
// PTSTR pIniString - NULL при полуавтоматической инициализации и
//						указатель на строку с инициализационными данными при автоматической инициализации
//************************************************
S32 CBambpci::Init(short CurDevNum, BRDCHAR* pBoardName, BRDCHAR* pIniString)
{
//	int const nBufSize = 128;
//	TCHAR ErrVitalText[nBufSize];
//	int ret = GetStringText(IDS_STRINGVITAL, ErrVitalText);
//	int ret = GetStringText(MSG_NON_VITAL_DATA, ErrVitalText);

//	WriteEventLog(MSG_NON_VITAL_DATA, ErrVitalText);
//	MessageBox(NULL, "<Init> Enter", "BAMBP", MB_OK);
	if(m_pIniData)
	{
        if(m_pIniData->PID == (U32)-1 && (m_pIniData->BusNumber == (U32)-1 || m_pIniData->DevNumber == (U32)-1) && m_pIniData->SlotNumber == (U32)-1)
//			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
			return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
							  CURRFILE, //ErrVitalText);
							  _BRDC("<Init> Undefined vital data into init source -1"));

		if(!CurDevNum && m_pIniData->PID)
			flagPID = 1;

		if(flagPID == 1 && m_pIniData->PID == 0)
//			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
			return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
							  CURRFILE, //ErrVitalText);
							  _BRDC("<Init> Undefined vital data into init source 0"));

		if(m_pIniData->PID != 0)
			CurDevNum = 0;
	}
	AMB_LOCATION AmbLocation;
	S32	errorCode = BRDerr_OK;

	//=== Find PCI Card
//	for(int DeviceNumber = CurDevNum; ; DeviceNumber++)
	int DeviceNumber = CurDevNum;
	do {
		IPC_str deviceName[256] = _BRDC("");
		m_hWDM = IPC_openDevice(deviceName, AmbDeviceName, DeviceNumber);
		if(!m_hWDM) {
//		if(OpenDevice(DeviceNumber) != BRDerr_OK)
//		{
			if(!pIniString)
				ErrorPrintf(DRVERR(BRDerr_WARN),
								  CURRFILE,
								  _BRDC("<Init> No device driver for %s"), deviceName);
				return DRVERR(BRDerr_NO_KERNEL_SUPPORT);
		}

//		MessageBox(NULL, "<Init> CreateFile", "BAMBP", MB_OK);
		//TCHAR buf[MAX_STRING_LEN];
		char buf[MAX_STRING_LEN];
		GetVersion(buf);
//		ULONG pldStatus;
//		ReadRegData(0, 4, pldStatus);
		// Get DeviceID, RevisionID, PCI Bus, PCI Device, PCI Slot, PID
		if(GetDeviceID(m_DeviceID) != BRDerr_OK)
		{
			IPC_closeDevice(m_hWDM);
			//CloseHandle(m_hWDM);
//			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
			return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
							  CURRFILE, //ErrVitalText);
							  _BRDC("<Init> Undefined vital data (RevisionID) into init source"));
		}
		if(m_DeviceID != AMBPCX_DEVID &&
			m_DeviceID != AMBPCD_DEVID &&
			m_DeviceID != AMBPEX1_DEVID &&
			m_DeviceID != SYNCDAC_DEVID &&
			m_DeviceID != SYNCBCO_DEVID &&
			m_DeviceID != VK3_DEVID &&
			m_DeviceID != AMBPEX2_DEVID &&
			m_DeviceID != SYNCCP6_DEVID &&
			m_DeviceID != SYNCCP6R_DEVID &&
			m_DeviceID != DR16_DEVID &&
			m_DeviceID != RFDR4_DEVID &&
			m_DeviceID != C520F1_DEVID &&
			m_DeviceID != GRADC_DEVID)
		{
			IPC_closeDevice(m_hWDM);
			//CloseHandle(m_hWDM);	// Wrong Board
			if(pIniString)
				return DRVERR(BRDerr_NO_KERNEL_SUPPORT);
			DeviceNumber++;
			continue;
		}
		if(GetRevisionID(m_RevisionID) != BRDerr_OK)
		{
			IPC_closeDevice(m_hWDM);
			//CloseHandle(m_hWDM);
//			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
			return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
							  CURRFILE, //ErrVitalText);
							  _BRDC("<Init> Undefined vital data (RevisionID) into init source"));
		}
		if(GetLocation(&AmbLocation) != BRDerr_OK)
		{
			IPC_closeDevice(m_hWDM);
			//CloseHandle(m_hWDM);
//			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
			return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
							  CURRFILE, //ErrVitalText);
							  _BRDC("<Init> Undefined vital data (Location) into init source"));
		}
		//MessageBox(NULL, "<Init> GetDevice&GetLocation", "BAMBP", MB_OK);
/*
		AMB_CONFIGURATION AmbConfiguration;
		if(GetConfiguration(&AmbConfiguration) != BRDerr_OK)
		{
			CloseDevice();
			//CloseHandle(m_hWDM);
//			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
			return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
							  CURRFILE, //ErrVitalText);
							  "<Init> Undefined vital data into init source");
		}
		m_pSDRAM = AmbConfiguration.VirtAddress[2];
*/
		if(GetBaseIcrDataSize() != BRDerr_OK)
		{
			IPC_closeDevice(m_hWDM);
			//CloseHandle(m_hWDM);
//			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
			return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
							  CURRFILE, //ErrVitalText);
							  _BRDC("<Init> Undefined vital data (BaseIcr) into init source"));
		}

		if(GetPid() != BRDerr_OK)
		{
			IPC_closeDevice(m_hWDM);
			//CloseHandle(m_hWDM);
//			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
			return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
							  CURRFILE, //ErrVitalText);
							  _BRDC("<Init> Undefined vital data (Pid) into init source"));
		}
		//ErrorPrintf(DRVERR(BRDerr_WARN), CURRFILE, "<Init> PID = %d", m_PID);

		//MessageBox(NULL, "<Init> GetPid", "BAMBP", MB_OK);
		//
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
				if(!m_pIniData->PID || m_pIniData->PID == m_PID)
					break;
			}
			IPC_closeDevice(m_hWDM);
			//CloseHandle(m_hWDM);	// Wrong Board
			DeviceNumber++;
		}
	} while(m_pIniData);

	if(!m_PID)
		m_PID = DeviceNumber;

//	pBoard->m_DeviceID = DeviceID;
//	pBoard->m_RevisionID = RevisionID;
//	pBoard->m_PID    = PID;
	m_BusNum = AmbLocation.BusNumber;
	m_DevNum = AmbLocation.DeviceNumber;
	m_SlotNum = AmbLocation.SlotNumber;

	BRDC_sprintf(m_name, _BRDC("CAMBP_%X"), m_PID);
	BRDC_strcpy(pBoardName, m_name);

	if(pIniString)
	{
		BRDC_sprintf(pIniString, _BRDC("pid=0x%X\npcibus=0x%X\npcidev=0x%X\npcislot=0x%X\n"), m_PID, m_BusNum, m_DevNum, m_SlotNum);
	}

//	MessageBox(NULL, "BoardName", "BAMBP", MB_OK);
//	MessageBox(NULL, m_name, "BAMBP", MB_OK);

//	PUCHAR pBaseCfgMem = new UCHAR[MAX_BASE_CFGMEM_SIZE];
	PUCHAR pBaseCfgMem = new UCHAR[m_sizeICR];
	ULONG BaseCfgSize = m_sizeICR;
	if(GetBaseEEPROM(pBaseCfgMem, BaseCfgSize) != BRDerr_OK)
	{
		delete[] pBaseCfgMem;
		pBaseCfgMem = NULL;
	}
	if(m_DeviceID == AMBPEX1_DEVID)
		//errorCode = AdmPldStartTest();
		errorCode = AdmPldWorkAndCheck();

	ULONG SubCfgSize = 0;
	//S32 errAdmIdCfgRom = 0;
	for(int i = 0; i < m_AdmIfCnt; i++)
		errorCode = SetSubEeprom(i, SubCfgSize);
//	if(BRDerr_HW_ERROR == errorCode)
//	{
//		MessageBox(NULL, "<Init> ERROR by reading submodule ICR", "BAMBP", MB_OK);
//		//errAdmIdCfgRom = 1;
//		//CloseHandle(m_hWDM);
////		return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
////						  CURRFILE, //ErrVitalText);
////						  "<Init> ERROR by reading submodule ICR");
//	}
//	else
//		MessageBox(NULL, "<Init> OK by reading submodule ICR", "BAMBP", MB_OK);
	errorCode = SetPuInfo(pBaseCfgMem, BaseCfgSize);
	errorCode = SetSdramConfig(m_pIniData, pBaseCfgMem, BaseCfgSize);
	errorCode = SetDspNodeConfig(m_pIniData, pBaseCfgMem, BaseCfgSize);
	if(pBaseCfgMem)
		delete[] pBaseCfgMem;

//	MessageBox(NULL, "<Init> pBaseCfgMem", "BAMBP", MB_OK);
	//errorCode = SetPuInfo();
	//errorCode = SetSdramConfig(m_pIniData);
	//errorCode = SetDspNodeConfig(m_pIniData);
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
//	AllocMemForMainRegs();

	errorCode = SetServices();

//	TCHAR msg_buf[64];
//	BRDC_sprintf(msg_buf, "errorCode=0x%X", errorCode);
//	MessageBox(NULL, msg_buf, "BAMBP", MB_OK);
//	MessageBox(NULL, "<Init> Exit", "BAMBP", MB_OK);
	//if(errAdmIdCfgRom)
	//	return DRVERR(BRDerr_HW_ERROR);
	return DRVERR(errorCode);
}

//***************************************************************************************
void CBambpci::CleanUp()
{
	if(m_hWDM)
	{
//		FreeMemForMainRegs();
		IPC_closeDevice(m_hWDM);
		//CloseHandle(m_hWDM);
	}
}

//***************************************************************************************
void CBambpci::GetDeviceInfo(BRD_Info* pInfo) const
{
	pInfo->boardType	= ((ULONG)m_DeviceID << 16) | (ULONG)m_RevisionID;
	pInfo->pid			= m_PID;
	switch(m_DeviceID)
	{
	case AMBPCD_DEVID:
		BRDC_strcpy(pInfo->name, _BRDC("AMBPCD"));
		break;
	case AMBPCX_DEVID:
		BRDC_strcpy(pInfo->name, _BRDC("AMBPCX"));
		break;
	case AMBPEX1_DEVID:
		BRDC_strcpy(pInfo->name, _BRDC("AMBPEX1"));
		break;
	case SYNCDAC_DEVID:
		BRDC_strcpy(pInfo->name, _BRDC("SYNCDAC"));
		break;
	case SYNCBCO_DEVID:
		BRDC_strcpy(pInfo->name, _BRDC("SYNCBCO"));
		break;
	case VK3_DEVID:
		BRDC_strcpy(pInfo->name, _BRDC("VK3"));
		break;
	case SYNCCP6_DEVID:
		BRDC_strcpy(pInfo->name, _BRDC("SYNC-cP6"));
		break;
	case SYNCCP6R_DEVID:
		BRDC_strcpy(pInfo->name, _BRDC("SYNC-cP6R"));
		break;
	case DR16_DEVID:
		BRDC_strcpy(pInfo->name, _BRDC("DR16"));
		break;
	case RFDR4_DEVID:
		BRDC_strcpy(pInfo->name, _BRDC("RFDR4"));
		break;
	case C520F1_DEVID:
		BRDC_strcpy(pInfo->name, _BRDC("C520F1"));
		break;
	case GRADC_DEVID:
		BRDC_strcpy(pInfo->name, _BRDC("GR-ADC"));
		break;
	case AMBPEX2_DEVID:
		BRDC_strcpy(pInfo->name, _BRDC("AMBPEX2"));
		break;
	default:
		BRDC_strcpy(pInfo->name, _BRDC("Unknow"));
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
		pInfo->subunitType[i] = m_Subunit[i].type;
	for(i = 0; i < 8; i++)
		pInfo->base[i] = 0;
	if(m_pSDRAM)
		pInfo->base[0] = m_pSDRAM;
}

#define FOTR3G_TETR_ID		0x49
#define FOTR3G_SUBMODUL_ID	0x0D20

////***************************************************************************************
//ULONG CBambpci::CheckPldFile(PCTSTR PldFileName)
//{
//	char *FirstChar;
//	char FullFileName[MAX_PATH];
//	GetFullPathName(PldFileName, MAX_PATH, FullFileName, &FirstChar);
//	HANDLE hFile = CreateFile(FullFileName,
//								GENERIC_READ,
//								FILE_SHARE_READ, NULL,
//								OPEN_EXISTING,
//								FILE_ATTRIBUTE_NORMAL,
//								NULL);
//    if(hFile == INVALID_HANDLE_VALUE)
//	{
////	    return BRDerr_BAD_FILE;
//		return ErrorPrintf( BRDerr_BAD_FILE,
//							CURRFILE,
//							"<LoadPldFile>  Can't open STAPL file '%s'", FullFileName);
//	}
//	CloseHandle(hFile);
//	return BRDerr_OK;
//}

//***************************************************************************************
ULONG CBambpci::HardwareInit()
{
	ULONG Status = BRDerr_OK;
	if(m_DeviceID == AMBPEX1_DEVID ||
		m_DeviceID == SYNCDAC_DEVID ||
		m_DeviceID == SYNCBCO_DEVID ||
		m_DeviceID == VK3_DEVID ||
		m_DeviceID == SYNCCP6_DEVID ||
		m_DeviceID == SYNCCP6R_DEVID ||
		m_DeviceID == DR16_DEVID ||
		m_DeviceID == RFDR4_DEVID ||
		m_DeviceID == C520F1_DEVID ||
		m_DeviceID == GRADC_DEVID)
	{
		//Status = AdmPldStartTest();
//		if(Status != BRDerr_OK)
//			return;
		Status = AdmPldWorkAndCheck();
		if(Status != BRDerr_OK)
			return BRDerr_OK;
		Status = SetAdmPldFileInfo();
		if(BRDerr_OK != Status)
			return PLD_errROM;

		if(m_pldFileInfo.submodId == FOTR3G_SUBMODUL_ID && m_pldFileInfo.submodVer == 0x0100)
		{
			DEVS_CMD_Reg RegParam;
			RegParam.idxMain = 0;
			int i = 0;
			for(i = 0; i < MAX_TETRNUM; i++)
			{
				RegParam.tetr = i;
				RegParam.reg = ADM2IFnr_ID;
				Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
				if(Status != BRDerr_OK)
					return Status;
				if(FOTR3G_TETR_ID == RegParam.val)
				{
					RegParam.tetr = i;
					RegParam.reg = ADM2IFnr_STATUS;
					RegCtrl(DEVScmd_REGREADDIR, &RegParam);
					if(!(RegParam.val & 0x100))
					{
						Status = PLD_errNOTRDY;
						//Status = GetMemoryAddress();
						//Status = CheckPldFile(m_FotrJamPldFileName);
						//if(Status == BRDerr_OK)
						//	Status = LoadJamPld(m_FotrJamPldFileName);
						break;
					}
				}
			}
		}
	}
	else
	{
		ULONG pld_status;
		if(GetPldStatus(pld_status, 0) != BRDerr_OK)
			return BRDerr_HW_ERROR;
			//return ErrorPrint(BRDerr_HW_ERROR,
			//				  CURRFILE,
			//				  _BRDC("<HardwareInit> GetPldStatus != OK"));
		if(pld_status)
			Status = SetAdmPldFileInfo();
	}
	return Status;
}

//***************************************************************************************
void CBambpci::GetPldCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAdmPld pCfgPld)
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
/*
// ***************************************************************************************
ULONG CBambpci::GetPldDescription(TCHAR* Description)
{
		// Check base module ICR
	ULONG icr_base_status;
	if(GetBaseIcrStatus(icr_base_status) != BRDerr_OK)
//		return BRDerr_HW_ERROR;
		return ErrorPrint(BRDerr_HW_ERROR,
						  CURRFILE,
						  "<GetPldInfoICR> Hardware error by get base ICR status");
	if(icr_base_status)
	{	// получаем конфигурацию из ППЗУ
		PUCHAR pBaseCfgMem = new UCHAR[MAX_BASE_CFGMEM_SIZE];
		ULONG BufferSize = MAX_BASE_CFGMEM_SIZE;
		ULONG Offset = OFFSET_BASE_CFGMEM;
		if(ReadNvRAM(pBaseCfgMem, BufferSize, Offset) != BRDerr_OK)
//			return BRDerr_BAD_DEVICE_VITAL_DATA;
			return ErrorPrint(BRDerr_BAD_DEVICE_VITAL_DATA,
							  CURRFILE,
							  "<GetPldInfoICR> Bad device vital data by read base ICR");
		ULONG dRealBaseCfgSize;
		if(*(PUSHORT)pBaseCfgMem == BASE_ID_TAG)
		{
			dRealBaseCfgSize = *((PUSHORT)pBaseCfgMem + 2);
			ICR_CfgAdmPld cfgPld;
			GetPldCfgICR(pBaseCfgMem, dRealBaseCfgSize, &cfgPld);
			sprintf(Description, _BRDC("%d %d %d"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
			switch(cfgPld.bType)
			{
			case 3:
				sprintf(Description, _BRDC("XCVE%d-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
				break;
			case 4:
				sprintf(Description, _BRDC("XC2V%d-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
				break;
			case 5:
				sprintf(Description, _BRDC("XC2S%d-%dFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
				break;
			case 6:
				sprintf(Description, _BRDC("XC2S%dE-%dFG%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
				break;
			}
			icr_base_status = BRDerr_OK;
		}
		else
//			return BRDerr_BAD_DEVICE_VITAL_DATA;
			icr_base_status = ErrorPrint(BRDerr_BAD_DEVICE_VITAL_DATA,
							  CURRFILE,
							  "<GetPldInfoICR> Invalid BASE ID tag getted from ICR");
		delete pBaseCfgMem;
	}
	return icr_base_status;
}

// ***************************************************************************************
ULONG CBambpci::GetDspPld(UCHAR& DspPldCnt)
{
		// Check base module ICR
	ULONG icr_base_status;
	if(GetBaseIcrStatus(icr_base_status) != BRDerr_OK)
//		return BRDerr_HW_ERROR;
		return ErrorPrint(BRDerr_HW_ERROR,
						  CURRFILE,
						  "<GetPldInfoICR> Hardware error by get base ICR status");
	if(icr_base_status)
	{	// получаем конфигурацию из ППЗУ
		PUCHAR pBaseCfgMem = new UCHAR[MAX_BASE_CFGMEM_SIZE];
		ULONG BufferSize = MAX_BASE_CFGMEM_SIZE;
		ULONG Offset = OFFSET_BASE_CFGMEM;
		if(ReadNvRAM(pBaseCfgMem, BufferSize, Offset) != BRDerr_OK)
//			return BRDerr_BAD_DEVICE_VITAL_DATA;
			return ErrorPrint(BRDerr_BAD_DEVICE_VITAL_DATA,
							  CURRFILE,
							  "<GetPldInfoICR> Bad device vital data by read base ICR");
		ULONG dRealBaseCfgSize;
		if(*(PUSHORT)pBaseCfgMem == BASE_ID_TAG)
		{
			dRealBaseCfgSize = *((PUSHORT)pBaseCfgMem + 2);
			ICR_CfgAmbp cfgAmbp;
			GetBaseCfgICR(pBaseCfgMem, dRealBaseCfgSize, &cfgAmbp);
			DspPldCnt = cfgAmbp.bDspNodeCfgCnt;
			icr_base_status = BRDerr_OK;
		}
		else
//			return BRDerr_BAD_DEVICE_VITAL_DATA;
			icr_base_status = ErrorPrint(BRDerr_BAD_DEVICE_VITAL_DATA,
										CURRFILE,
										"<GetPldInfoICR> Invalid BASE ID tag getted from ICR");
		delete pBaseCfgMem;
	}
	return icr_base_status;
}
*/
//***************************************************************************************
ULONG CBambpci::GetPldDescription(BRDCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
	if(BaseEEPROMSize)
	{
		ICR_CfgAdmPld cfgPld;
		GetPldCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &cfgPld);
		m_pldType = cfgPld.bType;
		BRDC_sprintf(Description, _BRDC("%d %d %d"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
		switch(cfgPld.bType)
		{
		case 3:
			BRDC_sprintf(Description, _BRDC("XCVE%d-FF%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
			break;
		case 4:
			BRDC_sprintf(Description, _BRDC("XC2V%d-FF%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
			break;
		case 5:
			BRDC_sprintf(Description, _BRDC("XC2S%d-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
			break;
		case 6:
			BRDC_sprintf(Description, _BRDC("XC2S%dE-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
			break;
		case 7:
			BRDC_sprintf(Description, _BRDC("XC3S%d-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
			break;
		case 15:
			BRDC_sprintf(Description, _BRDC("XC5VLX%d-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
			break;
		case 16:
			BRDC_sprintf(Description, _BRDC("XC5VSX%d-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
			break;
		case 17:
			BRDC_sprintf(Description, _BRDC("XC5VFX%d-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
			break;
		case 18:
			BRDC_sprintf(Description, _BRDC("XC5VLX%dT-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
			break;
		case 19:
			BRDC_sprintf(Description, _BRDC("XC5VSX%dT-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
			break;
		case 20:
			BRDC_sprintf(Description, _BRDC("XC5VFX%dT-%dFF%dC"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
			break;
		}
		return 1;
	}
	return 0;
}
/*
// ***************************************************************************************
void CBambpci::GetDspNodeCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDspNode pCfgDsp)
{
	PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
	int end_flag = 0;
	do
	{
		USHORT sign = *((USHORT*)pCfgMem);
		USHORT size = 0;
		if(sign == DSPNODE_CFG_TAG)
		{
			PICR_CfgAdmPld pCfg = (PICR_CfgDspNode)pCfgMem;
			memcpy(pCfgDsp, pCfg, sizeof(ICR_CfgDspNode));
			size += sizeof(PICR_CfgDspNode);
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
*/

//***************************************************************************************
ULONG CBambpci::GetDspPldDescription(TCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
	if(BaseEEPROMSize)
	{
		ICR_CfgDspNode cfgDspNode;
		ICR_CfgSram cfgSram;
		GetDspNodeCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &cfgDspNode, &cfgSram);
		BRDC_sprintf(Description, _BRDC("%d %d %d"), cfgDspNode.wPldVolume, cfgDspNode.wPldPins, cfgDspNode.bPldSpeedGrade);
		switch(cfgDspNode.bPldType)
		{
		case 3:
			BRDC_sprintf(Description, _BRDC("XCVE%d-FF%d-%dC"), cfgDspNode.wPldVolume, cfgDspNode.wPldPins, cfgDspNode.bPldSpeedGrade);
			break;
		case 4:
			BRDC_sprintf(Description, _BRDC("XC2V%d-FF%d-%dC"), cfgDspNode.wPldVolume, cfgDspNode.wPldPins, cfgDspNode.bPldSpeedGrade);
			break;
		case 5:
			BRDC_sprintf(Description, _BRDC("XC2S%d-FG%d-%dC"), cfgDspNode.wPldVolume, cfgDspNode.wPldPins, cfgDspNode.bPldSpeedGrade);
			break;
		case 6:
			BRDC_sprintf(Description, _BRDC("XC2S%dE-FG%d-%dC"), cfgDspNode.wPldVolume, cfgDspNode.wPldPins, cfgDspNode.bPldSpeedGrade);
			break;
		case 7:
			BRDC_sprintf(Description, _BRDC("XC3S%d-FG%d-%dC"), cfgDspNode.wPldVolume, cfgDspNode.wPldPins, cfgDspNode.bPldSpeedGrade);
			break;
		}
		return 1;
	}
	return 0;
}

//***************************************************************************************
ULONG CBambpci::GetDspPld(PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize, UCHAR& DspPldCnt)
{
	if(BaseEEPROMSize)
	{
		if(m_DeviceID == AMBPEX1_DEVID ||
			m_DeviceID == SYNCDAC_DEVID ||
			m_DeviceID == SYNCBCO_DEVID ||
			m_DeviceID == VK3_DEVID ||
			m_DeviceID == SYNCCP6_DEVID ||
			m_DeviceID == SYNCCP6R_DEVID ||
			m_DeviceID == DR16_DEVID ||
			m_DeviceID == RFDR4_DEVID ||
			m_DeviceID == C520F1_DEVID ||
			m_DeviceID == GRADC_DEVID)
			DspPldCnt = 0;
		else
		{
			ICR_CfgAmbp cfgAmbp;
			GetAmbpCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &cfgAmbp);
			DspPldCnt = cfgAmbp.bDspNodeCfgCnt;
		}
		return 1;
	}
	else
		DspPldCnt = 0;
	return 0;
}

//***************************************************************************************
ULONG CBambpci::SetPuInfo(PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
	m_puCnt = 1;
	m_AdmPldInfo.Id = BRDpui_PLD;//0x100;
	m_AdmPldInfo.Type = PLD_CFG_TAG;
	m_AdmPldInfo.Attr = BRDpua_Load;
	BRDCHAR Description0[MAX_DESCRIPLEN] = _BRDC("ADM PLD ");
//	int ret = GetStringText(IDS_STRINGADMPLD, Description0);
//	int ret = GetStringText(MSG_ADM_PLD, Description0);
	BRDCHAR Description[MAX_DESCRIPLEN] = _BRDC("ADM PLD XILINX");// Programmable Unit Description Text
//	ret = GetStringText(IDS_STRINGADMPLD, Description);
//	ret = GetStringText(MSG_ADM_PLD, Description);
//	_tcscat(Description, _TEXT("- XILINX"));
//	GetPldDescription(Description);
	if(GetPldDescription(Description, pBaseEEPROMMem, BaseEEPROMSize))
	{
//		_tcscpy(m_AdmPldInfo.Description, _TEXT("ADM PLD "));
		BRDC_strcpy(m_AdmPldInfo.Description, Description0);
		BRDC_strcat(m_AdmPldInfo.Description, Description);
	}
	else
		BRDC_strcpy(m_AdmPldInfo.Description, Description);

	m_PuInfos.push_back(m_AdmPldInfo);

	UCHAR DspPldCnt = 0;
//	ULONG Status = GetDspPld(DspPldCnt);
//	if(Status == BRDerr_OK && DspPldCnt)
	GetDspPld(pBaseEEPROMMem, BaseEEPROMSize, DspPldCnt);
	if(DspPldCnt)
	{
		m_puCnt++;
		m_DspPldInfo.Id = BRDpui_PLD + 1;
		m_DspPldInfo.Type = PLD_CFG_TAG;
		m_DspPldInfo.Attr = BRDpua_Load;
		BRDCHAR Description0[MAX_DESCRIPLEN] = _BRDC("DSP PLD ");
//		ret = GetStringText(IDS_STRINGDSPPLD, Description0);
//		ret = GetStringText(MSG_DSP_PLD, Description0);
		BRDCHAR Description[MAX_DESCRIPLEN] = _BRDC("DSP PLD XILINX");// Programmable Unit Description Text
//		ret = GetStringText(IDS_STRINGDSPPLD, Description);
//		ret = GetStringText(MSG_DSP_PLD, Description);
//		_tcscat(Description, _TEXT("- XILINX"));
		if(GetDspPldDescription(Description, pBaseEEPROMMem, BaseEEPROMSize))
		{
//			_tcscpy(m_DspPldInfo.Description, _TEXT("DSP PLD "));
			BRDC_strcpy(m_DspPldInfo.Description, Description0);
			BRDC_strcat(m_DspPldInfo.Description, Description);
		}
		else
			BRDC_strcpy(m_DspPldInfo.Description, Description);

		m_PuInfos.push_back(m_DspPldInfo);
	}

	if(1)
	{
		m_puCnt++;
		m_BaseIcrInfo.Id = BRDpui_BASEICR;
		m_BaseIcrInfo.Type = BASE_ID_TAG;
		m_BaseIcrInfo.Attr = BRDpua_Read | BRDpua_Write | BRDpua_Danger;
//		ret = GetStringText(IDS_STRINGBASEICR, m_BaseIcrInfo.Description);
//		ret = GetStringText(MSG_BASE_ICR, m_BaseIcrInfo.Description);
		BRDC_strcpy(m_BaseIcrInfo.Description, _BRDC("ID & CFG EEPROM on Base module"));

		m_PuInfos.push_back(m_BaseIcrInfo);
	}


	 // определяет просто наличие ППЗУ на субмодуле
//	PUSHORT pSubCfgMem = (PUSHORT)(m_Subunit[0].pSubEEPROM);
//	if(( pSubCfgMem[0] != 0xA55A) && (pSubCfgMem[1] != 0x5AA5))
//	if(( pSubCfgMem[0] != 0) && (pSubCfgMem[1] != 0))
	{
		m_puCnt++;
		m_AdmIcrInfo.Id = BRDpui_SUBICR;
		m_AdmIcrInfo.Type = ADM_ID_TAG;
		m_AdmIcrInfo.Attr = BRDpua_Read | BRDpua_Write | BRDpua_Danger;
		BRDC_strcpy(m_AdmIcrInfo.Description, _BRDC("ID & CFG EEPROM on subunit"));
		m_PuInfos.push_back(m_AdmIcrInfo);
	}
/*
	USHORT buf_read[2];
	buf_read[0] = buf_read[1] = 0xA55A;
	ULONG Status = ReadAdmIdROM(buf_read, 4, 0);
//	if((Status != BRDerr_OK) ||(buf_read[0] == 0 && buf_read[1] == 0))
	if((Status == BRDerr_OK) && (buf_read[0] != 0 || buf_read[1] != 0))
	{
		m_puCnt++;
		m_AdmIcrInfo.Id = BRDpui_SUBICR;
		m_AdmIcrInfo.Type = ADM_ID_TAG;
		m_AdmIcrInfo.Attr = BRDpua_Read | BRDpua_Write;
//		ret = GetStringText(IDS_STRINGADMICR, m_AdmIcrInfo.Description);
//		ret = GetStringText(MSG_ADM_ICR, m_AdmIcrInfo.Description);
		BRDC_strcpy(m_AdmIcrInfo.Description, _TEXT("ID & CFG EEPROM on subunit"));

		m_PuInfos.push_back(m_AdmIcrInfo);
	}
*/
	return BRDerr_OK;
}

//***************************************************************************************
void CBambpci::GetPuList(BRD_PuList* pu_list) const
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

//***************************************************************************************
ULONG CBambpci::PuFileLoad(ULONG id, const BRDCHAR* fileName, PUINT pState)
{
	*pState = 0;
	if(BRDC_strlen(fileName) == 0)
	{
//		return BRDerr_BAD_FILE;
		return ErrorPrintf( BRDerr_BAD_FILE,
							CURRFILE,
							_BRDC("<PuFileLoad> Bad File Name = %s"), fileName);
	}
	// Check ADM or DSP PLD
	if(id == m_AdmPldInfo.Id || id == m_DspPldInfo.Id)
	{
		ULONG status = LoadPldFile(fileName, id - BRDpui_PLD);
		//if(status != BRDerr_OK)
		//	return status;
		if(status == BRDerr_HW_ERROR)
			return status;

		ULONG pld_status;
		if(GetPldStatus(pld_status, id - BRDpui_PLD) != BRDerr_OK)
//			return BRDerr_HW_ERROR;
			return ErrorPrint(BRDerr_HW_ERROR,
							  CURRFILE,
							  _BRDC("<PuFileLoad> Hardware error"));
		*pState = pld_status;
		return status;
	}
	else
	{
//	    return BRDerr_BAD_ID;
		return ErrorPrintf( BRDerr_BAD_ID,
							CURRFILE,
							_BRDC("<PuFileLoad> Bad Programable Unit ID %d"), id);
	}
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::GetPuState(ULONG id, PUINT pState)
{
	// Check ADM PLD
	if(id == m_AdmPldInfo.Id)
	{
		ULONG pld_status;
		if(m_DeviceID == AMBPEX1_DEVID ||
			m_DeviceID == SYNCDAC_DEVID ||
			m_DeviceID == SYNCBCO_DEVID ||
			m_DeviceID == VK3_DEVID	||
			m_DeviceID == SYNCCP6_DEVID ||
			m_DeviceID == SYNCCP6R_DEVID ||
			m_DeviceID == DR16_DEVID ||
			m_DeviceID == RFDR4_DEVID ||
			m_DeviceID == C520F1_DEVID ||
			m_DeviceID == GRADC_DEVID)
			pld_status = 1;
		else
			if(GetPldStatus(pld_status, id - BRDpui_PLD) != BRDerr_OK)
	//			return BRDerr_HW_ERROR;
				return ErrorPrint(BRDerr_HW_ERROR,
								  CURRFILE,
								  _BRDC("<GetPuState> Hardware error"));
		*pState = pld_status;
	}
	else
	{
		// Check DSP PLD
		if(id == m_DspPldInfo.Id)
		{
			ULONG pld_status;
			if(GetPldStatus(pld_status, id - BRDpui_PLD) != BRDerr_OK)
	//			return BRDerr_HW_ERROR;
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
	//				return BRDerr_HW_ERROR;
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
					if(GetAdmIcrStatus(adm_status) != BRDerr_OK)
	//					return BRDerr_HW_ERROR;
						return ErrorPrint(BRDerr_HW_ERROR,
										CURRFILE,
										_BRDC("<GetPuState> Hardware error"));
					*pState = adm_status;
				}
				else
				{
	//				return BRDerr_BAD_ID;
					return ErrorPrintf( BRDerr_BAD_ID,
										CURRFILE,
										_BRDC("<GetPuState> Bad Programable Unit ID %d"), id);
				}
			}
		}
	}
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::GetPid()
{
	if(m_DeviceID == AMBPCX_DEVID ||
	   m_DeviceID == AMBPCD_DEVID ||
	   m_DeviceID == AMBPEX1_DEVID ||
	   m_DeviceID == AMBPEX2_DEVID ||
//	   m_DeviceID == VK3_DEVID)
		m_DeviceID == VK3_DEVID ||
		m_DeviceID == SYNCCP6_DEVID ||
		m_DeviceID == SYNCCP6R_DEVID ||
		m_DeviceID == DR16_DEVID ||
		m_DeviceID == RFDR4_DEVID ||
		m_DeviceID == C520F1_DEVID ||
		m_DeviceID == GRADC_DEVID)
//	if(1)
	{
		if(m_sizeICR)
		{
			ULONG BufferSize = 4;
			ULONG Offset = 128 + 6;
			if(ReadNvRAM(&m_PID, BufferSize, Offset) != BRDerr_OK)
			{
	//			return BRDerr_BAD_DEVICE_VITAL_DATA;
				return ErrorPrint(BRDerr_BAD_DEVICE_VITAL_DATA,
								CURRFILE,
								_BRDC("<GetPid> Undefined vital data into ICR"));
			}
		}
		else
			m_PID = 0;
	}
	else
		m_PID = 0;
	return BRDerr_OK;
}

// ***************************************************************************************
ULONG CBambpci::ReadPldFile(const BRDCHAR* PldFileName, UCHAR*& fileBuffer, ULONG& fileSize)
{
	BRDCHAR FullFileName[MAX_PATH];
	IPC_getFullPath(PldFileName, FullFileName);
    IPC_handle hFile = IPC_openFile(FullFileName, IPC_OPEN_FILE | IPC_FILE_RDONLY);
    if(!hFile)
	{
		return ErrorPrintf( BRDerr_BAD_FILE,
							CURRFILE,
							_BRDC("<LoadPldFile>  Can't open PLD file '%s'"), FullFileName);
	}
    long long file_size;
	if(IPC_getFileSize(hFile, &file_size) != IPC_OK)
	{
		return ErrorPrintf( BRDerr_BAD_FILE,
							CURRFILE,
							_BRDC("<LoadPldFile>  Can't open PLD file '%s'"), FullFileName);
	}
	fileSize = (ULONG)file_size;
	//fileBuffer = VirtualAlloc(NULL, fileSize, MEM_COMMIT, PAGE_READWRITE);
	fileBuffer = new UCHAR[fileSize];
	if(fileBuffer == NULL)
	{
		return ErrorPrint(  BRDerr_NOT_ENOUGH_MEMORY,
							CURRFILE,
							_BRDC("<LoadPldFile> Not Enough memory for file buffer"));
	}
	int dwNumBytesRead = IPC_readFile(hFile, fileBuffer, fileSize);
	if(dwNumBytesRead < 0)
	{
		//VirtualFree(fileBuffer, 0, MEM_RELEASE);
        delete fileBuffer;
		return ErrorPrintf( BRDerr_BAD_FILE,
							CURRFILE,
							_BRDC("<LoadPldFile>  Read error PLD file '%s'"), FullFileName);
	}
	if(IPC_closeFile(hFile) != IPC_OK)
	{
		//VirtualFree(fileBuffer, 0, MEM_RELEASE);
		delete fileBuffer;
		return ErrorPrintf( BRDerr_BAD_FILE,
							CURRFILE,
							_BRDC("<LoadPldFile>  Close error PLD file '%s'"), FullFileName);
	}
	return BRDerr_OK;
}
//ULONG CBambpci::ReadPldFile(PCTSTR PldFileName, LPVOID& fileBuffer, DWORD& fileSize)
//{
//	char *FirstChar;
//	char FullFileName[MAX_PATH];
//	GetFullPathName(PldFileName, MAX_PATH, FullFileName, &FirstChar);
//	HANDLE hFile = CreateFile(FullFileName,
//								GENERIC_READ,
//								FILE_SHARE_READ, NULL,
//								OPEN_EXISTING,
//								FILE_ATTRIBUTE_NORMAL,
//								NULL);
//    if(hFile == INVALID_HANDLE_VALUE)
//	{
////	    return BRDerr_BAD_FILE;
//		return ErrorPrintf( BRDerr_BAD_FILE,
//							CURRFILE,
//							_BRDC("<LoadPldFile>  Can't open HEX file '%s'"), FullFileName);
//	}
//	fileSize = SetFilePointer(hFile, 0, NULL, FILE_END);
//	if(fileSize == 0xffffffff)
//	{
////	    return BRDerr_BAD_FILE;
//		return ErrorPrintf( BRDerr_BAD_FILE,
//							CURRFILE,
//							_BRDC("<LoadPldFile>  Can't open HEX file '%s'"), FullFileName);
//	}
//	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
//	fileBuffer = VirtualAlloc(NULL, fileSize, MEM_COMMIT, PAGE_READWRITE);
//	if(fileBuffer == NULL)
//	{
////		return BRDerr_NOT_ENOUGH_MEMORY;
//		return ErrorPrint(  BRDerr_NOT_ENOUGH_MEMORY,
//							CURRFILE,
//							_BRDC("<LoadPldFile> Not Enough memory for file buffer"));
//	}
//	DWORD dwNumBytesRead;
//	if(ReadFile(hFile, fileBuffer, fileSize, &dwNumBytesRead, NULL) == 0)
//	{
//		VirtualFree(fileBuffer, 0, MEM_RELEASE);
////	    return BRDerr_BAD_FILE;
//		return ErrorPrintf( BRDerr_BAD_FILE,
//							CURRFILE,
//							_BRDC("<LoadPldFile>  Read error HEX file '%s'"), FullFileName);
//	}
//	if(CloseHandle(hFile) == 0)
//	{
//		VirtualFree(fileBuffer, 0, MEM_RELEASE);
////	    return BRDerr_BAD_FILE;
//		return ErrorPrintf( BRDerr_BAD_FILE,
//							CURRFILE,
//							_BRDC("<LoadPldFile>  Close error HEX file '%s'"), FullFileName);
//	}
//	return BRDerr_OK;
//}

// ***************************************************************************************
ULONG CBambpci::LoadPldFile(const BRDCHAR* PldFileName, ULONG PldNum)
{
	ULONG Status = BRDerr_OK;
	UCHAR PldLoadStatus = PLD_errROM; // PLD_errROM - нет EEPROM

	if(!PldNum &&
		(m_DeviceID == AMBPCX_DEVID || m_DeviceID == AMBPEX2_DEVID))
	{
		Status = LoadPldRom(PldLoadStatus);
		if(Status != BRDerr_OK)
			PldLoadStatus = PLD_errROM;
	}
	if(PldLoadStatus == PLD_errROM)
	{
		UCHAR* fileBuffer = NULL;
		DWORD fileSize;
		Status = ReadPldFile(PldFileName, fileBuffer, fileSize);
		if(Status != BRDerr_OK)
			return Status;

		BRDC_printf(_BRDC("<LoadPldFile> %s\n"), PldFileName);

		int ind_ext = lstrlen(PldFileName) - 3*sizeof(TCHAR);
        const BRDCHAR* PldFileExt = PldFileName + ind_ext;
		if(!lstrcmpi(PldFileExt,_BRDC("ace")) && m_DeviceID == AMBPEX2_DEVID)
			Status = LoadAcePld(PldLoadStatus, fileBuffer, fileSize, PldNum);
		else
			Status = LoadPld(PldLoadStatus, fileBuffer, fileSize, PldNum);
		//VirtualFree(fileBuffer, 0, MEM_RELEASE);
        delete fileBuffer;
		if(Status != BRDerr_OK)
			return Status;
	}
	switch(PldLoadStatus)
	{
	case PLD_errOK:
		break;
	case PLD_errFORMAT:
		Status = BRDerr_BAD_FILE_FORMAT;
		break;
//	case PLD_errPROGR:
//	case PLD_errRECLOST:
	case PLD_errNOTRDY:
		Status = BRDerr_HW_ERROR;
		break;
	case PLD_errROM:
		break;
	}
	if(Status != BRDerr_OK)
		return Status;
    if(!PldNum)
	{
//		Status = AdmPldStartTest();
//		if(Status != BRDerr_OK)
//			return Status;
		Status = AdmPldWorkAndCheck();
		if(Status != BRDerr_OK)
			return Status;
		Status = SetAdmPldFileInfo();
	}
	else
	{
		Status = DspPldWorkAndCheck();
		if(Status != BRDerr_OK)
			return Status;
	}

	return Status;
}

//***************************************************************************************
ULONG CBambpci::AdmPldStartTest()
{
	ULONG Status = BRDerr_OK;
	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = 0;
	RegParam.tetr = ADM2IFnt_MAIN;
	RegParam.reg = ADM2IFnr_DATA;
	RegParam.val = 1; // переключить ПЛИС в рабочий режим
	Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);
	if(m_DeviceID == SYNCDAC_DEVID ||
		m_DeviceID == SYNCBCO_DEVID ||
		m_DeviceID == VK3_DEVID ||
		m_DeviceID == AMBPEX2_DEVID ||
		m_DeviceID == SYNCCP6_DEVID ||
		m_DeviceID == SYNCCP6R_DEVID ||
		m_DeviceID == DR16_DEVID ||
		m_DeviceID == RFDR4_DEVID ||
		m_DeviceID == C520F1_DEVID ||
		m_DeviceID == GRADC_DEVID)
		Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);
	RegParam.reg = ADM2IFnr_FTYPE;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	int fifo_width = (int)RegParam.val; // получить ширину регистра тестовой последовательности
	RegParam.reg = ADM2IFnr_DATA;
	RegParam.val = 2; // обратно переключить ПЛИС в режим начального тестирования
	Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);
	if(m_DeviceID == SYNCDAC_DEVID ||
		m_DeviceID == SYNCBCO_DEVID ||
		m_DeviceID == VK3_DEVID ||
		m_DeviceID == AMBPEX2_DEVID ||
		m_DeviceID == SYNCCP6_DEVID ||
		m_DeviceID == SYNCCP6R_DEVID ||
		m_DeviceID == DR16_DEVID ||
		m_DeviceID == RFDR4_DEVID ||
		m_DeviceID == C520F1_DEVID ||
		m_DeviceID == GRADC_DEVID)
		Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);

	int i = 0;
	if( (m_DeviceID == AMBPEX1_DEVID) ||
		(m_DeviceID == AMBPEX2_DEVID))
	{
		// тест шины данных
		ULONG test_mask = 0;
		for(i = 0; i < fifo_width; i++)
		{
			ULONG value = 0;
			Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			if(Status != BRDerr_OK)
				return Status;
			value = RegParam.val;
			if(test_mask != value)
				return BRDerr_PLD_TEST_DATA_ERROR;
			test_mask = (test_mask << 1) + 1;
		}
		for(i = 0; i < fifo_width; i++)
		{
			ULONG value = 0;
			Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			if(Status != BRDerr_OK)
				return Status;
			value = RegParam.val;
			if(test_mask != value)
				return BRDerr_PLD_TEST_DATA_ERROR;
			test_mask = (test_mask << 1) & 0xFFFFFFFE;
		}
	}
	else
	{
		// тест шины данных
		__int64 test_mask = 0;
		for(i = 0; i < fifo_width; i++)
		{
			__int64 value = 0;
			Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			if(Status != BRDerr_OK)
				return Status;
			value = int64_t(RegParam.val);
			Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			if(Status != BRDerr_OK)
				return Status;
			value |= (int64_t(RegParam.val) << 32);
			if(test_mask != value)
				return BRDerr_PLD_TEST_DATA_ERROR;
			test_mask = (test_mask << 1) + 1;
		}
		for(i = 0; i < fifo_width; i++)
		{
			__int64 value = 0;
			Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			if(Status != BRDerr_OK)
				return Status;
			value = int64_t(RegParam.val);
			Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			if(Status != BRDerr_OK)
				return Status;
			value |= (int64_t(RegParam.val) << 32);
			if(test_mask != value)
				return BRDerr_PLD_TEST_DATA_ERROR;
            test_mask = (test_mask << 1) & 0xFFFFFFFFFFFFFFFEULL;
		}
	}
	// тест шины адреса
	if(m_DeviceID != SYNCDAC_DEVID &&
		m_DeviceID != SYNCBCO_DEVID &&
		m_DeviceID != VK3_DEVID &&
		m_DeviceID != AMBPEX2_DEVID &&
		m_DeviceID != SYNCCP6_DEVID &&
		m_DeviceID != SYNCCP6R_DEVID &&
		m_DeviceID != DR16_DEVID &&
		m_DeviceID != RFDR4_DEVID &&
		m_DeviceID != C520F1_DEVID &&
		m_DeviceID != GRADC_DEVID)
	{
		for(i = 0; i < 64; i++) // всего 64 регистра (16 тетрад)
		{
			RegParam.tetr = i >> 2;
			RegParam.reg = i;
			RegParam.val = 0;
			Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);
			if(Status != BRDerr_OK)
				return Status;
			RegParam.tetr = 0;
			RegParam.reg = ADM2IFnr_DATA;
			Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			ULONG value = RegParam.val;
			Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			if(Status != BRDerr_OK)
				return Status;
			if((value & 0x3f) != i)
				return BRDerr_PLD_TEST_ADDRESS_ERROR;
		}
	}
	return Status;
}

//***************************************************************************************
ULONG CBambpci::AdmPldWorkAndCheck()
{
	ULONG Status;
	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = 0;
	RegParam.tetr = ADM2IFnt_MAIN;
	RegParam.reg = ADM2IFnr_DATA;
	RegParam.val = 1; // переключить ПЛИС в рабочий режим
	Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);
	if(m_DeviceID == SYNCDAC_DEVID ||
		m_DeviceID == SYNCBCO_DEVID ||
		m_DeviceID == VK3_DEVID ||
		m_DeviceID == AMBPEX2_DEVID ||
		m_DeviceID == SYNCCP6_DEVID ||
		m_DeviceID == SYNCCP6R_DEVID ||
		m_DeviceID == DR16_DEVID ||
		m_DeviceID == RFDR4_DEVID ||
		m_DeviceID == C520F1_DEVID ||
		m_DeviceID == GRADC_DEVID)
		Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);
	RegParam.reg = ADM2IFnr_STATUS;
	Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
/*	RegParam.reg = ADM2IFnr_ID;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_IDMOD;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_VER;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_TRES;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_FSIZE;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_FTYPE;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_PATH;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_IDNUM;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
*/
/*
	AMB_CONFIGURATION AmbConfiguration;
	Status = GetConfiguration(&AmbConfiguration);
	if(Status != BRDerr_OK)
		return Status;
	m_pSDRAM = AmbConfiguration.VirtAddress[2];
*/
	if(m_DeviceID == AMBPEX1_DEVID)
	{ // workaround (костыль) - на платах AMBPEX1 не всегда читается ID тетрады MAIN (но почему-то пишется)
		RegParam.reg = ADM2IFnr_ID;
		Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
		if(RegParam.val != 1)
		{
			RegParam.val = 1;
			Status = RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
		}
	}

	Status = ReadRegData(0, 0, 0, RegParam.val);
	if(Status != BRDerr_OK)
		return BRDerr_HW_ERROR;

	RegParam.reg = MAINnr_SIG;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
//	if(INSYS_VENDOR_ID != RegParam.val)
	if(BASE_ID_TAG != RegParam.val)
		return BRDerr_HW_ERROR;
	RegParam.reg = MAINnr_ADMVER;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
	if(ADM_VERSION != RegParam.val)
		return BRDerr_HW_ERROR;

/*
	PULONG pBuf = (PULONG)m_pSDRAM;
	ULONG buf_mem[128];
	int kkk = 0;
	for(kkk = 0; kkk < 128; kkk++)
		pBuf[kkk] = kkk;
//	while(1)
//	{
////		RegParam.reg = MAINnr_SIG;
////		Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
//		pBuf[0] = kkk++;
//		Sleep(100);
//	}
	while(1)
	{
		for(kkk = 0; kkk < 128; kkk++)
			buf_mem[kkk] = pBuf[kkk];
		Sleep(100);
	}
	*/

	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::DspPldWorkAndCheck()
{
//	ULONG Status;
	U32 val = 1;
	WriteRegDataDir(1, ADM2IFnt_MAIN, ADM2IFnr_DATA, val);
	ReadRegDataDir(1, ADM2IFnt_MAIN, ADM2IFnr_STATUS, val);
	ReadRegData(1, ADM2IFnt_MAIN, ADM2IFnr_ID, val);
	val = 1;
	WriteRegData(1, ADM2IFnt_MAIN, ADM2IFnr_MODE0, val);
	val = 0;
	WriteRegData(1, ADM2IFnt_MAIN, ADM2IFnr_MODE0, val);
/*
	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = 1;
	RegParam.tetr = ADM2IFnt_MAIN;
	RegParam.val = 1; // переключить ПЛИС в рабочий режим
//	RegParam.reg = ADM2IFnr_DATA | 0x80000000;
//	Status = RegCtrl(DEVScmd_REGWRITE, &RegParam);
	RegParam.reg = ADM2IFnr_DATA;
	Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
	RegParam.reg = ADM2IFnr_STATUS;
	Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
	RegParam.reg = ADM2IFnr_ID;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_IDMOD;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_VER;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_TRES;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_FSIZE;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_FTYPE;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_PATH;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = ADM2IFnr_IDNUM;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);

	RegParam.reg = MAINnr_SIG;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
//	if(INSYS_VENDOR_ID != RegParam.val)
	if(BASE_ID_TAG != RegParam.val)
		return BRDerr_HW_ERROR;
	RegParam.reg = MAINnr_ADMVER;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
	if(ADM_VERSION != RegParam.val)
		return BRDerr_HW_ERROR;*/
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::SetAdmPldFileInfo()
{
	ULONG Status;
	m_pldFileInfo.pldId = m_AdmPldInfo.Id;
	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = 0;
	RegParam.tetr = ADM2IFnt_MAIN;
	RegParam.reg = MAINnr_FPGAVER;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.version = RegParam.val;
	RegParam.reg = MAINnr_FPGAMODE;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.modification = RegParam.val;
//	RegParam.reg = MAINnr_TMASK;
//	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
//	RegParam.reg = MAINnr_MRES;
//	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = MAINnr_BASEID;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.baseId = RegParam.val;
	RegParam.reg = MAINnr_BASEVER;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.baseVer = RegParam.val;
	RegParam.reg = MAINnr_SMODID;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.submodId = RegParam.val;
	RegParam.reg = MAINnr_SMODVER;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.submodVer = RegParam.val;
	RegParam.reg = MAINnr_FPGABLD;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if(Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.build = RegParam.val;
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::GetPldInfo(PBRDextn_PLDINFO pInfo)
{
	SetAdmPldFileInfo();
	if(pInfo->pldId == m_pldFileInfo.pldId)
	{
		pInfo->version = m_pldFileInfo.version;
		pInfo->modification = m_pldFileInfo.modification;
		pInfo->build = m_pldFileInfo.build;
	}
	else
//		return ErrorPrint( BRDerr_BAD_PARAMETER,
//							CURRFILE,
//							_BRDC("<GetPldInfo> PLD ID is wrong"));
		return ErrorPrintf( BRDerr_BAD_PARAMETER,
							CURRFILE,
						  _BRDC("<GetPldInfo> PLD ID is wrong (%X != %X)"), pInfo->pldId, m_pldFileInfo.pldId);
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::GetPldFileInfo(PBRDextn_PLDFILEINFO pInfo)
{
	SetAdmPldFileInfo();
	if(pInfo->pldId == m_pldFileInfo.pldId)
		GetAdmPldFileInfo(pInfo);
	else
		return ErrorPrintf( BRDerr_BAD_PARAMETER,
							CURRFILE,
							//_BRDC("<GetPldFileInfo> PLD ID is wrong"));
						  _BRDC("<GetPldFileInfo> PLD ID is wrong (%X != %X)"), pInfo->pldId, m_pldFileInfo.pldId);
	return BRDerr_OK;
}

//***************************************************************************************
void CBambpci::GetAdmPldFileInfo(PBRDextn_PLDFILEINFO pInfo) const
{
//	pInfo->pldId = m_pldFileInfo.pldId;
	pInfo->version = m_pldFileInfo.version;
	pInfo->modification = m_pldFileInfo.modification;
	pInfo->build = m_pldFileInfo.build;
	pInfo->baseId = m_pldFileInfo.baseId;
	pInfo->baseVer = m_pldFileInfo.baseVer;
	pInfo->submodId = m_pldFileInfo.submodId;
	pInfo->submodVer = m_pldFileInfo.submodVer;
}

//***************************************************************************************
ULONG CBambpci::ShowDialog(PBRDextn_PropertyDlg pDlg)
{
#ifdef _WIN32
	if(pDlg->ListCnt)
	{
//		HINSTANCE hLib = LoadLibrary("BambpDlg.dll");
		HINSTANCE hLib = LoadLibrary(m_pDlgDllName);
		if(hLib <= (HINSTANCE)HINSTANCE_ERROR)
			return BRDerr_BAD_DEVICE_VITAL_DATA;

		AMBINFO_DlgProp_Type* pDlgProp;
		pDlgProp = (AMBINFO_DlgProp_Type*)GetProcAddress(hLib, "AMBINFO_DialogProperty");
		if(!pDlgProp)
		{
			FreeLibrary(hLib);
			return BRDerr_BAD_DEVICE_VITAL_DATA;
		}
		int nResponse = (pDlgProp)(this, pDlg->DlgMode, pDlg->ListCnt, pDlg->pList, pDlg->pChangeFlag, pDlg->pCurPage);
		if (nResponse == IDOK)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with OK
//			SetPropertyFromDialog(pDev, pInfo);
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with Cancel
		}
		pDlg->hLib = NULL;
		if(pDlg->DlgMode & 1)
			FreeLibrary(hLib);
		else
			pDlg->hLib = hLib;
	}
	else
	{
		return ErrorPrint(BRDerr_BAD_PARAMETER,
						  CURRFILE,
						  _BRDC("<ShowDialog> Not Property Pages for Dialog"));
	}
#endif
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::DeleteDialog(PBRDextn_PropertyDlg pDlg)
{
#ifdef _WIN32
	if(pDlg->hLib)
	{
		AMBINFO_DelDlg_Type* pDelDlg;
		pDelDlg = (AMBINFO_DelDlg_Type*)GetProcAddress(pDlg->hLib, "AMBINFO_DeleteDialog");
		int nResponse = (pDelDlg)(this, pDlg->DlgMode, pDlg->ListCnt, pDlg->pList, pDlg->pChangeFlag, pDlg->pCurPage);
		FreeLibrary(pDlg->hLib);
		pDlg->hLib = NULL;
	}
	else
	{
		return ErrorPrint(BRDerr_BAD_PARAMETER,
						  CURRFILE,
						  _BRDC("<DeleteDialog> Error HINSTANCE Dialog library"));
	}
#endif
	return BRDerr_OK;
}

//***************************************************************************************
// определение наличия и размера ICR-данных, записанных в ППЗУ базового модуля
//***************************************************************************************
ULONG CBambpci::GetBaseIcrDataSize()
{
	USHORT buf_read[3];
	buf_read[0] = buf_read[1] = buf_read[2] = 0xffff;
	ULONG ret_status = ReadNvRAM(&buf_read, 6, 128);
	if(ret_status == BRDerr_OK)
	{
		if(buf_read[0] == BASE_ID_TAG)
			m_sizeICR = buf_read[2];
		else
			m_sizeICR = 0;
		return ret_status;
	}
	else
		return ret_status;
}

//***************************************************************************************
ULONG CBambpci::GetBaseIcrStatus(ULONG& Status)
{
	USHORT buf_read;
	buf_read = 0xffff;
	ULONG ret_status = ReadNvRAM(&buf_read, 2, 128);
	if(ret_status == BRDerr_OK)
	{
//		if(buf_read == INSYS_VENDOR_ID)
		if(buf_read == BASE_ID_TAG)
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
//***************************************************************************************
ULONG CBambpci::GetAdmIcrStatus(ULONG& Status)
{
	Status = 0;
	if(m_Subunit[0].pSubEEPROM)
	{
		USHORT buf_read = *(PUSHORT)m_Subunit[0].pSubEEPROM;
		if(buf_read == ADM_ID_TAG)
			Status = 1;
	}
	return BRDerr_OK;

	//USHORT buf_read;
	//buf_read = 0xffff;
	//ULONG ret_status = ReadAdmIdROM(&buf_read, 2, 0);
	//if(ret_status == BRDerr_OK)
	//{
	//	if(buf_read == ADM_ID_TAG)
	//		Status = 1;
	//	else
	//		Status = 0;
	//	return ret_status;
	//}
	//else
	//	return ret_status;
}

//***************************************************************************************
void CBambpci::GetAmbpexCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAmbpex pCfgBase)
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
			RealBaseCfgSize = idBase.wSizeAll;
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

//***************************************************************************************
void CBambpci::GetSdramCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDdrSdram pCfgSdram)
{
	PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
	int idxMem = 0;
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
		case DDRSDRAM_CFG_TAG:
		{
			PICR_CfgDdrSdram pSdramCfg = (PICR_CfgDdrSdram)pCfgMem;
			memcpy(pCfgSdram + idxMem, pSdramCfg, sizeof(ICR_CfgDdrSdram));
			idxMem++;
			size += sizeof(ICR_CfgDdrSdram);
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

//***************************************************************************************
ULONG CBambpci::SetSdramConfig(PBASE_INI pIniData, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
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
	if( (m_DeviceID == VK3_DEVID) ||
		(m_DeviceID == AMBPEX2_DEVID))
	{
		m_MemCfg[0].SlotCnt = 1;
		//m_MemCfg[0].ModuleCnt = 1;
		//return BRDerr_OK;
	}
	else
	{
		//PUCHAR pBaseCfgMem = new UCHAR[MAX_BASE_CFGMEM_SIZE];
		//ULONG BaseCfgSize = MAX_BASE_CFGMEM_SIZE;
		//if(GetBaseEEPROM(pBaseCfgMem, BaseCfgSize) == BRDerr_OK)
		if(BaseEEPROMSize)
		{
			int SdramCfgCnt = 0;
			int SdramSlotCnt = 0;
			if(m_DeviceID == AMBPEX1_DEVID)
			{
				ICR_CfgAmbpex icr_base;
				GetAmbpexCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &icr_base);
				m_SysGen = icr_base.dSysGen;
			}
			else
			{
				if(m_DeviceID == C520F1_DEVID)
				{
					ICR_CfgC520F1 icr_base;
					GetAmbpCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &icr_base);
					m_SysGen = 125000000;
					SdramCfgCnt = 0;
					SdramSlotCnt = 0;
				}
				else
				{
					if( m_DeviceID != RFDR4_DEVID  && m_DeviceID != SYNCCP6R_DEVID && m_DeviceID != SYNCCP6_DEVID)
					{
						ICR_CfgAmbp icr_base;
						GetAmbpCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &icr_base);
						m_SysGen = icr_base.dSysGen;
						SdramCfgCnt = icr_base.bSdramCfgCnt;
						SdramSlotCnt = icr_base.bSdramSlotCnt;
					}
				}
			}
			ICR_CfgDdrSdram icr_memory[MAX_MEMORY_SRV];
	//		GetSdramCfgICR(pBaseCfgMem, BaseCfgSize, icr_memory);
			GetSdramCfgICR(pBaseEEPROMMem, BaseEEPROMSize, icr_memory);
	//		for(int iMem = 0; iMem < MAX_MEMORY_SRV; iMem++)
			for(int iMem = 0; iMem < SdramCfgCnt; iMem++)
			{
				m_MemCfg[iMem].SlotCnt = SdramSlotCnt;
				m_MemCfg[iMem].ModuleCnt = icr_memory[iMem].bModuleCnt;
				m_MemCfg[iMem].ModuleBanks = icr_memory[iMem].bModuleBanks;
				m_MemCfg[iMem].RowAddrBits = icr_memory[iMem].bRowAddrBits;
				m_MemCfg[iMem].ColAddrBits = icr_memory[iMem].bColAddrBits;
				m_MemCfg[iMem].ChipBanks = icr_memory[iMem].bChipBanks;
				m_MemCfg[iMem].PrimWidth = icr_memory[iMem].bPrimaryWidth;
				m_MemCfg[iMem].CasLat = icr_memory[iMem].bCasLatency;
			}
		}
	//	delete pBaseCfgMem;
	}
	if(pIniData)
	{	// уточняем конфигурацию из источника инициализации
		if(pIniData->SysGen != 0xffffffff)
			m_SysGen = pIniData->SysGen;
		for(int iMem = 0; iMem < MAX_MEMORY_SRV; iMem++)
		{
			if(BRDC_strlen(m_pIniData->MemIni[iMem].DlgDllName))
				BRDC_strcpy(m_MemCfg[iMem].DlgDllName, m_pIniData->MemIni[iMem].DlgDllName);
			if(pIniData->MemIni[iMem].SlotCnt != 0xffffffff)
				m_MemCfg[iMem].SlotCnt = pIniData->MemIni[iMem].SlotCnt;
			if(pIniData->MemIni[iMem].ModuleCnt != 0xffffffff)
				m_MemCfg[iMem].ModuleCnt = pIniData->MemIni[iMem].ModuleCnt;
			if(pIniData->MemIni[iMem].ModuleBanks != 0xffffffff)
				m_MemCfg[iMem].ModuleBanks = pIniData->MemIni[iMem].ModuleBanks;
			if(pIniData->MemIni[iMem].RowAddrBits != 0xffffffff)
				m_MemCfg[iMem].RowAddrBits = pIniData->MemIni[iMem].RowAddrBits;
			if(pIniData->MemIni[iMem].ColAddrBits != 0xffffffff)
				m_MemCfg[iMem].ColAddrBits = pIniData->MemIni[iMem].ColAddrBits;
			if(pIniData->MemIni[iMem].ChipBanks != 0xffffffff)
				m_MemCfg[iMem].ChipBanks = pIniData->MemIni[iMem].ChipBanks;
			if(pIniData->MemIni[iMem].PrimWidth != 0xffffffff)
				m_MemCfg[iMem].PrimWidth = pIniData->MemIni[iMem].PrimWidth;
			if(pIniData->MemIni[iMem].CasLatency != 0xffffffff)
				m_MemCfg[iMem].CasLat = pIniData->MemIni[iMem].CasLatency;
			if(pIniData->MemIni[iMem].Attributes != 0xffffffff)
				m_MemCfg[iMem].Attributes = pIniData->MemIni[iMem].Attributes;
		}
	}
	return BRDerr_OK;
}

//***************************************************************************************
void CBambpci::GetDspNodeCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDspNode pCfgDspNode, PICR_CfgSram pCfgSram)
{
	PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
	int idxMem = 0, idxDsp = 0;
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
		case DSPNODE_CFG_TAG:
		{
			PICR_CfgDspNode pDspNodeCfg = (PICR_CfgDspNode)pCfgMem;
			memcpy(pCfgDspNode + idxDsp, pDspNodeCfg, sizeof(ICR_CfgDspNode));
			idxDsp++;
			size += sizeof(ICR_CfgDspNode);
			break;
		}
		case SRAM_CFG_TAG:
		{
			PICR_CfgSram pSramCfg = (PICR_CfgSram)pCfgMem;
			memcpy(pCfgSram + idxMem, pSramCfg, sizeof(ICR_CfgSram));
			idxMem++;
			size += sizeof(ICR_CfgSram);
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

//***************************************************************************************
ULONG CBambpci::SetDspNodeConfig(PBASE_INI pIniData, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
		// устанавливаем конфигурацию по умолчанию
	for(int i = 0; i < MAX_DSPNODE_SRV; i++)
		memcpy(&m_DspCfg[i], &DspNodeCfg_dflt, sizeof(DSPNODE_CFG));

	//PUCHAR pBaseCfgMem = new UCHAR[MAX_BASE_CFGMEM_SIZE];
	//ULONG BaseCfgSize = MAX_BASE_CFGMEM_SIZE;
	//if(GetBaseEEPROM(pBaseCfgMem, BaseCfgSize) == BRDerr_OK)
	if(BaseEEPROMSize)
	{
//		ICR_CfgAmbp icr_base;
////		GetBaseCfgICR(pBaseCfgMem, BaseCfgSize, &icr_base);
//		GetBaseCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &icr_base);
		ICR_CfgDspNode icr_dspnode[MAX_DSPNODE_SRV];
		ICR_CfgSram icr_sram[MAX_DSPNODE_SRV];
//		GetDspNodeCfgICR(pBaseCfgMem, BaseCfgSize, icr_dspnode, icr_sram);
		GetDspNodeCfgICR(pBaseEEPROMMem, BaseEEPROMSize, icr_dspnode, icr_sram);
		for(int iNode = 0; iNode < MAX_DSPNODE_SRV; iNode++)
		{
			m_DspCfg[iNode].PldType = icr_dspnode[iNode].bPldType;
			m_DspCfg[iNode].PldVolume = icr_dspnode[iNode].wPldVolume;
			m_DspCfg[iNode].PldPins = icr_dspnode[iNode].wPldPins;
			m_DspCfg[iNode].PldSpeedGrade = icr_dspnode[iNode].bPldSpeedGrade;
			m_DspCfg[iNode].LoadRom = icr_dspnode[iNode].bLoadRom;
			m_DspCfg[iNode].PioType = icr_dspnode[iNode].bPioType;
			if(icr_dspnode[iNode].bSramCfgCnt)
			{
				m_DspCfg[iNode].SramChipCnt = icr_sram[iNode].bChipCnt;
				m_DspCfg[iNode].SramChipDepth = icr_sram[iNode].bChipDepth;
				m_DspCfg[iNode].SramChipBitsWidth = icr_sram[iNode].bChipBitsWidth;
			}
		}
	}
//	delete pBaseCfgMem;
	if(pIniData)
	{	// уточняем конфигурацию из источника инициализации
		for(int iNode = 0; iNode < MAX_DSPNODE_SRV; iNode++)
		{
			if(pIniData->DspIni[iNode].PldType != 0xffffffff)
				m_DspCfg[iNode].PldType = pIniData->DspIni[iNode].PldType;
			if(pIniData->DspIni[iNode].PldVolume != 0xffffffff)
				m_DspCfg[iNode].PldVolume = pIniData->DspIni[iNode].PldVolume;
			if(pIniData->DspIni[iNode].PldPins != 0xffffffff)
				m_DspCfg[iNode].PldPins = pIniData->DspIni[iNode].PldPins;
			if(pIniData->DspIni[iNode].PldSpeedGrade != 0xffffffff)
				m_DspCfg[iNode].PldSpeedGrade = pIniData->DspIni[iNode].PldSpeedGrade;
			if(pIniData->DspIni[iNode].LoadRom != 0xffffffff)
				m_DspCfg[iNode].LoadRom = pIniData->DspIni[iNode].LoadRom;
			if(pIniData->DspIni[iNode].PioType != 0xffffffff)
				m_DspCfg[iNode].PioType = pIniData->DspIni[iNode].PioType;
			if(pIniData->DspIni[iNode].SramChipCnt != 0xffffffff)
				m_DspCfg[iNode].SramChipCnt = pIniData->DspIni[iNode].SramChipCnt;
			if(pIniData->DspIni[iNode].SramChipDepth != 0xffffffff)
				m_DspCfg[iNode].SramChipDepth = pIniData->DspIni[iNode].SramChipDepth;
			if(pIniData->DspIni[iNode].SramChipBitsWidth != 0xffffffff)
				m_DspCfg[iNode].SramChipBitsWidth = pIniData->DspIni[iNode].SramChipBitsWidth;
		}
	}
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::GetAdmIfCntICR(int& AdmIfCnt)
{
		// Check base module ICR
	ULONG icr_base_status;
	if(GetBaseIcrStatus(icr_base_status) != BRDerr_OK)
//		return BRDerr_HW_ERROR;
		return ErrorPrint(BRDerr_HW_ERROR,
						  CURRFILE,
						  _BRDC("<GetAdmIfCntICR> Hardware error by get base ICR status"));
	if(icr_base_status)
	{	// получаем конфигурацию из ППЗУ
		PUCHAR pBaseCfgMem = new UCHAR[MAX_BASE_CFGMEM_SIZE];
		ULONG BufferSize = MAX_BASE_CFGMEM_SIZE;
		ULONG Offset = OFFSET_BASE_CFGMEM;
		if(ReadNvRAM(pBaseCfgMem, BufferSize, Offset) != BRDerr_OK)
//			return BRDerr_BAD_DEVICE_VITAL_DATA;
			return ErrorPrint(BRDerr_BAD_DEVICE_VITAL_DATA,
							  CURRFILE,
							  _BRDC("<GetAdmIfCntICR> Bad device vital data by read base ICR"));
		ULONG dRealBaseCfgSize;
		if(*(PUSHORT)pBaseCfgMem == BASE_ID_TAG)
		{
			dRealBaseCfgSize = *((PUSHORT)pBaseCfgMem + 2);
			if(m_DeviceID == AMBPEX1_DEVID)
			{
				ICR_CfgAmbpex cfgBase;
				GetAmbpexCfgICR(pBaseCfgMem, dRealBaseCfgSize, &cfgBase);
				AdmIfCnt = cfgBase.bAdmIfCnt;
			}
			else
			{
				U08	mem_cfg[256];
				ICR_CfgAmbp *cfgBase = (ICR_CfgAmbp *)mem_cfg;
				GetAmbpCfgICR(pBaseCfgMem, dRealBaseCfgSize, cfgBase);
				AdmIfCnt = cfgBase->bAdmIfCnt;
			}
		}
		else
//			return BRDerr_BAD_DEVICE_VITAL_DATA;
			return ErrorPrint(BRDerr_BAD_DEVICE_VITAL_DATA,
							  CURRFILE,
							  _BRDC("<GetAdmIfCntICR> Invalid BASE ID tag getted from ICR"));
		delete[] pBaseCfgMem;
	}
	return BRDerr_OK;
}
/*
// ***************************************************************************************
ULONG CBambpci::GetBaseEEPROM(PUCHAR pBaseCfgMem, ULONG& BaseCfgSize)
{
		// Check base module ICR
	ULONG icr_base_status;
	if(GetBaseIcrStatus(icr_base_status) != BRDerr_OK)
//		return BRDerr_HW_ERROR;
		return ErrorPrint(BRDerr_HW_ERROR,
						  CURRFILE,
						  "<GetBaseEEPROM> Hardware error by get base ICR status");
	if(icr_base_status)
	{	// получаем содержимое ППЗУ базового модуля
		ULONG BufferSize = MAX_BASE_CFGMEM_SIZE;
		ULONG Offset = OFFSET_BASE_CFGMEM;
		if(ReadNvRAM(pBaseCfgMem, BufferSize, Offset) != BRDerr_OK)
			return BRDerr_BAD_DEVICE_VITAL_DATA;
		if(*(PUSHORT)pBaseCfgMem == BASE_ID_TAG)
		{
			BaseCfgSize = *((PUSHORT)pBaseCfgMem + 2);
			if(!BaseCfgSize)
				return BRDerr_BAD_DEVICE_VITAL_DATA;
		}
		else
			return BRDerr_BAD_DEVICE_VITAL_DATA;
	}
	else
		return BRDerr_BAD_DEVICE_VITAL_DATA;
	return BRDerr_OK;
}
*/
//***************************************************************************************
ULONG CBambpci::GetBaseEEPROM(PUCHAR pBaseCfgMem, ULONG& BaseCfgSize)
{
	if(BaseCfgSize)
	{	// получаем содержимое ППЗУ базового модуля
		ULONG BufferSize = BaseCfgSize;
		ULONG Offset = OFFSET_BASE_CFGMEM;
		if(ReadNvRAM(pBaseCfgMem, BufferSize, Offset) != BRDerr_OK)
			return BRDerr_BAD_DEVICE_VITAL_DATA;
	}
	else
		return BRDerr_BAD_DEVICE_VITAL_DATA;
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpci::SetSubEeprom(int idxSub, ULONG& SubCfgSize)
{
	if(!m_Subunit[idxSub].pSubEEPROM)
	{
		TCHAR nameFileMap[MAX_PATH];
		BRDC_sprintf(nameFileMap, _BRDC("subeeprom_%s_%d_%d"), m_name, m_PID, idxSub);
		//m_Subunit[idxSub].hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,
		//												NULL, PAGE_READWRITE,
		//												0, SUBMOD_CFGMEM_SIZE,
		//												nameFileMap);
		//int isAlreadyCreated = ( GetLastError() == ERROR_ALREADY_EXISTS ) ? 1 : 0;
		//m_Subunit[idxSub].pSubEEPROM = (PVOID)MapViewOfFile(m_Subunit[idxSub].hFileMap,
		//													FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
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
//			ULONG Status = ReadAdmIdROM(pSubCfgMem, 4, 0);
//			if(Status != BRDerr_OK)
//				return BRDerr_HW_ERROR;
			// получаем конфигурацию субмодуля из его ППЗУ
			//ULONG Status = ReadAdmIdROM(m_Subunit[idxSub].pSubEEPROM, SUBMOD_CFGMEM_SIZE, 0);
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

//***************************************************************************************
ULONG CBambpci::GetSubEEPROM(int idxSub, PUCHAR pSubCfgMem, ULONG& SubCfgSize)
{
	// Check subunit ICR
	ULONG icr_adm_status;
	if(GetAdmIcrStatus(icr_adm_status) != BRDerr_OK)
		return BRDerr_HW_ERROR;
	if(icr_adm_status)
	{	// получаем конфигурацию субмодуля из его ППЗУ
		memcpy(pSubCfgMem, m_Subunit[idxSub].pSubEEPROM, SubCfgSize);
//		if(ReadAdmIdROM(pSubCfgMem, SubCfgSize, 0) != BRDerr_OK)
//			return BRDerr_BAD_DEVICE_VITAL_DATA;
		if(*(PUSHORT)pSubCfgMem == ADM_ID_TAG)
		{
			SubCfgSize = *((PUSHORT)pSubCfgMem + 2);
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

//***************************************************************************************
ULONG CBambpci::SetServices()
{
	DeleteServices();
	int iSrv = 0, iMemorySrv = 0, iTestSrv = 0, iStreamSrv = 0, iSysMonSrv = 0;
	for(int iTst = 0; iTst < MAX_TEST_SRV; iTst++)
	{
		TESTSRV_CFG tst_srv_cfg;
		tst_srv_cfg.isAlreadyInit = 0;
		m_pTestSrv[iTestSrv] = new CTestSrv(iSrv++, iTestSrv, this, &tst_srv_cfg);
		m_SrvList.push_back(m_pTestSrv[iTestSrv++]);
	}
	for(int iMem = 0; iMem < MAX_MEMORY_SRV; iMem++)
	{
//		m_pMemorySrv[iMemorySrv] = new CSdramSrv(iSrv++, iMemorySrv, this, &mem_srv_cfg);
		if(AMBPCD_DEVID == m_DeviceID ||
			AMBPEX2_DEVID == m_DeviceID||
			AMBPEX1_DEVID == m_DeviceID ||
			VK3_DEVID == m_DeviceID)
		{
			SDRAMAMBPCDSRV_CFG mem_srv_cfg;
			BRDC_strcpy(mem_srv_cfg.SdramCfg.DlgDllName, m_MemCfg[iMem].DlgDllName);
//			mem_srv_cfg.SdramCfg.pPropDlg = DEVS_propDlg;
			mem_srv_cfg.SdramCfg.isAlreadyInit = 0;
			mem_srv_cfg.SdramCfg.SlotCnt = m_MemCfg[iMem].SlotCnt;
			mem_srv_cfg.SdramCfg.ModuleCnt = m_MemCfg[iMem].ModuleCnt;
			mem_srv_cfg.SdramCfg.ModuleBanks = m_MemCfg[iMem].ModuleBanks;
			mem_srv_cfg.SdramCfg.RowAddrBits = m_MemCfg[iMem].RowAddrBits;
			mem_srv_cfg.SdramCfg.ColAddrBits = m_MemCfg[iMem].ColAddrBits;
			mem_srv_cfg.SdramCfg.ChipBanks = m_MemCfg[iMem].ChipBanks;
			mem_srv_cfg.PrimWidth = m_MemCfg[iMem].PrimWidth;
			mem_srv_cfg.CasLatency = m_MemCfg[iMem].CasLat;
			mem_srv_cfg.Attributes = m_MemCfg[iMem].Attributes;
			m_pMemorySrv[iMemorySrv] = new CSdramAmbpcdSrv(iSrv++, iMemorySrv, this, &mem_srv_cfg);
            m_pMemorySrv[iMemorySrv]->SetPropDlg((void*)DEVS_propDlg);
		}
		else
		{
			SDRAMSRV_CFG mem_srv_cfg;
			BRDC_strcpy(mem_srv_cfg.DlgDllName, m_MemCfg[iMem].DlgDllName);
//			mem_srv_cfg.pPropDlg = DEVS_propDlg;
			mem_srv_cfg.isAlreadyInit = 0;
			mem_srv_cfg.SlotCnt = m_MemCfg[iMem].SlotCnt;
			mem_srv_cfg.ModuleCnt = m_MemCfg[iMem].ModuleCnt;
			mem_srv_cfg.ModuleBanks = m_MemCfg[iMem].ModuleBanks;
			mem_srv_cfg.RowAddrBits = m_MemCfg[iMem].RowAddrBits;
			mem_srv_cfg.ColAddrBits = m_MemCfg[iMem].ColAddrBits;
			mem_srv_cfg.ChipBanks = m_MemCfg[iMem].ChipBanks;
			m_pMemorySrv[iMemorySrv] = new CSdramAmbpcxSrv(iSrv++, iMemorySrv, this, &mem_srv_cfg);
            m_pMemorySrv[iMemorySrv]->SetPropDlg((void*)DEVS_propDlg);
		}
		m_SrvList.push_back(m_pMemorySrv[iMemorySrv++]);
	}
	for(int iStrm = 0; iStrm < MAX_STREAM_SRV; iStrm++)
	{
		STREAMSRV_CFG strm_srv_cfg;
		m_pStreamSrv[iStreamSrv] = new CStreamSrv(iSrv++, iStreamSrv, this, &strm_srv_cfg);
		m_SrvList.push_back(m_pStreamSrv[iStreamSrv++]);
	}
	for(int iSysMon = 0; iSysMon < MAX_SYSMON_SRV; iSysMon++)
	{
		SYSMONSRV_CFG sysmon_srv_cfg;
		sysmon_srv_cfg.isAlreadyInit = 0;
		sysmon_srv_cfg.DeviceID = m_DeviceID;
		sysmon_srv_cfg.PldType = m_pldType;
		m_pSysMonSrv[iSysMonSrv] = new CSysMonSrv(iSrv++, iSysMonSrv, this, &sysmon_srv_cfg);
		m_SrvList.push_back(m_pSysMonSrv[iSysMonSrv++]);
	}
	return BRDerr_OK;
}

//******************************************
void CBambpci::DeleteServices()
{
	for(int i = 0; i < MAX_SYSMON_SRV; i++)
	{
		if(m_pSysMonSrv[i]) {
			delete m_pSysMonSrv[i];
			m_pSysMonSrv[i] = NULL;
		}
	}
	for(int i = 0; i < MAX_TEST_SRV; i++)
	{
		if(m_pTestSrv[i]) {
			delete m_pTestSrv[i];
			m_pTestSrv[i] = NULL;
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

//***************************************************************************************
void CBambpci::SetCandidateSrv()
{
	int srv_num = (int)m_ServInfoList.size();
//	int srv_num = m_SrvList.size();
	BRDCHAR* pchar;
	for(int i = 0; i < srv_num; i++)
	{
		ULONG attr = m_ServInfoList[i]->attribute;
//		LPTSTR srv_name = m_ServInfoList[i]->name;
//		ULONG attr = m_SrvList[i]->GetAttribute();
		int ius = 0;
		if(attr & BRDserv_ATTR_CMPABLE)
		{
			int j = 0;
			do {
				pchar = BRDC_strstr(m_ServInfoList[j++]->name, _BRDC("CMPSC"));
//				LPTSTR srv_name = m_SrvList[j++]->GetName();
//				pchar = _tcsstr(srv_name, "CMP");
			}while(!pchar && j < srv_num);
			if(pchar)
				m_ServInfoList[i]->idxCandidSrv[ius++] = j - 1;
//				m_SrvList[i]->SetCandidIndex(ius++, j - 1);
		}
		if(attr & BRDserv_ATTR_STREAMABLE_OUT)
		{
			for(int j = 0; j < srv_num; j++)
			{
				if( (m_ServInfoList[j]->attribute & BRDserv_ATTR_STREAM) &&
					(m_ServInfoList[j]->attribute & BRDserv_ATTR_DIRECTION_OUT))
					m_ServInfoList[i]->idxCandidSrv[ius++] = j;
//				if( (m_SrvList[j]->GetAttribute() & BRDserv_ATTR_STREAM) &&
//					(m_SrvList[j]->GetAttribute() & BRDserv_ATTR_DIRECTION_OUT))
//					m_SrvList[i]->SetCandidIndex(ius++, j);
			}
		}
		if(attr & BRDserv_ATTR_STREAMABLE_IN)
		{
			for(int j = 0; j < srv_num; j++)
			{
				if( (m_ServInfoList[j]->attribute & BRDserv_ATTR_STREAM) &&
					(m_ServInfoList[j]->attribute & BRDserv_ATTR_DIRECTION_IN))
					m_ServInfoList[i]->idxCandidSrv[ius++] = j;
//				if( (m_SrvList[j]->GetAttribute() & BRDserv_ATTR_STREAM) &&
//					(m_SrvList[j]->GetAttribute() & BRDserv_ATTR_DIRECTION_IN))
//					m_SrvList[i]->SetCandidIndex(ius++, j);
			}
		}
		if(attr & BRDserv_ATTR_SDRAMABLE)
		{
			for(int j = 0; j < srv_num; j++)
			{
//				if(m_ServInfoList[j]->attribute & BRDserv_ATTR_SDRAM)
				{
					pchar = BRDC_strstr(m_ServInfoList[j]->name, _BRDC("BASESDRAM"));
					if(pchar)
					{
						m_ServInfoList[i]->idxCandidSrv[ius++] = j;
						break;
					}
				}
			}
		}
		if(attr & BRDserv_ATTR_DSPABLE)
		{
			for(int j = 0; j < srv_num; j++)
			{
//				if(m_ServInfoList[j]->attribute & BRDserv_ATTR_DSPNODE)
				{
					pchar = BRDC_strstr(m_ServInfoList[j]->name, _BRDC("DSPNODE"));
					if(pchar)
					{
						m_ServInfoList[i]->idxCandidSrv[ius++] = j;
						break;
					}
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
//		if(attr & BRDserv_ATTR_TESTABLE)
//		{
//			for(int j = 0; j < srv_num; j++)
//			{
//				pchar = _tcsstr(m_ServInfoList[j]->name, "TEST");
//				if(pchar)
//				{
//					m_ServInfoList[i]->idxCandidSrv[ius++] = j;
//					break;
//				}
//			}
//		}
	}
}

#ifndef __linux__
#include <conio.h>
#endif

//***************************************************************************************
LONG CBambpci::RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam)
{
	BRDCHAR buf[SERVNAME_SIZE];
	ULONG admif = 0;
	if(m_DeviceID == AMBPCD_DEVID)
	{
		BRDC_strcpy(buf, m_ServInfoList[pRegParam->idxMain]->name);
	//	PTCHAR pBuf = buf + (strlen(buf) - 2);
	//	if(_tcsstr(pBuf, _BRDC("DSPNODE")))
		if(BRDC_strstr(buf, _BRDC("DSPNODE")))
			admif = 1;
	}
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
			//if(!pRegParam->reg)
			//	printf("MODE0 = 0X%04X\n", pRegParam->val);
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
			//if(!pRegParam->reg)
			//{
			//	printf("MODE0 = 0X%04X\n", pRegParam->val);
			//	printf("Press any key for leaving program...\n");
			//	_getch();
			//}
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
			status = SetStatusIrq(admif, pRegParam->tetr, pRegParam->reg, pRegParam->val, pRegParam->pBuf);
			break;
		case DEVScmd_CLEARSTATIRQ:
			status = ClearStatusIrq(admif, pRegParam->tetr, pRegParam->reg, pRegParam->val, pRegParam->pBuf);
			break;
		case DEVScmd_GETBASEADR:
		{   // FIXME: не реализована в bambpci
			PULONG* pBar = (PULONG*)pRegParam;
			pBar[0] = nullptr;
			pBar[1] = nullptr;
			break;
		}
		default:
//			return BRDerr_CMD_UNSUPPORTED;
			return ErrorPrint(BRDerr_CMD_UNSUPPORTED,
							  CURRFILE,
							  _BRDC("<RegCtrl> Command not supported"));
	}

#ifdef _WIN32
	if(ERROR_SEM_TIMEOUT == status)
//			return ErrorPrint(BRDerr_WAIT_TIMEOUT,
//							  CURRFILE,
//							  "<RegCtrl> The time-out interval elapsed");
			return ErrorPrintf(BRDerr_WAIT_TIMEOUT,
							  CURRFILE,
							  _BRDC("<RegCtrl> Tetr = 0x%04X, Reg = 0x%04X, The time-out interval elapsed"), pRegParam->tetr, pRegParam->reg);

	if(BRDerr_OK != status)
		return BRDerr_HW_ERROR;
	return BRDerr_OK;
#endif
	return status;
}

//********************** StreamCtrl *****************************************************
// управление стримами
//***************************************************************************************
ULONG CBambpci::StreamCtrl(ULONG cmd, PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
    //DEVS_CMD_Reg RegParam;
    //RegParam.idxMain = 0;
	switch(cmd)
	{
		case STRMcmd_SETMEMORY:
		{
			//SERV_CMD_IsAvailable srvAvail;
			//int idx	 = (m_curHandle >> 16) & 0x3F;
			//if(idx < (long)m_SrvList.size())
			//	Status = m_SrvList[idx]->DoCmd(this, SERVcmd_SYS_ISAVAILABLE, 0, &srvAvail, NULL);
			//else
			//{
			//	DEV_CMD_Ctrl param;
			//	param.handle = m_curHandle;
			//	param.cmd = SERVcmd_SYS_ISAVAILABLE;
			//	param.arg = &srvAvail;
			//	if(m_ServInfoList[idx]->baseORsideDLL)
			//	{
			//		SIDE_API_entry* pSIDE_Entry = (SIDE_API_entry*)m_ServInfoList[idx]->pSideEntry;
			//		Status = pSIDE_Entry(m_ServInfoList[idx]->pSideDll, NULL, SIDEcmd_CTRL, &param);
			//	}
			//	else
			//	{
			//		DEV_API_entry* pDEV_Entry = (DEV_API_entry*)m_ServInfoList[idx]->pSideEntry;
			//		Status = pDEV_Entry(m_ServInfoList[idx]->pSideDll, DEVcmd_CTRL, &param);
			//	}
			//}
			ULONG addr_data = 0;
			if(m_curHandle)
			{
				int idx	 = (m_curHandle >> 16) & 0x3F;
				if(idx < (long)m_SrvList.size())
					Status = m_SrvList[idx]->DoCmd(this, SERVcmd_SYS_GETADDRDATA, 0, &addr_data, NULL);
				else
				{
					DEV_CMD_Ctrl param;
					param.handle = m_curHandle;
					param.cmd = SERVcmd_SYS_GETADDRDATA;
					param.arg = &addr_data;
					if(m_ServInfoList[idx]->baseORsideDLL)
					{
						SIDE_API_entry* pSIDE_Entry = (SIDE_API_entry*)m_ServInfoList[idx]->pSideEntry;
						Status = pSIDE_Entry(m_ServInfoList[idx]->pSideDll, NULL, SIDEcmd_CTRL, &param);
					}
					else
					{
						DEV_API_entry* pDEV_Entry = (DEV_API_entry*)m_ServInfoList[idx]->pSideEntry;
						Status = pDEV_Entry(m_ServInfoList[idx]->pSideDll, DEVcmd_CTRL, &param);
					}
				}
				if(Status == BRDerr_OK)
					((PAMB_MEM_DMA_CHANNEL)pParam)->LocalAddr = addr_data;
				else
					((PAMB_MEM_DMA_CHANNEL)pParam)->LocalAddr = 0;
			}
			else
				((PAMB_MEM_DMA_CHANNEL)pParam)->LocalAddr = 0;
			//RegParam.tetr = 0;
			//RegParam.reg = 16;
			//ReadRegData(0, RegParam.tetr, RegParam.reg, RegParam.val);
			//ULONG val_tmp = RegParam.val;
			Status = SetMemory(pParam, sizeParam, pOverlap);
			//RegParam.tetr = tetr_num;
			//RegParam.reg = 0;
			//ReadRegData(0, RegParam.tetr, RegParam.reg, RegParam.val);
			//RegParam.val &= 0x0FFF;
			//WriteRegData(0, RegParam.tetr, RegParam.reg, RegParam.val);

//			ReadRegData(0, RegParam.tetr, RegParam.reg, RegParam.val);
//			printf("Before alloc memory SEL0=%x, after SEL0=%x\n", val_tmp, RegParam.val);
		}
			break;
		case STRMcmd_FREEMEMORY:
			Status = FreeMemory(pParam, sizeParam, pOverlap);
			break;
		case STRMcmd_STARTMEMORY:
		{
			//RegParam.tetr = 0;
			//RegParam.reg = 16;
			//ReadRegData(0, RegParam.tetr, RegParam.reg, RegParam.val);
			//ULONG sel_val = RegParam.val;
			//RegParam.tetr = 4;
			//RegParam.reg = 0;
			//ReadRegData(0, RegParam.tetr, RegParam.reg, RegParam.val);
			//ULONG mode0_val = RegParam.val;
			Status = StartMemory(pParam, sizeParam, pOverlap);
//			RegParam.tetr = 0;
//			RegParam.reg = 16;
//			ReadRegData(0, RegParam.tetr, RegParam.reg, RegParam.val);
//			ULONG tmp_val = RegParam.val;
//			RegParam.tetr = 4;
//			RegParam.reg = 0;
//			ReadRegData(0, RegParam.tetr, RegParam.reg, RegParam.val);
//			printf("Before start SEL0=%x MODE0=%x, after SEL0=%x MODE0=%x\n", sel_val, mode0_val, tmp_val, RegParam.val);
		}
			break;
		case STRMcmd_STOPMEMORY:
			Status = StopMemory(pParam, sizeParam, pOverlap);
			break;
		case STRMcmd_STATEMEMORY:
			Status = StateMemory(pParam, sizeParam, pOverlap);
			break;
		case STRMcmd_SETDIR:
			Status = SetDirection(pParam, sizeParam, pOverlap);
			break;
		case STRMcmd_SETSRC:
			Status = SetSource(pParam, sizeParam, pOverlap);
			break;
		case STRMcmd_SETDRQ:
			Status = SetDmaRequest(pParam, sizeParam, pOverlap);
			break;
#ifdef __linux__
		case STRMcmd_WAITDMABUFFER:
			Status = WaitDmaBuffer(pParam, sizeParam);
			break;
		case STRMcmd_WAITDMABLOCK:
			Status = WaitDmaBlock(pParam, sizeParam);
			break;
#endif
		default:
			Status = BRDerr_CMD_UNSUPPORTED;
	}
	return Status;
}

//***************** End of file bambpci.cpp *****************
