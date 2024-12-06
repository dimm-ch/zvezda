// clang-format off
#define	DEVS_API_EXPORTS

#include "bambusb.h"

#ifdef _WINDOWS
	#include <windows.h>
#endif

#define	CURRFILE _BRDC("BAMBUSB")

LONG GetMCSDataSize(const BRDCHAR *FileName)
{
	LONG nDataSize = 0;
	FILE *pFile;
	pFile = BRDC_fopen(FileName, _BRDC("rb"));
	if (pFile != 0)
	{
		UINT nOffset = 0;
		UINT nAddr = 0, nType = 0, nLength = 0, nAddrPrev = 0;
		while(!feof(pFile))
		{
			char str[256] = { 0 };
			nAddr = 0; nType = 0; nLength = 0;
			fgets(str, 256, pFile);
			if (feof(pFile))
				break;
			if (str[0] != ':')
			{
				if (feof(pFile))
					break;
				else
					return 0;
			}
			sscanf(str + 1, "%2x", &nLength);
			sscanf(str + 3, "%4x", &nAddr);
			sscanf(str + 7, "%2x", &nType);
			nLength = (nLength & 0xFF);
			nAddr = (nAddr & 0xFFFF);
			nType = (nType & 0xFF);
			if (nType == 4)
			{
				continue;
			}
			if (nType == 1)
			{
				break;
			}
			if ((nAddr != nAddrPrev) && (nAddrPrev != 0))
				nAddrPrev = nAddr;
			nAddrPrev = nAddr + nLength;
			nDataSize += nLength;
			if (feof(pFile))
				break;
		}
		BRDC_fclose(pFile);
	}
	return nDataSize;
}

typedef struct _SERVICE_THREAD_PARAM {
	U08* m_TaskID;
	U08 mEPin;
	CCyFX3Device* pDev;
	HANDLE hSemaphore;
} SERV_PARAM, *PSERV_PARAM;

HANDLE g_hServStream = NULL;

CBambusb::CBambusb() : CBaseModule(MAX_BASE_CFGMEM_SIZE)
{
	//BRDC_strcpy(m_pDlgDllName, _BRDC("BambpDlg.dll"));
	//m_pDataInSrv = NULL;
	//m_pDataOutSrv = NULL;
	for (int i = 0; i < MAX_TEST_SRV; i++)
		m_pTestSrv[i] = NULL;
	for (int i = 0; i < MAX_SYSMON_SRV; i++)
		m_pSysMonSrv[i] = NULL;
	for(int i = 0; i < MAX_MEMORY_SRV; i++)
		m_pMemorySrv[i] = NULL;
	for(int i = 0; i < MAX_STREAM_SRV; i++)
	{
		m_pStreamSrv[i] = NULL;
		m_pStreamDscr[i] = new USB_STREAM_DESC();
		m_pStreamDscr[i]->bEndPoint = 0xFF;
		m_pStreamDscr[i]->nTetrNum = 0xFF;
		m_pStreamDscr[i]->pBlock = 0;
		m_pStreamDscr[i]->nDone = -1;
		m_pStreamDscr[i]->isAdjust = 0;
	}
	m_pIniData = NULL;
	m_pDevice = new CCyFX3Device();
	m_rICRmap.hBaseICR = 0;
	m_rICRmap.hSubICR = 0;
	m_rICRmap.hFRUID = 0;
	m_RegLog = 0;
	m_isDLMode = 0;
	m_isArmProg = 0;
	m_TaskID = 0;
}

CBambusb::CBambusb(PBRD_InitData pBrdInitData, LONG sizeInitData) :
	CBaseModule(MAX_BASE_CFGMEM_SIZE)
{
	//m_pDataInSrv = NULL;
	//m_pDataOutSrv = NULL;
	for (int i = 0; i < MAX_TEST_SRV; i++)
		m_pTestSrv[i] = NULL;
	for (int i = 0; i < MAX_SYSMON_SRV; i++)
		m_pSysMonSrv[i] = NULL;
	for(int i = 0; i < MAX_MEMORY_SRV; i++)
		m_pMemorySrv[i] = NULL;
	for(int i = 0; i < MAX_STREAM_SRV; i++)
	{
		m_pStreamSrv[i] = NULL;
		m_pStreamDscr[i] = new USB_STREAM_DESC();
		m_pStreamDscr[i]->bEndPoint = 0xFF;
		m_pStreamDscr[i]->nTetrNum = 0xFF;
		m_pStreamDscr[i]->pBlock = 0;
		m_pStreamDscr[i]->nDone = -1;
		m_pStreamDscr[i]->isAdjust = 0;
	}
	m_rICRmap.hBaseICR = 0;
	m_rICRmap.hSubICR = 0;
	m_rICRmap.hFRUID = 0;
	m_pIniData = NULL;
	m_pDevice = new CCyFX3Device();
	m_pIniData = new BASE_INI;
	LONG size = sizeInitData;

	PINIT_Data pInitData = (PINIT_Data)pBrdInitData;
	// идентификация
	memset(m_pIniData->PID, 0, 8);
	FindKeyWord(_BRDC("prid"), m_pIniData->PRID, pInitData, size);
	FindKeyWord(_BRDC("vid"), m_pIniData->VID, pInitData, size);
	FindKeyWord(_BRDC("pid"), m_pIniData->PID, pInitData, size);
	FindKeyWord(_BRDC("reset"), m_pIniData->RESET, pInitData, size);
	FindKeyWord(_BRDC("isdebug"), m_pIniData->ISDEBUG, pInitData, size);
	FindKeyWord(_BRDC("fmcpower"), m_pIniData->FMCPOWER, pInitData, size);
	FindKeyWord(_BRDC("log"), m_pIniData->LOGGING, pInitData, size);
	FindKeyWord(_BRDC("loadtime"), m_pIniData->LOAD_WAIT, pInitData, size);
	FindKeyWord(_BRDC("enumtime"), m_pIniData->ENUM_WAIT, pInitData, size);
	FindKeyWord(_BRDC("reglog"), m_pIniData->REGLOG, pInitData, size);

	BRDCHAR key_name[64];
	for (int iMem = 0; iMem < MAX_MEMORY_SRV; iMem++)
	{
		if (iMem)
			BRDC_sprintf(key_name, _BRDC("memtetrnum%d"), iMem);
		else
			BRDC_strcpy(key_name, _BRDC("memtetrnum"));
		FindKeyWord(key_name, m_pIniData->MemIni[iMem].TetrNum, pInitData, size);

		// настройка адреса при разделении одной физической памяти между двумя терадами
		if (iMem)
			BRDC_sprintf(key_name, _BRDC("memadrmask%d"), iMem);
		else
			BRDC_strcpy(key_name, _BRDC("memadrmask"));
		FindKeyWord(key_name, m_pIniData->MemIni[iMem].AdrMask, pInitData, size);
		if (iMem)
			BRDC_sprintf(key_name, _BRDC("memadrconst%d"), iMem);
		else
			BRDC_strcpy(key_name, _BRDC("memadrconst"));
		FindKeyWord(key_name, m_pIniData->MemIni[iMem].AdrConst, pInitData, size);
	}

	//FindKeyWord(_BRDC("memtetrnum"), m_pIniData->MemIni[0].TetrNum, pInitData, size);
	//FindKeyWord(_BRDC("memadrmask"), m_pIniData->MemIni[0].AdrMask, pInitData, size);
	//FindKeyWord(_BRDC("memadrconst"), m_pIniData->MemIni[0].AdrConst, pInitData, size);
	//FindKeyWord(_BRDC("memtetrnum1"), m_pIniData->MemIni[1].TetrNum, pInitData, size);
	//FindKeyWord(_BRDC("memadrmask1"), m_pIniData->MemIni[1].AdrMask, pInitData, size);
	//FindKeyWord(_BRDC("memadrconst1"), m_pIniData->MemIni[1].AdrConst, pInitData, size);
	FindKeyWord(_BRDC("armprog"), m_pIniData->ARMPROG, pInitData, size);
	m_RegLog = 0;
	if (m_pIniData->ENUM_WAIT == -1)
		m_pIniData->ENUM_WAIT = 0;
	if (m_pIniData->LOAD_WAIT == -1)
		m_pIniData->LOAD_WAIT = 0;
	if (m_pIniData->FMCPOWER == -1)
		m_pIniData->FMCPOWER = 1;
	m_Trans = 0;
	m_isDLMode = 0;
	m_TaskID = 0;
}

CBambusb::~CBambusb()
{
	if (m_pDevice)
	{
		if (m_EPin != 0)//если инициализация была проведена
		{
			//TerminateThread(m_hServThread, 0);
			for (int i = 0; i < MAX_STREAM_SRV; i++)
			{
				if (m_pStreamDscr[i]->hThread != NULL)//проверка что все стрим закрыт
				{//закрываем стрим
					int errCnt = 0;
					m_pStreamDscr[i]->isBreak = 1;
					while (errCnt < 5)
					{
						if (IPC_waitThread(m_pStreamDscr[i]->hThread, 100) == NULL)
							break;
						errCnt++;
					}
					if (errCnt >= 5)
						IPC_stopThread(m_pStreamDscr[i]->hThread);
				}
				if ((m_pStreamDscr[i]->bEndPoint != 0xFF)&&(m_pStreamDscr[i]->bEndPoint < (m_EPin + 4)))
				{
					U32 Value=0, TetrNumber, RegNum = 0, RegVal = 0;
					U08 bType = 1, bDir = 0;
					TetrNumber = m_pStreamDscr[i]->nTetrNum;
					bType = 3;
					bDir = 0;
					if (m_pStreamDscr[i]->bEndPoint > 6)
						bDir = 1;
					RegVal = 7;
					SendCommand(0, ((m_pStreamDscr[i]->bEndPoint - 2 - (bDir * (m_EPin - 1))) << 5), 0, Value, bType, 0, RegVal, 1); //close
					//m_pDevice->EndPoints[m_pStreamDscr[i]->bEndPoint]->Abort();
					//m_pDevice->EndPoints[m_pStreamDscr[i]->bEndPoint]->Reset();
				}
				if (m_pStreamDscr[i])
				{
					if (m_pStreamDscr[i]->pBlock != 0)
						delete [] m_pStreamDscr[i]->pBlock;
					delete m_pStreamDscr[i];
				}
			}
			U32 lVal = 0, hVal = 0;
			if (m_TaskID != 0)
				SendCommand(0, 255, hVal, lVal, 0, 0, 1, 1); //закрытие TaskID
			m_pDevice->Close();
			//DeleteServices();
			IPC_deleteMutex(m_hCommand);
			IPC_deleteMutex(m_hStartStop);
#ifdef _WIN32
			if (m_rICRmap.hBaseICR)
				CloseHandle(m_rICRmap.hBaseICR);
			if (m_rICRmap.hSubICR)
				CloseHandle(m_rICRmap.hSubICR);
#endif
		}
		if (m_pIniData)
			delete m_pIniData;
		CleanUp();
	}
}

S32 CBambusb::Init(short CurDevNum, BRDCHAR* pBoardName, BRDCHAR* pIniString)
{
	m_LastComTime = IPC_getTickCount();
	m_EPin = 0;
	U08 isAuto = 0;
	if (m_pIniData)
	{
		if (m_pIniData->VID == -1)
		{//идентификаторы по умолчанию
			m_pIniData->VID = 0x4953;
		}
	}
	else
	{//авто-инициализация
		isAuto = 1;
		m_pIniData = new BASE_INI();
		m_pIniData->ISDEBUG = -1;
		m_pIniData->PRID = -1;
		memset(m_pIniData->PID, 0, 8);
		m_pIniData->MemIni[0].TetrNum = -1;
		m_pIniData->VID = 0x4953;
		m_pIniData->RESET = -1;
		m_pIniData->LOGGING = 0;
		m_pIniData->ENUM_WAIT = 5;
		m_pIniData->LOAD_WAIT = 3;
		m_pIniData->ARMPROG = 0;
		m_pIniData->FMCPOWER = 1;
		//return ErrorPrint(-1,  CURRFILE, _BRDC("<Init> Auto init not supported"));
	}
	if (m_pDevice->DeviceCount() == 0)//драйвер видит устройства
	{
		if (m_pIniData->ENUM_WAIT <= 0)
			m_pIniData->ENUM_WAIT = 5000;
		else
			m_pIniData->ENUM_WAIT *= 1000;
		while (m_pIniData->ENUM_WAIT > 0)
		{
			IPC_delay(100);
			if (m_pDevice->DeviceCount() != 0)
				break;
			m_pIniData->ENUM_WAIT -= 100;
		}
	}
	if (m_pDevice->DeviceCount() == 0)//драйвер видит устройства
		return ErrorPrint(-1, CURRFILE, _BRDC("<Init> No devices found"));
	//	ICR_IdBase Info;
	S32	errorCode = BRDerr_OK;
	U32 devNum;
	wchar_t *tmpWChar;
	for (devNum = 0; devNum < m_pDevice->DeviceCount(); devNum++)
	{
		int DeviceNumber = devNum;
		BRDCHAR DeviceName[32];
		BRDC_sprintf(DeviceName, _BRDC("%d:%d"), m_pIniData->VID, m_pIniData->PRID);
		bool isOpened = m_pDevice->Open(DeviceNumber);
		if (!isOpened)
			continue;//return ErrorPrint(-1, CURRFILE, _BRDC("<Init> No devices found"));
		U32 devVID = m_pDevice->VendorID;
		U32 devPRID = m_pDevice->ProductID;
		U32	devPID = wcstol(m_pDevice->SerialNumber, &tmpWChar, 10);//_wtoi(m_pDevice->SerialNumber);
		if (devVID != m_pIniData->VID)
		{
			if (devNum == (m_pDevice->DeviceCount()-1))
			{//подходящих устройств не найдено
				m_pDevice->Close();
				return ErrorPrint(-1,
								  CURRFILE, //ErrVitalText);
								  _BRDC("<Init> Device with needed PRID/VID not found"));
			}
			else
				continue;
		}
		if (m_pIniData->PRID != -1)
		{
			if (devPRID != m_pIniData->PRID)
			{
				if (devNum == (m_pDevice->DeviceCount()-1))
				{//подходящих устройств не найдено
					m_pDevice->Close();
					return ErrorPrint(-1,
									  CURRFILE, //ErrVitalText);
									  _BRDC("<Init> Device with needed PRID/VID not found"));
				}
				else
				{
					m_pDevice->Close();
					continue;
				}
			}
		}
		m_PID = devPID;

		if(m_pIniData)
		{
			BRDCHAR *pStr;
			if (BRDC_strtol(m_pIniData->PID, &pStr, 0) == 0)//серийник не указан в brd.ini
			{
				if (CurDevNum <= DeviceNumber)
					break;
				else
					if (CurDevNum >= m_pDevice->DeviceCount())
					{
						m_pDevice->Close();
						return BRDerr_NONE_DEVICE;
						//return ErrorPrint(-1,
						//				  CURRFILE, //ErrVitalText);
						//				  _BRDC("<Init> Device with needed PRID/VID not found"));
					}
			}
			if(BRDC_strtol(m_pIniData->PID, &pStr, 0) == m_PID)//серийник совпал с указанным в brd.ini
				break;
		}
	}
	if (m_pDevice->IsOpen() != true)
		return ErrorPrint(-1, CURRFILE, _BRDC("<Init> No devices found"));
	m_PID = wcstol(m_pDevice->SerialNumber, &tmpWChar, 10);//_wtoi(m_pDevice->SerialNumber);
	if(m_pIniData)
	{
		BRDCHAR *pStr;
		if (BRDC_strtol(m_pIniData->PID, &pStr, 0) != 0)
			if(BRDC_strtol(m_pIniData->PID, &pStr, 0) != m_PID)//серийник совпал с указанным в brd.ini
			{
				m_pDevice->Close();
				return ErrorPrint(-1,
									  CURRFILE, //ErrVitalText);
									  _BRDC("<Init> Device with needed PID not found"));
			}
	}
	if (m_pDevice->BcdDevice < VER_BCD)
		BRDC_printf(_BRDC("Wrong firmware version. Update recommended\n"));
	if (m_pIniData->ARMPROG == 0x768)
		m_isArmProg = 1;
	else
		m_isArmProg = 0;
	BRDC_sprintf(m_name, _BRDC("CAMBUSB_%X"), m_PID);
	lstrcpy(pBoardName, m_name);
	m_isLog = 0;
	PUCHAR pBaseCfgMem = new UCHAR[m_sizeICR];
	ULONG BaseCfgSize = m_sizeICR;
	
	for (int ii = 1; ii < m_pDevice->EndPointCount(); ii++)
	{
		m_pDevice->EndPoints[ii]->TimeOut = 50000;
		//m_pDevice->EndPoints[ii]->Abort();
		//m_pDevice->EndPoints[ii]->Reset();
	}
	for (int ii = 1; ii < m_pDevice->EndPointCount(); ii++)//определяем EndPoint для приёма ответов на команды
		if (m_pDevice->EndPoints[ii]->Address == 0x81)
		{
			m_EPin = ii;
			break;
		}
	m_pDevice->EndPoints[1]->TimeOut = 200;
	m_pDevice->EndPoints[m_EPin]->TimeOut = 200;
	
	m_DeviceID = m_pDevice->ProductID;
	m_RevisionID = 0x10;
	U32 lVal = 0, hVal = 8;
	if (m_pIniData->ISDEBUG == 1)
		m_TaskID = 0xFF;
	else
		m_TaskID = 0;

	if (m_pIniData->LOAD_WAIT <= 0)
		m_pIniData->LOAD_WAIT = 7000;
	else
		m_pIniData->LOAD_WAIT *= 1000;
//#ifdef _WIN32
//	{
//		BRDCHAR mutexName[128] = {0};
//		BRDCHAR pStr[10];
//		BRDC_strcat(mutexName, _BRDC("InSys_USB3_Commands_"));
//		BRDC_strcat(mutexName, BRDC_itoa(m_PID, pStr, 10));
//		m_hCommand = OpenMutex(MUTEX_ALL_ACCESS, true, mutexName);
//		if (m_hCommand != 0)
//		{
//			ReleaseMutex(m_hCommand);
//			m_pIniData->LOAD_WAIT = 0;
//		}
//	}
//#endif
	{
		BRDCHAR mutexName[128] = {0};
		BRDCHAR pStr[10];
		BRDC_strcat(mutexName, _BRDC("InSys_USB3_Commands_"));
		BRDC_strcat(mutexName, BRDC_itoa(m_PID, pStr, 10));
		for (int i = 0; i < 100; i++)
		{
			m_hCommand = IPC_createMutex(mutexName, false);
			if (m_hCommand != 0)
				break;
		}
	}
	//m_hCommand = CreateSemaphore(NULL, 1, 1, NULL);
	{
		BRDCHAR mutexName[128] = {0};
		BRDCHAR pStr[10];
		BRDC_strcat(mutexName, _BRDC("InSys_USB3_StartStop_"));
		BRDC_strcat(mutexName, BRDC_itoa(m_PID, pStr, 10));
		for (int i = 0; i < 100; i++)
		{
			m_hStartStop = IPC_createMutex(mutexName, false);
			if (m_hStartStop != 0)
				break;
		}
	}
	if (m_pIniData->LOGGING == 1)
	{
		m_isLog = 1;
		FILE *pFile;
		while (1)
		{
			pFile = fopen("commands.txt", "w");
			if (pFile != NULL)
				break;
		}
		fprintf(pFile, "%d\n", m_PID);
		fclose(pFile);
	}
	//IPC_delay(m_pIniData->LOAD_WAIT);

	//if (m_pIniData->ISDEBUG == 1)
	//{
	//	m_pDevice->SetAltIntfc(1);
	//	m_pDevice->SetAltIntfc(0);
	//}
	//else
		if (m_pIniData->RESET == 1)
		{
			m_pDevice->SetAltIntfc(1);
			m_pDevice->SetAltIntfc(0);
			IPC_delay(3000);
		}
	lVal = 0;
	if (SendCommand(0, 0, 0, lVal, 0, 0, 1, 1) != BRDerr_OK)//получение TaskID для захвата тетрад и стримов
	{
		while (m_pIniData->LOAD_WAIT > 0)
		{
			IPC_delay(100);
			errorCode = SendCommand(0, 0, 0, lVal, 0, 0, 1, 1);
			if (errorCode == BRDerr_OK)
				break;
			//m_pDevice->Close();
			//m_pDevice->Open(devNum);
			m_pIniData->LOAD_WAIT -= 100;
		}
	}
	if (m_TaskID == 0)
	{
		m_pDevice->SetAltIntfc(2);
		m_pDevice->SetAltIntfc(0);
		IPC_delay(5000);
		lVal = 0;
		if (SendCommand(0, 0, 0, lVal, 0, 0, 1, 1) != BRDerr_OK)//получение TaskID для захвата тетрад и стримов
		{
			m_isDLMode = 1;
			m_isArmProg = 1;
		}
	}
	else
	{
		if (errorCode != BRDerr_OK)
			return ErrorPrint(-1, CURRFILE, _BRDC("<Init> Device don't answer"));
		if (m_TaskID == 0xFF)
			SendCommand(0, 0, 0, lVal, 0, 0, 1, 1);//сброс ARM
		else
			if (m_pIniData->RESET == 1)
				SendCommand(0, 0, 0, lVal, 0, 0, 1, 1);//сброс ARM
			else
			{
				//сброс reset плис
				SendCommand(0, 0, hVal, lVal, 1, 2, 0, 0);
				if ((lVal & 0xf) != 0xf)
					SendCommand(0, 0, 0, lVal, 0, 0, 1, 1);//сброс ARM
			}
		if (errorCode != BRDerr_OK)
			return ErrorPrint(-2, CURRFILE, _BRDC("<Init> Device don't answer"));
		if (m_TaskID == 0)
			return ErrorPrint(-3, CURRFILE, _BRDC("<Init> No devices found"));
	}
	if (m_pIniData->REGLOG == 1)
	{
		m_RegLog = 1;
		FILE *pFile;
		pFile = BRDC_fopen(_BRDC("usb reglog.txt"), _BRDC("w"));
		fclose(pFile);
	}

	for (int i = 0; i < MAX_MEMORY_SRV; i++)
	{
		m_memIni[i].TetrNum = -1;
		m_memIni[i].AdrMask = 0;
		m_memIni[i].AdrConst = 0;
	}

	//read base and sub ICR
	memset(m_baseICR, 0, 512);
	memset(m_subICR, 0, 512);
	memset(m_FRUID, 0, 1024);
	BRDCHAR abNameICR[MAX_PATH];

	BRDC_sprintf(abNameICR, _BRDC("base_icr_%s_%d"), m_name, m_PID);
	int isAlreadyCreated = 0;
#ifdef _WIN32
	m_rICRmap.hBaseICR = CreateFileMapping(INVALID_HANDLE_VALUE,
											NULL, PAGE_READWRITE,
											0, 512,
											abNameICR);
	//if(!m_rICRmap.hBaseICR)
	//		return BRDerr_ERROR;
	isAlreadyCreated = ( GetLastError() == ERROR_ALREADY_EXISTS ) ? 1 : 0;
	//if (isAlreadyCreated)
		m_rICRmap.pBaseICR = (PVOID)MapViewOfFile(m_rICRmap.hBaseICR, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
#endif
	if (0 == m_isDLMode)
	{
		if (!isAlreadyCreated)
			errorCode = GetICR(m_baseICR, 0);//чтение базового ICR
		else
			errorCode = BRDerr_OK;
		if (errorCode != BRDerr_OK)
		{
			delete[] pBaseCfgMem;
			pBaseCfgMem = NULL;
		}
		else
		{
#ifdef _WIN32
			if (isAlreadyCreated)
				memcpy(m_baseICR, m_rICRmap.pBaseICR, 512);
			else
				memcpy(m_rICRmap.pBaseICR, m_baseICR, 512);
#endif
		}
	}
	BRDC_sprintf(abNameICR, _BRDC("sub_icr_%s_%d"), m_name, m_PID);
	isAlreadyCreated = 0;
#ifdef _WIN32
	m_rICRmap.hSubICR = CreateFileMapping(INVALID_HANDLE_VALUE,
											NULL, PAGE_READWRITE,
											0, 512,
											abNameICR);
	//if(!m_rICRmap.hBaseICR)
	//		return BRDerr_ERROR;
	isAlreadyCreated = ( GetLastError() == ERROR_ALREADY_EXISTS ) ? 1 : 0;
	//if (isAlreadyCreated)
		m_rICRmap.pSubICR = (PVOID)MapViewOfFile(m_rICRmap.hSubICR, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
#endif
	if (0 == m_isDLMode)
	{
		if (!isAlreadyCreated)
			errorCode = GetICR(m_subICR, 1);//чтение ICR субмодуля
		else
			errorCode = BRDerr_OK;
		if (errorCode == BRDerr_OK)
		{
#ifdef _WIN32
			if (isAlreadyCreated)
				memcpy(m_subICR, m_rICRmap.pSubICR, 512);
			else
				memcpy(m_rICRmap.pSubICR, m_subICR, 512);
#endif
		}
	}
	//errorCode = GetICR(m_FRUID, 2);//чтение FRU ID

	if (pBaseCfgMem)
		GetBaseEEPROM(pBaseCfgMem, BaseCfgSize);
	
	ULONG SubCfgSize = 0;
	for(int i = 0; i < m_AdmIfCnt; i++)
		errorCode = SetSubEeprom(i, SubCfgSize);
	if (pBaseCfgMem)
	{
		for (int i = 0; i < MAX_MEMORY_SRV; i++)
		{
			m_memIni[i].TetrNum = m_pIniData->MemIni[i].TetrNum;
			if (m_pIniData->MemIni[i].AdrConst != -1)
			m_memIni[i].AdrConst = m_pIniData->MemIni[i].AdrConst;
			if (m_pIniData->MemIni[i].AdrMask != -1)
			m_memIni[i].AdrMask = m_pIniData->MemIni[i].AdrMask;
		}
		
		errorCode = SetPuInfo(pBaseCfgMem, BaseCfgSize);
		errorCode = SetSdramConfig(m_pIniData, pBaseCfgMem, BaseCfgSize);
	}

	if (isAuto != 0)
	{
		if (GetAdmIfCntICR(m_AdmIfCnt) == BRDerr_OK)
			errorCode = SetSubunitAuto();
	}
	else
	{
		m_AdmIfCnt = 0;
		m_SubunitCnt = 0;
	}
	errorCode = SetServices();

	if(pBaseCfgMem)
			delete [] pBaseCfgMem;
	
	hVal = 0;
	lVal = 0;
	if (0 == m_isDLMode)
		if (m_TaskID != 0xFF)
		{
			SendCommand(0, 255, hVal, lVal, 0, 0, 1, 1); //закрытие TaskID
			m_TaskID = 0;
		}

	if (pIniString)
	{
		BRDC_sprintf(pIniString, _BRDC("pid=0x%X\n"), m_PID);
	}
	//m_pDevice->Close();
	return DRVERR(errorCode);
}

void CBambusb::Open()
{
	U32 errorCode = 0, lVal = 0;
	if (0 == m_isDLMode)
		if (m_TaskID != 0xFF)
			SendCommand(0, 0, 0, lVal, 0, 0, 1, 1);//получение TaskID для захвата тетрад и стримов
}

void CBambusb::Close()
{
	U32 lVal = 0, hVal = 0;
	if (0 == m_isDLMode)
		if ((m_TaskID != 0)&&(m_TaskID != 0xFF))
			SendCommand(0, 255, hVal, lVal, 0, 0, 1, 1); //закрытие TaskID
	m_TaskID = 0;
}

void CBambusb::CleanUp()
{
	if (m_pDevice->IsOpen())
		m_pDevice->Close();
	delete m_pDevice;
}

void CBambusb::GetDeviceInfo(BRD_Info* pInfo) const
{
	pInfo->boardType	= ((ULONG)m_DeviceID << 16) | (ULONG)m_RevisionID;
	pInfo->pid			= m_PID;
#ifndef _WIN64
	BRDCHAR name[128] = { 0 };
	memcpy(name, m_pDevice->DeviceName, 128);
	BRDC_strcpy(pInfo->name, name/*(BRDCHAR *)m_pDevice->DeviceName*/);
#else
	size_t chars = 0, wsize = 64;
	BRDCHAR devName[128] = {0};
	mbstowcs_s(&chars, devName, wsize, m_pDevice->DeviceName, _TRUNCATE);
	BRDC_strcpy(pInfo->name, devName);
#endif
	pInfo->busType		= BRDbus_USB;
    pInfo->bus			= 0;
    pInfo->slot			= 0;
	pInfo->verMajor		= VER_MAJOR;
	pInfo->verMinor		= VER_MINOR;
	int i = 0;
	for(i = 0; i < MAX_ADMIF; i++)
		pInfo->subunitType[i] = m_Subunit[i].type;
	for(i = 0; i < 8; i++)
		pInfo->base[i] = NULL;
}

ULONG CBambusb::GetIdInfo(U16& devId, U08& revId)
{
	ULONG Status = BRDerr_OK;
	devId = m_pDevice->ProductID;
	revId = 0;
	return Status;
}

ULONG CBambusb::HardwareInit()
{
	ULONG Status = BRDerr_OK;
	U32 lVal = 0, hVal = 8;
	U08 j = 0, isTaskRelease = 0;
	if (m_isDLMode != 0)
		return BRDerr_OK;
	for (int i = 0; i < 10; i++)
	{
		if (m_TaskID == 0)
		{
			SendCommand(0, 0, 0, lVal, 0, 0, 1, 1);//получение TaskID для захвата тетрад и стримов
			isTaskRelease = 1;
		}
		if (m_TaskID == 0)
		{
			m_pDevice->SetAltIntfc(2);
			m_pDevice->SetAltIntfc(0);
			IPC_delay(5000);
		}
		else
			break;
	}
	SendCommand(0, 0, hVal, lVal, 1, 2, 0, 0);
	if (((lVal & 0x0F) != 0x0F)||(m_pIniData->RESET == 1)||(m_TaskID == 0xFF))
	{
		//SendCommand(0, 0, 0, lVal, 0, 0, 1, 1);//сброс ARM
		//инициализация блока main
		hVal = 8;
		lVal = 0;
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);
		while (j < 20)
		{
			hVal = 8;
			lVal = 1;
			SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);
			lVal = 3;
			SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);
			lVal = 7;
			SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);
			lVal = 0;
			hVal = 0x10;
			SendCommand(0, 0, hVal, lVal, 1, 2, 0, 0);
			if ((lVal & 1) == 1)
				break;
			j++;
			IPC_delay(50);
		}
		if (j >= 10)
		{
			if (isTaskRelease != 0)
			{
				hVal = 0;
				lVal = 0;
				SendCommand(0, 255, hVal, lVal, 0, 0, 1, 1); //закрытие TaskID
				m_TaskID = 0;
			}
			return BRDerr_HW_ERROR;
		}
		hVal = 8;
		lVal = 0x0F;
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);
	}
	if (m_pIniData->FMCPOWER)
	{
		BRDextn_FMCPOWER power = {0};//включение питания на субмодуле
		power.onOff = 1;
		PowerOnOff(&power, 0);
	}
	if (isTaskRelease != 0)
	{
		hVal = 0;
		lVal = 0;
		SendCommand(0, 255, hVal, lVal, 0, 0, 1, 1); //закрытие TaskID
		m_TaskID = 0;
	}
	return Status;
}

ULONG CBambusb::GetPower(PBRDextn_FMCPOWER pow)
{
	ULONG Status = BRDerr_OK;
	if (m_isDLMode != 0)
		return Status;
	U32 hVal, lVal;
	hVal = 0x20 + 0x2;
	Status = SendCommand(0, 0, hVal, lVal, 1, 2, 0, 0); //чтение значения из V0
	pow->value = lVal;
	hVal = 0x20 + 0xB;
	Status = SendCommand(0, 0, hVal, lVal, 1, 2, 0, 0);
	pow->onOff = (lVal >> 15) & 1;
	return Status;
}

//***************************************************************************************
ULONG CBambusb::PowerOnOff(PBRDextn_FMCPOWER pow, int force)
{
	ULONG Status = BRDerr_OK;
	if (m_isDLMode != 0)
		return Status;
	U32 hVal = 0x20 + 0xB, lVal = 0;
	SendCommand(0, 0, hVal, lVal, 1, 2, 0, 0);
	if (((lVal & 0x8000) != 0) && pow->onOff == 1)
		return Status;
	if (pow->onOff == 1)
		lVal = (1 << 15) + 9;//POWER_EN + PROGRAM ENABLE
	else
		lVal = 9; //POWER_OFF + PROGRAM ENABLE
	hVal = 0x20 + 0xB;
	Status = SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);
	return Status;
}

//***************************************************************************************
ULONG CBambusb::ReadFmcExt(PBRDextn_FMCEXT ext)
{
	return BRDerr_CMD_UNSUPPORTED;
}

//***************************************************************************************
ULONG CBambusb::WriteFmcExt(PBRDextn_FMCEXT ext)
{
	return BRDerr_CMD_UNSUPPORTED;
}

void CBambusb::GetPldCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAdmPld pCfgPld)
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

ULONG CBambusb::GetPldDescription(TCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
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
		case 8:
			BRDC_sprintf(Description, _BRDC("XC4VLX%d-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
			break;
		case 9:
			BRDC_sprintf(Description, _BRDC("XC4VSX%d-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
			break;
		case 10:
			BRDC_sprintf(Description, _BRDC("XC4VFX%d-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
			break;
		}
		return 1;
	}
	return 0;
}

ULONG CBambusb::SetPuInfo(PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
	m_puCnt = 1;

	m_AdmRomInfo.Id = BRDpui_PLD;
	m_AdmRomInfo.Type = PLD_CFG_TAG;
	m_AdmRomInfo.Attr = BRDpua_Load | BRDpua_Read;
	BRDCHAR Description0[MAX_DESCRIPLEN] = _BRDC("ADM PLD ");
	BRDCHAR Description[MAX_DESCRIPLEN] = _BRDC("ADM PLD XILINX");// Programmable Unit Description Text
	if (GetPldDescription(Description, pBaseEEPROMMem, BaseEEPROMSize))
	{
		BRDC_strcpy(m_AdmRomInfo.Description, Description0);
		BRDC_strcat(m_AdmRomInfo.Description, Description);
	}
	else
		BRDC_strcpy(m_AdmRomInfo.Description, Description);
	m_PuInfos.push_back(m_AdmRomInfo);

	if (m_pDevice->AltIntfcCount() > 1)
	{
		m_puCnt++;
		m_AdmPldInfo.Id = BRDpui_FLASH;//0x100;
		m_AdmPldInfo.Type = PLD_CFG_TAG;
		m_AdmPldInfo.Attr = BRDpua_Load;
		BRDC_strcpy(m_AdmPldInfo.Description, _BRDC("ADM PLD XILINX"));
		m_PuInfos.push_back(m_AdmPldInfo);
	}

	if(1)
	{
		m_puCnt++;
		m_BaseIcrInfo.Id = BRDpui_BASEICR;
		m_BaseIcrInfo.Type = BASE_ID_TAG;
		m_BaseIcrInfo.Attr = BRDpua_Read | BRDpua_Write;
		BRDC_strcpy(m_BaseIcrInfo.Description, _BRDC("ID & CFG EEPROM on Base module"));
		m_PuInfos.push_back(m_BaseIcrInfo);
	}

	//add FRU ID
	m_puCnt++;
	m_BaseIcrInfo.Id = BRDpui_FRUID;
	m_BaseIcrInfo.Type = 0;
	BRDC_strcpy(m_BaseIcrInfo.Description, _BRDC("FRU ID on subunit"));
	m_PuInfos.push_back(m_BaseIcrInfo);

	// определяет просто наличие ППЗУ на субмодуле
	PUSHORT pSubCfgMem = (PUSHORT)(m_Subunit[0].pSubEEPROM);
	if(pSubCfgMem && (pSubCfgMem[0] != 0xA55A) && (pSubCfgMem[1] != 0x5AA5))
	{
		m_puCnt++;
		m_AdmIcrInfo.Id = BRDpui_SUBICR;
		m_AdmIcrInfo.Type = ADM_ID_TAG;
		m_AdmIcrInfo.Attr = BRDpua_Read | BRDpua_Write;
		BRDC_strcpy(m_AdmIcrInfo.Description, _BRDC("ID & CFG EEPROM on subunit"));
		m_PuInfos.push_back(m_AdmIcrInfo);
	}

	if (m_pDevice->AltIntfcCount() > 1)
		if (m_isArmProg)
		{//add USB ARM
			m_puCnt++;
			m_ArmPldInfo.Id = BRDpui_ARM;//0x300;
			m_ArmPldInfo.Type = PLD_CFG_TAG;
			m_ArmPldInfo.Attr = BRDpua_Load;
			BRDC_strcpy(m_ArmPldInfo.Description, _BRDC("ARM ROM"));
			m_PuInfos.push_back(m_ArmPldInfo);
		}
	return BRDerr_OK;
}

void CBambusb::GetPuList(BRD_PuList* pu_list) const
{
	int pu_num = (int)m_PuInfos.size();
	for(int i = 0; i < pu_num; i++)
	{
		pu_list[i].puId		= m_PuInfos[i].Id;
		pu_list[i].puCode	= m_PuInfos[i].Type;
		pu_list[i].puAttr	= m_PuInfos[i].Attr;
		BRDC_strcpy( pu_list[i].puDescription, m_PuInfos[i].Description);
	}
}

ULONG CBambusb::PuFileLoad(ULONG id, const BRDCHAR *fileName, PUINT pState)
{
	ULONG Status = BRDerr_ERROR;
	*pState = 0;
	if(BRDC_strlen(fileName) == 0)
	{
		return ErrorPrintf( BRDerr_BAD_FILE,
							CURRFILE,
							_BRDC("<PuFileLoad> Bad File Name = %s"), fileName);
	}

	if (id == m_ArmPldInfo.Id)
	{
		if (m_isArmProg)
		{
			Status = LoadArmFile(fileName);
			if (Status == BRDerr_OK)
				*pState = 1;
		}
		return Status;
	}

	if (id == m_AdmPldInfo.Id)
	{
		Status = LoadBitFile(fileName);
		if (Status == BRDerr_OK)
			*pState = 1;
		else
			*pState = 0;
		return Status;
	}

	if(id == m_AdmRomInfo.Id)
	{
		Status = LoadPldFile(fileName, id - BRDpui_PLD);
		if(Status != BRDerr_OK)
			return Status;

		ULONG pld_status;
		if(GetPldStatus(pld_status, id - BRDpui_PLD) != BRDerr_OK)
			return ErrorPrint(BRDerr_HW_ERROR,
							  CURRFILE,
							  _BRDC("<PuFileLoad> Hardware error"));
		*pState = pld_status;
	}
	else
	{
		return ErrorPrintf( BRDerr_BAD_ID,
							CURRFILE,
							_BRDC("<PuFileLoad> Bad Programable Unit ID %d"), id);
	}
	return BRDerr_OK;
}

ULONG CBambusb::LoadBitFile(const BRDCHAR *fileName)
{
	unsigned int ep_in = 2, nComm = 0, maxBuf = 0, nDevName = 0, nSerial = 0, nStage = 0;
	int isRes = 0;
	LONG nLen;
	ULONG Status = BRDerr_INSUFFICIENT_RESOURCES;
	USHORT wTransPld = 0;
	switch (m_pDevice->ProductID)
	{
	case 0x3018:
		nDevName = 0xFC00118E;
		break;
	case 0x3019:
		nDevName = 0xFC00119E;
		break;
	case 0x3028:
		nDevName = 0xFC00128E;
		break;
	case 0x3029:
		nDevName = 0xFC00129E;
		break;
	default:
		break;
	}
	wchar_t* tmpWChar;
	wchar_t wChar[32] = { 0 };
	for (int i = 0; i < 8; i++)
	{
		wChar[0] = m_pDevice->SerialNumber[7 - i];
		nSerial += (wcstol(wChar, &tmpWChar, 10) << (i * 4));
	}

	rUsbHostCommand rOutComms[32] = { 0 }, rInComms[32] = { 0 };
	//for (int nRepeat = 0; nRepeat < 10; nRepeat++)
	int nRepeat = 5;
	{
		nComm = 0;
		CallBackLoadProgress(0, 2, nRepeat, 9);
		isRes = m_pDevice->SetAltIntfc(0);
#ifdef __CYPRESS_API
		IPC_delay(1000);
#endif
		CallBackLoadProgress(0, 2, nRepeat, 9);
		isRes = m_pDevice->SetAltIntfc(2);
		IPC_delay(1000);
		CallBackLoadProgress(0, 2, nRepeat, 9);
		for (int i = 0; i < m_pDevice->EndPointCount(); i++)
			if (m_pDevice->EndPoints[i]->Address == 0x81)
			{
				ep_in = i;
				break;
			}
		DWORD x = IPC_getTickCount();
		rOutComms[nComm].signatureValue = 0xC1;
		rOutComms[nComm].command.setup.type = 0;
		rOutComms[nComm].command.setup.request = 0xff;
		rOutComms[nComm].command.setup.requestH = 0x1f;
		rOutComms[nComm].valueL = nDevName;
		rOutComms[nComm].valueH = nSerial;
		rOutComms[nComm].transaction = wTransPld++;
		nComm++;
		rOutComms[nComm].signatureValue = 0xC1;
		rOutComms[nComm].command.setup.type = 0;
		rOutComms[nComm].command.setup.request = 0;
		rOutComms[nComm].valueL = 0;
		rOutComms[nComm].valueH = 0;
		rOutComms[nComm].transaction = wTransPld++;
		nComm++;
		nLen = nComm * 16;
		isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)rOutComms, nLen);
		isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)rInComms, nLen);
		maxBuf = rInComms[0].command.setup.request + 1;
		IPC_delay(300);
		CallBackLoadProgress(0, 2, nRepeat, 9);
		if (((rInComms[0].valueL) & 0xF) > 1)
		{
			nStage = 2;
		}
		else
		{
			if (((rInComms[0].valueH) & 0xF) > 1)
			{
				nStage = 3;
			}
			else
			{
				if (((rInComms[0].valueL) & 0xF) == 1)
				{
					if (((rInComms[0].valueH) & 0xF) == 1)
						nStage = 1;
					else
						nStage = 4;
				}
			}
		}

		if (nStage == 2)
		{
			FILE* pFile;
			//BRDCHAR bTempFileName[256] = { 0 };
			switch (m_pDevice->ProductID)
			{
			case 0x3018:
				rInComms[1].command.status.swap = 1;
				break;
			case 0x3019:
				break;
			case 0x3028:
				break;
			case 0x3029:
				rInComms[1].command.status.swap = 1;
				break;
			default:
				break;
			}
			pFile = BRDC_fopen(fileName, _BRDC("rb"));
			if (pFile != 0)
			{
				BRDC_fclose(pFile);
				Status = LoadPldFileDMA(fileName, rInComms[0].valueL & 0xF, rInComms[0].valueH & 0xF, ep_in, maxBuf, &wTransPld, rInComms[1].command.status.doubling, rInComms[1].command.status.endean, rInComms[1].command.status.swap);
			}
		}
		else
			if (nStage == 1)
			{
				FILE* pFile;
				//BRDCHAR bTempFileName[256] = { 0 };
				switch (m_pDevice->ProductID)
				{
				case 0x3018:
					rInComms[1].command.status.swap = 1;
					break;
				case 0x3019:
					break;
				case 0x3028:
					break;
				case 0x3029:
					break;
				default:
					break;
				}
				pFile = BRDC_fopen(fileName, _BRDC("rb"));
				if (pFile != 0)
				{
					BRDC_fclose(pFile);
					Status = LoadPldFileCom(false, fileName, maxBuf, ep_in, 0xC1, rInComms[1].command.status.doubling, rInComms[1].command.status.endean, rInComms[1].command.status.swap, &wTransPld);
				}
			}
		//if (Status == 0)
		//	break;
	}
	m_pDevice->SetAltIntfc(0);
	for (int i = 0; i < 20; i++)
	{
		CallBackLoadProgress(2, 2, 99, 100);
		IPC_delay(250);
	}
	CallBackLoadProgress(2, 2, 100, 100);
	return Status;
}

ULONG CBambusb::LoadArmFile(const BRDCHAR *fileName)
{
	rUsbHostCommand rOutComms[2] = {0}, rInComms[2] = {0};
	ULONG Status = 0, nDevName, nSerial = 0;
	UINT nComm = 0, ep_in = 0, maxBuf = 0;
	int isRes = 0;
	LONG nLen = 0;
	USHORT wTransPld = 0;
	isRes = m_pDevice->SetAltIntfc(0);
	isRes = m_pDevice->SetAltIntfc(1);
	if (!isRes)
		return BRDerr_HW_ERROR;
	for (int i = 1; i <= m_pDevice->EndPointCount(); i++)
		if (m_pDevice->EndPoints[i]->Address == 0x81)
		{
			ep_in = i;
			break;
		}
	if (ep_in == 0)
		return BRDerr_HW_ERROR;
	switch (m_pDevice->ProductID)
	{
	case 0x3018:
		nDevName = 0xFC00118E;
		break;
	case 0x3019:
		nDevName = 0xFC00119E;
		break;
	case 0x3028:
		nDevName = 0xFC00128E;
		break;
	case 0x3029:
		nDevName = 0xFC00129E;
		break;
	default:
		break;
	}
	wchar_t* tmpWChar;
	wchar_t wChar[2] = {0};
	for (int i = 0; i < 8; i++)
	{
		wChar[0] = m_pDevice->SerialNumber[7-i];
		nSerial += (wcstol(wChar, &tmpWChar, 10) << (i * 4));
	}
	rOutComms[nComm].signatureValue = 0xC0;
	rOutComms[nComm].command.setup.type = 0;
	rOutComms[nComm].command.setup.request = 0xFF;
	rOutComms[nComm].command.setup.requestH = 0x1F;
	rOutComms[nComm].valueH = nSerial;
	rOutComms[nComm].valueL = nDevName;
	rOutComms[nComm].transaction = wTransPld++;
	nComm++;
	rOutComms[nComm].signatureValue = 0xC0;
	rOutComms[nComm].command.setup.type = 0;
	rOutComms[nComm].valueH = 0;
	rOutComms[nComm].valueL = 0;
	rOutComms[nComm].transaction = wTransPld++;
	nComm++;
	nLen = nComm * 16;
	isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)&rOutComms, nLen);
	if (!isRes)
		return BRDerr_HW_ERROR;
	isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)&rInComms, nLen);
	if (!isRes)
		return BRDerr_HW_ERROR;
	maxBuf = rInComms[0].command.setup.request + 1;
	//определение режима работы: командный или потоковый
	bool isStream = true;
	if ((rInComms[0].valueL & 0xF) == 1)
		Status = LoadPldFileCom(false, fileName, maxBuf, ep_in, 0xC0, rInComms[1].command.status.doubling, rInComms[1].command.status.endean, rInComms[1].command.status.swap, &wTransPld);
	else
		Status = LoadPldFileDMA(fileName, rInComms[0].valueL & 0xF, rInComms[0].valueH & 0xF, ep_in, maxBuf, &wTransPld, rInComms[1].command.status.doubling, rInComms[1].command.status.endean, rInComms[1].command.status.swap);
	//
	m_pDevice->SetAltIntfc(0);
	return Status;
}

ULONG CBambusb::GetPuState(ULONG id, PUINT pState)
{
	if (id == BRDpui_ARM)
	{
		ULONG pld_status = 1;
		*pState = pld_status;
		return BRDerr_OK;
	}
	if (id == m_AdmRomInfo.Id)
	{
		ULONG pld_status;
		if (GetPldStatus(pld_status, id - BRDpui_PLD) != BRDerr_OK)
			return ErrorPrint(BRDerr_HW_ERROR,
				CURRFILE,
				_BRDC("<GetPuState> Hardware error"));
		*pState = pld_status;
		return BRDerr_OK;
	}
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
			if(id == m_AdmIcrInfo.Id)
			{
				ULONG adm_status;
				if(GetAdmIcrStatus(adm_status) != BRDerr_OK)
					return ErrorPrint(BRDerr_HW_ERROR,
									CURRFILE,
									_BRDC("<GetPuState> Hardware error"));
				*pState = adm_status;
			}
			else
			{
				return ErrorPrintf( BRDerr_BAD_ID,
									CURRFILE,
									_BRDC("<GetPuState> Bad Programable Unit ID %d"), id);
			}
		}
	}
	return BRDerr_OK;
}

ULONG CBambusb::PuVerify(PBRDextn_PuVerify arg)
{
	BRDCHAR PldFileName[256] = {0};
	BRDC_strcpy(PldFileName, arg->fileName);
	ULONG Status = BRDerr_INSUFFICIENT_RESOURCES;
	if (arg->puId != m_AdmRomInfo.Id)
		return BRDerr_BAD_PARAMETER;
	LONG nLen;
	int nTetr = -1;
	rUsbHostCommand outComms = { 0 }, inComms = { 0 };
	outComms.signatureValue = 0xA5;
	outComms.taskID = m_TaskID;
	outComms.shortTransaction = m_Trans++;
	outComms.command.tetrad.type = 2;
	outComms.command.tetrad.condition = 0;
	outComms.valueH = 0x100;
	if (0 == m_isDLMode)
		for (int i = 0; i < 16; i++)//ищем тетраду FLASH (ID=0xC3)
		{
			outComms.command.tetrad.number = i;
			nLen = 16;
			SendBuffer(&outComms, &inComms, nLen);
			if (inComms.valueL == 0xC3)
			{
				nTetr = i;
				break;
			}
		}
	//if (nTetr != -1)
	//	Status = ReadPldFileTetrad(nTetr, PldFileName);
	//else
	{
		int nIntrfc = m_pDevice->AltIntfcCount();//проверяем наличие альтернативных интерфейсов
		if (nIntrfc <= 1)
			if (nTetr == -1)
			{//тетрада FLASH не найдена, альтернативных интерфейсов нет
				Status = BRDerr_INSUFFICIENT_RESOURCES;
			}
			else
			{//тетрада FLASH найдена, работаем через неё
				Status = ReadPldFileTetrad(nTetr, PldFileName);
			}
		else
			Status = ReadPldFileAlt(PldFileName);
	}
	if (Status == BRDerr_OK)
		*(arg->state) = 1;
	else
		*(arg->state) = 0;
	return Status;
}

ULONG CBambusb::LoadPldFile(const BRDCHAR *PldFileName, ULONG PldNum)
{
	ULONG Status = BRDerr_INSUFFICIENT_RESOURCES;
	LONG nLen;
	int nTetr = -1;
	rUsbHostCommand outComms = {0}, inComms = {0};
	outComms.signatureValue = 0xA5;
	outComms.taskID = m_TaskID;
	outComms.shortTransaction = m_Trans++;
	outComms.command.tetrad.type = 2;
	outComms.command.tetrad.condition = 0;
	outComms.valueH = 0x100;
	if (0 == m_isDLMode)
		for (int i = 0; i < 16; i++)//ищем тетраду FLASH (ID=0xC3)
		{
			outComms.command.tetrad.number = i;
			nLen = 16;
			SendBuffer(&outComms, &inComms, nLen);
			if (inComms.valueL == 0xC3)
			{
				nTetr = i;
				break;
			}
		}
	//if (nTetr != -1)
	//	Status = LoadPldFileTetrad(nTetr, PldFileName);
	//else
	{
		int nIntrfc = m_pDevice->AltIntfcCount();//проверяем наличие альтернативных интерфейсов
		if (nIntrfc <= 1)
			if (nTetr == -1)
			{//тетрада FLASH не найдена, альтернативных интерфейсов нет
				Status = BRDerr_INSUFFICIENT_RESOURCES;
			}
			else
			{//тетрада FLASH найдена, работаем через неё
				Status = LoadPldFileTetrad(nTetr, PldFileName);
			}
		else
			Status = LoadPldFileAlt(PldFileName);
	}
	return Status;
}

ULONG CBambusb::LoadPldFileAlt(const BRDCHAR *PldFileName)
{
	unsigned int ep_in = 2, nComm = 0, maxBuf = 0, nDevName = 0, nSerial = 0, nStage[8] = {0}, nStages = 0;
	int isRes = 0;
	LONG nLen;
	ULONG Status = 0;
	USHORT wTransPld = 0;
	rUsbHostCommand rOutComms[2] = {0}, rInComms[2] = {0};
	isRes = m_pDevice->SetAltIntfc(0);
	isRes = m_pDevice->SetAltIntfc(2);
	IPC_delay(1000);
	for (int i = 0; i < m_pDevice->EndPointCount(); i++)
		if (m_pDevice->EndPoints[i]->Address == 0x81)
		{
			ep_in = i;
			break;
		}
	switch (m_pDevice->ProductID)
	{
	case 0x3018:
		nDevName = 0xFC00118E;
		break;
	case 0x3019:
		nDevName = 0xFC00119E;
		break;
	case 0x3028:
		nDevName = 0xFC00128E;
		break;
	case 0x3029:
		nDevName = 0xFC00129E;
		break;
	default:
		break;
	}
	wchar_t* tmpWChar;
	wchar_t wChar[2] = { 0 };
	for (int i = 0; i < 8; i++)
	{
		wChar[0] = m_pDevice->SerialNumber[7 - i];
		nSerial += (wcstol(wChar, &tmpWChar, 10) << (i * 4));
	}
	DWORD x = IPC_getTickCount();
	rOutComms[nComm].signatureValue = 0xC1;
	rOutComms[nComm].command.setup.type = 0;
	rOutComms[nComm].command.setup.request = 0xff;
	rOutComms[nComm].command.setup.requestH = 0x1f;
	rOutComms[nComm].valueL = 0x1;
	rOutComms[nComm].valueH = 0x1;
	rOutComms[nComm].transaction = wTransPld++;
	nComm++;
	rOutComms[nComm].signatureValue = 0xC1;
	rOutComms[nComm].command.setup.type = 0;
	rOutComms[nComm].command.setup.request = 0;
	rOutComms[nComm].valueL = 0;
	rOutComms[nComm].valueH = 0;
	rOutComms[nComm].transaction = wTransPld++;
	nComm++;
	nLen = nComm*16;
	isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)rOutComms, nLen);
	isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)rInComms, nLen);
	maxBuf = rInComms[0].command.setup.request + 1;
	//определение количества этапов и их типа 0-отсутствие этапа, 1-командный режим, 2-запись в потоке, 3-чтение в потоке, 4-стирание командами
	for (int i = 0; i < 8; i++)
		if (((rInComms[0].valueL >> (i * 4)) & 0xF) > 1)
		{
			nStage[i] = 2;
			nStages++;
		}
		else
		{
			if (((rInComms[0].valueH >> (i * 4)) & 0xF) > 1)
			{
				nStage[i] = 3;
				nStages++;
			}
			else
			{
				if (((rInComms[0].valueL >> (i * 4)) & 0xF) == 1)
				{
					if (((rInComms[0].valueH >> (i * 4)) & 0xF) == 1)
					{
						nStage[i] = 1;
						nStages++;
					}
					else
					{
						nStage[i] = 4;
						nStages++;
					}
				}
			}
		}
	//
	//первый этап
	if (nStage[0] == 2)
	{
		FILE* pFile;
		BRDCHAR bTempFileName[256] = { 0 };
		switch (m_pDevice->ProductID)
		{
		case 0x3018:
			BRDC_strcpy(bTempFileName, _BRDC("fmc118e_romprog.bit"));
			rInComms[1].command.status.swap = 1;
			break;
		case 0x3019:
			BRDC_strcpy(bTempFileName, _BRDC("fmc119e_romprog.bit"));
			break;
		case 0x3028:
			BRDC_strcpy(bTempFileName, _BRDC("fmc128e_romprog.bit"));
			break;
		case 0x3029:
			BRDC_strcpy(bTempFileName, _BRDC("fmc129e_romprog.bit"));
			break;
		default:
			break;
		}
		pFile = BRDC_fopen(bTempFileName, _BRDC("rb"));
		if (pFile != 0)
		{
			BRDC_fclose(pFile);
			Status = LoadPldFileDMA(bTempFileName, rInComms[0].valueL & 0xF, rInComms[0].valueH & 0xF, ep_in, maxBuf, &wTransPld, rInComms[1].command.status.doubling, rInComms[1].command.status.endean, rInComms[1].command.status.swap);
		}
	}
	else
	{
		FILE* pFile;
		BRDCHAR bTempFileName[256] = { 0 };
		switch (m_pDevice->ProductID)
		{
		case 0x3018:
			BRDC_strcpy(bTempFileName, _BRDC("fmc118e_romprog.bit"));
			rInComms[1].command.status.swap = 1;
			break;
		case 0x3019:
			BRDC_strcpy(bTempFileName, _BRDC("fmc119e_romprog.bit"));
			break;
		case 0x3028:
			BRDC_strcpy(bTempFileName, _BRDC("fmc128e_romprog.bit"));
			break;
		case 0x3029:
			BRDC_strcpy(bTempFileName, _BRDC("fmc129e_romprog.bit"));
			break;
		default:
			break;
		}
		pFile = BRDC_fopen(bTempFileName, _BRDC("rb"));
		if (pFile != 0)
		{
			BRDC_fclose(pFile);
			Status = LoadPldFileCom(false, bTempFileName, maxBuf, ep_in, 0xC1, rInComms[1].command.status.doubling, rInComms[1].command.status.endean, rInComms[1].command.status.swap, &wTransPld);
		}
	}
	//остальные этапы
	for (int i = 1; i < 8; i++)
	{
		nComm = 0;
		rOutComms[nComm].signatureValue = 0xC1;
		rOutComms[nComm].command.setup.type = 0;
		rOutComms[nComm].command.setup.request = 0xff;
		rOutComms[nComm].command.setup.requestH = 0x1f;
		rOutComms[nComm].valueL = 0x1;
		rOutComms[nComm].valueH = 0x1;
		rOutComms[nComm].transaction = wTransPld++;
		nComm++;
		rOutComms[nComm].signatureValue = 0xC1;
		rOutComms[nComm].command.setup.type = 0;
		rOutComms[nComm].command.setup.request = 0;
		rOutComms[nComm].valueL = 0;
		rOutComms[nComm].valueH = 0;
		rOutComms[nComm].transaction = wTransPld++;
		nComm++;
		nLen = nComm * 16;
		isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)rOutComms, nLen);
		isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)rInComms, nLen);
		maxBuf = rInComms[0].command.setup.request + 1;
		switch (nStage[i])
		{
		case 1://запись командами
			for (int j = 0; j < 8; j++)
				if (((rInComms[1].valueL >> (j * 4)) & 1) == 1)
				{
					maxBuf = 1 << (j * 4);
					maxBuf = maxBuf / 8;
					break;
				}
			if (m_pDevice->ProductID == 0x3018)
				rInComms[1].command.status.swap = 1;
			Status = LoadPldFileCom(true, PldFileName, maxBuf, ep_in, 0xC1, 0, 0, rInComms[1].command.status.swap, &wTransPld);
			break;
		case 2://запись в потоке
			Status = LoadPldFileDMA(PldFileName, ((rInComms[0].valueL >> (i * 4)) & 0xF), ((rInComms[0].valueH >> (i * 4)) & 0xF), ep_in, maxBuf, &wTransPld, rInComms[1].command.status.doubling, rInComms[1].command.status.endean, rInComms[1].command.status.swap);
			break;
		case 3://чтение в потоке

			break;
		case 4://стирание командами
			Status = ErasePld(rInComms[1].command.setup.requestH, ep_in, 0xC1, rInComms[1].command.setup.request, rInComms[1].valueH, rInComms[1].valueL, &wTransPld);
			break;
		default:
			break;
		}
	}
	if (Status == BRDerr_OK)
	{
		m_pDevice->SetAltIntfc(1);
		m_pDevice->SetAltIntfc(0);
	}
	return Status;
}

ULONG CBambusb::ReadPldFileTetrad(int nTetr, const BRDCHAR *PldFileName)
{
	ULONG Status = 0;
	LONG nLen;
	UINT32 ctrlReg = 0x203, addrLReg = 0x200, addrHReg = 0x201, sizeReg = 0x202, dataReg = 0x204, commNum = 0;
	rUsbHostCommand outComms[128] = { 0 }, inComms[128] = { 0 };
	DWORD xTime = 0;
	for (int i = 0; i < 128; i++)
	{
		outComms[i].signatureValue = 0xA5;
		outComms[i].taskID = m_TaskID;
	}
	outComms[0].shortTransaction = m_Trans++;
	outComms[0].command.tetrad.type = 2;
	outComms[0].command.tetrad.condition = 0;
	outComms[0].valueH = 0x100;
	//сброс тетрады
	xTime = IPC_getTickCount();
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 1;
	outComms[commNum].valueH = 0;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 0;
	outComms[commNum].valueH = 0;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	nLen = commNum * 16;
	SendBuffer(outComms, inComms, nLen);

	commNum = 0;
	//чтение статуса тетрады
	outComms[commNum].command.hardware.type = 1;
	outComms[commNum].command.hardware.port = 2;
	outComms[commNum].command.hardware.number = nTetr;
	outComms[commNum].command.hardware.write = 0;
	outComms[commNum].valueL = 0;
	outComms[commNum].valueH = 0;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	nLen = commNum * 16;
	SendBuffer(outComms, inComms, nLen);
	if (!(inComms[commNum - 1].valueL & 1))
		while (1)
		{
			//чтение статуса тетрады
			outComms[commNum].command.hardware.type = 1;
			outComms[commNum].command.hardware.port = 2;
			outComms[commNum].command.hardware.number = nTetr;
			outComms[commNum].command.hardware.write = 0;
			outComms[commNum].valueL = 0;
			outComms[commNum].valueH = 0;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
			nLen = commNum * 16;
			SendBuffer(outComms, inComms, nLen);
			if (inComms[commNum - 1].valueL & 1)
				break;
			commNum = 0;
		}
	//чтение конфиг регистра
	U32 regVal = 0;
	WriteRegData(0, nTetr, sizeReg, regVal);
	regVal = 0x65;
	WriteRegData(0, nTetr, ctrlReg, regVal);
	ReadRegData(0, nTetr, dataReg, regVal);
	U32 nConfReg = regVal;
	
	commNum = 0;
	//write enable
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 0x6;
	outComms[commNum].valueH = ctrlReg;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	//установка протокола
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = (nConfReg | 0xC0);
	outComms[commNum].valueH = dataReg;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 0x61; //write config register
	outComms[commNum].valueH = ctrlReg;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	nLen = commNum * 16;
	SendBuffer(outComms, inComms, nLen);

	regVal = 0;
	WriteRegData(0, nTetr, sizeReg, regVal);
	regVal = 0x65;
	WriteRegData(0, nTetr, ctrlReg, regVal);
	ReadRegData(0, nTetr, dataReg, regVal);

	regVal = 6;
	WriteRegData(0, nTetr, ctrlReg, regVal);

	regVal = 0;
	WriteRegData(0, nTetr, sizeReg, regVal);
	regVal = 0x5;
	WriteRegData(0, nTetr, ctrlReg, regVal);
	ReadRegData(0, nTetr, dataReg, regVal);

	if (regVal & 0x5C)
		return BRDerr_HW_ERROR;

	FILE *pFile;
	char str[261];
	U08 data[256], byte[261], isAddrSet = 0;
	U16 highAddress = 0, dataSize = 0;
	int recLen, offset, recType, checkSum, setAddress = 0;
	pFile = BRDC_fopen(PldFileName, _BRDC("rt"));
	//read and verification
	commNum = 0;
	isAddrSet = 0;
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].valueH = 10;
	outComms[commNum].valueL = 8;//dummy cycles for extended spi mode
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	dataSize = 0;
	U08 readData[256] = { 0 }, err = 0;
	pFile = BRDC_fopen(PldFileName, _BRDC("rt"));
	while (!feof(pFile))
	{
		fgets(str, 261, pFile);
		if (str[0] != ':')
			return BRDerr_BAD_FILE_FORMAT;
		sscanf(str + 1, "%2x", &recLen);
		sscanf(str + 3, "%4x", &offset);
		sscanf(str + 7, "%2x", &recType);
		byte[0] = (U08)recLen;
		byte[1] = (U08)(offset & 0xFF);
		byte[2] = (U08)(offset >> 8) & 0xFF;
		byte[3] = (U08)recType;
		if (recType == 1)//eof
		{
			if (isAddrSet == 1)
			{
				//write size of data
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = dataSize - 1;
				outComms[commNum].valueH = sizeReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				//fast read
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0xB;
				outComms[commNum].valueH = ctrlReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				nLen = 16 * commNum;
				SendBuffer(outComms, inComms, nLen);
				for (;;)
				{
					regVal = 0;
					ReadRegDataDir(0, nTetr, 0, regVal);
					if (regVal & 1)
						break;
				}
				//ReadRegBufDir(0, nTetr, 1, readData, 256);
				commNum = 0;
				for (int i = 0; i < 256; i += 4)
				{
					outComms[commNum].command.hardware.type = 1;
					outComms[commNum].command.hardware.write = 0;
					outComms[commNum].command.hardware.number = nTetr;
					outComms[commNum].command.hardware.port = 1;
					outComms[commNum].valueH = 1;
					outComms[commNum].valueL = 0;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
				}
				nLen = commNum * 16;
				SendBuffer(outComms, inComms, nLen);
				commNum = 0;
				for (int i = 0; i < 256; i += 4)
				{
					memcpy(&(readData[i]), &(inComms[commNum].valueL), 4);
					commNum++;
				}
				for (int i = 0; i < 256; i++)
					if (readData[i] != data[i])
						err++;
			}
			break;
		}
		for (int i = 0; i < recLen; i++)
			sscanf(str + 9 + 2 * i, "%2x", &byte[4 + i]);
		sscanf(str + 9 + 2 * recLen, "%2x", &checkSum);
		U08 crc, *pBlock = byte;
		U16 wLen;
		//подсчёт контрольной суммы
		crc = 0;
		wLen = recLen + 4;
		while (wLen--)
			crc += *pBlock++;
		crc = ~crc;
		crc += 1;
		crc &= 0xFF;
		if (crc != checkSum)
			return BRDerr_BAD_FILE_FORMAT;
		if (recType == 4)
		{
			highAddress = ((U16)byte[4] << 8) + byte[5];
			continue;
		}
		if (recType == 0)
		{
			if (isAddrSet == 0)
			{
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0x6;
				outComms[commNum].valueH = ctrlReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0;
				outComms[commNum].valueH = sizeReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				if (highAddress & 0x100)
				{
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 1;
					outComms[commNum].valueH = dataReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 0xC5;
					outComms[commNum].valueH = ctrlReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
				}
				else
				{
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 0;
					outComms[commNum].valueH = dataReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 0xC5;
					outComms[commNum].valueH = ctrlReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
				}
				//write high bytes of address
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = highAddress;
				outComms[commNum].valueH = addrHReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				//write low bytes of address
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = ((U32)byte[2] << 8) + byte[1];
				outComms[commNum].valueH = addrLReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				isAddrSet = 1;
			}
			if ((dataSize + byte[0]) <= 256)
			{
				for (int i = dataSize; i < dataSize + byte[0]; i++)
					data[i] = byte[i - dataSize + 4];
				dataSize = dataSize + byte[0];
				setAddress = ((U32)highAddress << 16) + ((U32)byte[2] << 8) + byte[1] + byte[0];
			}
			if (dataSize < 256)
				continue;
			//write size of data
			outComms[commNum].command.tetrad.type = 2;
			outComms[commNum].command.tetrad.condition = 4;
			outComms[commNum].command.tetrad.number = nTetr;
			outComms[commNum].valueL = dataSize - 1;
			outComms[commNum].valueH = sizeReg;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
			//fast read
			outComms[commNum].command.tetrad.type = 2;
			outComms[commNum].command.tetrad.condition = 4;
			outComms[commNum].command.tetrad.number = nTetr;
			outComms[commNum].valueL = 0xB;
			outComms[commNum].valueH = ctrlReg;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
			nLen = 16 * commNum;
			SendBuffer(outComms, inComms, nLen);
			for (;;)
			{
				regVal = 0;
				ReadRegDataDir(0, nTetr, 0, regVal);
				if (regVal & 1)
					break;
			}
			//ReadRegBufDir(0, nTetr, 1, readData, 256);
			commNum = 0;
			for (int i = 0; i < 256; i += 4)
			{
				outComms[commNum].command.hardware.type = 1;
				outComms[commNum].command.hardware.write = 0;
				outComms[commNum].command.hardware.number = nTetr;
				outComms[commNum].command.hardware.port = 1;
				outComms[commNum].valueH = 1;
				outComms[commNum].valueL = 0;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
			}
			nLen = commNum * 16;
			SendBuffer(outComms, inComms, nLen);
			commNum = 0;
			for (int i = 0; i < 256; i += 4)
			{
				memcpy(&(readData[i]), &(inComms[commNum].valueL), 4);
				commNum++;
			}
			for (int i = 0; i < 256; i++)
				if (readData[i] != data[i])
					err++;
			isAddrSet = 0;
			dataSize = 0;
			commNum = 0;
		}
	}
	fclose(pFile);
	xTime = IPC_getTickCount() - xTime;
	printf("verification in %d ms", xTime);
	Status = BRDerr_OK;
	if (err != 0)
		Status = BRDerr_FATAL;
	return Status;
}

ULONG CBambusb::ReadPldFileAlt(const BRDCHAR *PldFileName)
{
	unsigned int ep_in = 2, nComm = 0, maxBuf = 0, nDevName = 0, nSerial = 0, nStage[8] = { 0 };
	int isRes = 0;
	LONG nLen;
	ULONG Status = 0;
	USHORT wTransPld = 0;
	rUsbHostCommand rOutComms[256] = { 0 }, rInComms[256] = { 0 };
	isRes = m_pDevice->SetAltIntfc(0);
	isRes = m_pDevice->SetAltIntfc(2);
	IPC_delay(1000);
	for (int i = 0; i < m_pDevice->EndPointCount(); i++)
		if (m_pDevice->EndPoints[i]->Address == 0x81)
		{
			ep_in = i;
			break;
		}
	DWORD x = IPC_getTickCount();
	rOutComms[nComm].signatureValue = 0xC1;
	rOutComms[nComm].command.setup.type = 0;
	rOutComms[nComm].command.setup.request = 0xff;
	rOutComms[nComm].command.setup.requestH = 0x1f;
	rOutComms[nComm].valueL = 0x1;
	rOutComms[nComm].valueH = 0x1;
	rOutComms[nComm].transaction = wTransPld++;
	nComm++;
	rOutComms[nComm].signatureValue = 0xC1;
	rOutComms[nComm].command.setup.type = 0;
	rOutComms[nComm].command.setup.request = 0;
	rOutComms[nComm].valueL = 0;
	rOutComms[nComm].valueH = 0;
	rOutComms[nComm].transaction = wTransPld++;
	nComm++;
	nLen = nComm * 16;
	isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)rOutComms, nLen);
	isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)rInComms, nLen);
	maxBuf = rInComms[0].command.setup.request + 1;
	//определение количества этапов и их типа 0-отсутствие этапа, 1-командный режим, 2-запись в потоке, 3-чтение в потоке, 4-стирание командами
	for (int i = 0; i < 8; i++)
		if (((rInComms[0].valueL >> (i * 4)) & 0xF) > 1)
		{
			nStage[i] = 2;
		}
		else
		{
			if (((rInComms[0].valueH >> (i * 4)) & 0xF) > 1)
			{
				nStage[i] = 3;
			}
			else
			{
				if (((rInComms[0].valueL >> (i * 4)) & 0xF) == 1)
				{
					if (((rInComms[0].valueH >> (i * 4)) & 0xF) == 1)
						nStage[i] = 1;
					else
						nStage[i] = 4;
				}
			}
		}
	nComm = 0;
	FILE *pFile;
	ULONG fSize = 0;
	fSize = GetMCSDataSize(PldFileName);
	rOutComms[nComm].signatureValue = 0xC1;
	rOutComms[nComm].command.setup.type = 0;
	rOutComms[nComm].command.setup.request = 0xff;
	rOutComms[nComm].command.setup.requestH = 0x1f;
	rOutComms[nComm].valueL = 0;
	rOutComms[nComm].valueH = 0x1FFFFFFF;
	rOutComms[nComm].transaction = wTransPld++;
	nComm++;
	rOutComms[nComm].signatureValue = 0xC1;
	rOutComms[nComm].command.setup.type = 0;
	rOutComms[nComm].command.setup.request = 0;
	rOutComms[nComm].valueL = 0;
	rOutComms[nComm].valueH = 0;
	rOutComms[nComm].transaction = wTransPld++;
	nComm++;
	nLen = nComm * 16;
	isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)rOutComms, nLen);
	isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)rInComms, nLen);
	maxBuf = rInComms[0].command.setup.request + 1;
	//read
	if (nStage[1] == 3)
	{
		UINT nStreamEp = (rInComms[0].valueH >> 4) & 0xF;
		nStreamEp = ep_in + nStreamEp - 1;
		//FILE *pFile;
		ULONG nFileSize = 0, nBytesTransferred = 0;
		nFileSize = GetMCSDataSize(PldFileName);
		pFile = BRDC_fopen(PldFileName, _BRDC("r"));
		CallBackLoadProgress(1, 1, 0, nFileSize);
		nComm = 0;
		if (pFile != 0)
		{
			ULONG nRealRead = 0;
			UCHAR abBuf[4096] = { 0 }, abRead[4096] = { 0 };
			while (!feof(pFile))
			{
				UINT nOffset = 0;
				UINT nAddr = 0, nType = 0, nLength = 0, nAddrPrev = 0;
				for (int i = 0; i < 1024; i++)
				{
					char str[256] = { 0 };
					nAddr = 0; nType = 0; nLength = 0;
					fgets(str, 256, pFile);
					if (feof(pFile))
						break;
					if (str[0] != ':')
					{
						if (feof(pFile))
							break;
						else
							return BRDerr_BAD_FILE_FORMAT;
					}
					sscanf(str + 1, "%2x", &nLength);
					sscanf(str + 3, "%4x", &nAddr);
					sscanf(str + 7, "%2x", &nType);
					nLength = (nLength & 0xFF);
					nAddr = (nAddr & 0xFFFF);
					nType = (nType & 0xFF);
					if (nType == 4)
					{
						continue;
					}
					if (nType == 1)
					{
						break;
					}
					if ((nAddr != nAddrPrev) && (nAddrPrev != 0))
						nAddrPrev = nAddr;
					nAddrPrev = nAddr + nLength;
					for (UINT j = 0; j < nLength; j++)
						sscanf(str + 9 + 2 * j, "%2x", abBuf + nOffset + j);
					nOffset += nLength;
					if (nOffset + 1 >= 2048)
						break;
					if (feof(pFile))
						break;
				}
				nRealRead = nOffset;
				nLen = nRealRead;
				m_pDevice->EndPoints[nStreamEp]->XferData(abRead, nLen);
				for (UINT i = 0; i < nRealRead; i++)
					if (abBuf[i] != abRead[i])
					{
						Status = BRDerr_ERROR;
						break;
					}
				nBytesTransferred += nRealRead;
				CallBackLoadProgress(1, 1, nBytesTransferred, nFileSize);
				if (Status != 0)
					break;
			}
			BRDC_fclose(pFile);
		}
		else
			Status = BRDerr_BAD_FILE;
	}
	else
		Status = BRDerr_INSUFFICIENT_RESOURCES;
	//
	//if (Status == BRDerr_OK)
	{
		bool res = false;
		U32 lVal = 0;
		res = m_pDevice->SetAltIntfc(1);
		res = m_pDevice->SetAltIntfc(0);
		IPC_delay(3000);
		if (m_TaskID != 0xFF)
		{
			m_TaskID = 0;
			SendCommand(0, 0, 0, lVal, 0, 0, 1, 1);//получение TaskID для захвата тетрад и стримов
		}
	}
	return Status;
}

UINT32 LtoBend(UCHAR x)
{
	UINT32 y = 0;
	for (int i = 0; i < 8; i++)
		y += (((x >> i) & 1) << (7 - i));
	return y;
}

ULONG CBambusb::ErasePld(int nTimeExp, unsigned int ep_in, unsigned char bSignature, unsigned char bEdge, unsigned int nHighAddr, unsigned int nLastAddr, USHORT *wTrans)
{
	ULONG Status = 0;
	int nSecSize = 0;
	for (int i = 0; i < 8; i++)
		if (((nLastAddr >> (i * 4)) & 1) == 1)
			nSecSize = 1 << (i * 4);
	int nSecCount = (nHighAddr + 1) / nSecSize;
	int isRes = 0;
	USHORT wTransPld = *wTrans;
	rUsbHostCommand outComm[2] = { 0 }, inComm[2] = { 0 };
	LONG nLen = 0;
	ULONG nTotalTime = (ULONG)(nSecCount * pow(2, nTimeExp / 2));
	DWORD xTime = IPC_getTickCount();
	if ((bEdge & 0x7F) != 0x7F)
	{//стирание целиком
		CallBackLoadProgress(3, 4, 0, nTotalTime);
		outComm[0].signatureValue = bSignature;
		outComm[0].transaction = wTransPld++;
		outComm[0].command.program.type = 3;
		outComm[0].command.program.address = 0;
		outComm[0].command.program.addrH = 0;
		outComm[0].valueL = 0;
		outComm[0].valueH = 0xFFFFFFFF;
		nLen = 16;
		isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)&outComm, nLen);
		isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)&inComm, nLen);
		for (int i = 0; i < 7000; i++)
		{
			outComm[0].signatureValue = bSignature;
			outComm[0].transaction = wTransPld++;
			outComm[0].command.read.type = 2;
			outComm[0].command.read.address = 0;
			outComm[0].command.read.addrH = 0;
			outComm[0].valueL = 0;
			outComm[0].valueH = 0;
			outComm[1].signatureValue = bSignature;
			outComm[1].transaction = wTransPld++;
			outComm[1].command.setup.type = 0;
			outComm[1].command.setup.request = 0;
			outComm[1].command.setup.requestH = 0;
			outComm[1].value = 0;
			nLen = 32;
			isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)&outComm, nLen);
			isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)&inComm, nLen);
			if (inComm[1].command.status.ready == 1)
				if ((inComm[1].command.setup.requestH & 1) == 0)
					break;
			IPC_delay(50);
			CallBackLoadProgress(3, 4, (ULONG)(IPC_getTickCount() - xTime), nTotalTime);
		}
		CallBackLoadProgress(3, 4, (ULONG)(IPC_getTickCount() - xTime), nTotalTime);
	}
	else
	{//стирание секторами
		UINT nAddr = 0;
		CallBackLoadProgress(3, 4, 0, nSecCount);
		for (int i = 0; i < nSecCount; i++)
		{
			outComm[0].signatureValue = bSignature;
			outComm[0].transaction = wTransPld++;
			outComm[0].command.program.type = 3;
			outComm[0].command.program.address = 0;
			outComm[0].command.program.addrH = 0;
			outComm[0].valueL = nAddr;
			outComm[0].valueH = nSecSize - 1;
			nAddr += nSecSize;
			nLen = 16;
			isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)&outComm, nLen);
			isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)&inComm, nLen);
			for (int j = 0; j < 700; j++)
			{
				outComm[0].signatureValue = bSignature;
				outComm[0].transaction = wTransPld++;
				outComm[0].command.read.type = 2;
				outComm[0].command.read.address = 0;
				outComm[0].command.read.addrH = 0;
				outComm[0].valueL = 0;
				outComm[0].valueH = 0;
				outComm[1].signatureValue = bSignature;
				outComm[1].transaction = wTransPld++;
				outComm[1].command.setup.type = 0;
				outComm[1].command.setup.request = 0;
				outComm[1].command.setup.requestH = 0;
				outComm[1].value = 0;
				nLen = 32;
				isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)&outComm, nLen);
				isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)&inComm, nLen);
				if (inComm[1].command.status.ready == 1)
					if ((inComm[1].command.setup.requestH & 1) == 0)
						break;
				IPC_delay(50);
			}
			CallBackLoadProgress(3, 4, i, nSecCount);
		}
	}
	*wTrans = wTransPld;
	return Status;
}

ULONG CBambusb::LoadPldFileCom(bool isFPGAROM, const BRDCHAR *PldFileName, int nMaxBuf, unsigned int ep_in, unsigned char bSignature, unsigned char bDoubling, unsigned char bEndean, unsigned char bSwap, USHORT *wTrans)
{
	FILE *pFile;
	rUsbHostCommand outComms[256] = {0}, inComms[256] = {0};
	int isRes, nComm = 0, addrBuf = 0, nBuf = 0, addrROM = 0, nAddrRomNext = -2;
	LONG nLen, nLenR;
	ULONG Status = 0, nFileSize = 0, nBytesTransferred = 0;
	UCHAR bBuf[1024] = {0}, bByte[8] = {0};
	USHORT wTransPld = *wTrans;
	//IPC_handle hComm, hCommR;
	OVERLAPPED rOv = {0}, rOv2 = {0};
	UCHAR isWaitReply = 0;
	PUCHAR rRes = 0, rRes2 = 0;
	size_t nRealRead = 0;
	UCHAR nBufStep = (1 << (bDoubling - 1));
	if (bDoubling == 0)
		nBufStep = 8;
	if (isFPGAROM)
		nFileSize = GetMCSDataSize(PldFileName);
	pFile = BRDC_fopen(PldFileName, _BRDC("rb"));
	if (!isFPGAROM)
	{
		fseek(pFile, 0, SEEK_END);
		nFileSize = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);
	}
	for (int i = 0; i < 256; i++)
		outComms[i].signatureValue = bSignature;
	while(!feof(pFile))
	{
		if (isFPGAROM)
		{
			UINT nOffset = 0;
			UINT nAddr = 0, nType = 0, nLength = 0, nAddrPrev = 0;
			for (int i = 0; i < 1024; i++)
			{
				char str[256] = {0};
				nAddr = 0; nType = 0; nLength = 0;
				fgets(str, 256, pFile);
				if (feof(pFile))
					break;
				if (str[0] != ':')
				{
					if (feof(pFile))
						break;
					else
						return BRDerr_BAD_FILE_FORMAT;
				}
				sscanf(str + 1, "%2x", &nLength);
				sscanf(str + 3, "%4x", &nAddr);
				sscanf(str + 7, "%2x", &nType);
				nLength = (nLength & 0xFF);
				nAddr = (nAddr & 0xFFFF);
				nType = (nType & 0xFF);
				if (nType == 4)
				{
					continue;
					/*sscanf(str + 9, "%4x", &nAddrRomNext);
					nAddrPrev = 0;
					if (i == 0)
						continue;
					else
						break;*/
				}
				if (nType == 1)
				{
					break;
				}
				if ((nAddr != nAddrPrev) && (nAddrPrev != 0))
					nAddrPrev = nAddr;
				nAddrPrev = nAddr + nLength;
				for (UINT j = 0; j < nLength; j++)
					sscanf(str + 9 + 2*j, "%2x", bBuf + nOffset + j);
				nOffset += nLength;
				CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
				if (nOffset + 1 >= 512)
					break;
				if (feof(pFile))
					break;
			}
			nRealRead = nOffset;
		}
		else
			nRealRead = fread(bBuf, 1, 512, pFile);
		//вариант для записи покомандно
		/*for (nBuf = 0; nBuf < nRealRead; nBuf += nBufStep)
		{
			((int*)bByte)[0] = ((int*)(bBuf + nBuf))[0];
			((int*)bByte)[1] = ((int*)(bBuf + nBuf))[1];
			nComm = 0;
			outComms[nComm].transaction = wTransPld++;
			outComms[nComm].command.buffer.type = 1;
			outComms[nComm].command.buffer.address = 0;
			outComms[nComm].command.buffer.addrH = 0;
			outComms[nComm].valueL = (bByte[0]) + (((bByte[1])) << 8) + (((bByte[2])) << 16) + (((bByte[3])) << 24);
			outComms[nComm].valueH = (bByte[4]) + (((bByte[5])) << 8) + (((bByte[6])) << 16) + (((bByte[7])) << 24);
			nComm++;
			outComms[nComm].transaction = wTransPld++;
			outComms[nComm].command.program.type = 3;
			outComms[nComm].command.program.address = 0;
			outComms[nComm].command.program.addrH = 0;
			outComms[nComm].valueL = addrROM;
			outComms[nComm].valueH = 7;
			addrROM += 8;
			nComm++;
			nLen = nComm * 16;
			m_pDevice->EndPoints[1]->XferData((PUCHAR)&outComms, nLen);
			m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)&inComms, nLen);
		}*/
		CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
		for (nBuf = 0; nBuf < nRealRead; nBuf += nBufStep)
		{
			((int*)bByte)[0] = ((int*)(bBuf+nBuf))[0];
			switch (bDoubling)
			{
			case 0:
				((int*)bByte)[1] = ((int*)(bBuf + nBuf))[1];
				break;
			case 1:
				for (int i = 0; i < 8; i += 2)
				{
					bByte[i] = bBuf[nBuf + i];
					bByte[i+1] = bBuf[nBuf + i];
				}
				break;
			case 2:
				for (int i = 0; i < 4; i += 2)
				{
					((short*)bByte)[i] = ((short*)(bBuf + nBuf))[i];
					((short*)bByte)[i+1] = ((short*)(bBuf + nBuf))[i];
				}
				break;
			case 3:
				((int*)bByte)[1] = ((int*)(bBuf + nBuf))[0];
				break;
			default:
				((int*)bByte)[1] = ((int*)(bBuf + nBuf))[1];
				break;
			}
			outComms[nComm].transaction = wTransPld++;
			outComms[nComm].command.buffer.type = 1;
			outComms[nComm].command.buffer.address = addrBuf;
			if (addrBuf > 0xFF)
				outComms[nComm].command.buffer.addrH = (addrBuf >> 8);
			else
				outComms[nComm].command.buffer.addrH = 0;
			outComms[nComm].value = 0;
			if (bSwap)
			{
				for (int i = 0; i < nBufStep; i++)
					if (i > 3)
						switch (bEndean)
						{
						case 0:
							outComms[nComm].valueH += (LtoBend(bByte[i]) << ((i - 4) * 8));
							break;
						case 1:
							if (i > 1)
								outComms[nComm].valueH += (LtoBend(bByte[13 - i]) << ((i - 4) * 8));
							else
								outComms[nComm].valueH += (LtoBend(bByte[9 - i]) << ((i - 4) * 8));
							break;
						case 2:
							outComms[nComm].valueH += (LtoBend(bByte[11 - i]) << ((i - 4) * 8));
							break;
						case 3:
							outComms[nComm].valueH += (LtoBend(bByte[7 - i]) << ((i - 4) * 8));
							break;
						default:
							outComms[nComm].valueH += (LtoBend(bByte[i]) << ((i - 4) * 8));
							break;
						}
					else
						switch (bEndean)
						{
						case 0:
							outComms[nComm].valueL += (LtoBend(bByte[i]) << ((i - 4) * 8));
							break;
						case 1:
							if (i > 1)
								outComms[nComm].valueL += (LtoBend(bByte[5 - i]) << ((i - 4) * 8));
							else
								outComms[nComm].valueL += (LtoBend(bByte[1 - i]) << ((i - 4) * 8));
							break;
						case 2:
							outComms[nComm].valueL += (LtoBend(bByte[11 - i]) << ((i - 4) * 8));
							break;
						case 3:
							outComms[nComm].valueL += (LtoBend(bByte[7 - i]) << ((i - 4) * 8));
							break;
						default:
							outComms[nComm].valueL += (LtoBend(bByte[i]) << ((i - 4) * 8));
							break;
						}

				//outComms[nComm].valueL = LtoBend(bByte[0]) + ((LtoBend(bByte[1])) << 8) + ((LtoBend(bByte[2])) << 16) + ((LtoBend(bByte[3])) << 24);
				//outComms[nComm].valueH = LtoBend(bByte[4]) + ((LtoBend(bByte[5])) << 8) + ((LtoBend(bByte[6])) << 16) + ((LtoBend(bByte[7])) << 24);
			}
			else
			{
				for (int i = 0; i < nBufStep; i++)
					if (i > 3)
						switch (bEndean)
						{
						case 0:
							outComms[nComm].valueH += ((bByte[i]) << ((i - 4) * 8));
							break;
						case 1:
							if (i > 1)
								outComms[nComm].valueH += ((bByte[13 - i]) << ((i - 4) * 8));
							else
								outComms[nComm].valueH += ((bByte[9 - i]) << ((i - 4) * 8));
							break;
						case 2:
							outComms[nComm].valueH += ((bByte[11 - i]) << ((i - 4) * 8));
							break;
						case 3:
							outComms[nComm].valueH += ((bByte[7 - i]) << ((i - 4) * 8));
							break;
						default:
							outComms[nComm].valueH += ((bByte[i]) << ((i - 4) * 8));
							break;
						}
					else
						switch (bEndean)
						{
						case 0:
							outComms[nComm].valueL += ((bByte[i]) << ((i - 4) * 8));
							break;
						case 1:
							if (i > 1)
								outComms[nComm].valueL += ((bByte[5 - i]) << ((i - 4) * 8));
							else
								outComms[nComm].valueL += ((bByte[1 - i]) << ((i - 4) * 8));
							break;
						case 2:
							outComms[nComm].valueL += ((bByte[11 - i]) << ((i - 4) * 8));
							break;
						case 3:
							outComms[nComm].valueL += ((bByte[7 - i]) << ((i - 4) * 8));
							break;
						default:
							outComms[nComm].valueL += ((bByte[i]) << ((i - 4) * 8));
							break;
						}
				//outComms[nComm].valueL = (bByte[0]) + (((bByte[1])) << 8) + (((bByte[2])) << 16) + (((bByte[3])) << 24);
				//outComms[nComm].valueH = (bByte[4]) + (((bByte[5])) << 8) + (((bByte[6])) << 16) + (((bByte[7])) << 24);
			}
			CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
			nComm++;
			addrBuf++;
			if (nComm == 128)
			{
				if (isWaitReply & 1)
				{
					while (1)
					{
						isRes = m_pDevice->EndPoints[ep_in]->WaitForXfer(&rOv, 10);
						CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
						if (isRes)
							break;
					}
					isRes = m_pDevice->EndPoints[ep_in]->FinishDataXfer((PUCHAR)inComms, nLenR, &rOv, rRes);
					nBytesTransferred += (nLenR / 2);
					CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
					if (!isRes)
					{
						Status = BRDerr_HW_ERROR;
						break;
					}
					isWaitReply &= 2;
				}
				nLen = nComm * 16;
				isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)outComms, nLen);
				if (!isRes)
				{
					Status = BRDerr_HW_ERROR;
					break;
				}
				rRes = m_pDevice->EndPoints[ep_in]->BeginDataXfer((PUCHAR)inComms, nLen, &rOv);
				nLenR = nLen;
				isWaitReply |= 1;
			}
			if (nComm == 256)
			{
				if (isWaitReply & 2)
				{
					while (1)
					{
						isRes = m_pDevice->EndPoints[ep_in]->WaitForXfer(&rOv2, 10);
						CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
						if (isRes)
							break;
					}
					isRes = m_pDevice->EndPoints[ep_in]->FinishDataXfer((PUCHAR)&(inComms[128]), nLenR, &rOv2, rRes2);
					nBytesTransferred += (nLenR / 2);
					CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
					if (!isRes)
					{
						Status = BRDerr_HW_ERROR;
						break;
					}
					isWaitReply &= 1;
				}
				nLen = (nComm/2) * 16;
				isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)&(outComms[128]), nLen);
				if (!isRes)
				{
					Status = BRDerr_HW_ERROR;
					break;
				}
				rRes2 = m_pDevice->EndPoints[ep_in]->BeginDataXfer((PUCHAR)&(inComms[128]), nLen, &rOv2);
				nLenR = nLen;
				isWaitReply |= 2;
				nComm = 0;
				if (isFPGAROM)
				{
					if (isWaitReply & 1)
					{
						while (1)
						{
							isRes = m_pDevice->EndPoints[ep_in]->WaitForXfer(&rOv, 10);
							CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
							if (isRes)
								break;
						}
						isRes = m_pDevice->EndPoints[ep_in]->FinishDataXfer((PUCHAR)inComms, nLenR, &rOv, rRes);
						nBytesTransferred += (nLenR / 2);
						CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
						if (!isRes)
						{
							Status = BRDerr_HW_ERROR;
						}
						isWaitReply &= 2;
					}
					while (1)
					{
						isRes = m_pDevice->EndPoints[ep_in]->WaitForXfer(&rOv2, 10);
						CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
						if (isRes)
							break;
					}
					isRes = m_pDevice->EndPoints[ep_in]->FinishDataXfer((PUCHAR)&(inComms[128]), nLenR, &rOv2, rRes2);
					nBytesTransferred += (nLenR / 2);
					CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
					if (!isRes)
					{
						Status = BRDerr_HW_ERROR;
					}
					isWaitReply &= 1;
					for (int i = 0; i < (256 / nMaxBuf); i++)
					{
						outComms[nComm].command.program.type = 3;
						outComms[nComm].command.program.address = i * nMaxBuf * 8;
						outComms[nComm].command.program.addrH = 0;
						outComms[nComm].transaction = wTransPld++;
						outComms[nComm].valueL = addrROM;
						outComms[nComm].valueH = (nMaxBuf * 8) - 1;
						nComm++;
						addrROM += (nMaxBuf * 8);
						nLen = nComm * 16;
						isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)outComms, nLen);
						if (!isRes)
						{
							Status = BRDerr_HW_ERROR;
						}
						isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)inComms, nLen);
						if (!isRes)
						{
							Status = BRDerr_HW_ERROR;
						}
						nBytesTransferred += (nLen / 2);
						CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
						nComm = 0;
						for (int j = 0; j < 5000; j++)
						{
							outComms[0].signatureValue = bSignature;
							outComms[0].transaction = wTransPld++;
							outComms[0].command.read.type = 2;
							outComms[0].command.read.address = 0;
							outComms[0].command.read.addrH = 0;
							outComms[0].valueL = 0;
							outComms[0].valueH = 0;
							outComms[1].signatureValue = bSignature;
							outComms[1].transaction = wTransPld++;
							outComms[1].command.setup.type = 0;
							outComms[1].command.setup.request = 0;
							outComms[1].command.setup.requestH = 0;
							outComms[1].value = 0;
							nLen = 32;
							isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)&outComms, nLen);
							isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)&inComms, nLen);
							if (inComms[1].command.status.ready == 1)
								if ((inComms[1].command.setup.requestH & 1) == 0)
								{
									break;
								}
							IPC_delay(1);
							CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
						}
					}
				}
			}
			if (addrBuf >= nMaxBuf)
			{
				if (isFPGAROM)
				{
					for (int i = 0; i < 5000; i++)
					{
						outComms[nComm].signatureValue = bSignature;
						outComms[nComm].transaction = wTransPld++;
						outComms[nComm].command.read.type = 2;
						outComms[nComm].command.read.address = 0;
						outComms[nComm].command.read.addrH = 0;
						outComms[nComm].valueL = 0;
						outComms[nComm].valueH = 0;
						nComm++;
						outComms[nComm].signatureValue = bSignature;
						outComms[nComm].transaction = wTransPld++;
						outComms[nComm].command.setup.type = 0;
						outComms[nComm].command.setup.request = 0;
						outComms[nComm].command.setup.requestH = 0;
						outComms[nComm].value = 0;
						nComm++;
						nLen = nComm * 16;
						isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)&outComms, nLen);
						isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)&inComms, nLen);
						if (nLen > 32)
						{
							nBytesTransferred += (nLen - 32) / 2;
							CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
						}
						if (inComms[nComm-1].command.status.ready == 1)
							if ((inComms[nComm-1].command.setup.requestH & 1) == 0)
							{
								break;
							}
						IPC_delay(1);
						nComm = 0;
						CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
					}
					nComm = 0;
				}
				outComms[nComm].command.program.type = 3;
				outComms[nComm].command.program.address = 0;
				outComms[nComm].command.program.addrH = 0;
				outComms[nComm].transaction = wTransPld++;
				outComms[nComm].valueL = addrROM;
				outComms[nComm].valueH = (8*nMaxBuf) - 1;
				addrBuf = 0;
				addrROM += 8*nMaxBuf;
				nComm++;
				if (isFPGAROM)
				{
					nLen = nComm * 16;
					isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)outComms, nLen);
					if (!isRes)
					{
						Status = BRDerr_HW_ERROR;
					}
					isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)inComms, nLen);
					if (!isRes)
					{
						Status = BRDerr_HW_ERROR;
					}
					nComm = 0;
					for (int j = 0; j < 5000; j++)
					{
						outComms[0].signatureValue = bSignature;
						outComms[0].transaction = wTransPld++;
						outComms[0].command.read.type = 2;
						outComms[0].command.read.address = 0;
						outComms[0].command.read.addrH = 0;
						outComms[0].valueL = 0;
						outComms[0].valueH = 0;
						outComms[1].signatureValue = bSignature;
						outComms[1].transaction = wTransPld++;
						outComms[1].command.setup.type = 0;
						outComms[1].command.setup.request = 0;
						outComms[1].command.setup.requestH = 0;
						outComms[1].value = 0;
						nLen = 32;
						isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)&outComms, nLen);
						isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)&inComms, nLen);
						if (inComms[1].command.status.ready == 1)
							if ((inComms[1].command.setup.requestH & 1) == 0)
							{
								break;
							}
						IPC_delay(1);
						CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
					}
				}
				else
				{
					if (nComm == 128)
					{
						if (isWaitReply & 1)
						{
							while (1)
							{
								isRes = m_pDevice->EndPoints[ep_in]->WaitForXfer(&rOv, 10);
								CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
								if (isRes)
									break;
							}
							isRes = m_pDevice->EndPoints[ep_in]->FinishDataXfer((PUCHAR)inComms, nLenR, &rOv, rRes);
							nBytesTransferred += (nLenR / 2);
							CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
							if (!isRes)
							{
								Status = BRDerr_HW_ERROR;
								break;
							}
							isWaitReply &= 2;
						}
						nLen = nComm * 16;
						isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)outComms, nLen);
						if (!isRes)
						{
							Status = BRDerr_HW_ERROR;
							break;
						}
						rRes = m_pDevice->EndPoints[ep_in]->BeginDataXfer((PUCHAR)inComms, nLen, &rOv);
						nLenR = nLen;
						isWaitReply |= 1;
						//nComm = 0;
					}
					if (nComm == 256)
					{
						if (isWaitReply & 2)
						{
							while (1)
							{
								isRes = m_pDevice->EndPoints[ep_in]->WaitForXfer(&rOv2, 10);
								CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
								if (isRes)
									break;
							}
							isRes = m_pDevice->EndPoints[ep_in]->FinishDataXfer((PUCHAR)&(inComms[128]), nLenR, &rOv2, rRes2);
							nBytesTransferred += (nLenR / 2);
							CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
							if (!isRes)
							{
								Status = BRDerr_HW_ERROR;
								break;
							}
							isWaitReply &= 1;
						}
						nLen = (nComm / 2) * 16;
						isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)&(outComms[128]), nLen);
						if (!isRes)
						{
							Status = BRDerr_HW_ERROR;
							break;
						}
						rRes2 = m_pDevice->EndPoints[ep_in]->BeginDataXfer((PUCHAR)&(inComms[128]), nLen, &rOv2);
						nLenR = nLen;
						isWaitReply |= 2;
						nComm = 0;
					}
				}
			}
		}
	}
	
	if (isWaitReply & 1)
	{
		while (1)
		{
			isRes = m_pDevice->EndPoints[ep_in]->WaitForXfer(&rOv, 10);
			CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
			if (isRes)
				break;
		}
		isRes = m_pDevice->EndPoints[ep_in]->FinishDataXfer((PUCHAR)inComms, nLenR, &rOv, rRes);
		nBytesTransferred += (nLenR / 2);
		CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
		if (!isRes)
		{
			Status = BRDerr_HW_ERROR;
		}
		isWaitReply &= 2;
	}
	if (isWaitReply & 2)
	{
		while (1)
		{
			isRes = m_pDevice->EndPoints[ep_in]->WaitForXfer(&rOv2, 10);
			CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
			if (isRes)
				break;
		}
		isRes = m_pDevice->EndPoints[ep_in]->FinishDataXfer((PUCHAR)&(inComms[128]), nLenR, &rOv2, rRes2);
		nBytesTransferred += (nLenR / 2);
		CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
		if (!isRes)
		{
			Status = BRDerr_HW_ERROR;
		}
		isWaitReply &= 1;
	}
	if (nComm != 0)
	{
		if (addrBuf != 0)
		{
			outComms[nComm].command.program.type = 3;
			outComms[nComm].command.program.address = 0;
			outComms[nComm].command.program.addrH = 0;
			outComms[nComm].transaction = wTransPld++;
			outComms[nComm].valueL = addrROM;
			outComms[nComm].valueH = (nMaxBuf*8) - 1;
			nComm++;
		}
		if (nComm > 128)
		{
			nLen = (nComm-128) * 16;
			isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)&(outComms[128]), nLen);
		}
		else
		{
			nLen = nComm * 16;
			isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)outComms, nLen);
		}
		if (!isRes)
		{
			Status = BRDerr_HW_ERROR;
		}
		isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)inComms, nLen);
		if (!isRes)
		{
			Status = BRDerr_HW_ERROR;
		}
		nBytesTransferred += (nLen / 2);
		CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
		nComm = 0;
		if (isFPGAROM)
		{
			Status = BRDerr_ERROR;
			for (int j = 0; j < 700; j++)
			{
				outComms[0].signatureValue = bSignature;
				outComms[0].transaction = wTransPld++;
				outComms[0].command.read.type = 2;
				outComms[0].command.read.address = 0;
				outComms[0].command.read.addrH = 0;
				outComms[0].valueL = 0;
				outComms[0].valueH = 0;
				outComms[1].signatureValue = bSignature;
				outComms[1].transaction = wTransPld++;
				outComms[1].command.setup.type = 0;
				outComms[1].command.setup.request = 0;
				outComms[1].command.setup.requestH = 0;
				outComms[1].value = 0;
				nLen = 32;
				isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)&outComms, nLen);
				isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)&inComms, nLen);
				if (nLen > 32)
				{
					nBytesTransferred += (nLen - 32) / 2;
					CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
				}
				if (inComms[1].command.status.ready == 1)
					if ((inComms[1].command.setup.requestH & 1) == 0)
					{
						Status = BRDerr_OK;
						break;
					}
				IPC_delay(50);
				CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
			}
		}
		else
		{
			outComms[nComm].command.program.type = 3;
			outComms[nComm].command.program.address = 0;
			outComms[nComm].command.program.addrH = 0;
			outComms[nComm].transaction = wTransPld++;
			outComms[nComm].valueL = addrROM;
			outComms[nComm].valueH = (nMaxBuf * 8) - 1;
			nComm++;
			nLen = nComm * 16;
			isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)outComms, nLen);
			if (!isRes)
			{
				Status = BRDerr_HW_ERROR;
			}
			isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)inComms, nLen);
			if (!isRes)
			{
				Status = BRDerr_HW_ERROR;
			}
			if (nLen > 32)
			{
				nBytesTransferred += (nLen - 32) / 2;
				CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
			}
			nComm = 0;
			outComms[nComm].transaction = wTransPld++;
			outComms[nComm].command.setup.type = 0;
			outComms[nComm].command.setup.request = 0;
			outComms[nComm].command.setup.requestH = 0;
			outComms[nComm].value = 0;
			nComm++;
			nLen = nComm * 16;
			isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)outComms, nLen);
			if (!isRes)
			{
				Status = BRDerr_HW_ERROR;
			}
			isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)inComms, nLen);
			if (!isRes)
			{
				Status = BRDerr_HW_ERROR;
			}
			if (inComms[0].command.status.ready == 1)
				Status = BRDerr_OK;
			else
				Status = BRDerr_ERROR;
			CallBackLoadProgress(4, 4, nBytesTransferred, nFileSize);
		}
	}
	*wTrans = wTransPld;
	BRDC_fclose(pFile);
	return Status;
}

ULONG CBambusb::LoadPldFileDMA(const BRDCHAR *PldFileName, int nStreamEp, int nInStreamEp, unsigned int ep_in, int nMaxBuf, USHORT *wTrans, unsigned char bDoubling, unsigned char bEndean, unsigned char bSwap)
{
	FILE *pFile;
	rUsbHostCommand outComms = {0}, inComms = {0};
	int isRes = 0, nComm = 0;
	LONG nLen = 0;
	UCHAR bBuf[4096] = { 0 }, bBufR[4096] = {0};
	USHORT wTransPld = *wTrans;
	//IPC_handle hComm, hCommR;
	//OVERLAPPED rOv = {0}, rOv2 = {0};
	//UCHAR isWaitReply = 0;
	size_t nRealRead = 0;
	pFile = BRDC_fopen(PldFileName, _BRDC("rb"));
	ULONG nFileSize = 0, nBytesTransferred = 0;
	fseek(pFile, 0, SEEK_END);
	nFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	int nTransferred = 0;
	m_pDevice->EndPoints[nStreamEp]->Abort();
	m_pDevice->EndPoints[nStreamEp]->Reset();
	while(!feof(pFile))
	{
		nLen = nMaxBuf * 8;
		if (nStreamEp > 1)
		{
			nRealRead = fread(bBuf, 1, nMaxBuf * 8, pFile);
			if (bSwap)
				for (int i = 0; i < nRealRead; i++)
					bBuf[i] = LtoBend(bBuf[i]);
			nLen = nRealRead;
			nTransferred += nLen;
			nBytesTransferred += nLen;
			if (nTransferred >= (nMaxBuf * 8))
				nTransferred -= ((nMaxBuf * 8) * (UINT)(nTransferred / (nMaxBuf * 8)));
			if ((nRealRead < nMaxBuf * 8) || (feof(pFile)))
			{
				for (int i = nRealRead; i < nRealRead + (nMaxBuf * 8 - nTransferred); i++)
					bBuf[i] = 0xFF;
				nLen += nMaxBuf * 8 - nTransferred;
				nTransferred += nMaxBuf * 8 - nTransferred;
			}
			isRes = m_pDevice->EndPoints[nStreamEp]->XferData(bBuf, nLen);
			//IPC_delay(500);
			CallBackLoadProgress(1, 2, nBytesTransferred, nFileSize);
		}
		if (!isRes)
		{
			BRDC_fclose(pFile);
			//free(bBuf);
			//free(bBufR);
			return BRDerr_HW_ERROR;
		}
		if (nInStreamEp > 1)
			isRes = m_pDevice->EndPoints[nInStreamEp]->XferData(bBufR, nLen);
		if (!isRes)
		{
			BRDC_fclose(pFile);
			//free(bBuf);
			//free(bBufR);
			return BRDerr_HW_ERROR;
		}
		//CallBackLoadProgress(1, 2, nBytesTransferred, nFileSize);
	}
	BRDC_fclose(pFile);
	for (int i = 0; i < nMaxBuf * 8; i++)
		bBuf[i] = 0xFF;
	//while (nTransferred < (nMaxBuf * 8))
	{
		//if ((nMaxBuf * 8) - nTransferred > 4096)
		//	nLen = 4096;
		//else
		//	nLen = (nMaxBuf * 8) - nTransferred;
		//nTransferred += nLen;
		nLen = nMaxBuf * 8;
		nBytesTransferred += nLen;
		isRes = m_pDevice->EndPoints[nStreamEp]->XferData(bBuf, nLen);
		if (!isRes)
		{
			//free(bBuf);
			//free(bBufR);
			return BRDerr_HW_ERROR;
		}
		CallBackLoadProgress(1, 2, nBytesTransferred, nFileSize);
		if (nInStreamEp > 1)
			isRes = m_pDevice->EndPoints[nInStreamEp]->XferData(bBufR, nLen);
		if (!isRes)
		{
			//free(bBuf);
			//free(bBufR);
			return BRDerr_HW_ERROR;
		}
	}
	//PUCHAR nBuf = (PUCHAR)malloc(0x200);
	//for (int i = 0; i < 0x200; i++)
	//	nBuf[i] = 0xFF;
	//nLen = 0x200;
	//isRes = m_pDevice->EndPoints[nStreamEp]->XferData(bBuf, nLen);
	//free(nBuf);
	//free(bBuf);
	//free(bBufR);
	//for (int i = 0; i < 5; i++)
	//{
	//	nTransferred = 0;
	//	while (nTransferred < (nMaxBuf * 8))
	//	{
	//		if ((nMaxBuf * 8) - nTransferred > 4096)
	//			nLen = 4096;
	//		else
	//			nLen = (nMaxBuf * 8) - nTransferred;
	//		nTransferred += nLen;
	//		nBytesTransferred += nLen;
	//		isRes = m_pDevice->EndPoints[nStreamEp]->XferData(bBuf, nLen);
	//		if (!isRes)
	//		{
	//			BRDC_fclose(pFile);
	//			return BRDerr_HW_ERROR;
	//		}
	//		CallBackLoadProgress(1, 4, nBytesTransferred, nFileSize);
	//		if (nInStreamEp > 1)
	//			isRes = m_pDevice->EndPoints[nInStreamEp]->XferData(bBufR, nLen);
	//		if (!isRes)
	//		{
	//			BRDC_fclose(pFile);
	//			return BRDerr_HW_ERROR;
	//		}
	//	}
	//}
	CallBackLoadProgress(2, 2, 0, 5);
	IPC_delay(500);
	for (int i = 0; i <= 5; i++)
	{
		outComms.signatureValue = 0xC1;
		outComms.transaction = wTransPld++;
		outComms.command.setup.type = 0;
		outComms.command.setup.request = 0;
		outComms.command.setup.requestH = 0;
		outComms.value = 0xFFFFFFFF;
		nLen = 16;
		//IPC_delay(500);
		isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)&outComms, nLen);
		if (!isRes)
			return BRDerr_HW_ERROR;
		IPC_delay(50);
		isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)&inComms, nLen);
		if (!isRes)
			return BRDerr_HW_ERROR;
		if (inComms.command.setup.request == 1)
		{
			*wTrans = wTransPld;
			CallBackLoadProgress(2, 2, i, 5);
			return BRDerr_OK;
		}
		CallBackLoadProgress(2, 2, i, 5);
		//IPC_delay(500);
	}
	*wTrans = wTransPld;
	return BRDerr_HW_BUSY;
}

ULONG CBambusb::LoadPldFileTetrad(int nTetr, const BRDCHAR *PldFileName)
{
	ULONG Status = 0;
	LONG nLen;
	UINT32 ctrlReg = 0x203, addrLReg = 0x200, addrHReg = 0x201, sizeReg = 0x202, dataReg = 0x204, commNum = 0;
	rUsbHostCommand outComms[128] = {0}, inComms[128] = {0};
	DWORD xTime = 0;
	for (int i = 0; i < 128; i++)
	{
		outComms[i].signatureValue = 0xA5;
		outComms[i].taskID = m_TaskID;
	}
	outComms[0].shortTransaction = m_Trans++;
	outComms[0].command.tetrad.type = 2;
	outComms[0].command.tetrad.condition = 0;
	outComms[0].valueH = 0x100;
	//сброс тетрады
	xTime = IPC_getTickCount();
	CallBackLoadProgress(1, 3, 0, 1);
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 1;
	outComms[commNum].valueH = 0;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 0;
	outComms[commNum].valueH = 0;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	nLen = commNum*16;
	SendBuffer(outComms, inComms, nLen);

	commNum = 0;
	//чтение статуса тетрады
	outComms[commNum].command.hardware.type = 1;
	outComms[commNum].command.hardware.port = 2;
	outComms[commNum].command.hardware.number = nTetr;
	outComms[commNum].command.hardware.write = 0;
	outComms[commNum].valueL = 0;
	outComms[commNum].valueH = 0;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	nLen = commNum*16;
	SendBuffer(outComms, inComms, nLen);
	if (!(inComms[commNum-1].valueL & 1))
		while (1)
		{
			//чтение статуса тетрады
			outComms[commNum].command.hardware.type = 1;
			outComms[commNum].command.hardware.port = 2;
			outComms[commNum].command.hardware.number = nTetr;
			outComms[commNum].command.hardware.write = 0;
			outComms[commNum].valueL = 0;
			outComms[commNum].valueH = 0;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
			nLen = commNum*16;
			SendBuffer(outComms, inComms, nLen);
			if (inComms[commNum-1].valueL & 1)
				break;
			commNum = 0;
		}
	//чтение конфиг регистра
	U32 regVal = 0;
	WriteRegData(0, nTetr, sizeReg, regVal);
	regVal = 0x65;
	WriteRegData(0, nTetr, ctrlReg, regVal);
	ReadRegData(0, nTetr, dataReg, regVal);
	U32 nConfReg = regVal;
	
	commNum = 0;
	//write enable
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 0x6;
	outComms[commNum].valueH = ctrlReg;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	//установка протокола
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = (nConfReg | 0xC0);
	outComms[commNum].valueH = dataReg;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 0x61; //write config register
	outComms[commNum].valueH = ctrlReg;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	nLen = commNum*16;
	SendBuffer(outComms, inComms, nLen);
	
	regVal = 0;
	WriteRegData(0, nTetr, sizeReg, regVal);
	regVal = 0x65;
	WriteRegData(0, nTetr, ctrlReg, regVal);
	ReadRegData(0, nTetr, dataReg, regVal);
	
	regVal = 6;
	WriteRegData(0, nTetr, ctrlReg, regVal);

	regVal = 0;
	WriteRegData(0, nTetr, sizeReg, regVal);
	regVal = 0x5;
	WriteRegData(0, nTetr, ctrlReg, regVal);
	ReadRegData(0, nTetr, dataReg, regVal);
	
	if (regVal & 0x5C)
		return BRDerr_HW_ERROR;

	//erase
	regVal = 0xC7;
	WriteRegData(0, nTetr, ctrlReg, regVal);

	regVal = 6;
	WriteRegData(0, nTetr, ctrlReg, regVal);
	while (1)
	{
		regVal = 0;
		WriteRegData(0, nTetr, sizeReg, regVal);
		regVal = 0x5;
		WriteRegData(0, nTetr, ctrlReg, regVal);
		ReadRegData(0, nTetr, dataReg, regVal);
		if ((regVal & 1) == 0)
			break;
	}

	commNum = 0;
	//read flag status register
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 0;
	outComms[commNum].valueH = sizeReg;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 0x70; //read status register
	outComms[commNum].valueH = ctrlReg;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 0;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueH = dataReg;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	nLen = commNum*16;
	SendBuffer(outComms, inComms, nLen);
	if (inComms[commNum-1].valueL & 0x20)
		return BRDerr_HW_ERROR;
	xTime = IPC_getTickCount() - xTime;
	printf("erase in %d ms\n", xTime);
	CallBackLoadProgress(1, 3, 1, 1);
	xTime = IPC_getTickCount();
	commNum = 0;
	//read .hex file
	FILE *pFile;
	ULONG nFileSize = 0, nBytesTransferred = 0;
	char str[261];
	U08 data[256], byte[261], isAddrSet = 0;
	U16 highAddress = 0, dataSize = 0;
	int recLen, offset, recType, checkSum, setAddress = 0;
	nFileSize = GetMCSDataSize(PldFileName);
	pFile = BRDC_fopen(PldFileName, _BRDC("rt"));
	CallBackLoadProgress(2, 3, 0, nFileSize);
	while (!feof(pFile))
	{
		fgets(str, 261, pFile);
		//разбор строчки
		if (str[0] != ':')
			return BRDerr_BAD_FILE_FORMAT;
		sscanf(str+1, "%2x", &recLen);
		sscanf(str+3, "%4x", &offset);
		sscanf(str+7, "%2x", &recType);
		byte[0]=(U08)recLen;
		byte[1]= (U08)(offset & 0xFF);
		byte[2]=(U08)(offset>>8) & 0xFF;
		byte[3]=(U08) recType;
		if (recType == 1)//eof
		{
			if (isAddrSet == 1)
			{
				//write size of data
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = dataSize - 1;
				outComms[commNum].valueH = sizeReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				//write data
				for (int i = 0; i < dataSize; i+=4)
				{
					outComms[commNum].command.hardware.type = 1;
					outComms[commNum].command.hardware.port = 1;
					outComms[commNum].command.hardware.number = nTetr;
					outComms[commNum].command.hardware.write = 1;
					outComms[commNum].valueL = ((U32)data[i+3] << 24) + ((U32)data[i+2] << 16) + ((U32)data[i+1] << 8) + data[i+0];
					outComms[commNum].valueH = 1;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
				}
				//PAGE PROGRAM 0x2 command
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0x2;
				outComms[commNum].valueH = ctrlReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				//read STATUS REGISTER 0x5
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0;
				outComms[commNum].valueH = sizeReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0x5;
				outComms[commNum].valueH = ctrlReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 0;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0;
				outComms[commNum].valueH = dataReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				nLen = commNum * 16;
				SendBuffer(outComms, inComms, nLen);
				while (1)
				{
					if ((inComms[commNum - 1].valueL & 1) == 0)
						break;
					commNum = 0;
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 0;
					outComms[commNum].valueH = sizeReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 0x5;
					outComms[commNum].valueH = ctrlReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 0;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 0;
					outComms[commNum].valueH = dataReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
					nLen = commNum * 16;
					SendBuffer(outComms, inComms, nLen);
				}
			}
			break;
		}
		for (int i = 0; i < recLen; i++)
			sscanf(str+9+2*i, "%2x", &byte[4+i]);
		sscanf(str+9+2*recLen, "%2x", &checkSum);
		U08 crc, *pBlock = byte;
		U16 wLen;
		//подсчёт контрольной суммы
		crc = 0;
		wLen = recLen + 4;
		while (wLen--)
			crc += *pBlock++;
		crc=~crc;
		crc+=1;
		crc &= 0xFF;
		if (crc != checkSum)
			return BRDerr_BAD_FILE_FORMAT;
		
		//формирование пакета данных для записи
		if (recType == 4)
		{
			highAddress = ((U32)byte[4] << 8) + byte[5];
			continue;
		}
		else if (recType == 0)
		{
			if ((setAddress != ((U32)highAddress << 16) + ((U32)byte[2] << 8) + byte[1]) && isAddrSet)
			{
				//write size of data
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = dataSize - 1;
				outComms[commNum].valueH = sizeReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				//write data
				for (int i = 0; i < dataSize; i+=4)
				{
					outComms[commNum].command.hardware.type = 1;
					outComms[commNum].command.hardware.port = 1;
					outComms[commNum].command.hardware.number = nTetr;
					outComms[commNum].command.hardware.write = 1;
					outComms[commNum].valueL = ((U32)data[i+3] << 24) + ((U32)data[i+2] << 16) + ((U32)data[i+1] << 8) + data[i+0];
					outComms[commNum].valueH = 1;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
					if (commNum == 128)
					{
						nLen = commNum * 16;
						SendBuffer(outComms, inComms, nLen);
						commNum = 0;
					}
				}
				//PAGE PROGRAM 0x2 command
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0x2;
				outComms[commNum].valueH = ctrlReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				//read STATUS REGISTER 0x5
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0x5;
				outComms[commNum].valueH = ctrlReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 0;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0;
				outComms[commNum].valueH = dataReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				nLen = commNum * 16;
				SendBuffer(outComms, inComms, nLen);
				while (1)
				{
					if ((inComms[commNum - 1].valueL & 1) == 0)
						break;
					commNum = 0;
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 0;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 0;
					outComms[commNum].valueH = dataReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
					nLen = commNum * 16;
					SendBuffer(outComms, inComms, nLen);
				}
				nBytesTransferred += dataSize;
				CallBackLoadProgress(2, 3, nBytesTransferred, nFileSize);
				isAddrSet = 0;
				commNum = 0;
				dataSize = 0;
			}
			if ((dataSize + byte[0]) <= 256)
			{
				for (int i = dataSize; i < dataSize + byte[0]; i++)
					data[i] = byte[i - dataSize + 4];
				dataSize = dataSize + byte[0];
				setAddress = ((U32)highAddress << 16) + ((U32)byte[2] << 8) + byte[1] + byte[0];
			}
			else
			{
				for (int i = dataSize; i < 256; i++)
					data[i] = byte[i - dataSize + 4];
				dataSize = dataSize + byte[0];
			}
			if (0 == isAddrSet)
			{
				//write enable
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0x6;
				outComms[commNum].valueH = ctrlReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0;
				outComms[commNum].valueH = sizeReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				if (highAddress & 100)
				{
					//write EXTENDED ADDRESS
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 1;
					outComms[commNum].valueH = dataReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 0xC5;
					outComms[commNum].valueH = ctrlReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
				}
				else //low half memory
				{
					//write EXTENDED ADDRESS
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 0;
					outComms[commNum].valueH = dataReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 0xC5;
					outComms[commNum].valueH = ctrlReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
				}
				//write enable
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0x6;
				outComms[commNum].valueH = ctrlReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				//write high bytes of address
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = highAddress;
				outComms[commNum].valueH = addrHReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				//write low bytes of address
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = ((U32)byte[2] << 8) + byte[1];
				outComms[commNum].valueH = addrLReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				isAddrSet = 1;
			}
		}
		if (dataSize < 256)
			continue;
		//write size of data
		outComms[commNum].command.tetrad.type = 2;
		outComms[commNum].command.tetrad.condition = 4;
		outComms[commNum].command.tetrad.number = nTetr;
		outComms[commNum].valueL = dataSize - 1;
		outComms[commNum].valueH = sizeReg;
		outComms[commNum].shortTransaction = m_Trans++;
		commNum++;
		//write data
		for (int i = 0; i < 256; i+=4)
		{
			outComms[commNum].command.hardware.type = 1;
			outComms[commNum].command.hardware.port = 1;
			outComms[commNum].command.hardware.number = nTetr;
			outComms[commNum].command.hardware.write = 1;
			outComms[commNum].valueL = ((U32)data[i+3] << 24) + ((U32)data[i+2] << 16) + ((U32)data[i+1] << 8) + data[i+0];
			outComms[commNum].valueH = 1;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
			if (commNum == 128)
			{
				nLen = commNum * 16;
				SendBuffer(outComms, inComms, nLen);
				commNum = 0;
			}
		}
		//PAGE PROGRAM 0x2 command
		outComms[commNum].command.tetrad.type = 2;
		outComms[commNum].command.tetrad.condition = 4;
		outComms[commNum].command.tetrad.number = nTetr;
		outComms[commNum].valueL = 0x2;
		outComms[commNum].valueH = ctrlReg;
		outComms[commNum].shortTransaction = m_Trans++;
		commNum++;
		//read STATUS REGISTER 0x5
		outComms[commNum].command.tetrad.type = 2;
		outComms[commNum].command.tetrad.condition = 4;
		outComms[commNum].command.tetrad.number = nTetr;
		outComms[commNum].valueL = 0;
		outComms[commNum].valueH = sizeReg;
		outComms[commNum].shortTransaction = m_Trans++;
		commNum++;
		outComms[commNum].command.tetrad.type = 2;
		outComms[commNum].command.tetrad.condition = 4;
		outComms[commNum].command.tetrad.number = nTetr;
		outComms[commNum].valueL = 0x5;
		outComms[commNum].valueH = ctrlReg;
		outComms[commNum].shortTransaction = m_Trans++;
		commNum++;
		outComms[commNum].command.tetrad.type = 2;
		outComms[commNum].command.tetrad.condition = 0;
		outComms[commNum].command.tetrad.number = nTetr;
		outComms[commNum].valueL = 0;
		outComms[commNum].valueH = dataReg;
		outComms[commNum].shortTransaction = m_Trans++;
		commNum++;
		nLen = commNum * 16;
		SendBuffer(outComms, inComms, nLen);
		while (1)
		{
			if ((inComms[commNum - 1].valueL & 1) == 0)
				break;
			commNum = 0;
			outComms[commNum].command.tetrad.type = 2;
			outComms[commNum].command.tetrad.condition = 4;
			outComms[commNum].command.tetrad.number = nTetr;
			outComms[commNum].valueL = 0;
			outComms[commNum].valueH = sizeReg;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
			outComms[commNum].command.tetrad.type = 2;
			outComms[commNum].command.tetrad.condition = 4;
			outComms[commNum].command.tetrad.number = nTetr;
			outComms[commNum].valueL = 0x5;
			outComms[commNum].valueH = ctrlReg;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
			outComms[commNum].command.tetrad.type = 2;
			outComms[commNum].command.tetrad.condition = 0;
			outComms[commNum].command.tetrad.number = nTetr;
			outComms[commNum].valueL = 0;
			outComms[commNum].valueH = dataReg;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
			nLen = commNum * 16;
			SendBuffer(outComms, inComms, nLen);
		}
		isAddrSet = 0;
		commNum = 0;
		if (dataSize > 256)
		{
			nBytesTransferred += 256;
			dataSize -= 256;
		}
		else
		{
			nBytesTransferred += dataSize;
			dataSize = 0;
		}
		CallBackLoadProgress(2, 3, nBytesTransferred, nFileSize);
	}
	fclose(pFile);
	xTime = IPC_getTickCount() - xTime;
	printf("write in %d ms\n", xTime);
	xTime = IPC_getTickCount();
	return BRDerr_OK;
}

ULONG CBambusb::AdmPldStartTest()
{
	ULONG Status = BRDerr_OK;
	DEVS_CMD_Reg RegParam = {0};
	RegParam.idxMain = 0;
	RegParam.tetr = ADM2IFnt_MAIN;
	int fifo_width = 0x40;//(int)RegParam.val; // получить ширину регистра тестовой последовательности
	RegParam.reg = ADM2IFnr_DATA;
	RegParam.val = 2; // обратно переключить ПЛИС в режим начального тестирования
	Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);
	if(Status != BRDerr_OK)
		return Status;

	// тест шины данных
	ULONG test_mask = 0;
	int i = 0;
	for(i = 0; i < fifo_width; i++)
	{
		ULONG value = 0;
		Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
		if(Status != BRDerr_OK)
			return Status;
		value = RegParam.val;// & 0xffff;
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
	return Status;
}

ULONG CBambusb::AdmPldWorkAndCheck()
{
	return BRDerr_CMD_UNSUPPORTED;
}

ULONG CBambusb::SetAdmPldFileInfo()
{
	//ULONG Status;
	//m_pldFileInfo.pldId = m_AdmPldInfo.Id;
	//DEVS_CMD_Reg RegParam = {0};
	//RegParam.idxMain = 0;
	//RegParam.tetr = ADM2IFnt_MAIN;
	//RegParam.reg = ADM2IFnr_ID;
	//Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	//printf("MAIN SIG %X\n", RegParam.val);
	//if(Status != BRDerr_OK)
	//	return Status;
	//return BRDerr_OK;
	ULONG Status;
	m_pldFileInfo.pldId = m_AdmPldInfo.Id;
	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = 0;
	RegParam.tetr = ADM2IFnt_MAIN;
	RegParam.reg = MAINnr_FPGAVER;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if (Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.version = RegParam.val;
	RegParam.reg = MAINnr_FPGAMODE;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if (Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.modification = RegParam.val;
	//	RegParam.reg = MAINnr_TMASK;
	//	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	//	RegParam.reg = MAINnr_MRES;
	//	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	RegParam.reg = MAINnr_BASEID;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if (Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.baseId = RegParam.val;
	RegParam.reg = MAINnr_BASEVER;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if (Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.baseVer = RegParam.val;
	RegParam.reg = MAINnr_SMODID;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if (Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.submodId = RegParam.val;
	RegParam.reg = MAINnr_SMODVER;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if (Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.submodVer = RegParam.val;
	RegParam.reg = MAINnr_FPGABLD;
	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
	if (Status != BRDerr_OK)
		return Status;
	m_pldFileInfo.build = RegParam.val;
	return BRDerr_OK;
}

ULONG CBambusb::GetPldInfo(PBRDextn_PLDINFO pInfo)
{
	SetAdmPldFileInfo();
	if ((pInfo->pldId == m_pldFileInfo.pldId) || (pInfo->pldId == BRDpui_PLD))
	{
		pInfo->version = m_pldFileInfo.version;
		pInfo->modification = m_pldFileInfo.modification;
		pInfo->build = m_pldFileInfo.build;
	}
	else
		return ErrorPrint(BRDerr_BAD_PARAMETER,
			CURRFILE,
			_BRDC("<GetPldInfo> PLD ID is wrong"));
	return BRDerr_OK;
}

ULONG CBambusb::GetPldFileInfo(PBRDextn_PLDFILEINFO pInfo)
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

void CBambusb::GetAdmPldFileInfo(PBRDextn_PLDFILEINFO pInfo) const
{
	pInfo->version = m_pldFileInfo.version;
	pInfo->modification = m_pldFileInfo.modification;
	pInfo->build = m_pldFileInfo.build;
	pInfo->baseId = m_pldFileInfo.baseId;
	pInfo->baseVer = m_pldFileInfo.baseVer;
	pInfo->submodId = m_pldFileInfo.submodId;
	pInfo->submodVer = m_pldFileInfo.submodVer;
}

ULONG CBambusb::ShowDialog(PBRDextn_PropertyDlg pDlg)
{
	return BRDerr_CMD_UNSUPPORTED;
}

ULONG CBambusb::DeleteDialog(PBRDextn_PropertyDlg pDlg)
{
	return BRDerr_CMD_UNSUPPORTED;
}

ULONG CBambusb::GetBaseIcrDataSize()
{
	USHORT buf_read[3] = {0xFFFF};	
	ULONG ret_status = GetNvRAM(&buf_read, 6, 128);
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

ULONG CBambusb::RegisterCallBack(PVOID funcName)
{
	vPULoadProgressCB = (void (*)(ULONG, ULONG, ULONG, ULONG))funcName;
	return BRDerr_OK;
}

ULONG CBambusb::PldReadToFile(U32 puId, const BRDCHAR * fileName)
{
	ULONG nStatus = BRDerr_OK;
	//FILE *pFile;
	LONG nLen;
	int nTetr = -1;
	rUsbHostCommand outComms = { 0 }, inComms = { 0 };
	outComms.signatureValue = 0xA5;
	outComms.taskID = m_TaskID;
	outComms.shortTransaction = m_Trans++;
	outComms.command.tetrad.type = 2;
	outComms.command.tetrad.condition = 0;
	outComms.valueH = 0x100;
	if (0 == m_isDLMode)
		if (puId != m_ArmPldInfo.Id)
		{
			for (int i = 0; i < 16; i++)//ищем тетраду FLASH (ID=0xC3)
			{
				outComms.command.tetrad.number = i;
				nLen = 16;
				SendBuffer(&outComms, &inComms, nLen);
				if (inComms.valueL == 0xC3)
				{
					nTetr = i;
					break;
				}
			}
		}
	//if (nTetr == -1)
	{
		if (m_pDevice->AltIntfcCount() > 1)
		{
			if (puId != m_ArmPldInfo.Id)
				nStatus = PldReadToFileAlt(fileName);
			else
				nStatus = ArmReadToFile(fileName);
		}
		else
			nStatus = PldReadToFileTetrad(fileName, nTetr);
	}
	//else
	//{
	//	nStatus = PldReadToFileTetrad(fileName, nTetr);
	//}
	return nStatus;
}

ULONG CBambusb::PldReadToFileTetrad(const BRDCHAR * PldFileName, int nTetr)
{
	ULONG nStatus = BRDerr_OK;
	FILE *pFile;
	LONG nLen;
	UINT32 ctrlReg = 0x203, addrLReg = 0x200, addrHReg = 0x201, sizeReg = 0x202, dataReg = 0x204, commNum = 0;
	rUsbHostCommand outComms[128] = { 0 }, inComms[128] = { 0 };
	ULONG nFileSize = 0, nBytesTransferred = 0;
	char str[261];
	U08 data[256], byte[261], isAddrSet = 0;
	U16 highAddress = 0, dataSize = 0;
	int recLen, /*offset,*/ recType, checkSum, setAddress = 0;
	DWORD xTime = 0;
	for (int i = 0; i < 128; i++)
	{
		outComms[i].signatureValue = 0xA5;
		outComms[i].taskID = m_TaskID;
	}
	outComms[0].shortTransaction = m_Trans++;
	outComms[0].command.tetrad.type = 2;
	outComms[0].command.tetrad.condition = 0;
	outComms[0].valueH = 0x100;
	//сброс тетрады
	xTime = IPC_getTickCount();
	CallBackLoadProgress(1, 2, 0, 1);
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 1;
	outComms[commNum].valueH = 0;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 0;
	outComms[commNum].valueH = 0;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	nLen = commNum * 16;
	SendBuffer(outComms, inComms, nLen);

	commNum = 0;
	//чтение статуса тетрады
	outComms[commNum].command.hardware.type = 1;
	outComms[commNum].command.hardware.port = 2;
	outComms[commNum].command.hardware.number = nTetr;
	outComms[commNum].command.hardware.write = 0;
	outComms[commNum].valueL = 0;
	outComms[commNum].valueH = 0;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	nLen = commNum * 16;
	SendBuffer(outComms, inComms, nLen);
	if (!(inComms[commNum - 1].valueL & 1))
		while (1)
		{
			//чтение статуса тетрады
			outComms[commNum].command.hardware.type = 1;
			outComms[commNum].command.hardware.port = 2;
			outComms[commNum].command.hardware.number = nTetr;
			outComms[commNum].command.hardware.write = 0;
			outComms[commNum].valueL = 0;
			outComms[commNum].valueH = 0;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
			nLen = commNum * 16;
			SendBuffer(outComms, inComms, nLen);
			if (inComms[commNum - 1].valueL & 1)
				break;
			commNum = 0;
		}
	//чтение конфиг регистра
	U32 regVal = 0;
	WriteRegData(0, nTetr, sizeReg, regVal);
	regVal = 0x65;
	WriteRegData(0, nTetr, ctrlReg, regVal);
	ReadRegData(0, nTetr, dataReg, regVal);
	U32 nConfReg = regVal;

	commNum = 0;
	//write enable
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 0x6;
	outComms[commNum].valueH = ctrlReg;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	//установка протокола
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = (nConfReg | 0xC0);
	outComms[commNum].valueH = dataReg;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].valueL = 0x61; //write config register
	outComms[commNum].valueH = ctrlReg;
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	nLen = commNum * 16;
	SendBuffer(outComms, inComms, nLen);

	regVal = 0;
	WriteRegData(0, nTetr, sizeReg, regVal);
	regVal = 0x65;
	WriteRegData(0, nTetr, ctrlReg, regVal);
	ReadRegData(0, nTetr, dataReg, regVal);

	regVal = 6;
	WriteRegData(0, nTetr, ctrlReg, regVal);

	regVal = 0;
	WriteRegData(0, nTetr, sizeReg, regVal);
	regVal = 0x5;
	WriteRegData(0, nTetr, ctrlReg, regVal);
	ReadRegData(0, nTetr, dataReg, regVal);

	if (regVal & 0x5C)
		return BRDerr_HW_ERROR;
	//read and verification
	CallBackLoadProgress(2, 2, 0, nFileSize);
	nBytesTransferred = 0;
	commNum = 0;
	isAddrSet = 0;
	outComms[commNum].command.tetrad.type = 2;
	outComms[commNum].command.tetrad.number = nTetr;
	outComms[commNum].command.tetrad.condition = 4;
	outComms[commNum].valueH = 10;
	outComms[commNum].valueL = 8;//dummy cycles for extended spi mode
	outComms[commNum].shortTransaction = m_Trans++;
	commNum++;
	dataSize = 0;
	U08 readData[256] = { 0 }, err = 0;
	pFile = BRDC_fopen(PldFileName, _BRDC("wb"));
	{
		//запись адреса
		outComms[commNum].command.tetrad.type = 2;
		outComms[commNum].command.tetrad.condition = 4;
		outComms[commNum].command.tetrad.number = nTetr;
		outComms[commNum].valueL = 0x6;
		outComms[commNum].valueH = ctrlReg;
		outComms[commNum].shortTransaction = m_Trans++;
		commNum++;
		outComms[commNum].command.tetrad.type = 2;
		outComms[commNum].command.tetrad.condition = 4;
		outComms[commNum].command.tetrad.number = nTetr;
		outComms[commNum].valueL = 0;
		outComms[commNum].valueH = sizeReg;
		outComms[commNum].shortTransaction = m_Trans++;
		commNum++;
		if (highAddress & 0x100)
		{
			outComms[commNum].command.tetrad.type = 2;
			outComms[commNum].command.tetrad.condition = 4;
			outComms[commNum].command.tetrad.number = nTetr;
			outComms[commNum].valueL = 1;
			outComms[commNum].valueH = dataReg;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
			outComms[commNum].command.tetrad.type = 2;
			outComms[commNum].command.tetrad.condition = 4;
			outComms[commNum].command.tetrad.number = nTetr;
			outComms[commNum].valueL = 0xC5;
			outComms[commNum].valueH = ctrlReg;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
		}
		else
		{
			outComms[commNum].command.tetrad.type = 2;
			outComms[commNum].command.tetrad.condition = 4;
			outComms[commNum].command.tetrad.number = nTetr;
			outComms[commNum].valueL = 0;
			outComms[commNum].valueH = dataReg;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
			outComms[commNum].command.tetrad.type = 2;
			outComms[commNum].command.tetrad.condition = 4;
			outComms[commNum].command.tetrad.number = nTetr;
			outComms[commNum].valueL = 0xC5;
			outComms[commNum].valueH = ctrlReg;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
		}
		//write high bytes of address
		outComms[commNum].command.tetrad.type = 2;
		outComms[commNum].command.tetrad.condition = 4;
		outComms[commNum].command.tetrad.number = nTetr;
		outComms[commNum].valueL = highAddress;
		outComms[commNum].valueH = addrHReg;
		outComms[commNum].shortTransaction = m_Trans++;
		commNum++;
		//write low bytes of address
		outComms[commNum].command.tetrad.type = 2;
		outComms[commNum].command.tetrad.condition = 4;
		outComms[commNum].command.tetrad.number = nTetr;
		outComms[commNum].valueL = ((U32)byte[2] << 8) + byte[1];
		outComms[commNum].valueH = addrLReg;
		outComms[commNum].shortTransaction = m_Trans++;
		commNum++;
		//
	}
	//while (!feof(pFile))
	{
		//fgets(str, 261, pFile);
		//if (str[0] != ':')
		//	return BRDerr_BAD_FILE_FORMAT;
		//sscanf(str + 1, "%2x", &recLen);
		//sscanf(str + 3, "%4x", &offset);
		//sscanf(str + 7, "%2x", &recType);
		//byte[0] = (U08)recLen;
		//byte[1] = (U08)(offset & 0xFF);
		//byte[2] = (U08)(offset >> 8) & 0xFF;
		//byte[3] = (U08)recType;
		if (recType == 1)//eof
		{
			if (isAddrSet == 1)
			{
				//write size of data
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = dataSize - 1;
				outComms[commNum].valueH = sizeReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				//fast read
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0xB;
				outComms[commNum].valueH = ctrlReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				nLen = 16 * commNum;
				SendBuffer(outComms, inComms, nLen);
				for (;;)
				{
					regVal = 0;
					ReadRegDataDir(0, nTetr, 0, regVal);
					if (regVal & 1)
						break;
				}
				//ReadRegBufDir(0, nTetr, 1, readData, 256);
				commNum = 0;
				for (int i = 0; i < 256; i += 4)
				{
					outComms[commNum].command.hardware.type = 1;
					outComms[commNum].command.hardware.write = 0;
					outComms[commNum].command.hardware.number = nTetr;
					outComms[commNum].command.hardware.port = 1;
					outComms[commNum].valueH = 1;
					outComms[commNum].valueL = 0;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
				}
				nLen = commNum * 16;
				SendBuffer(outComms, inComms, nLen);
				commNum = 0;
				for (int i = 0; i < 256; i += 4)
				{
					memcpy(&(readData[i]), &(inComms[commNum].valueL), 4);
					commNum++;
				}
				for (int i = 0; i < 256; i++)
				{//write data to file

				}
				nBytesTransferred += 256;
				CallBackLoadProgress(3, 3, nBytesTransferred, nFileSize);
			}
			//break;
		}
		for (int i = 0; i < recLen; i++)
			sscanf(str + 9 + 2 * i, "%2x", &byte[4 + i]);
		sscanf(str + 9 + 2 * recLen, "%2x", &checkSum);
		U08 crc, *pBlock = byte;
		U16 wLen;
		//подсчёт контрольной суммы
		crc = 0;
		wLen = recLen + 4;
		while (wLen--)
			crc += *pBlock++;
		crc = ~crc;
		crc += 1;
		crc &= 0xFF;
		if (crc != checkSum)
			return BRDerr_BAD_FILE_FORMAT;
		if (recType == 4)
		{
			highAddress = ((U16)byte[4] << 8) + byte[5];
			//continue;
		}
		if (recType == 0)
		{
			if (isAddrSet == 0)
			{
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0x6;
				outComms[commNum].valueH = ctrlReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = 0;
				outComms[commNum].valueH = sizeReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				if (highAddress & 0x100)
				{
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 1;
					outComms[commNum].valueH = dataReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 0xC5;
					outComms[commNum].valueH = ctrlReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
				}
				else
				{
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 0;
					outComms[commNum].valueH = dataReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
					outComms[commNum].command.tetrad.type = 2;
					outComms[commNum].command.tetrad.condition = 4;
					outComms[commNum].command.tetrad.number = nTetr;
					outComms[commNum].valueL = 0xC5;
					outComms[commNum].valueH = ctrlReg;
					outComms[commNum].shortTransaction = m_Trans++;
					commNum++;
				}
				//write high bytes of address
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = highAddress;
				outComms[commNum].valueH = addrHReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				//write low bytes of address
				outComms[commNum].command.tetrad.type = 2;
				outComms[commNum].command.tetrad.condition = 4;
				outComms[commNum].command.tetrad.number = nTetr;
				outComms[commNum].valueL = ((U32)byte[2] << 8) + byte[1];
				outComms[commNum].valueH = addrLReg;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
				isAddrSet = 1;
			}
			if ((dataSize + byte[0]) <= 256)
			{
				for (int i = dataSize; i < dataSize + byte[0]; i++)
					data[i] = byte[i - dataSize + 4];
				dataSize = dataSize + byte[0];
				setAddress = ((U32)highAddress << 16) + ((U32)byte[2] << 8) + byte[1] + byte[0];
			}
			if (dataSize < 256)
				//continue;
			//write size of data
			outComms[commNum].command.tetrad.type = 2;
			outComms[commNum].command.tetrad.condition = 4;
			outComms[commNum].command.tetrad.number = nTetr;
			outComms[commNum].valueL = dataSize - 1;
			outComms[commNum].valueH = sizeReg;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
			//fast read
			outComms[commNum].command.tetrad.type = 2;
			outComms[commNum].command.tetrad.condition = 4;
			outComms[commNum].command.tetrad.number = nTetr;
			outComms[commNum].valueL = 0xB;
			outComms[commNum].valueH = ctrlReg;
			outComms[commNum].shortTransaction = m_Trans++;
			commNum++;
			nLen = 16 * commNum;
			SendBuffer(outComms, inComms, nLen);
			for (;;)
			{
				regVal = 0;
				ReadRegDataDir(0, nTetr, 0, regVal);
				if (regVal & 1)
					break;
			}
			//ReadRegBufDir(0, nTetr, 1, readData, 256);
			commNum = 0;
			for (int i = 0; i < 256; i += 4)
			{
				outComms[commNum].command.hardware.type = 1;
				outComms[commNum].command.hardware.write = 0;
				outComms[commNum].command.hardware.number = nTetr;
				outComms[commNum].command.hardware.port = 1;
				outComms[commNum].valueH = 1;
				outComms[commNum].valueL = 0;
				outComms[commNum].shortTransaction = m_Trans++;
				commNum++;
			}
			nLen = commNum * 16;
			SendBuffer(outComms, inComms, nLen);
			commNum = 0;
			for (int i = 0; i < 256; i += 4)
			{
				memcpy(&(readData[i]), &(inComms[commNum].valueL), 4);
				commNum++;
			}
			for (int i = 0; i < 256; i++)
				if (readData[i] != data[i])
					err++;
			nBytesTransferred += 256;
			CallBackLoadProgress(3, 3, nBytesTransferred, nFileSize);
			isAddrSet = 0;
			dataSize = 0;
			commNum = 0;
		}
	}
	fclose(pFile);
	xTime = IPC_getTickCount() - xTime;
	printf("verification in %d ms", xTime);
	if (err != 0)
		nStatus = BRDerr_FATAL;
	return nStatus;
}

ULONG CBambusb::PldReadToFileAlt(const BRDCHAR * PldFileName)
{
	ULONG nStatus = BRDerr_OK;
	FILE *pFile;
	unsigned int ep_in = 2, nComm = 0, maxBuf = 0, nDevName = 0, nSerial = 0, nStage[8] = { 0 };
	int isRes = 0;
	LONG nLen;
	USHORT wTransPld = 0;
	rUsbHostCommand rOutComms[256] = { 0 }, rInComms[256] = { 0 };
	isRes = m_pDevice->SetAltIntfc(0);
	isRes = m_pDevice->SetAltIntfc(2);
	IPC_delay(1000);
	for (int i = 0; i < m_pDevice->EndPointCount(); i++)
		if (m_pDevice->EndPoints[i]->Address == 0x81)
		{
			ep_in = i;
			break;
		}
	DWORD x = IPC_getTickCount();
	rOutComms[nComm].signatureValue = 0xC1;
	rOutComms[nComm].command.setup.type = 0;
	rOutComms[nComm].command.setup.request = 0xff;
	rOutComms[nComm].command.setup.requestH = 0x1f;
	rOutComms[nComm].valueL = 0x1;
	rOutComms[nComm].valueH = 0x1;
	rOutComms[nComm].transaction = wTransPld++;
	nComm++;
	rOutComms[nComm].signatureValue = 0xC1;
	rOutComms[nComm].command.setup.type = 0;
	rOutComms[nComm].command.setup.request = 0;
	rOutComms[nComm].valueL = 0;
	rOutComms[nComm].valueH = 0;
	rOutComms[nComm].transaction = wTransPld++;
	nComm++;
	nLen = nComm * 16;
	isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)rOutComms, nLen);
	isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)rInComms, nLen);
	maxBuf = rInComms[0].command.setup.request + 1;
	//определение количества этапов и их типа 0-отсутствие этапа, 1-командный режим, 2-запись в потоке, 3-чтение в потоке, 4-стирание командами
	for (int i = 0; i < 8; i++)
		if (((rInComms[0].valueL >> (i * 4)) & 0xF) > 1)
		{
			nStage[i] = 2;
		}
		else
		{
			if (((rInComms[0].valueH >> (i * 4)) & 0xF) > 1)
			{
				nStage[i] = 3;
			}
			else
			{
				if (((rInComms[0].valueL >> (i * 4)) & 0xF) == 1)
				{
					if (((rInComms[0].valueH >> (i * 4)) & 0xF) == 1)
						nStage[i] = 1;
					else
						nStage[i] = 4;
				}
			}
		}
	nComm = 0;
	LONG fSize = 0x1FFFFFFF;
	rOutComms[nComm].signatureValue = 0xC1;
	rOutComms[nComm].command.setup.type = 0;
	rOutComms[nComm].command.setup.request = 0xff;
	rOutComms[nComm].command.setup.requestH = 0x1f;
	rOutComms[nComm].valueL = 0;
	rOutComms[nComm].valueH = fSize;
	rOutComms[nComm].transaction = wTransPld++;
	nComm++;
	rOutComms[nComm].signatureValue = 0xC1;
	rOutComms[nComm].command.setup.type = 0;
	rOutComms[nComm].command.setup.request = 0;
	rOutComms[nComm].valueL = 0;
	rOutComms[nComm].valueH = 0;
	rOutComms[nComm].transaction = wTransPld++;
	nComm++;
	nLen = nComm * 16;
	isRes = m_pDevice->EndPoints[1]->XferData((PUCHAR)rOutComms, nLen);
	isRes = m_pDevice->EndPoints[ep_in]->XferData((PUCHAR)rInComms, nLen);
	maxBuf = rInComms[0].command.setup.request + 1;
	fSize = rInComms[1].valueH;
	//read
	if (nStage[1] == 3)
	{
		UINT nStreamEp = (rInComms[0].valueH >> 4) & 0xF;
		nStreamEp = ep_in + nStreamEp - 1;
		ULONG nFileSize = fSize, nBytesTransferred = 0;
		pFile = BRDC_fopen(PldFileName, _BRDC("w"));
		BRDC_fprintf(pFile, _BRDC(":020000040000FA\n"));
		CallBackLoadProgress(1, 1, 0, nFileSize);
		nComm = 0;
		if (pFile != 0)
		{
			ULONG nRealRead = 0, nAddr = 0, nHighAddr = 0;
			UCHAR abBuf[2048] = { 0 }, bLine[128] = { 0 }, abRead[2048] = { 0 }, bCrc = 0;
			while (fSize > 0)
			{
				nRealRead = 2048;
				nLen = nRealRead;
				m_pDevice->EndPoints[nStreamEp]->XferData(abRead, nLen);
				nBytesTransferred += nRealRead;
				for (UINT i = 0; i < nRealRead / 4; i+=4)
				{
					bCrc = 0x10;
					bCrc += ((nAddr & 0xFF) + ((nAddr >> 8) & 0xFF) + ((nAddr >> 16) & 0xFF) + ((nAddr >> 24) & 0xFF));
					BRDC_fprintf(pFile, _BRDC(":10%04X00"), nAddr);
					for (int j = i * 4; j < i * 4 + 16; j++)
					{
						BRDC_fprintf(pFile, _BRDC("%02X"), abRead[j]);
						bCrc += abRead[j];
					}
					bCrc = ~bCrc;
					bCrc += 1;
					bCrc &= 0xFF;
					BRDC_fprintf(pFile, _BRDC("%02X\n"), bCrc);
					if (nAddr >= 0xFFF0)
					{
						nAddr = 0;
						nHighAddr++;
						bCrc = 2+4;
						bCrc += ((nHighAddr & 0xFF) + ((nHighAddr >> 8) & 0xFF) + ((nHighAddr >> 16) & 0xFF) + ((nHighAddr >> 24) & 0xFF));
						bCrc = ~bCrc;
						bCrc += 1;
						bCrc &= 0xFF;
						BRDC_fprintf(pFile, _BRDC(":02000004%04X%02X\n"), nHighAddr, bCrc);
					}
					else
						nAddr += 0x10;
				}
				CallBackLoadProgress(1, 1, nBytesTransferred, nFileSize);
				fSize -= nLen;
			}
			BRDC_fprintf(pFile, _BRDC(":00000001FF\n"));
			CallBackLoadProgress(1, 1, nFileSize, nFileSize);
			BRDC_fclose(pFile);
		}
		else
			nStatus = BRDerr_BAD_FILE;
	}
	else
		nStatus = BRDerr_INSUFFICIENT_RESOURCES;
	//
	//if (Status == BRDerr_OK)
	{
		U32 lVal = 0;
		m_pDevice->SetAltIntfc(1);
		m_pDevice->SetAltIntfc(0);
		IPC_delay(3000);
		if (m_TaskID != 0xFF)
		{
			m_TaskID = 0;
			SendCommand(0, 0, 0, lVal, 0, 0, 1, 1);//получение TaskID для захвата тетрад и стримов
		}
	}
	return nStatus;
}

ULONG CBambusb::ArmReadToFile(const BRDCHAR * PldFileName)
{
	ULONG nStatus = BRDerr_OK;
	//FILE *pFile;

	return nStatus;
}

void CBambusb::CallBackLoadProgress(ULONG nStage, ULONG nStageMax, ULONG nCurrent, ULONG nMax)
{
	if (vPULoadProgressCB != 0)
		vPULoadProgressCB(nStage, nStageMax, nCurrent, nMax);
}

ULONG CBambusb::GetBaseIcrStatus(ULONG& Status)
{
	USHORT buf_read;
	buf_read = 0xffff;
	ULONG ret_status = GetNvRAM(&buf_read, 2, 128);
	if(ret_status == BRDerr_OK)
	{
		if(buf_read == BASE_ID_TAG)
			Status = 1;
		else
			Status = 0;
		return ret_status;
	}
	else
		return ret_status;
}

ULONG CBambusb::GetAdmIcrStatus(ULONG& Status)
{
	USHORT buf_read;
	buf_read = 0xffff;
	ULONG ret_status = GetNvRAM(&buf_read, 2, 128);
	if(ret_status == BRDerr_OK)
	{
		if(buf_read == BASE_ID_TAG)
			Status = 1;
		else
			Status = 0;
		return ret_status;
	}
	else
		return ret_status;

}

void CBambusb::GetBaseCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgFmc119e pCfgBase)
{
	PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
	int end_flag = 0;
	do
	{
		USHORT sign = *((USHORT*)pCfgMem);
		USHORT size = 0;
		if (sign == BASE_ID_TAG)
		{
			PICR_IdBase pBaseId = (PICR_IdBase)pCfgMem;
			ICR_IdBase	idBase;
			memcpy(&idBase, pBaseId, sizeof(ICR_IdBase));
			//RealBaseCfgSize = idBase.wSizeAll;
			PUCHAR pBaseCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdBase);
			size = sizeof(ICR_IdBase);
			PICR_CfgFmc119e pBambpexCfg = (PICR_CfgFmc119e)pBaseCfg;
			memcpy(pCfgBase, pBambpexCfg, sizeof(PICR_CfgFmc119e));
			size += sizeof(PICR_CfgFmc119e);
			end_flag = 1;
		}
		else
		{
			size = *((USHORT*)pCfgMem + 1);
			size += 4;
			//			break;
		}
		pCfgMem = (PUCHAR)pCfgMem + size;
	} while (!end_flag && pCfgMem < pEndCfgMem);
}

ULONG CBambusb::GetAdmIfCntICR(int& AdmIfCnt)
{
	// Check base module ICR
	ULONG icr_base_status;
	if (GetBaseIcrStatus(icr_base_status) != BRDerr_OK)
		//		return BRDerr_HW_ERROR;
		return ErrorPrint(BRDerr_HW_ERROR,
			CURRFILE,
			_BRDC("<GetAdmIfCntICR> Hardware error by get base ICR status"));
	if (icr_base_status)
	{
		PUCHAR pBaseCfgMem = new UCHAR[MAX_BASE_CFGMEM_SIZE];
		ULONG BufferSize = MAX_BASE_CFGMEM_SIZE;
		ULONG Offset = OFFSET_BASE_CFGMEM;
		//if(ReadNvRAM(pBaseCfgMem, BufferSize, Offset) != BRDerr_OK)
		if (GetNvRAM(pBaseCfgMem, BufferSize, Offset) != BRDerr_OK)
			//			return BRDerr_BAD_DEVICE_VITAL_DATA;
			return ErrorPrint(BRDerr_BAD_DEVICE_VITAL_DATA,
				CURRFILE,
				_BRDC("<GetAdmIfCntICR> Bad device vital data by read base ICR"));
		ULONG dRealBaseCfgSize;
		if (*(PUSHORT)pBaseCfgMem == BASE_ID_TAG)
		{
			dRealBaseCfgSize = *((PUSHORT)pBaseCfgMem + 2);
			{
				ICR_CfgFmc119e cfgBase;
				GetBaseCfgICR(pBaseCfgMem, dRealBaseCfgSize, &cfgBase);
				AdmIfCnt = cfgBase.bAdmIfCnt;
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

ULONG CBambusb::GetBaseEEPROM(PUCHAR pBaseCfgMem, ULONG& BaseCfgSize)
{
	// Check unit ICR
	ULONG icr_adm_status;
	if(GetAdmIcrStatus(icr_adm_status) != BRDerr_OK)
	{
		BaseCfgSize = 0;
		//return BRDerr_HW_ERROR;
	}
	if(icr_adm_status)
	{	// получаем конфигурацию модуля из его ППЗУ
		ReadNvRAM(pBaseCfgMem, BaseCfgSize, 0x80);
		if(*(PUSHORT)pBaseCfgMem == BASE_ID_TAG)
		{
			BaseCfgSize = *((PUSHORT)pBaseCfgMem + 2);
			if(!BaseCfgSize)
			{
				BaseCfgSize = 0;
				//return BRDerr_BAD_DEVICE_VITAL_DATA;
			}
		}
		else
		{
			BaseCfgSize = 0;
			//return BRDerr_BAD_DEVICE_VITAL_DATA;
		}
	}
	else
	{
		BaseCfgSize = 0;
		//return BRDerr_BAD_DEVICE_VITAL_DATA;
	}
	return BRDerr_OK;
}

ULONG CBambusb::SetSubEeprom(int idxSub, ULONG& SubCfgSize)
{
	if(!m_Subunit[idxSub].pSubEEPROM)
	{
		BRDCHAR nameFileMap[MAX_PATH];
		BRDC_sprintf(nameFileMap, _BRDC("subeeprom_%s_%d_%d"), m_name, m_PID, idxSub);

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		for (int i = 0; i < 100; i++)
		{
			m_Subunit[idxSub].hFileMap = IPC_createSharedMemory(nameFileMap, SUBMOD_CFGMEM_SIZE);
			if (m_Subunit[idxSub].hFileMap != 0)
				break;
		}
		if(!m_Subunit[idxSub].hFileMap)
			return BRDerr_ERROR;
		m_Subunit[idxSub].pSubEEPROM = IPC_mapSharedMemory(m_Subunit[idxSub].hFileMap);
		if(!m_Subunit[idxSub].pSubEEPROM)
		{
			IPC_deleteSharedMemory(m_Subunit[idxSub].hFileMap);
			return BRDerr_ERROR;
		}
#else
		m_Subunit[idxSub].hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,
											NULL, PAGE_READWRITE,
											0, SUBMOD_CFGMEM_SIZE,
											nameFileMap);
		if(!m_Subunit[idxSub].hFileMap)
			return BRDerr_ERROR;
//		int isAlreadyCreated = ( GetLastError() == ERROR_ALREADY_EXISTS ) ? 1 : 0;
		m_Subunit[idxSub].pSubEEPROM = (PVOID)MapViewOfFile(m_pStrmBlk[iBlk].hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		if(!m_Subunit[idxSub].pSubEEPROM)
		{
			UnmapViewOfFile(m_Subunit[idxSub].hFileMap);
			return BRDerr_ERROR;
		}
#endif
		//m_Subunit[idxSub].hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,
		//												NULL, PAGE_READWRITE,
		//												0, SUBMOD_CFGMEM_SIZE,
		//												nameFileMap);
		//int isAlreadyCreated = ( GetLastError() == ERROR_ALREADY_EXISTS ) ? 1 : 0;
		//m_Subunit[idxSub].pSubEEPROM = (PVOID)MapViewOfFile(m_Subunit[idxSub].hFileMap,
		//													FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		PUSHORT pSubCfgMem = (PUSHORT)(m_Subunit[idxSub].pSubEEPROM);
		if(m_Subunit[idxSub].pSubEEPROM)
		{	// если 1-й раз, то заполняем
			pSubCfgMem[0] = 0xA55A;
			pSubCfgMem[1] = 0x5AA5;
			// получаем конфигурацию субмодуля из его ППЗУ
			ULONG Status = ReadAdmIdROM(m_Subunit[idxSub].pSubEEPROM, SUBMOD_CFGMEM_SIZE, 0);
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

ULONG CBambusb::GetSubEEPROM(int idxSub, PUCHAR pSubCfgMem, ULONG& SubCfgSize)
{
	// Check subunit ICR
	ULONG icr_adm_status;
	if(GetAdmIcrStatus(icr_adm_status) != BRDerr_OK)
		return BRDerr_HW_ERROR;
	if(icr_adm_status)
	{	// получаем конфигурацию субмодуля из его ППЗУ
		memcpy(pSubCfgMem, m_Subunit[idxSub].pSubEEPROM, SubCfgSize);
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

void CBambusb::GetFmcCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgFmc119e pCfgBase)
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
			//RealBaseCfgSize = idBase.wSizeAll;
			PUCHAR pBaseCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdBase);
			size = sizeof(ICR_IdBase);
			PICR_CfgFmc119e pF119Cfg = (PICR_CfgFmc119e)pBaseCfg;
			memcpy(pCfgBase, pF119Cfg, sizeof(ICR_CfgFmc119e));
			size += sizeof(ICR_CfgFmc119e);
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

void CBambusb::GetDdr3CfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDdr3 pCfgDdr3)
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

ULONG CBambusb::SetSdramConfig(PBASE_INI pIniData, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
	ULONG Status = 0;
	m_SysGen = 125000000; // 125 МГц
	for(int i = 0; i < MAX_MEMORY_SRV; i++)
	{
		/*memcpy(&m_MemCfg[i], &MemCfg_dflt, sizeof(MEM_CFG));
		m_MemCfg[i].ModuleCnt |= 0x80;
		m_MemCfg[i].ModuleBanks |= 0x80;
		m_MemCfg[i].RowAddrBits |= 0x80;
		m_MemCfg[i].ColAddrBits |= 0x80;
		m_MemCfg[i].ChipBanks |= 0x80;
		m_MemCfg[i].PrimWidth |= 0x80;
		m_MemCfg[i].CasLat |= 0x80;
		m_MemCfg[i].Attributes |= 0x80;
		m_MemCfg[i].SlotCnt = 1;*/

		m_Ddr3Cfg[i].CfgSrc = 2;
		if(BaseEEPROMSize)
		{
			ICR_CfgFmc119e icr_base = {0xFF};
			GetFmcCfgICR(pBaseEEPROMMem, BaseEEPROMSize, (PICR_CfgFmc119e)&icr_base);
			m_SysGen = icr_base.dSysGen;
			if ((icr_base.bIsSodimDDR3 == 1) && (icr_base.bIsInternalDDR3 == 1))
			{
				ICR_CfgDdr3 icr_memory;
				GetDdr3CfgICR(pBaseEEPROMMem, BaseEEPROMSize, &icr_memory);
				m_Ddr3Cfg[i].ModuleCnt = icr_memory.bModuleCnt;
				m_Ddr3Cfg[i].ModuleBanks = icr_memory.bModuleBanks;
				m_Ddr3Cfg[i].RowAddrBits = icr_memory.bRowAddrBits;
				m_Ddr3Cfg[i].ColAddrBits = icr_memory.bColAddrBits;
				m_Ddr3Cfg[i].ChipBanks = icr_memory.bChipBanks;
				m_Ddr3Cfg[i].PrimWidth = icr_memory.bPrimaryWidth;
				m_Ddr3Cfg[i].ChipWidth = icr_memory.bChipWidth;
				m_Ddr3Cfg[i].CapacityMbits = (U64)icr_memory.wCapacityMbits * 1024 * 1024;
			}
		}
	}
	return Status;
}

ULONG CBambusb::SetServices()
{
	DeleteServices();
	int iSrv = 0, iStreamSrv = 0, iMemorySrv = 0, iSysMonSrv = 0, iFmc128Srv = 0, iTestSrv=0;

	for (int iTst = 0; iTst < MAX_TEST_SRV; iTst++)
	{
		TESTSRV_CFG tst_srv_cfg = {0};
		tst_srv_cfg.isAlreadyInit = 0;
		m_pTestSrv[iTestSrv] = new CTestSrv(iSrv++, iTestSrv, this, &tst_srv_cfg);
		m_SrvList.push_back(m_pTestSrv[iTestSrv++]);
	}

for(int iMem = 0; iMem < MAX_MEMORY_SRV; iMem++)
	{
		DDR3SDRAMSRV_CFG mem_srv_cfg = {0};
		BRDC_strcpy(mem_srv_cfg.DlgDllName, _BRDC(""));
		mem_srv_cfg.isAlreadyInit = 0;
		mem_srv_cfg.MemType = 0x0B; // DDR3
		mem_srv_cfg.CfgSource = m_Ddr3Cfg[iMem].CfgSrc;
		mem_srv_cfg.SlotCnt = m_Ddr3Cfg[iMem].ModuleCnt;
		mem_srv_cfg.ModuleCnt = m_Ddr3Cfg[iMem].ModuleCnt;
		mem_srv_cfg.ModuleBanks = m_Ddr3Cfg[iMem].ModuleBanks;
		mem_srv_cfg.RowAddrBits = m_Ddr3Cfg[iMem].RowAddrBits;
		mem_srv_cfg.ColAddrBits = m_Ddr3Cfg[iMem].ColAddrBits;
		mem_srv_cfg.ChipBanks = m_Ddr3Cfg[iMem].ChipBanks;
		mem_srv_cfg.PrimWidth = m_Ddr3Cfg[iMem].PrimWidth;
		mem_srv_cfg.ChipWidth = m_Ddr3Cfg[iMem].ChipWidth;
		mem_srv_cfg.CapacityMbits = m_Ddr3Cfg[iMem].CapacityMbits;
		mem_srv_cfg.TetrNum = m_memIni[iMem].TetrNum;
		mem_srv_cfg.AdrMask = m_memIni[iMem].AdrMask;
		mem_srv_cfg.AdrConst = m_memIni[iMem].AdrConst;
		m_pMemorySrv[iMemorySrv] = new CDdr3SdramSrv(iSrv++, iMemorySrv, this, &mem_srv_cfg);
#ifndef __linux__
		m_pMemorySrv[iMemorySrv]->SetPropDlg((void*)DEVS_propDlg);
#endif
		m_SrvList.push_back(m_pMemorySrv[iMemorySrv++]);
	}
	for(int iSysMon = 0; iSysMon < MAX_SYSMON_SRV; iSysMon++)
	{
		SYSMONSRV_CFG sysmon_srv_cfg = {0};
		sysmon_srv_cfg.isAlreadyInit = 0;
		sysmon_srv_cfg.DeviceID = m_DeviceID;
		sysmon_srv_cfg.PldType = m_pldType;
		m_pSysMonSrv[iSysMonSrv] = new CSysMonSrv(iSrv++, iSysMonSrv, this, &sysmon_srv_cfg);
		m_SrvList.push_back(m_pSysMonSrv[iSysMonSrv++]);
	}
	for(int iStrm = 0; iStrm < MAX_STREAM_SRV; iStrm++)
	{
		STREAMSRV_CFG strm_srv_cfg = {0};
		m_pStreamSrv[iStreamSrv] = new CStreamSrv(iSrv++, iStreamSrv, this, &strm_srv_cfg);
		m_SrvList.push_back(m_pStreamSrv[iStreamSrv++]);
	}
	//FMC128SRV_CFG fmc128srv_cfg;
	//fmc128srv_cfg.isAlreadyInit = 0;
	//fmc128srv_cfg.AdrSwitch = 0x48;
	//fmc128srv_cfg.SwitchType = 1;
	//m_pFmc128Srv = new Cfmc128srv(iSrv++, iFmc128Srv, this, &fmc128srv_cfg);
	return BRDerr_OK;
}

void CBambusb::DeleteServices()
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
	for (int i = 0; i < MAX_TEST_SRV; i++)
	{
		if (m_pTestSrv[i]) {
			delete m_pTestSrv[i];
			m_pTestSrv[i] = NULL;
		}
	}
	for(int i = 0; i < MAX_STREAM_SRV; i++)
	{
		if(m_pStreamSrv[i]) {
			delete m_pStreamSrv[i];
			m_pStreamSrv[i] = NULL;
		}
	}
	//if (m_pFmc128Srv)
	//{
	//	delete m_pFmc128Srv;
	//	m_pFmc128Srv = NULL;
	//}
}

void CBambusb::SetCandidateSrv()
{
	int srv_num = (int)m_ServInfoList.size();
	PTCHAR pchar;
	for(int i = 0; i < srv_num; i++)
	{
		ULONG attr = m_ServInfoList[i]->attribute;
		int ius = 0;
		if (attr & BRDserv_ATTR_SDRAMABLE)
		{
			for (int j = 0; j < srv_num; j++)
			{
				//				if(m_ServInfoList[j]->attribute & BRDserv_ATTR_SDRAM)
				{
					pchar = BRDC_strstr(m_ServInfoList[j]->name, _BRDC("BASESDRAM"));
					if (pchar)
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
		}
		if(attr & BRDserv_ATTR_STREAMABLE_OUT)
		{
			for(int j = 0; j < srv_num; j++)
			{
				if( (m_ServInfoList[j]->attribute & BRDserv_ATTR_STREAM) &&
					(m_ServInfoList[j]->attribute & BRDserv_ATTR_DIRECTION_OUT))
					if (ius < (MAX_UNDERSRV-1))
						m_ServInfoList[i]->idxCandidSrv[ius++] = j;
			}
		}
		if(attr & BRDserv_ATTR_STREAMABLE_IN)
		{
			for(int j = 0; j < srv_num; j++)
			{
				if( (m_ServInfoList[j]->attribute & BRDserv_ATTR_STREAM) &&
					(m_ServInfoList[j]->attribute & BRDserv_ATTR_DIRECTION_IN))
					if (ius < (MAX_UNDERSRV-1))
						m_ServInfoList[i]->idxCandidSrv[ius++] = j;
			}
		}
		if(attr & BRDserv_ATTR_ADCABLE)
		{
			for(int j = 0; j < srv_num; j++)
			{
				{
					pchar = BRDC_strstr(m_ServInfoList[j]->name, _BRDC("ADC"));
					if(pchar)
					{
						if (ius < (MAX_UNDERSRV-1))
							m_ServInfoList[i]->idxCandidSrv[ius++] = j;
						break;
					}
				}
			}
		}
	}
}

LONG CBambusb::RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam)
{
	if (m_isDLMode != 0)
		return BRDerr_BAD_MODE;
	FILE *pFile, *pFile2;
	if (m_RegLog)
		pFile = BRDC_fopen(_BRDC("usb reglog.txt"), _BRDC("a"));
	if (m_isLog)
		pFile2 = BRDC_fopen(_BRDC("commands.txt"), _BRDC("a"));
	ULONG admif = 0;
	ULONG status = BRDerr_OK;
	switch(cmd)
	{
		case DEVScmd_REGREADIND:
			status = ReadRegData(admif, pRegParam->tetr, pRegParam->reg, pRegParam->val);
			if (m_RegLog)
				BRDC_fprintf(pFile, _BRDC("read 0x%X reg indirect 0x%X tetrad result=0x%X value=0x%X\n"), pRegParam->reg, pRegParam->tetr, status, pRegParam->val);
			if (m_isLog)
				BRDC_fprintf(pFile2, _BRDC("read 0x%X reg indirect 0x%X tetrad result=0x%X value=0x%X\n"), pRegParam->reg, pRegParam->tetr, status, pRegParam->val);
			break;
		case DEVScmd_REGREADSIND:
			status = ReadRegBuf(admif, pRegParam->tetr, pRegParam->reg, pRegParam->pBuf, pRegParam->bytes);
			if (m_RegLog)
				BRDC_fprintf(pFile, _BRDC("read %d byte from 0x%X reg indirect 0x%X tetrad result=0x%X\n"), pRegParam->bytes, pRegParam->reg, pRegParam->tetr, status);
			if (m_isLog)
				BRDC_fprintf(pFile2, _BRDC("read %d byte from 0x%X reg indirect 0x%X tetrad result=0x%X\n"), pRegParam->bytes, pRegParam->reg, pRegParam->tetr, status);
			break;
		case DEVScmd_REGREADDIR:
			status = ReadRegDataDir(admif, pRegParam->tetr, pRegParam->reg, pRegParam->val);
			if (m_RegLog)
				BRDC_fprintf(pFile, _BRDC("read 0x%X reg direct 0x%X tetrad result=0x%X value=0x%X\n"), pRegParam->reg, pRegParam->tetr, status, pRegParam->val);
			if (m_isLog)
				BRDC_fprintf(pFile2, _BRDC("read 0x%X reg direct 0x%X tetrad result=0x%X value=0x%X\n"), pRegParam->reg, pRegParam->tetr, status, pRegParam->val);
			break;
		case DEVScmd_REGREADSDIR:
			status = ReadRegBufDir(admif, pRegParam->tetr, pRegParam->reg, pRegParam->pBuf, pRegParam->bytes);
			if (m_RegLog)
				BRDC_fprintf(pFile, _BRDC("read %d byte from 0x%X reg direct 0x%X tetrad result=0x%X\n"), pRegParam->bytes, pRegParam->reg, pRegParam->tetr, status);
			if (m_isLog)
				BRDC_fprintf(pFile2, _BRDC("read %d byte from 0x%X reg direct 0x%X tetrad result=0x%X\n"), pRegParam->bytes, pRegParam->reg, pRegParam->tetr, status);
			break;
		case DEVScmd_REGWRITEIND:
			status = WriteRegData(admif, pRegParam->tetr, pRegParam->reg, pRegParam->val);
			if (m_RegLog)
				BRDC_fprintf(pFile, _BRDC("write 0x%X reg indirect 0x%X tetrad result=0x%X value=0x%X\n"), pRegParam->reg, pRegParam->tetr, status, pRegParam->val);
			if (m_isLog)
				BRDC_fprintf(pFile2, _BRDC("write 0x%X reg indirect 0x%X tetrad result=0x%X value=0x%X\n"), pRegParam->reg, pRegParam->tetr, status, pRegParam->val);
			break;
		case DEVScmd_REGWRITESIND:
			status = WriteRegBuf(admif, pRegParam->tetr, pRegParam->reg, pRegParam->pBuf, pRegParam->bytes);
			if (m_RegLog)
				BRDC_fprintf(pFile, _BRDC("write %d byte to 0x%X reg indirect 0x%X tetrad result=0x%X\n"), pRegParam->bytes, pRegParam->reg, pRegParam->tetr, status);
			if (m_isLog)
				BRDC_fprintf(pFile2, _BRDC("write %d byte to 0x%X reg indirect 0x%X tetrad result=0x%X\n"), pRegParam->bytes, pRegParam->reg, pRegParam->tetr, status);
			break;
		case DEVScmd_REGWRITEDIR:
			status = WriteRegDataDir(admif, pRegParam->tetr, pRegParam->reg, pRegParam->val);
			if (m_RegLog)
				BRDC_fprintf(pFile, _BRDC("write 0x%X reg direct 0x%X tetrad result=0x%X value=0x%X\n"), pRegParam->reg, pRegParam->tetr, status, pRegParam->val);
			if (m_isLog)
				BRDC_fprintf(pFile2, _BRDC("write 0x%X reg direct 0x%X tetrad result=0x%X value=0x%X\n"), pRegParam->reg, pRegParam->tetr, status, pRegParam->val);
			break;
		case DEVScmd_REGWRITESDIR:
			status = WriteRegBufDir(admif, pRegParam->tetr, pRegParam->reg, pRegParam->pBuf, pRegParam->bytes);
			if (m_RegLog)
				BRDC_fprintf(pFile, _BRDC("write %d byte to 0x%X reg direct 0x%X tetrad result=0x%X\n"), pRegParam->bytes, pRegParam->reg, pRegParam->tetr, status);
			if (m_isLog)
				BRDC_fprintf(pFile2, _BRDC("write %d byte to 0x%X reg direct 0x%X tetrad result=0x%X\n"), pRegParam->bytes, pRegParam->reg, pRegParam->tetr, status);
			break;
		case DEVScmd_CLEARSTATIRQ:
			status = BRDerr_OK;
			break;
		case DEVScmd_SETSTATIRQ:
			status = BRDerr_OK;
			break;
		case DEVScmd_WAITSTATIRQ:
			status = BRDerr_OK;
			break;
		case DEVScmd_GETBASEADR:
			status = BRDerr_OK;
			break;
		case DEVScmd_PACKEXECUTE:
			status = PackExecute(admif, pRegParam->tetr, pRegParam->reg, pRegParam->pBuf, pRegParam->bytes);
			if (m_RegLog||m_isLog)
			{
				PDEVS_CMD_Reg commands = (PDEVS_CMD_Reg)pRegParam->pBuf;
				U32 nCommNum = pRegParam->bytes / sizeof(DEVS_CMD_Reg);
				for (UINT i = 0; i < nCommNum; i++)
				{
					switch (commands[i].idxMain)
					{
					case 0://REG READ DIR
						if (m_RegLog)
							BRDC_fprintf(pFile, _BRDC("pack read 0x%X reg direct 0x%X tetrad result=0x%X value=0x%X\n"), commands[i].reg, commands[i].tetr, status, commands[i].val);
						if (m_isLog)
							BRDC_fprintf(pFile2, _BRDC("pack read 0x%X reg direct 0x%X tetrad result=0x%X value=0x%X\n"), commands[i].reg, commands[i].tetr, status, commands[i].val);
						break;
					case 2://REG READ IND
						if (m_RegLog)
							BRDC_fprintf(pFile, _BRDC("pack read 0x%X reg indirect 0x%X tetrad result=0x%X value=0x%X\n"), commands[i].reg, commands[i].tetr, status, commands[i].val);
						if (m_isLog)
							BRDC_fprintf(pFile2, _BRDC("pack read 0x%X reg indirect 0x%X tetrad result=0x%X value=0x%X\n"), commands[i].reg, commands[i].tetr, status, commands[i].val);
						break;
					case 4://REG WRITE DIR
						if (m_RegLog)
							BRDC_fprintf(pFile, _BRDC("pack write 0x%X reg direct 0x%X tetrad result=0x%X value=0x%X\n"), commands[i].reg, commands[i].tetr, status, commands[i].val);
						if (m_isLog)
							BRDC_fprintf(pFile2, _BRDC("pack write 0x%X reg direct 0x%X tetrad result=0x%X value=0x%X\n"), commands[i].reg, commands[i].tetr, status, commands[i].val);
						break;
					case 6://REG WRITE IND
						if (m_RegLog)
							BRDC_fprintf(pFile, _BRDC("pack write 0x%X reg indirect 0x%X tetrad result=0x%X value=0x%X\n"), commands[i].reg, commands[i].tetr, status, commands[i].val);
						if (m_isLog)
							BRDC_fprintf(pFile2, _BRDC("pack write 0x%X reg indirect 0x%X tetrad result=0x%X value=0x%X\n"), commands[i].reg, commands[i].tetr, status, commands[i].val);
						break;
					default:
						if (m_RegLog)
							BRDC_fprintf(pFile, _BRDC("pack ERROR OP=0x%X 0x%X reg direct 0x%X tetrad result=0x%X value=0x%X\n"), commands[i].idxMain, commands[i].reg, commands[i].tetr, status, commands[i].val);
						if (m_isLog)
							BRDC_fprintf(pFile2, _BRDC("pack ERROR OP=0x%X 0x%X reg direct 0x%X tetrad result=0x%X value=0x%X\n"), commands[i].idxMain, commands[i].reg, commands[i].tetr, status, commands[i].val);
						break;
					}
				}
			}
			break;
		default:
			{
				if (m_RegLog) fclose(pFile);
				if (m_isLog) fclose(pFile2);
				return ErrorPrint(BRDerr_CMD_UNSUPPORTED,
							  CURRFILE,
							  _BRDC("<RegCtrl> Command not supported"));
			}
	}
	if (m_RegLog) fclose(pFile);
	if (m_isLog) fclose(pFile2);
	if(121 == status)
			return ErrorPrint(BRDerr_WAIT_TIMEOUT,
							  CURRFILE,
							  _BRDC("<RegCtrl> The time-out interval elapsed"));

	return status;
}

ULONG CBambusb::StreamCtrl(ULONG cmd, PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	ULONG Status = BRDerr_OK;
	if (m_isDLMode != 0)
		return BRDerr_BAD_MODE;
	switch(cmd)
	{
		case STRMcmd_SETMEMORY:
			Status = SetMemory(pParam, sizeParam, pOverlap);
			break;
		case STRMcmd_FREEMEMORY:
			Status = FreeMemory(pParam, sizeParam, pOverlap);
			break;
		case STRMcmd_STARTMEMORY:
			Status = StartMemory(pParam, sizeParam, pOverlap);
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
		case STRMcmd_GETDMACHANINFO:
			Status = GetDmaChanInfo(pParam, sizeParam, pOverlap);
			break;
		case STRMcmd_RESETFIFO:
			Status = ResetFIFO(pParam, sizeParam, pOverlap);
			break;
		case STRMcmd_WAITDMABLOCK:
			Status = WaitDMABlock(pParam, sizeParam, pOverlap);
			break;
		case STRMcmd_WAITDMABUFFER:
			Status = WaitDMABuffer(pParam, sizeParam, pOverlap);
			break;
		case STRMcmd_ADJUST:
			Status = Adjust(pParam, sizeParam, pOverlap);
			break;
		case STRMcmd_DONE:
			Status = BufferDone(pParam, sizeParam, pOverlap);
			break;
		default:
			Status = BRDerr_CMD_UNSUPPORTED;
	}
	return Status;
}

ULONG CBambusb::ReadSubICR(ULONG puId, void* buffer, U32 bufSize, U32 bufOffset)
{
	memcpy(buffer, m_subICR + bufOffset, bufSize);
	return 0;
}

ULONG CBambusb::WriteSubICR(ULONG puId, void* buffer, U32 bufSize, U32 bufOffset)
{
	if (m_isDLMode != 0)
		return BRDerr_BAD_MODE;
	U32 hVal, lVal=400000, bufIndex = 0, isBreak = 0;
	hVal = (1 << 28);
	ULONG Status = 0;
	//захват I2C
	Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
	while (1)
	{
		Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 0);
		if (lVal == 400000)
			break;
		else
		{
			lVal = 400000;
			Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);
		}
	}
	//
	for (unsigned int i = bufOffset; i < bufSize + bufOffset; i++)
	{
		while (1)
		{
			hVal = 0x20 + 0x016;//FID + SPD_CTRL
			lVal = 0;
			SendCommand(0, 0, hVal, lVal, 1, 2, 0, 0);//read SPD_CTRL
			if (((lVal >> 15) & 1) == 1) //ready
				break;
			isBreak++;
			if (isBreak > 10)
			{
				Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);//освобождение I2C
				return BRDerr_ERROR;
			}
		}
		hVal = 0x20 /* FID */ + 0x15;
		lVal = 0;
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//SPD_DEVICE = 0;
		hVal = 0x20 + 0x17;//FID + SPD_ADDR
		lVal = i;
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//SPD_ADDR
		hVal = 0x20 + 0x18;//FID + DATAL
		lVal = 0;
		memcpy(&lVal, (PUCHAR)buffer + bufIndex, 4);
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//DATAL
		hVal = 0x20 + 0x16;//FID + SPD_CTRL
		if (i >= 256)
			lVal = 2 + (0x55 << 4);
		else
			lVal = 2 + (0x54 << 4);
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//SPD_CTRL
		isBreak = 0;
		while (1)
		{
			hVal = 0x20 + 0x016;//FID + SPD_CTRL
			lVal = 0;
			SendCommand(0, 0, hVal, lVal, 1, 2, 0, 0);//read SPD_CTRL
			if (((lVal >> 15) & 1) == 1) //ready
				break;
			isBreak++;
			if (isBreak > 10)
			{
				Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);//освобождение I2C
				return BRDerr_ERROR;
			}
		}
		hVal = 0x20 + 0x16;//FID + SPD_CTRL
		lVal = 0;
		SendCommand(0, 0, hVal, lVal, 1, 2, 0, 1);//SPD_CTRL
		bufIndex++;
		IPC_delay(10);
	}
	memcpy(m_subICR + bufOffset, buffer, bufSize);//дублируем в локальную копию
	Status = SendCommand(0, 1, 0, lVal, 0, 0, 2, 1);//освобождение I2C
	return BRDerr_OK;
}

ULONG CBambusb::ReadI2C(U32 devnum, U32 devadr, U32 regadr, U32* pVal, int dblbyte)
{
	return BRDerr_FUNC_UNIMPLEMENTED;
}

ULONG CBambusb::WriteI2C(U32 devnum, U32 devadr, U32 regadr, U32 val, int dblbyte)
{
	return BRDerr_FUNC_UNIMPLEMENTED;
}

ULONG CBambusb::GetSensors(void* sens)
{
	return BRDerr_FUNC_UNIMPLEMENTED;
}