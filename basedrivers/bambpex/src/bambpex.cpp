//****************** File bambpex.cpp *******************************
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

#define DEVS_API_EXPORTS
#include "bambpex.h"

#define CURRFILE _BRDC("BAMBPEX")

//******************************************
CBambpex::CBambpex()
    : CBaseModule(MAX_BASE_CFGMEM_SIZE)
{
#ifdef _c6x_
    m_devMem = IPC_openDeviceRaw("/dev/mem");
    if (!m_devMem) {
        fprintf(stderr, "%s(): Error open /dev/mem\n", __FUNCTION__);
    }
#else
    m_devMem = 0;
#endif
    for (int i = 0; i < MAX_SYSMON_SRV; i++)
        m_pSysMonSrv[i] = NULL;
    for (int i = 0; i < MAX_TEST_SRV; i++)
        m_pTestSrv[i] = NULL;
    for (int i = 0; i < MAX_MEMORY_SRV; i++)
        m_pMemorySrv[i] = NULL;
    for (int i = 0; i < MAX_STREAM_SRV; i++)
        m_pStreamSrv[i] = NULL;
    m_pIniData = NULL;
    m_pMemBuf[0] = m_pMemBuf[1] = m_pMemBuf[2] = 0;
    BRDC_strcpy(m_pDlgDllName, _BRDC("BambpDlg.dll"));
    m_hMutex = NULL;
    m_DspPldInfo.Id = 0;
    m_FdspTetrNum = -1;
    m_hWDM = 0;
}

//********************** FindDlgDLLName *****************
// PTSTR* pLibName - указатель на название библиотеки (OUT)
// PBRD_InitData pInitData - указатель на массив данных инициализации вида "ключ-значение" (IN)
// long initDataSize - число оставшихся элементов массива pInitData (IN)
//*******************************************************
static void FindDlgDLLName(BRDCHAR** pLibName, PBRD_InitData pInitData, long initDataSize, const BRDCHAR* pKeyword)
{
    //	pLibName = NULL;
    // Search SubSections
    for (long i = 0; i < initDataSize; i++) {
        // if(!_stricmp(pInitData[i].key, pKeyword))	// Keyword
        if (!BRDC_stricmp(pInitData[i].key, pKeyword)) // Keyword
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
CBambpex::CBambpex(PBRD_InitData pBrdInitData, long sizeInitData)
    : CBaseModule(MAX_BASE_CFGMEM_SIZE)
{
#ifdef _c6x_
    m_devMem = IPC_openDeviceRaw("/dev/mem");
    if (!m_devMem) {
        fprintf(stderr, "%s(): Error open /dev/mem\n", __FUNCTION__);
    }
#else
    m_devMem = 0;
#endif

    for (int i = 0; i < MAX_SYSMON_SRV; i++)
        m_pSysMonSrv[i] = NULL;
    for (int i = 0; i < MAX_TEST_SRV; i++)
        m_pTestSrv[i] = NULL;
    for (int i = 0; i < MAX_MEMORY_SRV; i++)
        m_pMemorySrv[i] = NULL;
    for (int i = 0; i < MAX_STREAM_SRV; i++)
        m_pStreamSrv[i] = NULL;

    m_pIniData = new BASE_INI;
    // m_pIniData = (PBASE_INI)malloc(sizeof(BASE_INI));
    long size = sizeInitData;

    PINIT_Data pInitData = (PINIT_Data)pBrdInitData;
    // идентификация
    FindKeyWord(_BRDC("pid"), m_pIniData->PID, pInitData, size);

    // порядковый номер ПЛИС(для CompactPCI, когда на физически одном модуле размещено несколько видимых системой логических устройств)
    FindKeyWord(_BRDC("order"), m_pIniData->OrderNumber, pInitData, size);

    FindKeyWord(_BRDC("pcibus"), m_pIniData->BusNumber, pInitData, size);
    FindKeyWord(_BRDC("pcidev"), m_pIniData->DevNumber, pInitData, size);
    FindKeyWord(_BRDC("pcislot"), m_pIniData->SlotNumber, pInitData, size);

    // конфигурация
    //	FindKeyWord("dlgtype", m_pIniData->DlgDllName, pInitData, size);
    BRDCHAR* pDlgLibName = NULL;
    FindDlgDLLName(&pDlgLibName, pBrdInitData, size, _BRDC("bambpdlgdll"));
    if (pDlgLibName)
        BRDC_strcpy(m_pDlgDllName, pDlgLibName);
    else
        BRDC_strcpy(m_pDlgDllName, _BRDC("BambpDlg.dll"));

    pDlgLibName = NULL;
    FindDlgDLLName(&pDlgLibName, pBrdInitData, size, _BRDC("sdramdlgdll"));
    if (pDlgLibName) {
        for (int iMem = 0; iMem < MAX_MEMORY_SRV; iMem++)
            BRDC_strcpy(m_pIniData->MemIni[iMem].DlgDllName, pDlgLibName);
    } else {
        for (int iMem = 0; iMem < MAX_MEMORY_SRV; iMem++)
            BRDC_strcpy(m_pIniData->MemIni[iMem].DlgDllName, _BRDC(""));
    }

    FindKeyWord(_BRDC("sysgen"), m_pIniData->SysGen, pInitData, size);

    BRDCHAR key_name[64];
    for (int iMem = 0; iMem < MAX_MEMORY_SRV; iMem++) {
        BRDC_sprintf(key_name, _BRDC("slotcnt%d"), iMem);
        FindKeyWord(key_name, m_pIniData->MemIni[iMem].SlotCnt, pInitData, size);
        BRDC_sprintf(key_name, _BRDC("modulecnt%d"), iMem);
        FindKeyWord(key_name, m_pIniData->MemIni[iMem].ModuleCnt, pInitData, size);
        BRDC_sprintf(key_name, _BRDC("modulebanks%d"), iMem);
        FindKeyWord(key_name, m_pIniData->MemIni[iMem].ModuleBanks, pInitData, size);
        BRDC_sprintf(key_name, _BRDC("rowaddrbits%d"), iMem);
        FindKeyWord(key_name, m_pIniData->MemIni[iMem].RowAddrBits, pInitData, size);
        BRDC_sprintf(key_name, _BRDC("columnaddrbits%d"), iMem);
        FindKeyWord(key_name, m_pIniData->MemIni[iMem].ColAddrBits, pInitData, size);
        BRDC_sprintf(key_name, _BRDC("chipbanks%d"), iMem);
        FindKeyWord(key_name, m_pIniData->MemIni[iMem].ChipBanks, pInitData, size);
        BRDC_sprintf(key_name, _BRDC("caslatency%d"), iMem);
        FindKeyWord(key_name, m_pIniData->MemIni[iMem].CasLatency, pInitData, size);
        BRDC_sprintf(key_name, _BRDC("primwidth%d"), iMem);
        FindKeyWord(key_name, m_pIniData->MemIni[iMem].PrimWidth, pInitData, size);
        BRDC_sprintf(key_name, _BRDC("attibutes%d"), iMem);
        FindKeyWord(key_name, m_pIniData->MemIni[iMem].Attributes, pInitData, size);

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

    FindKeyWord(_BRDC("dsppldtype"), m_pIniData->DspIni.PldType, pInitData, size);
    FindKeyWord(_BRDC("dsppldvolume"), m_pIniData->DspIni.PldVolume, pInitData, size);
    FindKeyWord(_BRDC("dsppldpins"), m_pIniData->DspIni.PldPins, pInitData, size);
    FindKeyWord(_BRDC("dsppldrate"), m_pIniData->DspIni.PldSpeedGrade, pInitData, size);
    FindKeyWord(_BRDC("dsppiotype"), m_pIniData->DspIni.PioType, pInitData, size);

    // флаг включения питания FMC во время открытия устройства
    FindKeyWord(_BRDC("fmcpower"), m_pIniData->FmcPower, pInitData, size);
    if (m_pIniData->FmcPower == (U32)-1)
        m_pIniData->FmcPower = 1;

    m_hMutex = NULL;
    m_pMemBuf[0] = m_pMemBuf[1] = m_pMemBuf[2] = 0;
    m_DspPldInfo.Id = 0;
    m_FdspTetrNum = -1;
    m_hWDM = 0;
}

//******************************************
CBambpex::~CBambpex()
{
    if (m_hMutex) {
        // CloseHandle(m_hMutex);
        IPC_deleteMutex(m_hMutex);
        m_hMutex = NULL;
    }
    if (m_pIniData)
        delete m_pIniData;
    // free(m_pIniData);
    int srv_num = (int)m_SrvList.size();
    if (srv_num) {
        for (int i = 0; i < srv_num; i++) {
            // CRegSrv* pregsrv = m_SrvList.pop_back();
            m_SrvList.pop_back();
        }
    }
    DeleteServices();
    CleanUp();
}

static U16 flagPID = 0;
static U16 flagORDER = 0;

//******************** Init **********************
// Эта функция выполняет инициализацию (автоматическую и полуавтоматическую)
// U16 CurDevNum - начальный номер экземпляра модуля при поиске нужного базового модуля
// PTSTR pBoardName - сюда записывается оригинальное имя базового модуля
// PTSTR pIniString - NULL при полуавтоматической инициализации и
//						указатель на строку с инициализационными данными при автоматической инициализации
//************************************************
// S32 CBambpex::Init(short CurDevNum, PTSTR pBoardName, PTSTR pIniString)
S32 CBambpex::Init(short CurDevNum, BRDCHAR* pBoardName, BRDCHAR* pIniString)
{
    // int const nBufSize = 128;
    //	BRDCHAR ErrVitalText[nBufSize];
    //	int ret = GetStringText(IDS_STRINGVITAL, ErrVitalText);
    //	int ret = GetStringText(MSG_NON_VITAL_DATA, ErrVitalText);
    U32 save_pid = 0;
    U32 save_slot = 0;
    U32 save_order_num = 0;

    // SetProcessAffinityMask(GetCurrentProcess(), 0x1);

    //	WriteEventLog(MSG_NON_VITAL_DATA, ErrVitalText);
    //	MessageBox(NULL, "<Init> Enter", "BAMBP", MB_OK);
    if (m_pIniData) // NULL при автоматической инициализации
    {
        if (m_pIniData->PID == (U32)-1 && (m_pIniData->BusNumber == (U32)-1 || m_pIniData->DevNumber == (U32)-1) && m_pIniData->SlotNumber == (U32)-1)
            //			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                CURRFILE, // ErrVitalText);
                //_BRDC("<Init> Undefined vital data into init source"));
                _BRDC("<Init> Undefined vital data (PID, Slot or BUS) into init source"));

        if (!CurDevNum && m_pIniData->PID)
            flagPID = 1; // если у устройства 0 PID был НЕ равен 0

        if (flagPID == 1 && m_pIniData->PID == 0) // тогда и у других устройств PID НЕ должен быть 0
            //			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                CURRFILE, // ErrVitalText);
                //_BRDC("<Init> Undefined vital data into init source"));
                _BRDC("<Init> PID=0 by multyboard configuration"));

        // if(m_pIniData->PID || m_pIniData->OrderNumber != (U32)-1)
        if (m_pIniData->PID != 0)
            CurDevNum = 0;
        else if (flagORDER == 1 && m_pIniData->OrderNumber != (U32)-1)
            CurDevNum = 0;
    }
    AMB_LOCATION AmbLocation;
    S32 errorCode = BRDerr_OK;

    //=== Find PCI Card
    //	for(int DeviceNumber = CurDevNum; ; DeviceNumber++)
    int DeviceNumber = CurDevNum;
    do {
        // BRDCHAR DeviceName[32] = _BRDC("\\\\.\\");
        // BRDC_strcat(DeviceName, AmbDeviceName);
        // #ifdef _WIN64
        //		WCHAR cNumber[4];
        //		swprintf_s( cNumber, L"%d", DeviceNumber);
        //		BRDC_strcatW(DeviceName, cNumber);
        //		m_hWDM = CreateFileW(DeviceName,
        //							GENERIC_READ | GENERIC_WRITE,
        //							FILE_SHARE_READ | FILE_SHARE_WRITE,
        //							NULL,
        //							OPEN_EXISTING,
        //							0, //FILE_FLAG_OVERLAPPED,
        //							NULL);
        // #else
        //		strcat(DeviceName, AmbDeviceName);
        IPC_str deviceName[256] = _BRDC("");
        m_hWDM = IPC_openDevice(deviceName, AmbDeviceName, DeviceNumber);
        // #endif
        if (!m_hWDM) {
            //			BRDCHAR ErrNoDriverText[nBufSize];
            //			ret = GetStringText(IDS_STRINGERRNODRV, ErrNoDriverText);
            //			return DRVERR(BRDerr_NO_KERNEL_SUPPORT);
            if (!pIniString)
                // return ErrorPrintf(DRVERR(BRDerr_NO_KERNEL_SUPPORT),
                ErrorPrintf(DRVERR(BRDerr_WARN),
                    CURRFILE,
                    //								  "%s %s", ErrNoDriverText, DeviceName);
                    _BRDC("<Init> No device driver for %s"), deviceName);
            // else
            return DRVERR(BRDerr_NO_KERNEL_SUPPORT);
        }
        // BRDC_printf(_BRDC("IniData: Device is %s\n"), deviceName);

        //		MessageBox(NULL, "<Init> CreateFile", "BAMBPEX", MB_OK);
        // BRDCHAR buf[MAX_STRING_LEN];
        char buf[MAX_STRING_LEN];
        GetVersion(buf);

        if (!m_hMutex) {
            BRDCHAR nameMutex[MAX_PATH];
            BRDC_sprintf(nameMutex, _BRDC("mutex_BAMBPEX%d"), DeviceNumber);
            // m_hMutex = CreateMutex(NULL, FALSE, nameMutex);
            m_hMutex = IPC_createMutex(nameMutex, FALSE);
            // if(!m_hMutex)
            // MessageBox(NULL, nameMutex, "BAMBPEX", MB_OK);
            //			return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
            //							  CURRFILE, //ErrVitalText);
            //							  "<Init> Undefined vital data into init source");
        }

        //		ULONG pldStatus;
        //		ReadRegData(0, 4, pldStatus);
        // Get DeviceID, RevisionID, PCI Bus, PCI Device, PCI Slot, PID
        if (GetDeviceID(m_DeviceID) != BRDerr_OK) {
            if (m_hMutex) {
                // CloseHandle(m_hMutex);
                IPC_deleteMutex(m_hMutex);
                m_hMutex = NULL;
            }
            // CloseHandle(m_hWDM);
            IPC_closeDevice(m_hWDM);
            m_hWDM = NULL;
            //			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                CURRFILE, // ErrVitalText);
                //_BRDC("<Init> Undefined vital data into init source"));
                _BRDC("<Init> Error by GetDeviceID"));
        }
        if (m_DeviceID != AMBPEX8_DEVID && m_DeviceID != ADP201X1AMB_DEVID && m_DeviceID != ADP201X1DSP_DEVID && m_DeviceID != AMBPEX5_DEVID && m_DeviceID != FMC105P_DEVID && m_DeviceID != FMC106P_DEVID && m_DeviceID != FMC107P_DEVID && m_DeviceID != FMC103E_DEVID && m_DeviceID != FMC110P_DEVID && m_DeviceID != FMC111P_DEVID && m_DeviceID != FMC114V_DEVID && m_DeviceID != FMC113E_DEVID && m_DeviceID != FMC108V_DEVID && m_DeviceID != FMC115CP_DEVID && m_DeviceID != FMC117CP_DEVID && m_DeviceID != FMC112CP_DEVID && m_DeviceID != AC_ADC_DEVID && m_DeviceID != AC_DSP_DEVID && m_DeviceID != AC_SYNC_DEVID && m_DeviceID != PS_ADC_DEVID && m_DeviceID != PS_DSP_DEVID && m_DeviceID != PS_SYNC_DEVID && m_DeviceID != UADCPEX_DEVID && m_DeviceID != XM416X250M_DEVID && m_DeviceID != FMC121CP_DEVID && m_DeviceID != FMC123E_DEVID && m_DeviceID != FMC124P_DEVID && m_DeviceID != FMC125CP_DEVID && m_DeviceID != FMC127P_DEVID && m_DeviceID != FMC122P_DEVID && m_DeviceID != FMC126P_DEVID && m_DeviceID != FMC131VKU_DEVID && m_DeviceID != FMC131VZQ_DEVID && m_DeviceID != FMC141VKU1_DEVID && m_DeviceID != FMC141VKU2_DEVID && m_DeviceID != FMC141VZQ_DEVID && m_DeviceID != FMC143VKU_DEVID && m_DeviceID != FMC143VZQ_DEVID && m_DeviceID != FMC143VKUS_DEVID && m_DeviceID != FMC146VKU_DEVID && m_DeviceID != FMC146VZQ_DEVID && m_DeviceID != FMC132P_DEVID && m_DeviceID != FMC139P_DEVID && m_DeviceID != FMC133V_DEVID && m_DeviceID != DSP134V_DEVID && m_DeviceID != FMC121P_DEVID && m_DeviceID != RFG2_DEVID && m_DeviceID != PANORAMA_DEVID && m_DeviceID != D2XT006_DEVID && m_DeviceID != FMC130E_DEVID) // все типы поддерживаемых модулей
        {
            if (m_hMutex) {
                IPC_deleteMutex(m_hMutex);
                m_hMutex = NULL;
            }
            IPC_closeDevice(m_hWDM); // Wrong Board
            m_hWDM = NULL;
            DeviceNumber++;
            continue;
        }

        if (GetRevisionID(m_RevisionID) != BRDerr_OK) {
            if (m_hMutex) {
                IPC_deleteMutex(m_hMutex);
                m_hMutex = NULL;
            }
            IPC_closeDevice(m_hWDM);
            m_hWDM = NULL;
            //			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                CURRFILE, // ErrVitalText);
                //_BRDC("<Init> Undefined vital data into init source"));
                _BRDC("<Init> Error by GetRevisionID"));
        }
        if (GetLocation(&AmbLocation) != BRDerr_OK) {
            if (m_hMutex) {
                IPC_deleteMutex(m_hMutex);
                m_hMutex = NULL;
            }
            IPC_closeDevice(m_hWDM);
            m_hWDM = NULL;
            //			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                CURRFILE, // ErrVitalText);
                //_BRDC("<Init> Undefined vital data into init source"));
                _BRDC("<Init> Error by GetLocation"));
        }
        //		MessageBox(NULL, "<Init> GetDevice&GetLocation", "BAMBPEX", MB_OK);
        // errorCode = GetMemoryAddress();
        ICR_IdBase Info;
        if (ReadNvRAM(&Info, sizeof(ICR_IdBase), 0x80) != BRDerr_OK) {
            if (m_hMutex) {
                IPC_deleteMutex(m_hMutex);
                m_hMutex = NULL;
            }
            IPC_closeDevice(m_hWDM); // Wrong Board
            m_hWDM = NULL;
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                CURRFILE, // ErrVitalText);
                _BRDC("<Init> Error read data from ICR"));
        }
        if (GetBaseIcrDataSize() != BRDerr_OK) {
            if (m_hMutex) {
                IPC_deleteMutex(m_hMutex);
                m_hMutex = NULL;
            }
            IPC_closeDevice(m_hWDM);
            m_hWDM = NULL;
            //			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                CURRFILE, // ErrVitalText);
                //_BRDC("<Init> Undefined vital data into init source"));
                _BRDC("<Init> Error by GetBaseIcrDataSize"));
        }
        BaseIcrCorrectError();
        if (GetPid() != BRDerr_OK) {
            if (m_hMutex) {
                IPC_deleteMutex(m_hMutex);
                m_hMutex = NULL;
            }
            IPC_closeDevice(m_hWDM);
            m_hWDM = NULL;
            //			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
            return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
                CURRFILE, // ErrVitalText);
                //_BRDC("<Init> Undefined vital data into init source"));
                _BRDC("<Init> Error by GetPid"));
        }
        if (m_DeviceID == FMC115CP_DEVID || m_DeviceID == AC_SYNC_DEVID || m_DeviceID == PS_DSP_DEVID || m_DeviceID == PS_ADC_DEVID || m_DeviceID == FMC131VKU_DEVID || m_DeviceID == FMC131VZQ_DEVID || m_DeviceID == FMC141VKU1_DEVID || m_DeviceID == FMC141VKU2_DEVID || m_DeviceID == FMC141VZQ_DEVID || m_DeviceID == FMC143VKU_DEVID || m_DeviceID == FMC143VKUS_DEVID || m_DeviceID == FMC143VZQ_DEVID || m_DeviceID == FMC146VKU_DEVID || m_DeviceID == FMC146VZQ_DEVID || m_DeviceID == FMC117CP_DEVID) {
            flagORDER = 1;
            if (m_pIniData) {
                save_pid = m_pIniData->PID;
                save_slot = m_pIniData->SlotNumber;
                save_order_num = m_pIniData->OrderNumber;
                // BRDC_printf(_BRDC("IniData(before): PID = %d(%x), Slot = %d(%x), order = %d\n"),
                //						m_pIniData->PID, m_pIniData->PID, m_pIniData->SlotNumber, m_pIniData->SlotNumber, m_pIniData->OrderNumber);
                // BRDC_printf(_BRDC("Location(before): PID = %d(%x), SlotNumber = %x\n"), m_PID, m_PID, AmbLocation.SlotNumber);
                if ((m_pIniData->PID != (U32)-1) && (m_pIniData->OrderNumber != (U32)-1)) { // к PID из ini-файла замешиваю order из ini-файла
                    if (!m_pIniData->PID)
                        m_pIniData->PID = m_PID;
                    m_pIniData->PID += (m_pIniData->OrderNumber << 28);
                }
                if (m_pIniData->SlotNumber != (U32)-1) { // идентификация по номеру слота (географическому адресу) и order'у
                    m_pIniData->SlotNumber += (m_pIniData->OrderNumber << 16);
                }
            }
            if (AmbLocation.SlotNumber != (ULONG)-1) // если AmbLocation.SlotNumber не равно -1, то содержит геогр.адрес и order
            { // к PID из ICR замешиваю order из блока MAIN (который возвращается в 16 старших битах номера слота)
              // if(m_PID)
                m_PID = m_PID + ((AmbLocation.SlotNumber & 0xf0000) << 12);
                // else
                if (m_pIniData) // NULL при автоматической инициализации
                    if (m_pIniData->SlotNumber != (U32)-1)
                        m_PID = (AmbLocation.SlotNumber & 0xffff) + ((AmbLocation.SlotNumber & 0xf0000) << 12);
                // AmbLocation.SlotNumber &= 0xffff;
            }
            // BRDC_printf(_BRDC("IniData(after): PID = %d(%x), Slot = %d(%x), order = %d\n"),
            //						m_pIniData->PID, m_pIniData->PID, m_pIniData->SlotNumber, m_pIniData->SlotNumber, m_pIniData->OrderNumber);
            // BRDC_printf(_BRDC("Location(after): PID = %d(%x), SlotNumber = %x\n\n"), m_PID, m_PID, AmbLocation.SlotNumber);
        } else if (m_pIniData) {
            save_order_num = m_pIniData->OrderNumber;
            m_pIniData->OrderNumber = (U32)-1;
        }
        if (m_pIniData) {
            if (m_pIniData->PID == (U32)-1) {
                if (m_pIniData->SlotNumber == (U32)-1) {
                    if (m_pIniData->BusNumber == AmbLocation.BusNumber && m_pIniData->DevNumber == AmbLocation.DeviceNumber) {
                        m_PID += ((AmbLocation.BusNumber & 0xff) << 18);
                        break;
                    }
                } else {
                    if (m_pIniData->SlotNumber == AmbLocation.SlotNumber)
                        // if(m_pIniData->SlotNumber == (AmbLocation.SlotNumber & 0xFFFF))
                        break;
                }
            } else {
                if ((!m_pIniData->PID && m_pIniData->OrderNumber == (U32)-1) || m_pIniData->PID == m_PID)
                    // if(!m_pIniData->PID || m_pIniData->PID == m_PID)
                    break;
            }
            if (m_hMutex) {
                IPC_deleteMutex(m_hMutex);
                m_hMutex = NULL;
            }
            IPC_closeDevice(m_hWDM); // Wrong Board
            m_hWDM = NULL;
            DeviceNumber++;
            if (m_DeviceID == FMC115CP_DEVID || m_DeviceID == AC_SYNC_DEVID || m_DeviceID == PS_DSP_DEVID || m_DeviceID == PS_ADC_DEVID || m_DeviceID == FMC131VKU_DEVID || m_DeviceID == FMC131VZQ_DEVID || m_DeviceID == FMC141VKU1_DEVID || m_DeviceID == FMC141VKU2_DEVID || m_DeviceID == FMC141VZQ_DEVID || m_DeviceID == FMC143VKU_DEVID || m_DeviceID == FMC143VZQ_DEVID || m_DeviceID == FMC143VKUS_DEVID || m_DeviceID == FMC146VKU_DEVID || m_DeviceID == FMC146VZQ_DEVID || m_DeviceID == FMC117CP_DEVID) {
                m_pIniData->PID = save_pid;
                m_pIniData->SlotNumber = save_slot;
                m_pIniData->OrderNumber = save_order_num;
            } else if (m_pIniData)
                m_pIniData->OrderNumber = save_order_num;
        }
    } while (m_pIniData);

    // BRDCHAR buf[MAX_STRING_LEN];
    // GetVersion(buf);
    //	pBoard->m_DeviceID = DeviceID;
    //	pBoard->m_RevisionID = RevisionID;
    //	pBoard->m_PID    = PID;
    m_BusNum = AmbLocation.BusNumber;
    m_DevNum = AmbLocation.DeviceNumber;
    m_SlotNum = AmbLocation.SlotNumber;

    BRDC_sprintf(m_name, _BRDC("_%X"), m_PID);
    BRDC_strcpy(pBoardName, m_name);

    if (pIniString) {
        BRDC_sprintf(pIniString, _BRDC("pid=0x%X\npcibus=0x%X\npcidev=0x%X\npcislot=0x%X\n"), m_PID, m_BusNum, m_DevNum, m_SlotNum);
    }

    //	MessageBox(NULL, "BoardName", "BAMBP", MB_OK);
    //	MessageBox(NULL, m_name, "BAMBP", MB_OK);

    // if( m_DeviceID == AMBPEX8_DEVID ||
    //	m_DeviceID == ADP201X1AMB_DEVID ||
    //	m_DeviceID == ADP201X1DSP_DEVID ||
    //	m_DeviceID == AMBPEX5_DEVID ||
    //	m_DeviceID == FMC105P_DEVID ||
    //	m_DeviceID == FMC106P_DEVID ||
    //	m_DeviceID == FMC103E_DEVID ||
    //	m_DeviceID == FMC110P_DEVID ||
    //	m_DeviceID == FMC114V_DEVID ||
    //	m_DeviceID == FMC113E_DEVID ||
    //	m_DeviceID == FMC108V_DEVID ||
    //	m_DeviceID == FMC115CP_DEVID ||
    //	m_DeviceID == FMC112CP_DEVID
    //   )
    {
        errorCode = GetMemoryAddress();

        PULONG BlockMem = m_pMemBuf[0];
        ULONG BlockCnt = BlockMem[10];
        m_BlockFidAddr = 0;
        int idxFifo = 0;
        m_BlockFifoAddr[0] = 0;
        m_BlockFifoAddr[1] = 0;
        m_BlockFifoAddr[2] = 0;
        m_BlockFifoAddr[3] = 0;
        if (BlockCnt <= 0xFF)
            for (ULONG iBlock = 1; iBlock < BlockCnt; iBlock++) {
                ULONG BlkAddr = iBlock * 64;
                ULONG temp = BlockMem[BlkAddr];
                // temp = ReadOperationWordReg(PEFIDadr_BLOCK_ID + FidAddr);
                ULONG block_id = (temp & 0x0FFF);
                if (block_id == 0x0018)
                    m_BlockFifoAddr[idxFifo++] = BlkAddr;
                if (block_id == 0x0019)
                    m_BlockFidAddr = BlkAddr;
            }
        ULONG val = BlockMem[58];
        m_IcapSig = USHORT(val >> 16);
    }

    //	PUCHAR pBaseCfgMem = new UCHAR[MAX_BASE_CFGMEM_SIZE];
    PUCHAR pBaseCfgMem = new UCHAR[m_sizeICR];
    ULONG BaseCfgSize = m_sizeICR;
    if (GetBaseEEPROM(pBaseCfgMem, BaseCfgSize) != BRDerr_OK) {
        delete[] pBaseCfgMem;
        pBaseCfgMem = NULL;
    }
    ULONG SubCfgSize = 0;
    for (int i = 0; i < m_AdmIfCnt; i++) {
        errorCode = SetSubEeprom(i, SubCfgSize);
        // if(errorCode != BRDerr_OK)
        //	return ErrorPrint(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
        //					  CURRFILE, //ErrVitalText);
        //					  "<Init> Submodule EEPROM NOT read!");
    }
    errorCode = SetPuInfo(pBaseCfgMem, BaseCfgSize);
    errorCode = SetSdramConfig(m_pIniData, pBaseCfgMem, BaseCfgSize);
    errorCode = SetDspNodeConfig(m_pIniData, pBaseCfgMem, BaseCfgSize);
    if (pBaseCfgMem)
        delete[] pBaseCfgMem;

    if (!m_pIniData) {
        if (GetAdmIfCntICR(m_AdmIfCnt) == BRDerr_OK)
            errorCode = SetSubunitAuto();
    } else {
        m_AdmIfCnt = 0;
        m_SubunitCnt = 0;
    }

    errorCode = SetServices();

    if (m_DeviceID == FMC106P_DEVID)
        isFdsp();

    InitSensors();

    return DRVERR(errorCode);
}

//***************************************************************************************
void CBambpex::CleanUp()
{
    if (m_hWDM) {
        UnmapDeviceMemory(m_AmbConfiguration, 0);
        UnmapDeviceMemory(m_AmbConfiguration, 1);
        UnmapDeviceMemory(m_AmbConfiguration, 2);

        IPC_closeDevice(m_hWDM);
        m_hWDM = NULL;
    }
}

//***************************************************************************************
void CBambpex::GetDeviceInfo(BRD_Info* pInfo) const
{
    pInfo->boardType = ((ULONG)m_DeviceID << 16) | (ULONG)m_RevisionID;
    pInfo->pid = m_PID;
    switch (m_DeviceID) {
    case AMBPEX8_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("AMBPEX8"));
        break;
    case ADP201X1AMB_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("ADP201X1AMB"));
        break;
    case ADP201X1DSP_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("ADP201X1DSP"));
        break;
    case AMBPEX5_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("AMBPEX5"));
        break;
    case FMC105P_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC105P"));
        break;
    case FMC106P_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC106P"));
        break;
    case FMC107P_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC107P"));
        break;
    case FMC103E_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC103E2"));
        break;
    case FMC110P_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC110P"));
        break;
    case FMC114V_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC114V"));
        break;
    case FMC113E_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC113E"));
        break;
    case FMC108V_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC108V"));
        break;
    case FMC115CP_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC115CP"));
        break;
    case FMC112CP_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC112CP"));
        break;
    case FMC117CP_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC117CP"));
        break;
    case UADCPEX_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("UADCpex"));
        break;
    case XM416X250M_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("XM416x250M"));
        break;
    case AC_SYNC_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("Ac-Sync"));
        break;
    case PS_ADC_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("Ps-ADC"));
        break;
    case PS_DSP_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("Ps-DSP"));
        break;
    case PS_SYNC_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("Ps-SYNC"));
        break;
    case D2XT006_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("D2XT006"));
        break;
    case FMC121CP_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC121CP"));
        break;
    case FMC122P_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC122P"));
        break;
    case FMC123E_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC123E"));
        break;
    case FMC124P_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC124P"));
        break;
    case FMC125CP_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC125CP"));
        break;
    case FMC127P_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC127P"));
        break;
    case PANORAMA_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("PANORAMA"));
        break;
    case FMC111P_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC111P"));
        break;
    case FMC126P_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC126P"));
        break;
    case FMC130E_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC130E"));
        break;
    case FMC132P_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC132P"));
        break;
    case FMC121P_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC121cP(PCIe)"));
        break;
    case FMC133V_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC133V"));
        break;
    case DSP134V_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("DSP134V"));
        break;
    case RFG2_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("RFG2"));
        break;
    case FMC131VKU_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC131V KU"));
        break;
    case FMC131VZQ_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC131V Zynq"));
        break;
    case FMC141VKU1_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC141V KU1"));
        break;
    case FMC141VKU2_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC141V KU2"));
        break;
    case FMC141VZQ_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC141V Zynq"));
        break;
    case FMC143VKU_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC143V KU"));
        break;
    case FMC143VKUS_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC143V KU-Sl"));
        break;
    case FMC143VZQ_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC143V Zynq"));
        break;
    case FMC146VKU_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC146V KU"));
        break;
    case FMC146VZQ_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC146V Zynq"));
        break;
    case FMC139P_DEVID:
        BRDC_strcpy(pInfo->name, _BRDC("FMC139P"));
        break;
    default:
        BRDC_strcpy(pInfo->name, _BRDC("Unknown"));
        break;
    }
    pInfo->busType = BRDbus_PCI;
    pInfo->bus = m_BusNum;
    pInfo->dev = m_DevNum;
    pInfo->slot = m_SlotNum;
    pInfo->verMajor = VER_MAJOR;
    pInfo->verMinor = VER_MINOR;
    int i = 0;
    for (i = 0; i < MAX_ADMIF; i++)
        pInfo->subunitType[i] = 0xffff;
    for (i = 0; i < m_SubunitCnt; i++)
        pInfo->subunitType[i] = m_Subunit[i].type;
    for (i = 0; i < 8; i++)
        pInfo->base[i] = 0;
    for (i = 0; i < 3; i++)
        pInfo->base[i] = m_PhysMem[i];
    //	pInfo->base[7] = (U32)m_pMemBuf[0];

    memcpy(&pInfo->base[6], &m_pMemBuf[0], sizeof(m_pMemBuf[0]));
    // memcpy(&pRegParam->reg, &m_pMemBuf[1], sizeof(m_pMemBuf[1]));
}

//***************************************************************************************
ULONG CBambpex::HardwareInit()
{
    ULONG Status = BRDerr_OK;
    // if( m_DeviceID == AMBPEX8_DEVID ||
    //	m_DeviceID == ADP201X1AMB_DEVID ||
    //	m_DeviceID == ADP201X1DSP_DEVID ||
    //	m_DeviceID == AMBPEX5_DEVID ||
    //	m_DeviceID == FMC105P_DEVID ||
    //	m_DeviceID == FMC106P_DEVID ||
    //	m_DeviceID == FMC103E_DEVID ||
    //	m_DeviceID == FMC110P_DEVID ||
    //	m_DeviceID == FMC114V_DEVID ||
    //	m_DeviceID == FMC113E_DEVID ||
    //	m_DeviceID == FMC108V_DEVID ||
    //	m_DeviceID == FMC115CP_DEVID ||
    //	m_DeviceID == FMC112CP_DEVID
    //   )
    {
        ULONG pld_status = 1;
        if (GetPldStatus(pld_status, 0) != BRDerr_OK)
            //			return BRDerr_HW_ERROR;
            return ErrorPrint(BRDerr_HW_ERROR,
                CURRFILE,
                _BRDC("<HardwareInit> Hardware error"));
        if (pld_status) {
            Status = AdmPldStartTest();
            //		if(Status != BRDerr_OK)
            //			return;
            Status = AdmPldWorkAndCheck();
            //		if(Status != BRDerr_OK)
            //			return;
            Status = SetAdmPldFileInfo();
        }
        // Status = GetMemoryAddress();
    }

    if (m_DeviceID == FMC131VZQ_DEVID || m_DeviceID == FMC141VZQ_DEVID || m_DeviceID == FMC143VZQ_DEVID || m_DeviceID == FMC146VZQ_DEVID || m_DeviceID == DSP134V_DEVID)
        return Status;

    if ((!m_pIniData) || (m_pIniData && m_pIniData->FmcPower)) {
        BRDextn_FMCPOWER pow;
        pow.onOff = 1;
        PowerOnOff(&pow, 0);
    }

    // #ifdef _WIN32
    //	LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
    //	int bHighRes = QueryPerformanceFrequency (&Frequency);
    //	QueryPerformanceCounter (&StartPerformCount);
    // #endif
    //	UCHAR buf[128];
    //	if(m_BlockFidAddr)
    //	{
    // #ifdef _WIN32
    //		QueryPerformanceCounter (&StartPerformCount);
    // #endif
    //		Status = ReadFidSpd(0, 0x50, buf, 0x400, 128);
    // #ifdef _WIN32
    //		QueryPerformanceCounter (&StopPerformCount);
    //		double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
    //		BRDC_printf(_BRDC("ReadFidSpd() is %.2f ms\n"), msTime);
    //		//double mcsTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E6;
    //		//printf("Time is %.2f mcs\n", mcsTime);
    //		QueryPerformanceCounter (&StartPerformCount);
    // #endif
    //		Status = ReadAdmIdROM(buf, 128, 0);
    // #ifdef _WIN32
    //		QueryPerformanceCounter (&StopPerformCount);
    //		msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
    //		BRDC_printf(_BRDC("ReadAdmIdROM() is %.2f ms\n"), msTime);
    // #endif
    ////		//BRDextn_FMCEXT fmc_ext;
    ////		//fmc_ext.dev = 0x50;
    ////		//fmc_ext.addr = 0x400;
    ////		//fmc_ext.value = 0x94;
    ////		//Status = WriteFmcExt(&fmc_ext);
    ////		//fmc_ext.value = 0x00;
    ////		//Status = ReadFmcExt(&fmc_ext);
    ////		//fmc_ext.value = 0x80;
    ////		//Status = WriteFmcExt(&fmc_ext);
    ////		//Status = ReadFidSpd(0x50, buf, 0x400, 16);
    //	}
    //	if(m_DeviceID == FMC106P_DEVID)
    //		isFdsp();

    // отладка функций чтения/записи FMC-extender
    // BRDextn_FMCEXT fmc_ext;
    // fmc_ext.dev = 0x40;
    // fmc_ext.addr = 0x0;
    // fmc_ext.value = 0x00;
    // Status = ReadFmcExt(&fmc_ext);
    // fmc_ext.addr = 0x5;
    // fmc_ext.value = 0x00;
    // Status = ReadFmcExt(&fmc_ext);
    // fmc_ext.value = 0xa5b6;
    // Status = WriteFmcExt(&fmc_ext);
    // fmc_ext.value = 0x00;
    // Status = ReadFmcExt(&fmc_ext);
    // fmc_ext.value = 0x00;
    // Status = WriteFmcExt(&fmc_ext);
    return Status;
}

//***************************************************************************************
ULONG CBambpex::GetPower(PBRDextn_FMCPOWER pow)
{
    ULONG Status = BRDerr_OK;
    // if( (m_DeviceID == AMBPEX5_DEVID ||
    //	 m_DeviceID == FMC105P_DEVID ||
    //	 m_DeviceID == FMC106P_DEVID ||
    //	 m_DeviceID == FMC110P_DEVID ||
    //	 m_DeviceID == FMC114V_DEVID ||
    //	 m_DeviceID == FMC113E_DEVID ||
    //	 m_DeviceID == FMC108V_DEVID ||
    //	 m_DeviceID == FMC115CP_DEVID ||
    //	 m_DeviceID == FMC112CP_DEVID
    //	 ) && m_BlockFidAddr
    //	)
    if (m_BlockFidAddr) {
        PULONG BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
        // ULONG blk_id = BlockMem[0];
        // ULONG blk_ver = BlockMem[2];
        ULONG blk_v0 = BlockMem[4];
        // ULONG blk_v1 = BlockMem[6];
        // ULONG blk_v2 = BlockMem[8];
        // ULONG blk_v3 = BlockMem[10];

        ULONG PowerStat;
        PowerStat = BlockMem[32]; // POWER_STATUS
        ULONG PowerCtrl;
        PowerCtrl = BlockMem[22]; // POWER_CTRL
        PowerCtrl &= 0x8000; // mask POWER_CTRL[power_en]
        if (PowerCtrl)
            pow->onOff = 1;
        else
            pow->onOff = 0;
        pow->value = blk_v0;
        pow->slot = PowerStat;
    } else {
        pow->value = 0;
        pow->slot = 0;
        Status = BRDerr_INVALID_FUNCTION;
    }
    return Status;
}

//***************************************************************************************
ULONG CBambpex::PowerOnOff(PBRDextn_FMCPOWER pow, int force)
{
    ULONG Status = BRDerr_OK;
    // if( (m_DeviceID == AMBPEX5_DEVID ||
    //	 m_DeviceID == FMC105P_DEVID ||
    //	 m_DeviceID == FMC106P_DEVID ||
    //	 m_DeviceID == FMC110P_DEVID ||
    //	 m_DeviceID == FMC114V_DEVID ||
    //	 m_DeviceID == FMC113E_DEVID ||
    //	 m_DeviceID == FMC108V_DEVID ||
    //	 m_DeviceID == FMC115CP_DEVID ||
    //	 m_DeviceID == FMC112CP_DEVID
    //	 ) && m_BlockFidAddr
    //	)
    if (m_BlockFidAddr) { // программируем источник питания для FMC-субмодуля
        ULONG PowerStat;
        PULONG BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
        // ULONG blk_id = BlockMem[0];
        // ULONG blk_ver = BlockMem[2];
        ULONG blk_v0 = BlockMem[4];
        // ULONG blk_v1 = BlockMem[6];
        // ULONG blk_v2 = BlockMem[8];
        // ULONG blk_v3 = BlockMem[10];
        if (pow->onOff == 0) {
            BlockMem[22] = 0; // POWER_CTRL[power_en] = 0
            IPC_delay(100);
            PowerStat = BlockMem[32]; // POWER_STATUS
        } else {
            // ULONG PowerNum;
            // for(int i = 0; i < 4; i++)
            //{
            //	if(BlockMem[4 + (i<<1)])
            //		PowerNum = i;
            // }
            ULONG PowerStat0;
            PowerStat0 = BlockMem[32]; // POWER_STATUS
            PowerStat = PowerStat0;

            ULONG PowerCtrl;
            PowerCtrl = BlockMem[22]; // POWER_CTRL
            PowerStat0 = PowerCtrl & 0x8000; // mask POWER_CTRL[power_en]
            if (!PowerStat0 || force) {
                // ULONG PowerNum = 0;
                BlockMem[42] = 1; // SPD_DEVICE = 1
                for (int i = 0; i < 3; i++) {
                    BlockMem[24] = 0; // POWER_ROM[num] = 0
                    BlockMem[22] = 9; // POWER_CTRL[key] = 9
                    BlockMem[24] = 0x100; // POWER_ROM[start] = 1
                    IPC_delay(1);
                    for (int j = 0; j < 50; j++) {
                        IPC_delay(100);
                        PowerStat = BlockMem[32]; // POWER_STATUS
                        if (!(PowerStat & 0x10)) // POWER_RUN
                            break;
                    }
                    if (PowerStat & 1) // POWER_OK
                    {
                        BlockMem[22] = 0x8000; // POWER_CTRL[power_en] = 1
                        for (int j = 0; j < 50; j++) {
                            IPC_delay(100);
                            PowerStat = BlockMem[32]; // POWER_STATUS
                            if (PowerStat & 0x20) // POWER_GOOD
                                break;
                        }
                        if (PowerStat & 0x20) // POWER_GOOD
                        {
                            if (PowerStat & 0x1500) {
                                for (int j = 0; j < 30; j++) {
                                    IPC_delay(100);
                                    PowerStat = BlockMem[32]; // POWER_STATUS
                                    ULONG flag_on = (PowerStat & 0x1500) << 1; // FMCx_PRESENT -> FMCx_POWER_ON
                                    if ((PowerStat & 0x2A00) == flag_on)
                                        break;
                                }
                            }
                        }
                    }
                    if ((PowerStat & 2) || // POWER_ERROR
                        (!(PowerStat & 0x20)) // NO POWER_GOOD
                    ) {
                        BlockMem[22] = 0; // POWER_CTRL[key] = 0
                        IPC_delay(1);
                    } else
                        break;
                }
            }
        }
        pow->value = blk_v0;
        pow->slot = PowerStat;
    } else {
        pow->value = 0;
        pow->slot = 0;
        Status = BRDerr_INVALID_FUNCTION;
    }

    return Status;
}

// register addresses INA219
static const U32 CONFIGURATION_REGISTER_ADDR = 0x00;
static const U32 SHUNT_VOLTAGE_REGISTER_ADDR = 0x01;
static const U32 BUS_VOLTAGE_REGISTER_ADDR = 0x02;
static const U32 POWER_REGISTER_ADDR = 0x03;
static const U32 CURRENT_REGISTER_ADDR = 0x04;
static const U32 CALIBRATION_REGISTER_ADDR = 0x05;

// на модуле FMC107P установлено 2 микросхемы INA219 (Voltage, Current and Power)
static const U32 SENSOR_COUNT_FMC107P = 2;
// Номер устройства (INA219) на модуле FMC107P
static const U32 SENSOR_NUM_FMC107P = 0x10;
//  Адреса устройств (INA219) на шине I2C на модуле FMC107P
static const U32 SENSOR_ADR_FMC107P[] = { 0x40, 0x41 };

double g_Cur_LSB[SENSOR_COUNT_FMC107P];
//***************************************************************************************
ULONG CBambpex::InitSensorsFMC107()
{
    ULONG Status = BRDerr_INVALID_FUNCTION;

    const double R_SHUNT[] = { 0.004, 0.05 }; // резисторы
    double Cur_MAX[] = { 1000, 5000 }; // максимально ожидаемые токи
    double aPG_MAX[] = { 320, 320 }; // максимальное значение напряжения

    U32 i = 0;
    for (i = 0; i < SENSOR_COUNT_FMC107P; ++i) {
        // корректировка PG_MAX в сторону большего: 320, 160, 80, 40
        double PG_MAX = Cur_MAX[i] * R_SHUNT[i];
        if (PG_MAX <= 160)
            aPG_MAX[i] = 160;
        if (PG_MAX <= 80)
            aPG_MAX[i] = 80;
        if (PG_MAX <= 40)
            aPG_MAX[i] = 40;

        // коррекция g_Cur_MAX
        Cur_MAX[i] = aPG_MAX[i] / R_SHUNT[i];
    }

    INA219_CONFREG cfg_val[SENSOR_COUNT_FMC107P];
    for (i = 0; i < SENSOR_COUNT_FMC107P; i++) {
        cfg_val[i].EnBlock = 0xA5A5;
        Status = ReadI2C(SENSOR_NUM_FMC107P, SENSOR_ADR_FMC107P[i], CONFIGURATION_REGISTER_ADDR, &(cfg_val[i].EnBlock), 1);
        cfg_val[i].ByFields.BRNG = 0; // Bus Voltage Range = 16V
        if (aPG_MAX[i] == 40)
            cfg_val[i].ByFields.PG = 0x0;
        if (aPG_MAX[i] == 80)
            cfg_val[i].ByFields.PG = 0x1;
        if (aPG_MAX[i] == 160)
            cfg_val[i].ByFields.PG = 0x2;
        if (aPG_MAX[i] == 320)
            cfg_val[i].ByFields.PG = 0x3;
        Status = WriteI2C(SENSOR_NUM_FMC107P, SENSOR_ADR_FMC107P[i], CONFIGURATION_REGISTER_ADDR, cfg_val[i].EnBlock, 1);

        g_Cur_LSB[i] = Cur_MAX[i] / 1000 / 32768;
        U32 CLBR = (U32)(0.04096 / (g_Cur_LSB[i] * R_SHUNT[i]));
        Status = WriteI2C(SENSOR_NUM_FMC107P, SENSOR_ADR_FMC107P[i], CALIBRATION_REGISTER_ADDR, CLBR, 1);
    }
    return Status;
}

// Номер устройства (INA219 и LTC2991) на модуле FMC126P
// static const U32 DEVNUM_SENSOR_126P = 1; // 1 - использовалась в прошивке, но оказалось что там же и блокируется (так как ранее предназначался для управления питанием)
// Номер устройства (INA219 и LTC2991) на модуле FMC132P
static const U32 DEVNUM_SENSOR_132P = 4;
//  Адрес устройства INA219 на шине I2C на модуле FMC132P
static const U32 DEVADR_INA219_132P = 0x40;
//  Адрес устройства LTC2991 на шине I2C на модуле FMC132P
static const U32 DEVADR_LTC2991_132P = 0x48;

//***************************************************************************************
// INA219(Voltage, Current and Power) on FMC132P and FMC126P (also FMC133V and DSP134V)
ULONG CBambpex::InitSensorsFMC132(U32 devnum)
{
    ULONG Status = BRDerr_INVALID_FUNCTION;

    const double R_SHUNT = 0.05; // резистор
    double Cur_MAX = 5000; // максимально ожидаемый ток
    double aPG_MAX = 320; // максимальное значение напряжения

    // корректировка PG_MAX в сторону большего: 320, 160, 80, 40
    double PG_MAX = Cur_MAX * R_SHUNT;
    if (PG_MAX <= 160)
        aPG_MAX = 160;
    if (PG_MAX <= 80)
        aPG_MAX = 80;
    if (PG_MAX <= 40)
        aPG_MAX = 40;

    // коррекция g_Cur_MAX
    Cur_MAX = aPG_MAX / R_SHUNT;

    INA219_CONFREG cfg_val;

    cfg_val.EnBlock = 0xA5A5;
    Status = ReadI2C(devnum, DEVADR_INA219_132P, CONFIGURATION_REGISTER_ADDR, &(cfg_val.EnBlock), 1);
    cfg_val.ByFields.BRNG = 0; // Bus Voltage Range = 16V
    if (aPG_MAX == 40)
        cfg_val.ByFields.PG = 0x0;
    if (aPG_MAX == 80)
        cfg_val.ByFields.PG = 0x1;
    if (aPG_MAX == 160)
        cfg_val.ByFields.PG = 0x2;
    if (aPG_MAX == 320)
        cfg_val.ByFields.PG = 0x3;
    Status = WriteI2C(devnum, DEVADR_INA219_132P, CONFIGURATION_REGISTER_ADDR, cfg_val.EnBlock, 1);

    g_Cur_LSB[0] = Cur_MAX / 1000 / 32768;
    U32 CLBR = (U32)(0.04096 / (g_Cur_LSB[0] * R_SHUNT));
    Status = WriteI2C(devnum, DEVADR_INA219_132P, CALIBRATION_REGISTER_ADDR, CLBR, 1);

    return Status;
}

U32 g_FMC132ModeMonitor = 0xFF;
//***************************************************************************************
ULONG CBambpex::InitSensors()
{
    ULONG Status = BRDerr_INVALID_FUNCTION;

    if ((m_DeviceID == FMC107P_DEVID)
        && m_BlockFidAddr) {
        InitSensorsFMC107();
    }
    if ((m_DeviceID == FMC132P_DEVID || m_DeviceID == FMC126P_DEVID || m_DeviceID == FMC139P_DEVID || m_DeviceID == FMC133V_DEVID || m_DeviceID == DSP134V_DEVID)
        && m_BlockFidAddr) {
        InitSensorsFMC132(DEVNUM_SENSOR_132P);
        g_FMC132ModeMonitor = 0xFF;
    }
    return Status;
}

//***************************************************************************************
ULONG CBambpex::GetSensorsFMC132(PBRDextn_Fmc132PSensors pSensors, U32 dev_num)
{
    ULONG Status = BRDerr_OK;
    // на модуле установлена 1 микросхема INA219 (Voltage, Current and Power)
    {
        INA219_VOLTREG volt_val;
        volt_val.EnBlock = 0;
        Status = ReadI2C(dev_num, DEVADR_INA219_132P, BUS_VOLTAGE_REGISTER_ADDR, &volt_val.EnBlock, 1);

        U32 cur_val = 0;
        Status = ReadI2C(dev_num, DEVADR_INA219_132P, CURRENT_REGISTER_ADDR, &cur_val, 1);

        U32 pow_val = 0;
        Status = ReadI2C(dev_num, DEVADR_INA219_132P, POWER_REGISTER_ADDR, &pow_val, 1);

        pSensors->voltage = double(volt_val.ByFields.BVR) * 4 / 1000;

        pSensors->current = double(cur_val) * g_Cur_LSB[0];

        pSensors->power = double(pow_val) * (20 * g_Cur_LSB[0]);
    }

    // и 1 микросхема LTC2991 (Voltage, Temperature and fan)
    {
        U32 ctrl_val = 0x0000;
        U32 flten = 0;
        if (pSensors->mode) { // режим 1 - это режим с усреднением (Filter Enabled)
            flten = 1;
            ctrl_val |= 0x8888;
        }

        double _vchan[8];
        double _tempr;
        double _vcc;

        if (g_FMC132ModeMonitor != pSensors->mode) {
            g_FMC132ModeMonitor = pSensors->mode;
            Status = setModeSensors(dev_num, DEVADR_LTC2991_132P, flten, ctrl_val & 0xFF, ctrl_val >> 8);
        }

        Status = setPWM(dev_num, DEVADR_LTC2991_132P, pSensors->pwmEnbl, pSensors->pwmInv, pSensors->pwmThres);

        Status = readSensors(dev_num, DEVADR_LTC2991_132P, _vchan, &_tempr, &_vcc);

        for (int i = 0; i < 8; i++) {
            double out = _vchan[i];

            //			if (i == 0 || i == 1)
            //				out *= 6;

            pSensors->inpv[i] = out;
        }

        pSensors->tint = _tempr;
        pSensors->vcc = _vcc;
    }
    return Status;
}
//***************************************************************************************
ULONG CBambpex::GetSensors(void* parg)
{
    ULONG Status = BRDerr_INVALID_FUNCTION;
    if ((m_DeviceID == FMC107P_DEVID)
        && m_BlockFidAddr) {
        PBRDextn_Sensors pSensors = (PBRDextn_Sensors)parg;
        if (pSensors->chip > 1) // на модуле установлено 2 микросхемы INA219 (Voltage, Current and Power)
            return BRDerr_BAD_PARAMETER;

        U32 iChip = pSensors->chip;

        INA219_VOLTREG volt_val;
        volt_val.EnBlock = 0;
        Status = ReadI2C(SENSOR_NUM_FMC107P, SENSOR_ADR_FMC107P[iChip], BUS_VOLTAGE_REGISTER_ADDR, &volt_val.EnBlock, 1);

        U32 cur_val = 0;
        Status = ReadI2C(SENSOR_NUM_FMC107P, SENSOR_ADR_FMC107P[iChip], CURRENT_REGISTER_ADDR, &cur_val, 1);

        U32 pow_val = 0;
        Status = ReadI2C(SENSOR_NUM_FMC107P, SENSOR_ADR_FMC107P[iChip], POWER_REGISTER_ADDR, &pow_val, 1);

        pSensors->voltage = double(volt_val.ByFields.BVR) * 4 / 1000;

        pSensors->current = double(cur_val) * g_Cur_LSB[iChip];

        pSensors->power = double(pow_val) * (20 * g_Cur_LSB[iChip]);
    }

    if ((m_DeviceID == FMC132P_DEVID || m_DeviceID == FMC126P_DEVID || m_DeviceID == FMC139P_DEVID || m_DeviceID == FMC133V_DEVID || m_DeviceID == DSP134V_DEVID)
        && m_BlockFidAddr) {
        PBRDextn_Fmc132PSensors pSensors = (PBRDextn_Fmc132PSensors)parg;
        Status = GetSensorsFMC132(pSensors, DEVNUM_SENSOR_132P);
    }
    return Status;
}

//***************************************************************************************
ULONG CBambpex::setPWM(U32 dev_num, U32 dev_adr, U32 pwm_enbl, U32 pwm_inv, U32 pwm_thres)
{
    ULONG Status = BRDerr_OK;

    U32 ctrl_reg8 = 0x10 | (pwm_enbl << 5) | (pwm_inv << 6) | ((pwm_thres & 1) << 7);
    Status = WriteI2C(dev_num, dev_adr, 0x8, ctrl_reg8, 0); //  программируем регистр 8

    U32 pwm_reg9 = pwm_thres >> 1;
    Status = WriteI2C(dev_num, dev_adr, 0x9, pwm_reg9, 0); //  программируем регистр 9

    // U32 val = 0;
    // for (int i = 0; i < 16; i++)
    //{
    //	WriteI2C(DEVNUM_SENSOR_132P, dev_adr, 0x9, val, 0);	//  устанавливаем значение
    //	val += 0x10;
    // }

    // val = 0x78;
    // for (int i = 0; i < 16; i++)
    //{
    //	WriteI2C(DEVNUM_SENSOR_132P, dev_adr, 0x9, val, 0);	//  устанавливаем значение
    //	val++;
    // }

    // WriteI2C(DEVNUM_SENSOR_132P, dev_adr, 0x8, 0x30, 0);	//  устанавливаем режим Repeated Mode
    // val = 0x7F;
    // WriteI2C(DEVNUM_SENSOR_132P, dev_adr, 0x9, val, 0);	//  устанавливаем значение
    //
    // WriteI2C(DEVNUM_SENSOR_132P, dev_adr, 0x8, 0xB0, 0);	//  устанавливаем режим Repeated Mode
    // WriteI2C(DEVNUM_SENSOR_132P, dev_adr, 0x9, val, 0);	//  устанавливаем значение

    // WriteI2C(DEVNUM_SENSOR_132P, dev_adr, 0x8, 0x30, 0);	//  устанавливаем режим Repeated Mode
    // val = 0x80;
    // WriteI2C(DEVNUM_SENSOR_132P, dev_adr, 0x9, val, 0);	//  устанавливаем значение

    // val = 0;
    // WriteI2C(DEVNUM_SENSOR_132P, dev_adr, 0x9, val, 0);	//  устанавливаем значение
    //
    // for (int i = 0; i < 16; i++)
    //{
    //	WriteI2C(DEVNUM_SENSOR_132P, dev_adr, 0x9, val, 0);	//  устанавливаем значение
    //	val += 0x10;
    // }

    return Status;
}

//***************************************************************************************
ULONG CBambpex::setModeSensors(U32 dev_num, U32 dev_adr, U32 flt, U32 ctrl_reg6, U32 ctrl_reg7)
{
    ULONG Status = BRDerr_OK;

    Status = WriteI2C(dev_num, dev_adr, 0x6, ctrl_reg6, 0); // устанавливаем нужный режим для V1-V4
    IPC_delay(10);
    Status = WriteI2C(dev_num, dev_adr, 0x7, ctrl_reg7, 0); // устанавливаем нужный режим для V5-V8
    IPC_delay(10);
    Status = WriteI2C(dev_num, dev_adr, 0x8, 0x10, 0); //  устанавливаем режим Repeated Mode

    U32 stH = 0xF8;
    Status = WriteI2C(dev_num, dev_adr, 0x1, stH, 0); // разрешаем измерение всего (V1-V8, Vcc и Tinternal)

    if (flt) { // если включено усреднение, то требуется до 100 итераций, пока переходный процесс завершится
        U32 MSB, LSB;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 16; j++) {
                do {
                    Status = ReadI2C(dev_num, dev_adr, 0xA + i * 2, &MSB, 0);
                } while (!(MSB & 0x80));
                Status = ReadI2C(dev_num, dev_adr, 0xB + i * 2, &LSB, 0);
            }
        }
    }
    return Status;
}

//***************************************************************************************
ULONG CBambpex::readSensors(U32 dev_num, U32 dev_adr, double* v_chan, double* tempr, double* vcc)
{
    ULONG Status = BRDerr_OK;

    int icnt = 0;

    U32 MSB, LSB;

    //	ReadI2C(DEVNUM_SENSOR_132P, dev_adr, 0x1, &MSB);

    // Inputs V1-V8 (format D0-D13)
    for (int i = 0; i < 8; i++) {
        icnt = 0;
        do {
            IPC_delay(10);
            Status = ReadI2C(dev_num, dev_adr, 0xA + i * 2, &MSB, 0);
            icnt++;
        } while (!(MSB & 0x80) && (icnt < 10));
        Status = ReadI2C(dev_num, dev_adr, 0xB + i * 2, &LSB, 0);

        // BRDC_printf(_BRDC("Chan%d (0x%X, %d): 0x%02X%02X\n"), i , numb, icnt, MSB, LSB);

        S32 D = LSB | ((MSB & 0x3F) << 8);
        double V = (MSB & 0x40) ? (16384 - D) : D;

        v_chan[i] = (V * 0.30518) / 1000;
    }

    // temperature internal (format D0-D12)
    icnt = 0;
    do {
        IPC_delay(10);
        Status = ReadI2C(dev_num, dev_adr, 0x1A, &MSB, 0);
        icnt++;
    } while (!(MSB & 0x80) && (icnt < 10));
    Status = ReadI2C(dev_num, dev_adr, 0x1B, &LSB, 0);

    // BRDC_printf(_BRDC("Tin (0x%X, %d): 0x%02X%02X\n"), numb, icnt, MSB, LSB);

    MSB = MSB & 0x1F;
    MSB = (MSB << 27) >> 27;

    double Tin = (S32)(LSB | (MSB << 8));
    *tempr = Tin * 0.0625;

    // printf( "    Tin = %f \n", *tempr );

    // Vcc (format D0-D13)
    icnt = 0;
    do {
        IPC_delay(10);
        Status = ReadI2C(dev_num, dev_adr, 0x1C, &MSB, 0);
        icnt++;
    } while (!(MSB & 0x80) && (icnt < 10));
    Status = ReadI2C(dev_num, dev_adr, 0x1D, &LSB, 0);

    // BRDC_printf(_BRDC("Vcc (0x%X, %d): 0x%02X%02X\n"), numb, icnt, MSB, LSB);

    double V = LSB | ((MSB & 0x3F) << 8);
    V = (V * 0.30518) / 1000;
    *vcc = 2.5 + V;

    return Status;
}

ULONG g_SpdStatus;
//*****************************************************************
//  WaitSemaReady - wait semaphor flag or data ready flag (1)
//  Input:	SemaOrReady = 1 sema, 0 ready
//  Output:
//  Notes: при SemaOrReady = 1 ожидаем срабатывание флага семафора
//		   при SemaOrReady = 0 ожидаем готовности данных
//*****************************************************************
ULONG CBambpex::WaitSemaReady(ULONG* BlockMem, int SemaOrReady)
{
    ULONG SpdStatus;
    int stat = 0;
    int j = 0;
    for (j = 0; j < 1000; j++) {
        SpdStatus = BlockMem[44]; // SPD_CTRL
        if (SemaOrReady)
            stat = SpdStatus & 0x4000; // Sema
        else
            stat = SpdStatus & 0x8000; // Ready
        if (stat)
            break;
        IPC_pause(100);
        // Sleep(100);
    }
    g_SpdStatus = SpdStatus;
    // BRDC_printf(_BRDC("SpdStatus = 0x%08X\n"), SpdStatus);
    if (j >= 1000)
        return BRDerr_NOT_READY;

    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::ReadMainSpd(int devid, USHORT* buf, USHORT Offset, int Length)
{
    ULONG Status = BRDerr_OK;
    ULONG* BlockMem = m_pMemBuf[0];

    BlockMem[44] = 0; // SPD_CTRL
    ULONG spd_ctrl = 0x4000 + (devid << 8);
    BlockMem[44] = spd_ctrl; // SPD_CTRL: Sema = 1, Id = 0(sub EEPROM), Id = 1(base EEPROM)
    Status = WaitSemaReady(BlockMem, 1);
    // BRDCHAR buf_str[255];
    // BRDC_sprintf(buf_str, _BRDC("SpdStatus (wait sema) = 0x%08X\n"), g_SpdStatus);
    // MessageBox(NULL, buf_str, _BRDC("ReadMainSpd"), MB_OK);
    if (Status != BRDerr_OK)
        return BRDerr_NOT_READY;

    // if (WaitSemaReady(BlockMem, 1) != BRDerr_OK)
    // return BRDerr_NOT_READY;
    // Status = BRDerr_NOT_READY;
    // IPC_getch();
    // BlockMem[46] = Offset; // SPD_ADR
    // BlockMem[44] = spd_ctrl + 1; // SPD_CTRL, type operation - read
    // buf[0] = (USHORT)BlockMem[48]; // read data
    // if (WaitSemaReady(BlockMem, 1) != BRDerr_OK)
    //	Status = BRDerr_NOT_READY;

    // BRDC_sprintf(buf_str, _BRDC("SpdStatus2 = 0x%08X\n"), g_SpdStatus);
    // MessageBox(NULL, buf_str, _BRDC("ReadMainSpd"), MB_OK);

    for (int i = 0; i < Length; i++) {
        BlockMem[46] = Offset; // SPD_ADR
        BlockMem[44] = spd_ctrl + 1; // SPD_CTRL, type operation - read
        if (WaitSemaReady(BlockMem, 0) != BRDerr_OK) {
            buf[i] = (USHORT)BlockMem[48]; // read data
            Status = BRDerr_NOT_READY;
            break;
        }
        buf[i] = (USHORT)BlockMem[48]; // read data
        Offset++;
    }

    BlockMem[44] = 0; // SPD_CTRL

    return Status;
}

//***************************************************************************************
ULONG CBambpex::WaitFidReady(ULONG* BlockMem)
{
    ULONG SpdStatus;
    for (int j = 0; j < 1000; j++) {
        SpdStatus = BlockMem[44]; // SPD_CTRL
        if (SpdStatus & 0x8000) // Ready
            break;
        IPC_pause(10);
    }
    if (!(SpdStatus & 0x8000)) // Ready
        return BRDerr_NOT_READY;

    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::ReadFidSpd(int devnum, int devadr, UCHAR* buf, USHORT Offset, int Length)
{
    ULONG Status = BRDerr_OK;
    ULONG* BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
    // ULONG blk_id = BlockMem[0];
    // ULONG blk_ver = BlockMem[2];

    BlockMem[42] = devnum; // SPD_DEVICE
    BlockMem[44] = 0; // SPD_CTRL
    IPC_pause(1000);
    if (WaitFidReady(BlockMem) != BRDerr_OK)
        return BRDerr_NOT_READY;

    for (int i = 0; i < Length; i++) {
        int sector = Offset / 256;
        BlockMem[46] = Offset; // SPD_ADR
        BlockMem[44] = ((devadr + sector) << 4) + 1; // SPD_CTRL, type operation - read
        if (WaitFidReady(BlockMem) != BRDerr_OK) {
            buf[i] = (UCHAR)BlockMem[48]; // read data
            Status = BRDerr_NOT_READY;
            break;
        }
        buf[i] = (UCHAR)BlockMem[48]; // read data
        Offset++;
    }

    BlockMem[44] = 0; // SPD_CTRL

    return Status;
}

//***************************************************************************************
ULONG CBambpex::WriteFidSpd(int devnum, int devadr, UCHAR* buf, USHORT Offset, int Length)
{
    ULONG Status = BRDerr_OK;
    ULONG* BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
    // ULONG blk_id = BlockMem[0];
    // ULONG blk_ver = BlockMem[2];

    BlockMem[42] = devnum; // SPD_DEVICE
    BlockMem[44] = 0; // SPD_CTRL
    IPC_pause(1000);
    if (WaitFidReady(BlockMem) != BRDerr_OK)
        return BRDerr_NOT_READY;

    for (int i = 0; i < Length; i++) {
        int sector = Offset / 256;
        BlockMem[46] = Offset; // SPD_ADR
        BlockMem[48] = buf[i]; // write data
        BlockMem[44] = ((devadr + sector) << 4) + 2; // SPD_CTRL, type operation - write
        IPC_pause(10000);
        if (WaitFidReady(BlockMem) != BRDerr_OK)
            return BRDerr_NOT_READY;
        Offset++;
    }

    BlockMem[44] = 0; // SPD_CTRL

    return Status;
}

//***************************************************************************************
ULONG CBambpex::ReadFmcExt(PBRDextn_FMCEXT ext)
{
    ULONG Status = BRDerr_OK;
    if (!m_BlockFidAddr)
        Status = BRDerr_FUNC_UNIMPLEMENTED;

    ULONG* BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
    // ULONG blk_id = BlockMem[0];
    // ULONG blk_ver = BlockMem[2];

    BlockMem[42] = 0; // SPD_DEVICE
    BlockMem[44] = 0; // SPD_CTRL
    // IPC_delay(1);
    IPC_pause(1000);
    if (WaitFidReady(BlockMem) != BRDerr_OK)
        return BRDerr_NOT_READY;

    BlockMem[46] = ext->addr; // SPD_ADR
    BlockMem[44] = (ext->dev << 4) | 0x2001; // SPD_CTRL, type operation - read
    if (WaitFidReady(BlockMem) != BRDerr_OK)
        Status = BRDerr_NOT_READY;
    ext->value = (USHORT)BlockMem[48]; // read data

    BlockMem[44] = 0; // SPD_CTRL

    return Status;
}

//***************************************************************************************
ULONG CBambpex::WriteFmcExt(PBRDextn_FMCEXT ext)
{
    ULONG Status = BRDerr_OK;
    if (!m_BlockFidAddr)
        Status = BRDerr_FUNC_UNIMPLEMENTED;

    ULONG* BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
    // ULONG blk_id = BlockMem[0];
    // ULONG blk_ver = BlockMem[2];

    BlockMem[42] = 0; // SPD_DEVICE
    BlockMem[44] = 0; // SPD_CTRL
    IPC_pause(1000);
    if (WaitFidReady(BlockMem) != BRDerr_OK)
        return BRDerr_NOT_READY;

    BlockMem[46] = ext->addr; // SPD_ADR
    BlockMem[48] = ext->value; // write data
    BlockMem[44] = (ext->dev << 4) + 0x2002; // SPD_CTRL, type operation - write
    IPC_pause(10000);
    if (WaitFidReady(BlockMem) != BRDerr_OK)
        Status = BRDerr_NOT_READY;

    BlockMem[44] = 0; // SPD_CTRL

    return Status;
}

//***************************************************************************************
ULONG CBambpex::ReadI2C(U32 devnum, U32 devadr, U32 regadr, U32* pVal, int dblbyte)
{
    ULONG Status = BRDerr_OK;
    if (!m_BlockFidAddr)
        Status = BRDerr_FUNC_UNIMPLEMENTED;

    ULONG* BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
    // ULONG blk_id = BlockMem[0];
    // ULONG blk_ver = BlockMem[2];

    BlockMem[42] = devnum; // SPD_DEVICE
    BlockMem[44] = 0; // SPD_CTRL
                      // IPC_delay(1);
    IPC_pause(1000);
    if (WaitFidReady(BlockMem) != BRDerr_OK)
        return BRDerr_NOT_READY;

    BlockMem[46] = regadr; // SPD_ADR
    if (dblbyte)
        BlockMem[44] = (devadr << 4) | 0x2001; // SPD_CTRL, type operation - read // for INA219
    else
        BlockMem[44] = (devadr << 4) | 1; // SPD_CTRL, type operation - read // for LTC2991
    if (WaitFidReady(BlockMem) != BRDerr_OK)
        Status = BRDerr_NOT_READY;
    *pVal = (USHORT)BlockMem[48]; // read data

    BlockMem[44] = 0; // SPD_CTRL

    return Status;
}

//***************************************************************************************
ULONG CBambpex::WriteI2C(U32 devnum, U32 devadr, U32 regadr, U32 val, int dblbyte)
{
    ULONG Status = BRDerr_OK;
    if (!m_BlockFidAddr)
        Status = BRDerr_FUNC_UNIMPLEMENTED;

    ULONG* BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
    // ULONG blk_id = BlockMem[0];
    // ULONG blk_ver = BlockMem[2];

    BlockMem[42] = devnum; // SPD_DEVICE
    BlockMem[44] = 0; // SPD_CTRL
    IPC_pause(1000);
    if (WaitFidReady(BlockMem) != BRDerr_OK)
        return BRDerr_NOT_READY;

    BlockMem[46] = regadr; // SPD_ADR
    BlockMem[48] = val; // write data
    if (dblbyte)
        BlockMem[44] = (devadr << 4) + 0x2002; // SPD_CTRL, type operation - write // for INA219
    else
        BlockMem[44] = (devadr << 4) + 2; // SPD_CTRL, type operation - write // for LTC2991
    IPC_pause(10000);
    if (WaitFidReady(BlockMem) != BRDerr_OK)
        Status = BRDerr_NOT_READY;

    BlockMem[44] = 0; // SPD_CTRL

    return Status;
}

//***************************************************************************************
ULONG CBambpex::GetIrqMinTime(ULONG* pTimeVal)
{
    ULONG* BlockMem = m_pMemBuf[0];

    ULONG reg_val = BlockMem[36];
    if (((reg_val & 0xFF00) >> 8) == 0xB5) {
        *pTimeVal = reg_val & 0xf;
        return BRDerr_OK;
    }
    *pTimeVal = reg_val;
    return BRDerr_INVALID_FUNCTION;
}

//***************************************************************************************
ULONG CBambpex::SetIrqMinTime(ULONG* pTimeVal)
{
    ULONG* BlockMem = m_pMemBuf[0];

    ULONG reg_val = BlockMem[36];
    if (((reg_val & 0xFF00) >> 8) == 0xB5) {
        BlockMem[36] = *pTimeVal;
        *pTimeVal = BlockMem[36] & 0xf;
        return BRDerr_OK;
    }
    return BRDerr_INVALID_FUNCTION;
}

//***************************************************************************************
void CBambpex::GetPldCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAdmPld pCfgPld)
{
    PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
    int end_flag = 0;
    do {
        USHORT sign = *((USHORT*)pCfgMem);
        USHORT size = 0;
        if (sign == PLD_CFG_TAG) {
            PICR_CfgAdmPld pPldCfg = (PICR_CfgAdmPld)pCfgMem;
            memcpy(pCfgPld, pPldCfg, sizeof(ICR_CfgAdmPld));
            size += sizeof(ICR_CfgAdmPld);
            end_flag = 1;
        } else {
            size = *((USHORT*)pCfgMem + 1);
            size += 4;
            //			break;
        }
        pCfgMem = (PUCHAR)pCfgMem + size;
    } while (!end_flag && pCfgMem < pEndCfgMem);
}

//***************************************************************************************
ULONG CBambpex::GetPldDescription(BRDCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
    if (BaseEEPROMSize) {
        ICR_CfgAdmPld cfgPld;
        GetPldCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &cfgPld);
        m_pldType = cfgPld.bType;
        m_pldSpeedGrade = cfgPld.bSpeedGrade;
        BRDC_sprintf(Description, _BRDC("%d (%d %d %d)"), cfgPld.bType, cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
        switch (cfgPld.bType) {
        // case 3:
        //	sprintf_s(Description, MAX_DESCRIPLEN, _BRDC("XCVE%d-FF%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
        //	break;
        // case 4:
        //	sprintf_s(Description, MAX_DESCRIPLEN, _BRDC("XC2V%d-FF%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
        //	break;
        // case 5:
        //	sprintf_s(Description, MAX_DESCRIPLEN, _BRDC("XC2S%d-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
        //	break;
        // case 6:
        //	sprintf_s(Description, MAX_DESCRIPLEN, _BRDC("XC2S%dE-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
        //	break;
        // case 7:
        //	sprintf_s(Description, MAX_DESCRIPLEN, _BRDC("XC3S%d-FG%d-%dC"), cfgPld.wVolume, cfgPld.wPins, cfgPld.bSpeedGrade);
        //	break;
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
            if (m_DeviceID == FMC126P_DEVID)
                BRDC_sprintf(Description, _BRDC("XCKU%d-%dFFVA%dE"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            else
                BRDC_sprintf(Description, _BRDC("XCKU%d-%dFLVB%dE"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            break;
        case 26:
            if (m_DeviceID == FMC139P_DEVID)
                BRDC_sprintf(Description, _BRDC("XCKU%dP-%dFFVE%dE"), cfgPld.wVolume, cfgPld.bSpeedGrade, cfgPld.wPins);
            else
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

//***************************************************************************************
ULONG CBambpex::GetDspPldDescription(BRDCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
    if (BaseEEPROMSize) {
        ICR_CfgDspNode cfgDspNode;
        GetDspNodeCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &cfgDspNode);
        BRDC_sprintf(Description, _BRDC("%d (%d %d %d)"), cfgDspNode.bPldType, cfgDspNode.wPldVolume, cfgDspNode.bPldSpeedGrade, cfgDspNode.wPldPins);
        switch (cfgDspNode.bPldType) {
        case 21:
            BRDC_sprintf(Description, _BRDC("XC6VSX%dT-%dFFG%d"), cfgDspNode.wPldVolume, cfgDspNode.bPldSpeedGrade, cfgDspNode.wPldPins);
            break;
        case 22:
            BRDC_sprintf(Description, _BRDC("XC6VLX%dT-%dFFG%d"), cfgDspNode.wPldVolume, cfgDspNode.bPldSpeedGrade, cfgDspNode.wPldPins);
            break;
        }
        return 1;
    }
    return 0;
}

//***************************************************************************************
ULONG CBambpex::GetDspPld(PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize, UCHAR& DspPldCnt)
{
    if (BaseEEPROMSize) {
        if (m_DeviceID == FMC106P_DEVID) {
            ICR_CfgFmc105p cfgAmbp;
            GetFmcCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &cfgAmbp);
            DspPldCnt = cfgAmbp.bDspNodeCfgCnt;
            // if(m_FdspTetrNum == -1)
            //	DspPldCnt = 0;
        } else
            DspPldCnt = 0;
        return 1;
    } else
        DspPldCnt = 0;
    return 0;
}

//***************************************************************************************
ULONG CBambpex::SetPuInfo(PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
    if (m_DeviceID == DSP134V_DEVID) {
        m_puCnt = 0;
        return BRDerr_OK;
    }

    m_puCnt = 1;
    m_AdmPldInfo.Id = BRDpui_PLD; // 0x100;
    m_AdmPldInfo.Type = PLD_CFG_TAG;
    if (AMBPEX8_DEVID == m_DeviceID)
        m_AdmPldInfo.Attr = BRDpua_Load;
    else
        m_AdmPldInfo.Attr = 0;
    BRDCHAR Description0[MAX_DESCRIPLEN] = _BRDC("ADM PLD ");
    BRDCHAR Description[MAX_DESCRIPLEN] = _BRDC("ADM PLD XILINX"); // Programmable Unit Description Text
    if (GetPldDescription(Description, pBaseEEPROMMem, BaseEEPROMSize)) {
        BRDC_strcpy(m_AdmPldInfo.Description, Description0);
        BRDC_strcat(m_AdmPldInfo.Description, Description);
    } else
        BRDC_strcpy(m_AdmPldInfo.Description, Description);

    m_PuInfos.push_back(m_AdmPldInfo);

    UCHAR DspPldCnt = 0;
    GetDspPld(pBaseEEPROMMem, BaseEEPROMSize, DspPldCnt);
    if (DspPldCnt) {
        m_puCnt++;
        m_DspPldInfo.Id = BRDpui_PLD + 1;
        m_DspPldInfo.Type = PLD_CFG_TAG;
        m_DspPldInfo.Attr = BRDpua_Load;
        BRDCHAR Description0[MAX_DESCRIPLEN] = _BRDC("DSP PLD ");
        BRDCHAR Description[MAX_DESCRIPLEN] = _BRDC("DSP PLD XILINX"); // Programmable Unit Description Text
        if (GetDspPldDescription(Description, pBaseEEPROMMem, BaseEEPROMSize)) {
            BRDC_strcpy(m_DspPldInfo.Description, Description0);
            BRDC_strcat(m_DspPldInfo.Description, Description);
        } else
            BRDC_strcpy(m_DspPldInfo.Description, Description);

        m_PuInfos.push_back(m_DspPldInfo);
    }

    if (1) {
        m_puCnt++;
        m_BaseIcrInfo.Id = BRDpui_BASEICR;
        m_BaseIcrInfo.Type = BASE_ID_TAG;
        m_BaseIcrInfo.Attr = BRDpua_Read | BRDpua_Write | BRDpua_Danger;
        //		ret = GetStringText(IDS_STRINGBASEICR, m_BaseIcrInfo.Description);
        //		ret = GetStringText(MSG_BASE_ICR, m_BaseIcrInfo.Description);
        BRDC_strcpy(m_BaseIcrInfo.Description, _BRDC("ID & CFG EEPROM on Base module"));

        m_PuInfos.push_back(m_BaseIcrInfo);
    }

    int subm[2] = { 1, 0 }; // субмодуль FMC1 присутствует, а субмодуль FMC2 отсутствует

    if (m_BlockFidAddr) { // FMC субмодули
        PULONG BlockMem = m_pMemBuf[0];
        ULONG pldver = BlockMem[8];
        ULONG coreid = BlockMem[14];
        if (coreid != 0x22 || (coreid == 0x22 && pldver > 0x104)) {
            BlockMem = m_pMemBuf[0] + m_BlockFidAddr;
            ULONG PowerStat = BlockMem[32]; // POWER_STATUS
            if (!(PowerStat & 0x100)) {
                subm[0] = 0; // субмодуль FM0 отсутствует
                PUSHORT pSubCfgMem = (PUSHORT)(m_Subunit[0].pSubEEPROM); // пытаемся исправить ошибки на субмодуле (в частности, FM416x250M)
                if (pSubCfgMem) {
                    if (pSubCfgMem[0] == 0x0080) // определяем наличие прописанного ППЗУ на субмодуле
                        subm[0] = 1; // субмодуль FM0 все же присутствует
                }
            }
            if (PowerStat & 0x400)
                subm[1] = 1; // субмодуль FM1 присутствует
        }
    }

    for (int i = 0; i < m_AdmIfCnt; i++) {
        if (!subm[i])
            continue; // NO submodule
        PUSHORT pSubCfgMem = (PUSHORT)(m_Subunit[i].pSubEEPROM);
        if (!pSubCfgMem)
            break; // NO submodule EEPROM

        if ((pSubCfgMem[0] != 0xA55A) && (pSubCfgMem[1] != 0x5AA5)) // определяет просто наличие ППЗУ на субмодуле
        //	if(( pSubCfgMem[0] != 0) || (pSubCfgMem[1] != 0))
        {
            m_puCnt++;
            m_AdmIcrInfo.Id = BRDpui_SUBICR + i;
            m_AdmIcrInfo.Type = ADM_ID_TAG;
            m_AdmIcrInfo.Attr = BRDpua_Read | BRDpua_Write | BRDpua_Danger;
            if (m_DeviceID == FMC112CP_DEVID || m_DeviceID == FMC127P_DEVID || m_DeviceID == FMC114V_DEVID) {
                BRDC_strcpy(m_AdmIcrInfo.Description, _BRDC("ICR on FMC1"));
                if (i)
                    BRDC_strcpy(m_AdmIcrInfo.Description, _BRDC("ICR on FMC2"));
            } else
                BRDC_strcpy(m_AdmIcrInfo.Description, _BRDC("ID & CFG EEPROM on subunit"));
            m_PuInfos.push_back(m_AdmIcrInfo);
        }

        if (m_BlockFidAddr) {
            m_puCnt++;
            m_AdmFruInfo.Id = BRDpui_FRUID + i;
            m_AdmFruInfo.Type = 0;
            m_AdmFruInfo.Attr = BRDpua_Read | BRDpua_Write | BRDpua_Danger;
            BRDC_strcpy(m_AdmFruInfo.Description, _BRDC("FRU EEPROM on FMC1"));
            if (i)
                BRDC_strcpy(m_AdmFruInfo.Description, _BRDC("FRU EEPROM on FMC2"));
            m_PuInfos.push_back(m_AdmFruInfo);
        }
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

    if (ICAP_SIG == m_IcapSig) {
        // m_puCnt += 8; // BRDpui_PLD : 0x120-0x127
        m_puCnt++;
        m_PartPldInfo.Id = BRDpui_PLD + 0x20;
        m_PartPldInfo.Type = PLD_CFG_TAG;
        m_PartPldInfo.Attr = BRDpua_Load;
        BRDC_strcpy(m_PartPldInfo.Description, _BRDC("Reloaded PLD Area"));
        m_PuInfos.push_back(m_PartPldInfo);
    }

    return BRDerr_OK;
}

//***************************************************************************************
void CBambpex::GetPuList(BRD_PuList* pu_list) const
{
    int pu_num = (int)m_PuInfos.size();
    for (int i = 0; i < pu_num; i++) {
        pu_list[i].puId = m_PuInfos[i].Id;
        pu_list[i].puCode = m_PuInfos[i].Type;
        pu_list[i].puAttr = m_PuInfos[i].Attr;
        BRDC_strcpy((BRDCHAR*)pu_list[i].puDescription, m_PuInfos[i].Description);
    }
}

//***************************************************************************************
// ULONG CBambpex::PuFileLoad(ULONG id, PCTSTR fileName, PUINT pState)
ULONG CBambpex::PuFileLoad(ULONG id, const BRDCHAR* fileName, PUINT pState)
{
    *pState = 0;
    if (BRDC_strlen(fileName) == 0) {
        //		return BRDerr_BAD_FILE;
        return ErrorPrintf(BRDerr_BAD_FILE,
            CURRFILE,
            _BRDC("<PuFileLoad> Bad File Name = %s"), fileName);
    }
    if ((id >= m_PartPldInfo.Id && id < m_PartPldInfo.Id + 32) && (ICAP_SIG == m_IcapSig)) {
        ULONG status = LoadPldFile(fileName, id - BRDpui_PLD);
        if (status != BRDerr_OK)
            return status;

        ULONG pld_status;
        if (GetPldStatus(pld_status, id - BRDpui_PLD) != BRDerr_OK)
            //			return BRDerr_HW_ERROR;
            return ErrorPrint(BRDerr_HW_ERROR,
                CURRFILE,
                _BRDC("<PuFileLoad> Hardware error"));
        *pState = pld_status;
        return BRDerr_OK;
    }
    // BRDC_printf(_BRDC("DevID = 0x%X, DSP PLD ID = %d (%d), FdspTetrNum = %d\n"), m_DeviceID, id, m_DspPldInfo.Id, m_FdspTetrNum);
    //  Check ADM or DSP PLD
    if (id == m_AdmPldInfo.Id || (m_FdspTetrNum != -1 && id == m_DspPldInfo.Id && FMC106P_DEVID == m_DeviceID))
    //	if(id == m_AdmPldInfo.Id)
    {
        if (ADP201X1AMB_DEVID == m_DeviceID || ADP201X1DSP_DEVID == m_DeviceID)
            return ErrorPrintf(BRDerr_INVALID_FUNCTION,
                CURRFILE,
                _BRDC("<PuFileLoad> It is not Programable Unit on this device (%X)"), m_DeviceID);

        ULONG status = LoadPldFile(fileName, id - BRDpui_PLD);
        if (status != BRDerr_OK)
            return status;

        ULONG pld_status;
        if (GetPldStatus(pld_status, id - BRDpui_PLD) != BRDerr_OK)
            //			return BRDerr_HW_ERROR;
            return ErrorPrint(BRDerr_HW_ERROR,
                CURRFILE,
                _BRDC("<PuFileLoad> Hardware error"));
        *pState = pld_status;
    } else {
        //	    return BRDerr_BAD_ID;
        return ErrorPrintf(BRDerr_BAD_ID,
            CURRFILE,
            _BRDC("<PuFileLoad> Bad Programable Unit ID %d"), id);
    }
    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::GetPuState(ULONG id, PUINT pState)
{
    // Check Partial reloaded PLD
    if ((id >= m_PartPldInfo.Id && id < m_PartPldInfo.Id + 32) && (ICAP_SIG == m_IcapSig)) {
        ULONG pld_status;
        if (GetPldStatus(pld_status, id - BRDpui_PLD) != BRDerr_OK)
            //			return BRDerr_HW_ERROR;
            return ErrorPrint(BRDerr_HW_ERROR,
                CURRFILE,
                _BRDC("<GetPuState> Hardware error"));
        *pState = pld_status;
        return BRDerr_OK;
    }
    // Check ADM PLD
    if (id == m_AdmPldInfo.Id) {
        ULONG pld_status;
        if (GetPldStatus(pld_status, id - BRDpui_PLD) != BRDerr_OK)
            //			return BRDerr_HW_ERROR;
            return ErrorPrint(BRDerr_HW_ERROR,
                CURRFILE,
                _BRDC("<GetPuState> Hardware error"));
        *pState = pld_status;
    } else {
        if (id == m_DspPldInfo.Id) {
            ULONG pld_status;
            if (m_FdspTetrNum != -1) {
                if (GetPldStatus(pld_status, id - BRDpui_PLD) != BRDerr_OK)
                    //			return BRDerr_HW_ERROR;
                    return ErrorPrint(BRDerr_HW_ERROR,
                        CURRFILE,
                        _BRDC("<GetPuState> Hardware error"));
            } else
                pld_status = 0;
            *pState = pld_status;
        } else {
            // Check base module ICR
            if (id == m_BaseIcrInfo.Id) {
                ULONG base_status;
                if (GetBaseIcrStatus(base_status) != BRDerr_OK)
                    //				return BRDerr_HW_ERROR;
                    return ErrorPrint(BRDerr_HW_ERROR,
                        CURRFILE,
                        _BRDC("<GetPuState> Hardware error"));
                *pState = base_status;
            } else {
                // Check subunit ICR
                if (id == m_AdmIcrInfo.Id) {
                    ULONG adm_status;
                    if (GetAdmIcrStatus(adm_status, m_AdmIcrInfo.Id - BRDpui_SUBICR) != BRDerr_OK)
                        //					return BRDerr_HW_ERROR;
                        return ErrorPrint(BRDerr_HW_ERROR,
                            CURRFILE,
                            _BRDC("<GetPuState> Hardware error"));
                    *pState = adm_status;
                } else {
                    // Check subunit FRU EEPROM
                    if (id == m_AdmFruInfo.Id) {
                        *pState = 0;
                    } else {
                        //				return BRDerr_BAD_ID;
                        return ErrorPrintf(BRDerr_BAD_ID,
                            CURRFILE,
                            _BRDC("<GetPuState> Bad Programable Unit ID %d"), id);
                    }
                }
            }
        }
    }
    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::GetPid()
{
    // if( m_DeviceID == AMBPEX8_DEVID ||
    //	m_DeviceID == ADP201X1AMB_DEVID ||
    //	m_DeviceID == ADP201X1DSP_DEVID ||
    //	m_DeviceID == AMBPEX5_DEVID ||
    //	m_DeviceID == FMC105P_DEVID ||
    //	m_DeviceID == FMC106P_DEVID ||
    //     m_DeviceID == FMC110P_DEVID ||
    //	m_DeviceID == FMC103E_DEVID ||
    //	m_DeviceID == FMC114V_DEVID ||
    //	m_DeviceID == FMC113E_DEVID ||
    //	m_DeviceID == FMC108V_DEVID ||
    //	m_DeviceID == FMC115CP_DEVID ||
    //	m_DeviceID == FMC112CP_DEVID
    //   )
    //	if(1)
    {
        if (m_sizeICR) {
            ULONG BufferSize = 4;
            ULONG Offset = 128 + 6;
            // ULONG Offset = 6;
            // if(ReadNvRAM(&m_PID, BufferSize, Offset) != BRDerr_OK)
            if (GetNvRAM(&m_PID, BufferSize, Offset) != BRDerr_OK) {
                //			return BRDerr_BAD_DEVICE_VITAL_DATA;
                return ErrorPrint(BRDerr_BAD_DEVICE_VITAL_DATA,
                    CURRFILE,
                    _BRDC("<GetPid> Undefined vital data into ICR"));
            }
        } else
            m_PID = 0;
    }
    //	else
    //		m_PID = 0;
    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::ReadPldFile(const BRDCHAR* PldFileName, UCHAR*& fileBuffer, ULONG& fileSize)
{
    BRDCHAR FullFileName[MAX_PATH];
    IPC_getFullPath(PldFileName, FullFileName);
    IPC_handle hFile = IPC_openFile(FullFileName, IPC_OPEN_FILE | IPC_FILE_RDONLY);
    if (!hFile) {
        return ErrorPrintf(BRDerr_BAD_FILE,
            CURRFILE,
            _BRDC("<LoadPldFile>  Can't open PLD file '%s'"), FullFileName);
    }
    long long file_size;
    if (IPC_getFileSize(hFile, &file_size) != IPC_OK) {
        return ErrorPrintf(BRDerr_BAD_FILE,
            CURRFILE,
            _BRDC("<LoadPldFile>  Can't open PLD file '%s'"), FullFileName);
    }
    fileSize = (ULONG)file_size;
    // fileBuffer = VirtualAlloc(NULL, fileSize, MEM_COMMIT, PAGE_READWRITE);
    fileBuffer = new UCHAR[fileSize];
    if (fileBuffer == NULL) {
        return ErrorPrint(BRDerr_NOT_ENOUGH_MEMORY,
            CURRFILE,
            _BRDC("<LoadPldFile> Not Enough memory for file buffer"));
    }
    int dwNumBytesRead = IPC_readFile(hFile, fileBuffer, fileSize);
    if (dwNumBytesRead < 0) {
        // VirtualFree(fileBuffer, 0, MEM_RELEASE);
        delete fileBuffer;
        return ErrorPrintf(BRDerr_BAD_FILE,
            CURRFILE,
            _BRDC("<LoadPldFile>  Read error PLD file '%s'"), FullFileName);
    }
    if (IPC_closeFile(hFile) != IPC_OK) {
        // VirtualFree(fileBuffer, 0, MEM_RELEASE);
        delete fileBuffer;
        return ErrorPrintf(BRDerr_BAD_FILE,
            CURRFILE,
            _BRDC("<LoadPldFile>  Close error PLD file '%s'"), FullFileName);
    }
    return BRDerr_OK;
}
/*
ULONG CBambpex::ReadPldFile(const BRDCHAR* PldFileName, void*& fileBuffer, ULONG& fileSize)
{
//	BRDCHAR *FirstChar;
        BRDCHAR FullFileName[MAX_PATH];
        GetFullPathName(PldFileName, MAX_PATH, FullFileName, &FirstChar);
        HANDLE hFile = CreateFile(FullFileName,
                                                                GENERIC_READ,
                                                                FILE_SHARE_READ, NULL,
                                                                OPEN_EXISTING,
                                                                FILE_ATTRIBUTE_NORMAL,
                                                                NULL);
    if(hFile == INVALID_HANDLE_VALUE)
        {
//	    return BRDerr_BAD_FILE;
                return ErrorPrintf( BRDerr_BAD_FILE,
                                                        CURRFILE,
                                                        _BRDC("<LoadPldFile>  Can't open PLD file '%s'"), FullFileName);
        }
        fileSize = SetFilePointer(hFile, 0, NULL, FILE_END);
        if(fileSize == 0xffffffff)
        {
//	    return BRDerr_BAD_FILE;
                return ErrorPrintf( BRDerr_BAD_FILE,
                                                        CURRFILE,
                                                        _BRDC("<LoadPldFile>  Can't open PLD file '%s'"), FullFileName);
        }
        SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
        fileBuffer = VirtualAlloc(NULL, fileSize, MEM_COMMIT, PAGE_READWRITE);
        if(fileBuffer == NULL)
        {
//		return BRDerr_NOT_ENOUGH_MEMORY;
                return ErrorPrint(  BRDerr_NOT_ENOUGH_MEMORY,
                                                        CURRFILE,
                                                        _BRDC("<LoadPldFile> Not Enough memory for file buffer"));
        }
        DWORD dwNumBytesRead;
        if(ReadFile(hFile, fileBuffer, fileSize, &dwNumBytesRead, NULL) == 0)
        {
                VirtualFree(fileBuffer, 0, MEM_RELEASE);
//	    return BRDerr_BAD_FILE;
                return ErrorPrintf( BRDerr_BAD_FILE,
                                                        CURRFILE,
                                                        _BRDC("<LoadPldFile>  Read error PLD file '%s'"), FullFileName);
        }
        if(CloseHandle(hFile) == 0)
        {
                VirtualFree(fileBuffer, 0, MEM_RELEASE);
//	    return BRDerr_BAD_FILE;
                return ErrorPrintf( BRDerr_BAD_FILE,
                                                        CURRFILE,
                                                        _BRDC("<LoadPldFile>  Close error PLD file '%s'"), FullFileName);
        }
        return BRDerr_OK;
}
*/
//***************************************************************************************
ULONG CBambpex::CheckPldFile(const BRDCHAR* PldFileName)
{
    // BRDCHAR *FirsBRDCHAR;
    BRDCHAR FullFileName[MAX_PATH];
    // GetFullPathName(PldFileName, MAX_PATH, FullFileName, &FirsBRDCHAR);
    IPC_getFullPath(PldFileName, FullFileName);
    // HANDLE hFile = CreateFile(FullFileName,
    //							GENERIC_READ,
    //							FILE_SHARE_READ, NULL,
    //							OPEN_EXISTING,
    //							FILE_ATTRIBUTE_NORMAL,
    //							NULL);
    IPC_handle hFile = IPC_openFile(FullFileName, IPC_OPEN_FILE | IPC_FILE_RDONLY);
    // if(hFile == INVALID_HANDLE_VALUE)
    if (!hFile) {
        //	    return BRDerr_BAD_FILE;
        return ErrorPrintf(BRDerr_BAD_FILE,
            CURRFILE,
            _BRDC("<LoadPldFile>  Can't open PLD file '%s'"), FullFileName);
    }
    // CloseHandle(hFile);
    IPC_closeFile(hFile);
    return BRDerr_OK;
}

//***************************************************************************************
void* CBambpex::SearchInfoStart(UCHAR* fileBuffer, ULONG& fileSize)
{
    UCHAR* buf = fileBuffer;
    ULONG cnt = fileSize / 4;
    for (ULONG i = 0; i < cnt; i++) {
        ULONG uval = *(ULONG*)(buf + i);
        if (uval == 0xFFFFFFFF) {
            i += 4;
            fileSize -= i;
            return &(buf[i]);
        }
    }
    return NULL;
}
// void* CBambpex::SearchInfoStart(void* fileBuffer, ULONG& fileSize)
//{
//	UCHAR val = 0;
//	UCHAR* buf = (UCHAR*)fileBuffer;
//	for(ULONG i = 0; i < fileSize; i++)
//	{
//		if(buf[i] == 0xFF)
//		{
//			i += 4;
//			fileSize -= i;
//			return &(buf[i]);
//		}
//	}
//	return NULL;
// }

//***************************************************************************************
ULONG CBambpex::LoadPldFile(const BRDCHAR* PldFileName, ULONG PldNum)
{
    ULONG Status;
    UCHAR PldLoadStatus;
    if (PldNum) {
#ifdef _WIN32
        LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
        int bHighRes = QueryPerformanceFrequency(&Frequency);
        QueryPerformanceCounter(&StartPerformCount);
#endif
        UCHAR* fileBuffer = NULL;
        DWORD fileSize;
        Status = ReadPldFile(PldFileName, fileBuffer, fileSize);
        if (Status != BRDerr_OK)
            return Status;
        if (PldNum == 1) {
            LPVOID startBuffer = NULL;
            startBuffer = SearchInfoStart(fileBuffer, fileSize);
            if (startBuffer) {
                if (m_hMutex)
                    // WaitForSingleObject(m_hMutex, INFINITE);
                    IPC_captureMutex(m_hMutex, INFINITE);
                Status = LoadBitPld(PldLoadStatus, startBuffer, fileSize);
                if (PldLoadStatus)
                    Status = DspPldConnect();
                if (m_hMutex)
                    // ReleaseMutex(m_hMutex);
                    IPC_releaseMutex(m_hMutex);
            } else
                Status = BRDerr_BAD_FILE_FORMAT;
        } else {
            if (m_hMutex)
                IPC_captureMutex(m_hMutex, INFINITE);
            Status = LoadPartPld((UCHAR)PldNum - 0x20, PldLoadStatus, fileBuffer, fileSize);
            if (m_hMutex)
                IPC_releaseMutex(m_hMutex);
        }
        // VirtualFree(fileBuffer, 0, MEM_RELEASE);
        delete fileBuffer;
#ifdef _WIN32
        QueryPerformanceCounter(&StopPerformCount);
        double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
        BRDC_printf(_BRDC("Read&Load Time is %.2f ms\n"), msTime);
#endif
        return Status;
    }
    Status = CheckPldFile(PldFileName);
    if (Status != BRDerr_OK)
        return Status;

    Status = LoadPld(PldFileName, PldLoadStatus);
    if (Status == BRDerr_BAD_FILE)
        return ErrorPrintf(BRDerr_BAD_FILE,
            CURRFILE,
            _BRDC("<LoadPldFile>  Can't load jamdll.dll or export his function"));
    if (Status != BRDerr_OK)
        return Status;
    // Status = BRDerr_HW_ERROR;

    if (PldLoadStatus) {
        Status = AdmPldStartTest();
        //		if(Status != BRDerr_OK)
        //			return Status;
        Status = AdmPldWorkAndCheck();
        //		if(Status != BRDerr_OK)
        //			return Status;
        Status = SetAdmPldFileInfo();
    }
    return Status;
}

//***************************************************************************************
ULONG CBambpex::DspPldConnect()
{
    ULONG Status;
    DEVS_CMD_Reg RegParam;
    RegParam.idxMain = 0;
    RegParam.tetr = m_FdspTetrNum;
    RegParam.reg = 10; // MODE2
    RegParam.val = 0;
    Status = RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
    if (Status != BRDerr_OK)
        return BRDerr_HW_ERROR;
    IPC_delay(10);
    ULONG tdr_stat = 0;
    do {
        RegParam.reg = 10; // MODE2
        RegParam.val = 2; // разрешение работы
        Status = RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
        if (Status != BRDerr_OK)
            return BRDerr_HW_ERROR;
        IPC_delay(10);
        RegParam.reg = 0;
        Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
        if (Status != BRDerr_OK)
            return BRDerr_HW_ERROR;
        tdr_stat = RegParam.val;
        if (tdr_stat & 0x8000) {
            RegParam.reg = 10;
            RegParam.val = 6;
            Status = RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
            if (Status != BRDerr_OK)
                return BRDerr_HW_ERROR;
            RegParam.val = 0x0E;
            Status = RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
            if (Status != BRDerr_OK)
                return BRDerr_HW_ERROR;
            RegParam.val = 6;
            Status = RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
            if (Status != BRDerr_OK)
                return BRDerr_HW_ERROR;
            break;
        }
    } while (1);

    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::isFdsp()
{
    ULONG Status;
    DEVS_CMD_Reg RegParam;
    RegParam.idxMain = 0;
    int i = 0;
    for (i = 0; i < MAX_TETRNUM; i++) {
        RegParam.tetr = i;
        RegParam.reg = ADM2IFnr_ID;
        Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
        if (Status != BRDerr_OK)
            return BRDerr_HW_ERROR;
        if (0x8C == RegParam.val) // 0x8C - TRD_FDSP ID
            break;
    }
    m_FdspTetrNum = i;
    if (i >= MAX_TETRNUM) {
        m_FdspTetrNum = -1;
        //		m_DspPldInfo.Id = 0;
    }
    // m_DspPldInfo.Id может быть равно 0, если в конфигурационных данных нет ПЛИС ЦОС или если в прошивке ADM нет тетрады ПЛИС ЦОС
    // if(m_DspPldInfo.Id != 0)
    //{
    //	m_puCnt++;
    //	m_PuInfos.push_back(m_DspPldInfo);
    //}

    return BRDerr_OK;
}

//***************************************************************************************
// ULONG CBambpex::GetFdspPldStatus()
//{
//	ULONG Status;
//	DEVS_CMD_Reg RegParam;
//	RegParam.idxMain = 0;
//	RegParam.tetr = 8;
//	RegParam.reg = 0x200;
//	Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
//	if(RegParam.val & 1)
//		return 1;
//	else
//		return 0;
////	if(0x8C != RegParam.val)
////		return BRDerr_HW_ERROR;
//
//	return BRDerr_OK;
//}

//***************************************************************************************
ULONG CBambpex::AdmPldStartTest()
{
    ULONG Status = BRDerr_OK;
    DEVS_CMD_Reg RegParam;
    RegParam.idxMain = 0;
    RegParam.tetr = ADM2IFnt_MAIN;
    RegParam.reg = ADM2IFnr_DATA;
    RegParam.val = 1; // переключить ПЛИС в рабочий режим
    Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);

    Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
    Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
    Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
    Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);

    RegParam.reg = ADM2IFnr_CMDADR;
    RegParam.val = 0; // установили адрес регистра MODE0
    Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);
    RegParam.reg = ADM2IFnr_CMDDATA;
    RegParam.val = 2; // сброс FIFO тетрады MAIN
    Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);
    RegParam.reg = ADM2IFnr_CMDDATA;
    RegParam.val = 0; // снятие сброса FIFO тетрады MAIN
    Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);

    // RegParam.reg = ADM2IFnr_FTYPE;
    // Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
    // int fifo_width = (int)RegParam.val; // получить ширину регистра тестовой последовательности
    // RegParam.reg = ADM2IFnr_DATA;
    // RegParam.val = 2; // обратно переключить ПЛИС в режим начального тестирования
    // Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);

    int fifo_width = 64; // ширина тетрады MAIN (регистра тестовой последовательности)
    RegParam.reg = ADM2IFnr_DATA;

    int i = 0;

    // тест шины данных
    __int64 test_mask = 0;
    for (i = 0; i < fifo_width; i++) {
        __int64 value = 0;
        Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
        if (Status != BRDerr_OK)
            return Status;
        value = (__int64)RegParam.val;
        Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
        if (Status != BRDerr_OK)
            return Status;
        value |= ((__int64)RegParam.val) << 32;
        if (test_mask != value)
            return BRDerr_PLD_TEST_DATA_ERROR;
        test_mask = (test_mask << 1) + 1;
    }
    for (i = 0; i < fifo_width; i++) {
        __int64 value = 0;
        Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
        if (Status != BRDerr_OK)
            return Status;
        value = (__int64)RegParam.val;
        Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
        if (Status != BRDerr_OK)
            return Status;
        value |= ((__int64)RegParam.val) << 32;
        if (test_mask != value)
            return BRDerr_PLD_TEST_DATA_ERROR;
        test_mask = (test_mask << 1) & 0xFFFFFFFFFFFFFFFELL;
    }
    // тест шины адреса
    // for(i = 0; i < 64; i++) // всего 64 регистра (16 тетрад)
    //{
    //	RegParam.tetr = i >> 2;
    //	RegParam.reg = i;
    //	RegParam.val = 0;
    //	Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);
    //	if(Status != BRDerr_OK)
    //		return Status;
    //	RegParam.tetr = 0;
    //	RegParam.reg = ADM2IFnr_DATA;
    //	Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
    //	ULONG value = RegParam.val;
    //	Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
    //	if(Status != BRDerr_OK)
    //		return Status;
    //	if((value & 0x3f) != i)
    //		return BRDerr_PLD_TEST_ADDRESS_ERROR;
    //}

    /*

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

            // тест шины адреса
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
    //		Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
    //		if(Status != BRDerr_OK)
    //			return Status;
    //		if((value & 0x3f) != i)
                    if(value != i << 1)
                            return BRDerr_PLD_TEST_ADDRESS_ERROR;
            }
    */
    return Status;
}

//***************************************************************************************
ULONG CBambpex::AdmPldWorkAndCheck()
{
    ULONG Status;
    DEVS_CMD_Reg RegParam;
    RegParam.idxMain = 0;
    RegParam.tetr = ADM2IFnt_MAIN;
    RegParam.val = 1; // переключить ПЛИС в рабочий режим
    //	RegParam.reg = ADM2IFnr_DATA | 0x80000000;
    //	Status = RegCtrl(DEVScmd_REGWRITE, &RegParam);
    RegParam.reg = ADM2IFnr_DATA;
    Status = RegCtrl(DEVScmd_REGWRITEDIR, &RegParam);
    if (Status != BRDerr_OK)
        return Status;
    RegParam.reg = ADM2IFnr_STATUS;
    Status = RegCtrl(DEVScmd_REGREADDIR, &RegParam);
    if (Status != BRDerr_OK)
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
    RegParam.reg = MAINnr_SIG;
    Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
    if (Status != BRDerr_OK)
        return Status;
    //	if(INSYS_VENDOR_ID != RegParam.val)
    if (BASE_ID_TAG != RegParam.val)
        return BRDerr_HW_ERROR;
    RegParam.reg = MAINnr_ADMVER;
    Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
    if (Status != BRDerr_OK)
        return Status;
    if (ADM_VERSION != RegParam.val)
        return BRDerr_HW_ERROR;

    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::SetAdmPldFileInfo()
{
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

//***************************************************************************************
ULONG CBambpex::SetDspPldFileInfo()
{
    ULONG Status;
    m_FdspFileInfo.pldId = m_DspPldInfo.Id;
    DEVS_CMD_Reg RegParam;
    RegParam.idxMain = 0;
    RegParam.tetr = m_FdspTetrNum;
    RegParam.reg = 0x286; // MAINnr_FPGAVER;
    Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
    if (Status != BRDerr_OK)
        return Status;
    m_FdspFileInfo.version = RegParam.val;
    RegParam.reg = 0x285; // MAINnr_FPGAMODE;
    Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
    if (Status != BRDerr_OK)
        return Status;
    m_FdspFileInfo.modification = RegParam.val;
    // RegParam.reg = MAINnr_BASEID;
    // Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
    // if(Status != BRDerr_OK)
    //	return Status;
    // m_FdspFileInfo.baseId = RegParam.val;
    // RegParam.reg = MAINnr_BASEVER;
    // Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
    // if(Status != BRDerr_OK)
    //	return Status;
    // m_FdspFileInfo.baseVer = RegParam.val;
    RegParam.reg = 0x280; // MAINnr_SMODID;
    Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
    if (Status != BRDerr_OK)
        return Status;
    m_FdspFileInfo.submodId = RegParam.val;
    RegParam.reg = 0x282; // MAINnr_SMODVER;
    Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
    if (Status != BRDerr_OK)
        return Status;
    m_FdspFileInfo.submodVer = RegParam.val;
    RegParam.reg = 0x287; // MAINnr_FPGABLD;
    Status = RegCtrl(DEVScmd_REGREADIND, &RegParam);
    if (Status != BRDerr_OK)
        return Status;
    m_FdspFileInfo.build = RegParam.val;
    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::GetPldInfo(PBRDextn_PLDINFO pInfo)
{
    SetAdmPldFileInfo();
    if (m_FdspTetrNum != -1)
        SetDspPldFileInfo();
    if (pInfo->pldId == m_pldFileInfo.pldId) {
        pInfo->version = m_pldFileInfo.version;
        pInfo->modification = m_pldFileInfo.modification;
        pInfo->build = m_pldFileInfo.build;
    } else if (pInfo->pldId == m_FdspFileInfo.pldId) {
        pInfo->version = m_FdspFileInfo.version;
        pInfo->modification = m_FdspFileInfo.modification;
        pInfo->build = m_FdspFileInfo.build;
    } else
        return ErrorPrint(BRDerr_BAD_PARAMETER,
            CURRFILE,
            _BRDC("<GetPldInfo> PLD ID is wrong"));
    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::GetPldFileInfo(PBRDextn_PLDFILEINFO pInfo)
{
    SetAdmPldFileInfo();
    if (pInfo->pldId == m_pldFileInfo.pldId)
        GetAdmPldFileInfo(pInfo);
    else
        return ErrorPrint(BRDerr_BAD_PARAMETER,
            CURRFILE,
            _BRDC("<GetPldFileInfo> PLD ID is wrong"));
    return BRDerr_OK;
}

//***************************************************************************************
void CBambpex::GetAdmPldFileInfo(PBRDextn_PLDFILEINFO pInfo) const
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
ULONG CBambpex::ShowDialog(PBRDextn_PropertyDlg pDlg)
{
#ifdef _WIN32
    if (pDlg->ListCnt) {
        //		HINSTANCE hLib = LoadLibrary("BambpDlg.dll");
        HINSTANCE hLib = LoadLibrary(m_pDlgDllName);
        if (hLib <= (HINSTANCE)HINSTANCE_ERROR)
            return BRDerr_BAD_DEVICE_VITAL_DATA;

        AMBINFO_DlgProp_Type* pDlgProp;
        pDlgProp = (AMBINFO_DlgProp_Type*)GetProcAddress(hLib, "AMBINFO_DialogProperty");
        if (!pDlgProp) {
            FreeLibrary(hLib);
            return BRDerr_BAD_DEVICE_VITAL_DATA;
        }
        int nResponse = (pDlgProp)(this, pDlg->DlgMode, pDlg->ListCnt, pDlg->pList, pDlg->pChangeFlag, pDlg->pCurPage);
        if (nResponse == IDOK) {
            // TODO: Place code here to handle when the dialog is
            //  dismissed with OK
            //			SetPropertyFromDialog(pDev, pInfo);
        } else if (nResponse == IDCANCEL) {
            // TODO: Place code here to handle when the dialog is
            //  dismissed with Cancel
        }
        pDlg->hLib = NULL;
        if (pDlg->DlgMode & 1)
            FreeLibrary(hLib);
        else
            pDlg->hLib = hLib;
    } else {
        return ErrorPrint(BRDerr_BAD_PARAMETER,
            CURRFILE,
            _BRDC("<ShowDialog> Not Property Pages for Dialog"));
    }
#endif
    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::DeleteDialog(PBRDextn_PropertyDlg pDlg)
{
#ifdef _WIN32
    if (pDlg->hLib) {
        AMBINFO_DelDlg_Type* pDelDlg;
        pDelDlg = (AMBINFO_DelDlg_Type*)GetProcAddress(pDlg->hLib, "AMBINFO_DeleteDialog");
        int nResponse = (pDelDlg)(this, pDlg->DlgMode, pDlg->ListCnt, pDlg->pList, pDlg->pChangeFlag, pDlg->pCurPage);
        FreeLibrary(pDlg->hLib);
        pDlg->hLib = NULL;
    } else {
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
ULONG CBambpex::GetBaseIcrDataSize()
{
    // USHORT buf_read[16];
    // buf_read[0] = buf_read[1] = buf_read[2] = 0xffff;
    // ULONG ret_status = ReadNvRAM(&buf_read, 32, 128);
    USHORT buf_read[3];
    buf_read[0] = buf_read[1] = buf_read[2] = 0xffff;
    // ULONG ret_status = ReadNvRAM(&buf_read, 6, 128);
    ULONG ret_status = GetNvRAM(&buf_read, 6, 128);
    //	ULONG ret_status = ReadNvRAM(&buf_read, 6, 0);
    if (ret_status == BRDerr_OK) {
        if (buf_read[0] == BASE_ID_TAG)
            m_sizeICR = buf_read[2];
        else
            m_sizeICR = 0;
        if (m_sizeICR > MAX_BASE_CFGMEM_SIZE)
            m_sizeICR = 0;
        return ret_status;
    } else
        return ret_status;
}

//***************************************************************************************
ULONG CBambpex::GetBaseIcrStatus(ULONG& Status)
{
    USHORT buf_read[3];
    buf_read[0] = buf_read[1] = buf_read[2] = 0xffff;
    ULONG ret_status = GetNvRAM(&buf_read, 6, 128);
    if (ret_status == BRDerr_OK) {
        if (buf_read[0] == BASE_ID_TAG && buf_read[2] <= MAX_BASE_CFGMEM_SIZE)
            Status = 1;
        else
            Status = 0;
        return ret_status;
    } else
        return ret_status;
}

// определяет не просто наличие конфигурационного ППЗУ на субмодуле,
// а то что оно прописано корректной информацией
//***************************************************************************************
ULONG CBambpex::GetAdmIcrStatus(ULONG& Status, int idxSub)
{
    Status = 0;
    if (m_Subunit[idxSub].pSubEEPROM) {
        USHORT buf_read = *(PUSHORT)m_Subunit[idxSub].pSubEEPROM;
        if (buf_read == ADM_ID_TAG)
            Status = 1;
    }
    return BRDerr_OK;

    // USHORT buf_read;
    // buf_read = 0xffff;
    // ULONG ret_status = ReadAdmIdROM(&buf_read, 2, 0);
    // if(ret_status == BRDerr_OK)
    //{
    //	if(buf_read == ADM_ID_TAG)
    //		Status = 1;
    //	else
    //		Status = 0;
    //	return ret_status;
    // }
    // else
    //	return ret_status;
}

//***************************************************************************************
// void CBambpex::GetAmbpCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAmbp pCfgBase)
//{
//	PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
//	int end_flag = 0;
//	do
//	{
//		USHORT sign = *((USHORT*)pCfgMem);
//		USHORT size = 0;
//		if(sign == BASE_ID_TAG)
//		{
//			PICR_IdBase pBaseId = (PICR_IdBase)pCfgMem;
//			ICR_IdBase	idBase;
//			memcpy(&idBase, pBaseId, sizeof(ICR_IdBase));
//			RealBaseCfgSize = idBase.wSizeAll;
//			PUCHAR pBaseCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdBase);
//			size = sizeof(ICR_IdBase);
//			PICR_CfgAmbp pBambpCfg = (PICR_CfgAmbp)pBaseCfg;
//			memcpy(pCfgBase, pBambpCfg, sizeof(ICR_CfgAmbp));
//			size += sizeof(ICR_CfgAmbp);
//			end_flag = 1;
//		}
//		else
//		{
//			size = *((USHORT*)pCfgMem + 1);
//			size += 4;
////			break;
//		}
//		pCfgMem = (PUCHAR)pCfgMem + size;
//	} while(!end_flag && pCfgMem < pEndCfgMem);
//}

//***************************************************************************************
void CBambpex::GetAmbpexCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAmbpex pCfgBase)
{
    PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
    int end_flag = 0;
    do {
        USHORT sign = *((USHORT*)pCfgMem);
        USHORT size = 0;
        if (sign == BASE_ID_TAG) {
            PICR_IdBase pBaseId = (PICR_IdBase)pCfgMem;
            ICR_IdBase idBase;
            memcpy(&idBase, pBaseId, sizeof(ICR_IdBase));
            // RealBaseCfgSize = idBase.wSizeAll;
            PUCHAR pBaseCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdBase);
            size = sizeof(ICR_IdBase);
            PICR_CfgAmbpex pBambpexCfg = (PICR_CfgAmbpex)pBaseCfg;
            memcpy(pCfgBase, pBambpexCfg, sizeof(ICR_CfgAmbpex));
            size += sizeof(ICR_CfgAmbpex);
            end_flag = 1;
        } else {
            size = *((USHORT*)pCfgMem + 1);
            size += 4;
            //			break;
        }
        pCfgMem = (PUCHAR)pCfgMem + size;
    } while (!end_flag && pCfgMem < pEndCfgMem);
}

//***************************************************************************************
void CBambpex::GetAdp201x1CfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAdp201x1 pCfgBase)
{
    PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
    int end_flag = 0;
    do {
        USHORT sign = *((USHORT*)pCfgMem);
        USHORT size = 0;
        if (sign == BASE_ID_TAG) {
            PICR_IdBase pBaseId = (PICR_IdBase)pCfgMem;
            ICR_IdBase idBase;
            memcpy(&idBase, pBaseId, sizeof(ICR_IdBase));
            // RealBaseCfgSize = idBase.wSizeAll;
            PUCHAR pBaseCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdBase);
            size = sizeof(ICR_IdBase);
            PICR_CfgAdp201x1 pBadp201x1Cfg = (PICR_CfgAdp201x1)pBaseCfg;
            memcpy(pCfgBase, pBadp201x1Cfg, sizeof(ICR_CfgAdp201x1));
            size += sizeof(ICR_CfgAdp201x1);
            end_flag = 1;
        } else {
            size = *((USHORT*)pCfgMem + 1);
            size += 4;
            //			break;
        }
        pCfgMem = (PUCHAR)pCfgMem + size;
    } while (!end_flag && pCfgMem < pEndCfgMem);
}

//***************************************************************************************
void CBambpex::GetFmcCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgFmc105p pCfgBase)
{
    PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
    int end_flag = 0;
    do {
        USHORT sign = *((USHORT*)pCfgMem);
        USHORT size = 0;
        if (sign == BASE_ID_TAG) {
            PICR_IdBase pBaseId = (PICR_IdBase)pCfgMem;
            ICR_IdBase idBase;
            memcpy(&idBase, pBaseId, sizeof(ICR_IdBase));
            // RealBaseCfgSize = idBase.wSizeAll;
            PUCHAR pBaseCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdBase);
            size = sizeof(ICR_IdBase);
            PICR_CfgFmc105p pBfmcCfg = (PICR_CfgFmc105p)pBaseCfg;
            if (pBfmcCfg->wSize + 4 == sizeof(ICR_CfgFmc112cp)) {
                PICR_CfgFmc112cp pF112Cfg = (PICR_CfgFmc112cp)pBaseCfg;
                memcpy(pCfgBase, pF112Cfg, sizeof(ICR_CfgFmc112cp));
                size += sizeof(ICR_CfgFmc112cp);
            } else {
                memcpy(pCfgBase, pBfmcCfg, sizeof(ICR_CfgFmc105p));
                size += sizeof(ICR_CfgFmc105p);
            }
            end_flag = 1;
        } else {
            size = *((USHORT*)pCfgMem + 1);
            size += 4;
            //			break;
        }
        pCfgMem = (PUCHAR)pCfgMem + size;
    } while (!end_flag && pCfgMem < pEndCfgMem);
}

//***************************************************************************************
void CBambpex::GetXm5516CfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_Cfg5516 pCfgBase)
{
    PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
    int end_flag = 0;
    do {
        USHORT sign = *((USHORT*)pCfgMem);
        USHORT size = 0;
        if (sign == BASE_ID_TAG) {
            PICR_IdBase pBaseId = (PICR_IdBase)pCfgMem;
            ICR_IdBase idBase;
            memcpy(&idBase, pBaseId, sizeof(ICR_IdBase));
            // RealBaseCfgSize = idBase.wSizeAll;
            PUCHAR pBaseCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdBase);
            size = sizeof(ICR_IdBase);
            PICR_Cfg5516 pBambpexCfg = (PICR_Cfg5516)pBaseCfg;
            memcpy(pCfgBase, pBambpexCfg, sizeof(ICR_Cfg5516));
            size += sizeof(ICR_Cfg5516);
            end_flag = 1;
        } else {
            size = *((USHORT*)pCfgMem + 1);
            size += 4;
            //			break;
        }
        pCfgMem = (PUCHAR)pCfgMem + size;
    } while (!end_flag && pCfgMem < pEndCfgMem);
}

//***************************************************************************************
void CBambpex::GetFmc551FCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_Cfg551F pCfgBase)
{
    PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
    int end_flag = 0;
    do {
        USHORT sign = *((USHORT*)pCfgMem);
        USHORT size = 0;
        if (sign == BASE_ID_TAG) {
            PICR_IdBase pBaseId = (PICR_IdBase)pCfgMem;
            ICR_IdBase idBase;
            memcpy(&idBase, pBaseId, sizeof(ICR_IdBase));
            PUCHAR pBaseCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdBase);
            size = sizeof(ICR_IdBase);
            PICR_Cfg551F pBambpexCfg = (PICR_Cfg551F)pBaseCfg;
            memcpy(pCfgBase, pBambpexCfg, sizeof(ICR_Cfg551F));
            size += sizeof(ICR_Cfg551F);
            end_flag = 1;
        } else {
            size = *((USHORT*)pCfgMem + 1);
            size += 4;
            //			break;
        }
        pCfgMem = (PUCHAR)pCfgMem + size;
    } while (!end_flag && pCfgMem < pEndCfgMem);
}

//***************************************************************************************
void CBambpex::GetDdr3CfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDdr3 pCfgDdr3)
{
    PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
    int end_flag = 0;
    do {
        USHORT sign = *((USHORT*)pCfgMem);
        USHORT size = 0;
        switch (sign) {
        case END_TAG:
        case ALT_END_TAG:
            end_flag = 1;
            break;
        case DDR3_CFG_TAG: {
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
    } while (!end_flag && pCfgMem < pEndCfgMem);
}

//***************************************************************************************
ULONG CBambpex::SetSdramConfig(PBASE_INI pIniData, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
    // устанавливаем конфигурацию по умолчанию
    m_SysGen = 66000000; // 66 МГц
    for (int i = 0; i < MAX_MEMORY_SRV; i++) {
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

    // ОСТАЛЬНЫЕ
    // if (m_DeviceID != AMBPEX8_DEVID &&
    //	m_DeviceID != ADP201X1AMB_DEVID &&
    //	m_DeviceID != ADP201X1DSP_DEVID &&
    //	m_DeviceID != FMC103E_DEVID &&
    //	m_DeviceID != FMC108V_DEVID &&
    //	m_DeviceID != FMC111P_DEVID &&
    //	m_DeviceID != FMC113E_DEVID &&
    //	m_DeviceID != FMC123E_DEVID &&
    //	m_DeviceID != FMC124P_DEVID &&
    //	m_DeviceID != FMC122P_DEVID &&
    //	m_DeviceID != FMC126P_DEVID &&
    //	m_DeviceID != AC_ADC_DEVID &&
    //	m_DeviceID != AC_DSP_DEVID &&
    //	m_DeviceID != AC_SYNC_DEVID &&
    //	m_DeviceID != PS_ADC_DEVID &&
    //	m_DeviceID != PS_DSP_DEVID &&
    //	m_DeviceID != PS_SYNC_DEVID &&
    //	m_DeviceID != UADCPEX_DEVID &&
    //	m_DeviceID != D2XT006_DEVID

    if ((m_DeviceID == AMBPEX5_DEVID) || (m_DeviceID == FMC105P_DEVID) || (m_DeviceID == FMC106P_DEVID) || (m_DeviceID == FMC107P_DEVID) || (m_DeviceID == FMC115CP_DEVID) || (m_DeviceID == FMC112CP_DEVID) || (m_DeviceID == FMC114V_DEVID) || (m_DeviceID == FMC117CP_DEVID) || (m_DeviceID == FMC121CP_DEVID) || (m_DeviceID == FMC121P_DEVID) || (m_DeviceID == PANORAMA_DEVID) || (m_DeviceID == FMC125CP_DEVID) || (m_DeviceID == FMC127P_DEVID) || (m_DeviceID == FMC132P_DEVID) || (m_DeviceID == FMC139P_DEVID) || (m_DeviceID == FMC130E_DEVID) || (m_DeviceID == XM416X250M_DEVID) || (m_DeviceID == FMC110P_DEVID)) {
        m_MemCfg[0].SlotCnt = 1;
        m_MemCfg[1].SlotCnt = 0;
        // m_MemCfg[0].ModuleCnt = 1;
        // return BRDerr_OK;
    } else {
        if (m_DeviceID == FMC126P_DEVID)
            if (pIniData) { // уточняем конфигурацию из источника инициализации
                SetSdramfromIniFile(pIniData);
            }
        return BRDerr_OK;
    }
    // m_Ddr3Cfg.CfgSrc = 2;	// использовать SPD
    if (BaseEEPROMSize) {
        if (m_DeviceID == XM416X250M_DEVID || m_DeviceID == FMC117CP_DEVID || m_DeviceID == FMC121CP_DEVID || (m_DeviceID == FMC121P_DEVID) || (m_DeviceID == PANORAMA_DEVID) || m_DeviceID == FMC125CP_DEVID || m_DeviceID == FMC127P_DEVID || m_DeviceID == FMC112CP_DEVID) {
            ICR_CfgFmc112cp icr_base;
            if (m_DeviceID == XM416X250M_DEVID) {
                PICR_Cfg5516 pCfg5516 = (PICR_Cfg5516)&icr_base;
                GetXm5516CfgICR(pBaseEEPROMMem, BaseEEPROMSize, pCfg5516);
                m_Ddr3Cfg.CfgSrc = pCfg5516->isDDR3;
            } else if (m_DeviceID == FMC127P_DEVID) {
                ICR_Cfg551F* pCfg551F = (ICR_Cfg551F*)&icr_base;
                GetFmc551FCfgICR(pBaseEEPROMMem, BaseEEPROMSize, pCfg551F);
                m_Ddr3Cfg.CfgSrc = pCfg551F->bIsInternalDDR3;
            } else {
                GetFmcCfgICR(pBaseEEPROMMem, BaseEEPROMSize, (PICR_CfgFmc105p)&icr_base);
                m_Ddr3Cfg.CfgSrc = icr_base.isDDR3;
            }
            m_SysGen = icr_base.dSysGen;
            if (!m_Ddr3Cfg.CfgSrc) {
                m_Ddr3Cfg.ModuleCnt = 0;
                m_Ddr3Cfg.CapacityMbits = 0;
            }
            if (m_Ddr3Cfg.CfgSrc == 1) {
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
        } else {
            ICR_CfgAmbpex icr_base;
            GetAmbpexCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &icr_base);
            m_SysGen = icr_base.dSysGen;
            m_MemCfg[0].SlotCnt = icr_base.bSramCfgCnt;
        }
    }

    if (pIniData) { // уточняем конфигурацию из источника инициализации
        SetSdramfromIniFile(pIniData);
    }
    return BRDerr_OK;
}

//***************************************************************************************
void CBambpex::SetSdramfromIniFile(PBASE_INI pIniData)
{ // уточняем конфигурацию из источника инициализации
    if (pIniData->SysGen != 0xffffffff)
        m_SysGen = pIniData->SysGen;
    for (int iMem = 0; iMem < MAX_MEMORY_SRV; iMem++) {
        if (BRDC_strlen(m_pIniData->MemIni[iMem].DlgDllName))
            BRDC_strcpy(m_MemCfg[iMem].DlgDllName, m_pIniData->MemIni[iMem].DlgDllName);
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

//***************************************************************************************
void CBambpex::GetDspNodeCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDspNode pCfgDspNode)
{
    PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
    int idxDsp = 0;
    int end_flag = 0;
    do {
        USHORT sign = *((USHORT*)pCfgMem);
        USHORT size = 0;
        switch (sign) {
        case END_TAG:
        case ALT_END_TAG:
            end_flag = 1;
            break;
        case DSPNODE_CFG_TAG: {
            PICR_CfgDspNode pDspNodeCfg = (PICR_CfgDspNode)pCfgMem;
            memcpy(pCfgDspNode + idxDsp, pDspNodeCfg, sizeof(ICR_CfgDspNode));
            idxDsp++;
            size += sizeof(ICR_CfgDspNode);
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

//***************************************************************************************
ULONG CBambpex::SetDspNodeConfig(PBASE_INI pIniData, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize)
{
    // устанавливаем конфигурацию по умолчанию
    memcpy(&m_DspCfg, &DspNodeCfg_dflt, sizeof(DSPNODE_CFG));

    // PUCHAR pBaseCfgMem = new UCHAR[MAX_BASE_CFGMEM_SIZE];
    // ULONG BaseCfgSize = MAX_BASE_CFGMEM_SIZE;
    // if(GetBaseEEPROM(pBaseCfgMem, BaseCfgSize) == BRDerr_OK)
    if (BaseEEPROMSize) {
        //		ICR_CfgAmbp icr_base;
        ////		GetBaseCfgICR(pBaseCfgMem, BaseCfgSize, &icr_base);
        //		GetBaseCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &icr_base);
        ICR_CfgDspNode icr_dspnode;
        GetDspNodeCfgICR(pBaseEEPROMMem, BaseEEPROMSize, &icr_dspnode);
        m_DspCfg.PldType = icr_dspnode.bPldType;
        m_DspCfg.PldVolume = icr_dspnode.wPldVolume;
        m_DspCfg.PldPins = icr_dspnode.wPldPins;
        m_DspCfg.PldSpeedGrade = icr_dspnode.bPldSpeedGrade;
        m_DspCfg.PioType = icr_dspnode.bPioType;
    }
    //	delete pBaseCfgMem;
    if (pIniData) { // уточняем конфигурацию из источника инициализации
        if (pIniData->DspIni.PldType != 0xffffffff)
            m_DspCfg.PldType = pIniData->DspIni.PldType;
        if (pIniData->DspIni.PldVolume != 0xffffffff)
            m_DspCfg.PldVolume = pIniData->DspIni.PldVolume;
        if (pIniData->DspIni.PldPins != 0xffffffff)
            m_DspCfg.PldPins = pIniData->DspIni.PldPins;
        if (pIniData->DspIni.PldSpeedGrade != 0xffffffff)
            m_DspCfg.PldSpeedGrade = pIniData->DspIni.PldSpeedGrade;
        if (pIniData->DspIni.PioType != 0xffffffff)
            m_DspCfg.PioType = pIniData->DspIni.PioType;
    }
    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::GetAdmIfCntICR(int& AdmIfCnt)
{
    // Check base module ICR
    ULONG icr_base_status;
    if (GetBaseIcrStatus(icr_base_status) != BRDerr_OK)
        //		return BRDerr_HW_ERROR;
        return ErrorPrint(BRDerr_HW_ERROR,
            CURRFILE,
            _BRDC("<GetAdmIfCntICR> Hardware error by get base ICR status"));
    if (icr_base_status) { // получаем конфигурацию из ППЗУ
        PUCHAR pBaseCfgMem = new UCHAR[MAX_BASE_CFGMEM_SIZE];
        ULONG BufferSize = MAX_BASE_CFGMEM_SIZE;
        ULONG Offset = OFFSET_BASE_CFGMEM;
        // if(ReadNvRAM(pBaseCfgMem, BufferSize, Offset) != BRDerr_OK)
        if (GetNvRAM(pBaseCfgMem, BufferSize, Offset) != BRDerr_OK)
            //			return BRDerr_BAD_DEVICE_VITAL_DATA;
            return ErrorPrint(BRDerr_BAD_DEVICE_VITAL_DATA,
                CURRFILE,
                _BRDC("<GetAdmIfCntICR> Bad device vital data by read base ICR"));
        ULONG dRealBaseCfgSize;
        if (*(PUSHORT)pBaseCfgMem == BASE_ID_TAG) {
            dRealBaseCfgSize = *((PUSHORT)pBaseCfgMem + 2);
            if (m_DeviceID == ADP201X1AMB_DEVID || m_DeviceID == ADP201X1DSP_DEVID) {
                ICR_CfgAdp201x1 cfgBase;
                GetAdp201x1CfgICR(pBaseCfgMem, dRealBaseCfgSize, &cfgBase);
                AdmIfCnt = cfgBase.bAdmIfCnt;
            } else { // AMBPEX8, AMBPEX5, FMC105P
                ICR_CfgAmbpex cfgBase;
                GetAmbpexCfgICR(pBaseCfgMem, dRealBaseCfgSize, &cfgBase);
                AdmIfCnt = cfgBase.bAdmIfCnt;
            }
        } else
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
ULONG CBambpex::GetBaseEEPROM(PUCHAR pBaseCfgMem, ULONG& BaseCfgSize)
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
ULONG CBambpex::GetBaseEEPROM(PUCHAR pBaseCfgMem, ULONG& BaseCfgSize)
{
    if (BaseCfgSize) { // получаем содержимое ППЗУ базового модуля
        ULONG BufferSize = BaseCfgSize;
        ULONG Offset = OFFSET_BASE_CFGMEM;
        // if(ReadNvRAM(pBaseCfgMem, BufferSize, Offset) != BRDerr_OK)
        if (GetNvRAM(pBaseCfgMem, BufferSize, Offset) != BRDerr_OK)
            return BRDerr_BAD_DEVICE_VITAL_DATA;
        else {
            ULONG dRealBaseCfgSize;
            if (*(PUSHORT)pBaseCfgMem == BASE_ID_TAG) {
                dRealBaseCfgSize = *((PUSHORT)pBaseCfgMem + 2);
                if (m_DeviceID == ADP201X1AMB_DEVID || m_DeviceID == ADP201X1DSP_DEVID) {
                    ICR_CfgAdp201x1 cfgBase;
                    GetAdp201x1CfgICR(pBaseCfgMem, dRealBaseCfgSize, &cfgBase);
                    m_AdmIfCnt = cfgBase.bAdmIfCnt;
                } else {
                    if (m_DeviceID == RFG2_DEVID)
                        m_AdmIfCnt = 0;
                    else { // AMBPEX8, AMBPEX5, FMC105P
                        ICR_CfgAmbpex cfgBase;
                        GetAmbpexCfgICR(pBaseCfgMem, dRealBaseCfgSize, &cfgBase);
                        m_AdmIfCnt = cfgBase.bAdmIfCnt;
                    }
                }
            } else
                return BRDerr_BAD_DEVICE_VITAL_DATA;
        }

    } else
        return BRDerr_BAD_DEVICE_VITAL_DATA;
    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::SetSubEeprom(int idxSub, ULONG& SubCfgSize)
{
    if (!m_Subunit[idxSub].pSubEEPROM) {
        BRDCHAR nameFileMap[MAX_PATH];
        BRDC_sprintf(nameFileMap, _BRDC("subeeprom_%s_%d_%d"), m_name, m_PID, idxSub);
        // m_Subunit[idxSub].hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,
        //												NULL, PAGE_READWRITE,
        //												0, SUBMOD_CFGMEM_SIZE,
        //												nameFileMap);
        // int isAlreadyCreated = ( GetLastError() == ERROR_ALREADY_EXISTS ) ? 1 : 0;
        // m_Subunit[idxSub].pSubEEPROM = (PVOID)MapViewOfFile(m_Subunit[idxSub].hFileMap,
        //													FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
        int isAlreadyCreated = 0;
        m_Subunit[idxSub].hFileMap = IPC_createSharedMemoryEx(nameFileMap, SUBMOD_CFGMEM_SIZE, &isAlreadyCreated);
        m_Subunit[idxSub].pSubEEPROM = (PVOID)IPC_mapSharedMemory(m_Subunit[idxSub].hFileMap);

        if (m_Subunit[idxSub].pSubEEPROM == 0)
            return BRDerr_HW_ERROR;

        PUSHORT pSubCfgMem = (PUSHORT)(m_Subunit[idxSub].pSubEEPROM);
        if (m_Subunit[idxSub].pSubEEPROM && !isAlreadyCreated) { // если 1-й раз, то заполняем
            pSubCfgMem[0] = 0xA55A;
            pSubCfgMem[1] = 0x5AA5;
            // получаем конфигурацию субмодуля из его ППЗУ
            //			ULONG Status = ReadAdmIdROM(pSubCfgMem, 4, 0);
            // ULONG Status = ReadAdmIdROM(m_Subunit[idxSub].pSubEEPROM, SUBMOD_CFGMEM_SIZE, 0);
            ULONG Status = ReadSubICR(idxSub, m_Subunit[idxSub].pSubEEPROM, SUBMOD_CFGMEM_SIZE, 0);
            if (Status != BRDerr_OK)
                return BRDerr_HW_ERROR;
        }
        // в любом случае получаем размер
        if (pSubCfgMem[0] == ADM_ID_TAG) {
            SubCfgSize = pSubCfgMem[1];
            if (!SubCfgSize)
                return BRDerr_BAD_DEVICE_VITAL_DATA;
        } else
            return BRDerr_BAD_DEVICE_VITAL_DATA;
    }
    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::GetSubEEPROM(int idxSub, PUCHAR pSubCfgMem, ULONG& SubCfgSize)
{
    // Check subunit ICR
    ULONG icr_adm_status;
    if (GetAdmIcrStatus(icr_adm_status, idxSub + m_isFMC2) != BRDerr_OK)
        return BRDerr_HW_ERROR;
    if (icr_adm_status) { // получаем конфигурацию субмодуля из его ППЗУ
        memcpy(pSubCfgMem, m_Subunit[idxSub + m_isFMC2].pSubEEPROM, SubCfgSize);
        //		if(ReadAdmIdROM(pSubCfgMem, SubCfgSize, 0) != BRDerr_OK)
        //			return BRDerr_BAD_DEVICE_VITAL_DATA;

        // копирование необходимо для C6678 из-за его особенностей выравнивания
        USHORT tag = 0;
        USHORT size = 0;
        memcpy(&tag, pSubCfgMem, 2);
        memcpy(&size, (USHORT*)pSubCfgMem + 2, 2);

        if (tag == ADM_ID_TAG) {
            SubCfgSize = size;
            if (!SubCfgSize)
                return BRDerr_BAD_DEVICE_VITAL_DATA;
        } else
            return BRDerr_BAD_DEVICE_VITAL_DATA;
    } else
        return BRDerr_BAD_DEVICE_VITAL_DATA;
    return BRDerr_OK;
}

//***************************************************************************************
ULONG CBambpex::SetServices()
{
    DeleteServices();
    int iSrv = 0, iMemorySrv = 0, iTestSrv = 0, iStreamSrv = 0, iSysMonSrv = 0;
    for (int iTst = 0; iTst < MAX_TEST_SRV; iTst++) {
        TESTSRV_CFG tst_srv_cfg;
        tst_srv_cfg.isAlreadyInit = 0;
        m_pTestSrv[iTestSrv] = new CTestSrv(iSrv++, iTestSrv, this, &tst_srv_cfg);
        m_SrvList.push_back(m_pTestSrv[iTestSrv++]);
    }
    for (int iMem = 0; iMem < MAX_MEMORY_SRV; iMem++) {
        if (FMC126P_DEVID == m_DeviceID || FMC139P_DEVID == m_DeviceID || FMC131VKU_DEVID == m_DeviceID || FMC141VKU1_DEVID == m_DeviceID || FMC141VKU2_DEVID == m_DeviceID || FMC143VKU_DEVID == m_DeviceID || FMC146VKU_DEVID == m_DeviceID || FMC132P_DEVID == m_DeviceID) { // модули с DDR4
            DDR4SDRAMSRV_CFG mem_srv_cfg;
            BRDC_strcpy(mem_srv_cfg.DlgDllName, _BRDC(""));
            mem_srv_cfg.isAlreadyInit = 0;
            mem_srv_cfg.MemType = 0x0C; // DDR4
            mem_srv_cfg.TetrNum = m_MemCfg[iMem].TetrNum;
            mem_srv_cfg.AdrMask = m_MemCfg[iMem].AdrMask;
            mem_srv_cfg.AdrConst = m_MemCfg[iMem].AdrConst;
            mem_srv_cfg.CfgSource = 2; // from SPD only
            // mem_srv_cfg.CfgSource = m_Ddr3Cfg.CfgSrc;
            mem_srv_cfg.SlotCnt = 1;
            mem_srv_cfg.ModuleCnt = m_MemCfg[iMem].ModuleCnt;
            mem_srv_cfg.ModuleBanks = m_MemCfg[iMem].ModuleBanks;
            mem_srv_cfg.RowAddrBits = m_MemCfg[iMem].RowAddrBits;
            mem_srv_cfg.ColAddrBits = m_MemCfg[iMem].ColAddrBits;
            mem_srv_cfg.ChipBanks = m_MemCfg[iMem].ChipBanks;
            mem_srv_cfg.PrimWidth = m_MemCfg[iMem].PrimWidth;
            mem_srv_cfg.ChipWidth = m_Ddr3Cfg.ChipWidth;
            mem_srv_cfg.CapacityMbits = 0;
            m_pMemorySrv[iMemorySrv] = new CDdr4SdramSrv(iSrv++, iMemorySrv, this, &mem_srv_cfg);
        } else if ((FMC106P_DEVID == m_DeviceID) || (FMC107P_DEVID == m_DeviceID) ||
            //(PS_SYNC_DEVID == m_DeviceID) || // ВРЕМЕННЫЙ КОД - НЕ ЗАБЫТЬ УБРАТЬ !!!!!!!!!!!!!!!!!!!
            (FMC110P_DEVID == m_DeviceID) || (FMC111P_DEVID == m_DeviceID) || (FMC113E_DEVID == m_DeviceID) || (FMC112CP_DEVID == m_DeviceID) || (FMC114V_DEVID == m_DeviceID) || (FMC117CP_DEVID == m_DeviceID) || (FMC121CP_DEVID == m_DeviceID) || (FMC121P_DEVID == m_DeviceID) || (PANORAMA_DEVID == m_DeviceID) || (FMC123E_DEVID == m_DeviceID) || (FMC124P_DEVID == m_DeviceID) || (FMC125CP_DEVID == m_DeviceID) || (FMC127P_DEVID == m_DeviceID) || (FMC130E_DEVID == m_DeviceID) || (FMC122P_DEVID == m_DeviceID) || (XM416X250M_DEVID == m_DeviceID) || (FMC115CP_DEVID == m_DeviceID)) { // модули с DDR3
            DDR3SDRAMSRV_CFG mem_srv_cfg;
            BRDC_strcpy(mem_srv_cfg.DlgDllName, _BRDC(""));
            mem_srv_cfg.isAlreadyInit = 0;
            mem_srv_cfg.MemType = 0x0B; // DDR3
            mem_srv_cfg.TetrNum = m_MemCfg[iMem].TetrNum;
            mem_srv_cfg.AdrMask = m_MemCfg[iMem].AdrMask;
            mem_srv_cfg.AdrConst = m_MemCfg[iMem].AdrConst;
            if (XM416X250M_DEVID == m_DeviceID || FMC117CP_DEVID == m_DeviceID || FMC121CP_DEVID == m_DeviceID || FMC121P_DEVID == m_DeviceID || PANORAMA_DEVID == m_DeviceID || FMC125CP_DEVID == m_DeviceID || FMC127P_DEVID == m_DeviceID || FMC112CP_DEVID == m_DeviceID) {
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
            } else {
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
#ifndef __linux__
            m_pMemorySrv[iMemorySrv]->SetPropDlg((void*)DEVS_propDlg);
#endif
        } else { // модули с DDR2
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
#ifndef __linux__
            m_pMemorySrv[iMemorySrv]->SetPropDlg((void*)DEVS_propDlg);
#endif
        }
        m_SrvList.push_back(m_pMemorySrv[iMemorySrv++]);
    }
    for (int iStrm = 0; iStrm < MAX_STREAM_SRV; iStrm++) {
        STREAMSRV_CFG strm_srv_cfg;
        m_pStreamSrv[iStreamSrv] = new CStreamSrv(iSrv++, iStreamSrv, this, &strm_srv_cfg);
        m_SrvList.push_back(m_pStreamSrv[iStreamSrv++]);
    }
    for (int iSysMon = 0; iSysMon < MAX_SYSMON_SRV; iSysMon++) {
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

//******************************************
void CBambpex::DeleteServices()
{
    for (int i = 0; i < MAX_SYSMON_SRV; i++) {
        if (m_pSysMonSrv[i]) {
            delete m_pSysMonSrv[i];
            m_pSysMonSrv[i] = NULL;
        }
    }
    for (int i = 0; i < MAX_TEST_SRV; i++) {
        if (m_pTestSrv[i]) {
            delete m_pTestSrv[i];
            m_pTestSrv[i] = NULL;
        }
    }
    for (int i = 0; i < MAX_MEMORY_SRV; i++) {
        if (m_pMemorySrv[i]) {
            delete m_pMemorySrv[i];
            m_pMemorySrv[i] = NULL;
        }
    }
    for (int i = 0; i < MAX_STREAM_SRV; i++) {
        if (m_pStreamSrv[i]) {
            delete m_pStreamSrv[i];
            m_pStreamSrv[i] = NULL;
        }
    }
}

//***************************************************************************************
void CBambpex::SetCandidateSrv()
{
    int srv_num = (int)m_ServInfoList.size();
    //	int srv_num = m_SrvList.size();
    BRDCHAR* pchar;
    for (int i = 0; i < srv_num; i++) {
        ULONG attr = m_ServInfoList[i]->attribute;
        //		LPTSTR srv_name = m_ServInfoList[i]->name;
        //		ULONG attr = m_SrvList[i]->GetAttribute();
        int ius = 0;
        if (attr & BRDserv_ATTR_CMPABLE) {
            int j = 0;
            do {
                pchar = BRDC_strstr(m_ServInfoList[j++]->name, _BRDC("CMPSC"));
                //				LPTSTR srv_name = m_SrvList[j++]->GetName();
                //				pchar = _tcsstr(srv_name, "CMP");
            } while (!pchar && j < srv_num);
            if (pchar)
                m_ServInfoList[i]->idxCandidSrv[ius++] = j - 1;
            //				m_SrvList[i]->SetCandidIndex(ius++, j - 1);
        }
        if (attr & BRDserv_ATTR_STREAMABLE_OUT) {
            for (int j = 0; j < srv_num; j++) {
                if ((m_ServInfoList[j]->attribute & BRDserv_ATTR_STREAM) && (m_ServInfoList[j]->attribute & BRDserv_ATTR_DIRECTION_OUT))
                    m_ServInfoList[i]->idxCandidSrv[ius++] = j;
                //				if( (m_SrvList[j]->GetAttribute() & BRDserv_ATTR_STREAM) &&
                //					(m_SrvList[j]->GetAttribute() & BRDserv_ATTR_DIRECTION_OUT))
                //					m_SrvList[i]->SetCandidIndex(ius++, j);
            }
        }
        if (attr & BRDserv_ATTR_STREAMABLE_IN) {
            for (int j = 0; j < srv_num; j++) {
                if ((m_ServInfoList[j]->attribute & BRDserv_ATTR_STREAM) && (m_ServInfoList[j]->attribute & BRDserv_ATTR_DIRECTION_IN))
                    m_ServInfoList[i]->idxCandidSrv[ius++] = j;
                //				if( (m_SrvList[j]->GetAttribute() & BRDserv_ATTR_STREAM) &&
                //					(m_SrvList[j]->GetAttribute() & BRDserv_ATTR_DIRECTION_IN))
                //					m_SrvList[i]->SetCandidIndex(ius++, j);
            }
        }
        if (attr & BRDserv_ATTR_SDRAMABLE) {
            for (int j = 0; j < srv_num; j++) {
                //				if(m_ServInfoList[j]->attribute & BRDserv_ATTR_SDRAM)
                {
                    pchar = BRDC_strstr(m_ServInfoList[j]->name, _BRDC("BASESDRAM"));
                    if (pchar) {
                        if ((attr & BRDserv_ATTR_STREAMABLE_IN) && (m_ServInfoList[j]->attribute & BRDserv_ATTR_DIRECTION_IN))
                            m_ServInfoList[i]->idxCandidSrv[ius++] = j;
                        if ((attr & BRDserv_ATTR_STREAMABLE_OUT) && (m_ServInfoList[j]->attribute & BRDserv_ATTR_DIRECTION_OUT))
                            m_ServInfoList[i]->idxCandidSrv[ius++] = j;
                        // break;
                    }
                }
            }
        }
        //		if(attr & BRDserv_ATTR_DSPABLE)
        //		{
        //			for(int j = 0; j < srv_num; j++)
        //			{
        ////				if(m_ServInfoList[j]->attribute & BRDserv_ATTR_DSPNODE)
        //				{
        //					pchar = _tcsstr(m_ServInfoList[j]->name, "DSPNODE");
        //					if(pchar)
        //					{
        //						m_ServInfoList[i]->idxCandidSrv[ius++] = j;
        //						break;
        //					}
        //				}
        //			}
        //		}
        if (attr & BRDserv_ATTR_ADCABLE) {
            for (int j = 0; j < srv_num; j++) {
                //				if(m_ServInfoList[j]->attribute & BRDserv_ATTR_DSPNODE)
                {
                    pchar = BRDC_strstr(m_ServInfoList[j]->name, _BRDC("ADC"));
                    if (pchar) {
                        m_ServInfoList[i]->idxCandidSrv[ius++] = j;
                        break;
                    }
                }
            }
        }
    }
}

//***************************************************************************************
LONG CBambpex::RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam)
{
    //	BRDCHAR buf[SERVNAME_SIZE];
    ULONG admif = 0;
    //	BRDC_strcpy(buf, m_ServInfoList[pRegParam->idxMain]->name);
    //	if(_tcsstr(buf, _BRDC("DSPNODE")))
    //		admif = 1;
    ULONG status = BRDerr_OK;
    // исследование длительности выполнения команд
    // LARGE_INTEGER Frequency, StartPerformCount, StopPerformCount;
    // int bHighRes = QueryPerformanceFrequency (&Frequency);
    // QueryPerformanceCounter (&StartPerformCount);
    // if(m_hMutex)
    //	WaitForSingleObject(m_hMutex, INFINITE);

    if (cmd <= DEVScmd_REGWRITESDIR && (pRegParam->tetr >= 16 || pRegParam->reg >= 1024))
        return ErrorPrintf(BRDerr_BAD_PARAMETER,
            CURRFILE,
            _BRDC("<RegCtrl> Cmd = %d, Tetr = 0x%04X, Reg = 0x%04X, Val = 0x%04X, One or more parameters are bad"),
            cmd, pRegParam->tetr, pRegParam->reg, pRegParam->val);

    switch (cmd) {
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
        status = SetStatusIrq(admif, pRegParam->tetr, pRegParam->reg, pRegParam->val, (HANDLE)pRegParam->pBuf);
        break;
    case DEVScmd_CLEARSTATIRQ:
        status = ClearStatusIrq(admif, pRegParam->tetr, pRegParam->reg, pRegParam->val, (HANDLE)pRegParam->pBuf);
        break;
#ifdef __linux__
    case DEVScmd_WAITSTATIRQ:
        status = WaitStatusIrq(admif, pRegParam->tetr, pRegParam->reg, pRegParam->val, (HANDLE)pRegParam->pBuf);
        break;
#endif
    case DEVScmd_GETBASEADR:
        // это GCC не компилировал
        // pRegParam->idxMain = (S32)m_pMemBuf[0];
        // pRegParam->reg = (S32)m_pMemBuf[1];
        //  а это GCC компилирует и это правильнее )))
        {
            // memcpy(&pRegParam->idxMain, &m_pMemBuf[0], sizeof(m_pMemBuf[0]));
            // memcpy(&pRegParam->reg, &m_pMemBuf[1], sizeof(m_pMemBuf[1]));

            PULONG* pBar = (PULONG*)pRegParam;
            pBar[0] = m_pMemBuf[0];
            pBar[1] = m_pMemBuf[1];

            // void* pmem0 = (void*)&pRegParam->idxMain;
            // void* pmem1 = (void*)&pRegParam->reg;
            // pmem0 = (void*)m_pMemBuf[0];
            // pmem1 = (void*)m_pMemBuf[1];
        }
        break;
    case DEVScmd_PACKEXECUTE:
        return BRDerr_CMD_UNSUPPORTED;
        // break;
    default: {
        // if(m_hMutex)
        //	ReleaseMutex(m_hMutex);
        // return BRDerr_CMD_UNSUPPORTED;
        BRDCHAR msg[MAX_PATH];
        BRDC_sprintf(msg, _BRDC("<RegCtrl> Command (%d)not supported"), cmd);
        return ErrorPrint(BRDerr_CMD_UNSUPPORTED,
            CURRFILE,
            msg);
    }
    }
    // QueryPerformanceCounter (&StopPerformCount);
    // double msTime = (double)(StopPerformCount.QuadPart - StartPerformCount.QuadPart) / (double)Frequency.QuadPart * 1.E3;
    // if(pRegParam->tetr == 4 && cmd == 6)
    //	BRDC_printf(_BRDC("<RegCtrl> Cmd = %d, Tetr = 0x%04X, Reg = 0x%04X, Val = 0x%04X, The time-out interval elapsed"),
    //												cmd, pRegParam->tetr, pRegParam->reg, pRegParam->val);
    // if(m_hMutex)
    //	ReleaseMutex(m_hMutex);
#ifdef _WIN32
    if (ERROR_SEM_TIMEOUT == status)
        return ErrorPrintf(BRDerr_WAIT_TIMEOUT,
            CURRFILE,
            _BRDC("<RegCtrl> Cmd = %d, Tetr = 0x%04X, Reg = 0x%04X, Val = 0x%04X, The time-out interval elapsed"),
            cmd, pRegParam->tetr, pRegParam->reg, pRegParam->val);

    if (BRDerr_OK != status)
        return BRDerr_HW_ERROR;
    return BRDerr_OK;
#endif
    return status;
}

//********************** StreamCtrl *****************************************************
// управление стримами
//***************************************************************************************
ULONG CBambpex::StreamCtrl(ULONG cmd, PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
    ULONG Status = BRDerr_OK;
    // DEVS_CMD_Reg RegParam;
    // RegParam.idxMain = 0;
    // if(m_hMutex)
    //	WaitForSingleObject(m_hMutex, INFINITE);
    switch (cmd) {
    case STRMcmd_SETMEMORY: {
        // SERV_CMD_IsAvailable srvAvail;
        // int idx	 = (m_curHandle >> 16) & 0x3F;
        // if(idx < (long)m_SrvList.size())
        //	Status = m_SrvList[idx]->DoCmd(this, SERVcmd_SYS_ISAVAILABLE, 0, &srvAvail, NULL);
        // else
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
        // }
        ULONG addr_data = 0;
        if (m_curHandle) {
            int idx = (m_curHandle >> 16) & 0x3F;
            if (idx < (long)m_SrvList.size())
                Status = m_SrvList[idx]->DoCmd(this, SERVcmd_SYS_GETADDRDATA, 0, &addr_data, NULL);
            else {
                DEV_CMD_Ctrl param;
                param.handle = m_curHandle;
                param.cmd = SERVcmd_SYS_GETADDRDATA;
                param.arg = &addr_data;
                if (m_ServInfoList[idx]->baseORsideDLL) {
                    SIDE_API_entry* pSIDE_Entry = (SIDE_API_entry*)m_ServInfoList[idx]->pSideEntry;
                    Status = pSIDE_Entry(m_ServInfoList[idx]->pSideDll, NULL, SIDEcmd_CTRL, &param);
                } else {
                    DEV_API_entry* pDEV_Entry = (DEV_API_entry*)m_ServInfoList[idx]->pSideEntry;
                    Status = pDEV_Entry(m_ServInfoList[idx]->pSideDll, DEVcmd_CTRL, &param);
                }
            }
            if (Status == BRDerr_OK)
                ((PAMB_MEM_DMA_CHANNEL)pParam)->LocalAddr = addr_data;
            else
                ((PAMB_MEM_DMA_CHANNEL)pParam)->LocalAddr = 0;
        } else
            ((PAMB_MEM_DMA_CHANNEL)pParam)->LocalAddr = 0;
        Status = SetMemory(pParam, sizeParam);
    } break;
    case STRMcmd_FREEMEMORY:
        Status = FreeMemory(pParam, sizeParam);
        break;
    case STRMcmd_STARTMEMORY: {
        Status = StartMemory(pParam, sizeParam, pOverlap);
    } break;
    case STRMcmd_STOPMEMORY:
        Status = StopMemory(pParam, sizeParam);
        break;
    case STRMcmd_STATEMEMORY:
        Status = StateMemory(pParam, sizeParam);
        break;
    case STRMcmd_SETDIR:
        Status = SetDirection(pParam, sizeParam);
        break;
    case STRMcmd_SETSRC:
        Status = SetSource(pParam, sizeParam);
        break;
    case STRMcmd_SETDRQ:
        Status = SetDmaRequest(pParam, sizeParam);
        break;
    case STRMcmd_RESETFIFO:
        Status = ResetFifo(pParam, sizeParam);
        break;
    case STRMcmd_ADJUST:
        Status = Adjust(pParam, sizeParam);
        break;
    case STRMcmd_DONE:
        Status = Done(pParam, sizeParam);
        break;
    case STRMcmd_GETDMACHANINFO:
        Status = GetDmaChanInfo(pParam, sizeParam);
        break;
    case STRMcmd_SETIRQMODE:
        Status = SetIrqMode(pParam, sizeParam);
        break;

    case STRMcmd_GETIRQCNT: {
        ULONG* pBufpar = (ULONG*)pParam;
        ULONG chan_num = pBufpar[0];
        PULONG BlockMem = m_pMemBuf[0] + m_BlockFifoAddr[chan_num];
        ULONG irq_cnt = BlockMem[62];
        pBufpar[1] = irq_cnt;
        // Status = ReadLocalBusReg(regVal, PEFIFOadr_IRQ_CNT + extdma_offset[dmaChan], 2);
        break;
    }
#ifdef __linux__
    case STRMcmd_WAITDMABUFFER:
        Status = WaitDmaBuffer(pParam, sizeParam);
        break;
    case STRMcmd_WAITDMABLOCK:
        Status = WaitDmaBlock(pParam, sizeParam);
        break;
#endif
    }
    // if(m_hMutex)
    //	ReleaseMutex(m_hMutex);
    return Status;
}

//***************** End of file bambpex.cpp *****************
