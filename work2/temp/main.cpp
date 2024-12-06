#include "dev_util.h"

//=************************* main *************************
//=********************************************************

int getIntFromConsole(const char* out, int def_val)
{
    int rv = 0;
    char buff[50];
    BRDC_printf("?> %s", out);
    if (!BRDC_fgets(buff, 50, stdin)) {
        BRDC_printf(_BRDC("ERR> Fatal read console error!\n"));
        return def_val;
    }
    BRDC_sscanf(buff, "%d", &rv);
    return rv;
}

struct mainUtilParams_ {
    int numLid;
} mainUtilParams;

int BRDC_main()
{
    BRD_Handle hDev = -1, hStrm = -1, hReg = -1;
    BRD_ServList aServices[32] = { 0 };
    U32 nStatus = BRDerr_OK, aLidList[16] = { 0 }, nMode = 0;
    S32 nDevs = 0;
    BRDctrl_StreamCBufAlloc rStreamAlloc = { 0 };
    BRDctrl_StreamStub* pStub = 0;
    BRDctrl_StreamCBufStart rStreamStart = { 0 };
    BRDctrl_StreamSetSrc rStreamSrc = { 0 };
    BRDctrl_StreamCBufState rStreamState = { 0 };
    U08* pBuf[128] = { 0 };
    double dSpeed = 0.0, dTime = 0.0;
    IPC_TIMEVAL start = { 0 }, stop = { 0 };
    IPC_initKeyboard();
    setlocale(LC_ALL, "RUSSIAN");
    fflush(stdout);
    setbuf(stdout, NULL);
    BRD_displayMode(BRDdm_VISIBLE | BRDdm_CONSOLE);
    nStatus = BRD_init(_BRDC("brd.ini"), &nDevs);
    if (!BRD_errcmp(nStatus, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRD_init error 0x%X\n"), nStatus);
        return -1;
    }
    if (nDevs <= 0) {
        BRDC_printf(_BRDC("No device found\n"));
        return -1;
    }
    nStatus = BRD_lidList(aLidList, 16, (U32*)&nDevs);
    if (!BRD_errcmp(nStatus, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRD_lidList error 0x%X\n"), nStatus);
        BRD_cleanup();
        return -1;
    }
    if (nDevs <= 0) {
        BRDC_printf(_BRDC("No LIDs found\n"));
        BRD_cleanup();
        return -1;
    }

    GetOptions();
    mainUtilParams.numLid int x = getIntFromConsole("Select LID : ", 0);
    U32 frmList = aLidList[x];

    hDev = BRD_open(frmList, BRDopen_EXCLUSIVE, &nMode);
    if (hDev <= 0) {
        BRDC_printf(_BRDC("BRD_open error 0x%X\n"), hDev);
        BRD_cleanup();
        return -1;
    }

    BRDC_printf(_BRDC("Board %d opened in "), frmList);
    switch (nMode) {
    case BRDopen_EXCLUSIVE:
        BRDC_printf(_BRDC("EXCLUSIVE\n"));
        break;
    case BRDopen_SHARED:
        BRDC_printf(_BRDC("SHARED\n"));
        break;
    case BRDopen_SPY:
        BRDC_printf(_BRDC("SPY\n"));
        break;
    default:
        BRDC_printf(_BRDC("%d\n"), nMode);
        break;
    }
    nStatus = BRD_serviceList(hDev, NODE0, aServices, 32, (U32*)&nDevs);
    if (!BRD_errcmp(nStatus, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRD_serviceList error 0x%X\n"), nStatus);
        BRD_cleanup();
        return -1;
    }
    BRDC_printf(_BRDC("Services:\n"));
    for (int i = 0; i < nDevs; i++) {
        nMode = 0;
        BRDC_printf(_BRDC("Srv%d: %s"), i, aServices[i].name);
        if (BRDC_strstr(aServices[i].name, _BRDC("REG")) != nullptr) {
            hReg = BRD_capture(hDev, 0, &nMode, aServices[i].name, 5000);
            BRDC_printf(_BRDC(" captured (mode %d)"), nMode);
        }
        if (BRDC_strstr(aServices[i].name, _BRDC("STREAM")) != nullptr)
            if (hStrm == -1) {
                hStrm = BRD_capture(hDev, NODE0, &nMode, aServices[i].name, 5000);
                BRDC_printf(_BRDC(" captured (mode %d)"), nMode);
            }
        BRDC_printf(_BRDC("\n"));
    }
    if (hStrm <= 0) {
        BRDC_printf(_BRDC("STREAM service not found\n"));
        BRD_cleanup();
        return -1;
    }
    if (hReg <= 0) {
        BRDC_printf(_BRDC("REG service not found\n"));
        BRD_cleanup();
        return -1;
    }
    BRDC_printf(_BRDC("reset\n"));
    RegPokeInd(hReg, 0, 12, 1);
    RegPokeInd(hReg, 0, 0, 2012);
    IPC_delay(1);
    RegPokeInd(hReg, 0, 0, 2010);
    IPC_delay(1);
    for (int trd = 0; trd < 16; trd++) {
        for (int ii = 0; ii < 32; ii++) {
            RegPokeInd(hReg, trd, ii, 0);
        }
    }
    rStreamAlloc.blkNum = 2;
    rStreamAlloc.blkSize = 32 * 1024;
    rStreamAlloc.dir = BRDstrm_DIR_IN;
    rStreamAlloc.isCont = 0;
    rStreamAlloc.ppBlk = (void**)pBuf;
    rStreamAlloc.pStub = 0;
    BRDC_printf(_BRDC("CBUF_ALLOC\n"));
    nStatus = BRD_ctrl(hStrm, NODE0, BRDctrl_STREAM_CBUF_ALLOC, &rStreamAlloc);
    if ((!BRD_errcmp(nStatus, BRDerr_OK)) && (!BRD_errcmp(nStatus, BRDerr_PARAMETER_CHANGED)) && (!BRD_errcmp(nStatus, BRDerr_WARN))) {
        BRDC_printf(_BRDC("BRDctrl_STREAM_CBUF_ALLOC error 0x%X\n"), nStatus);
        BRD_cleanup();
        return -1;
    }
    if (BRD_errcmp(nStatus, BRDerr_PARAMETER_CHANGED)) {
        BRDC_printf(_BRDC("BRDctrl_STREAM_CBUF_ALLOC parameter "));
        if (rStreamAlloc.blkNum != 2)
            BRDC_printf(_BRDC("blknum changed to %d "), rStreamAlloc.blkNum);
        if (rStreamAlloc.blkSize != 8 * 1024 * 1024)
            BRDC_printf(_BRDC("blksize changed to %d "), rStreamAlloc.blkSize);
        BRDC_printf(_BRDC("\n"));
    }
    if (BRD_errcmp(nStatus, BRDerr_PARAMETER_CHANGED))
        BRDC_printf(_BRDC("BRDctrl_STREAM_CBUF_ALLOC return WARNING\n"));
    pStub = rStreamAlloc.pStub;
    rStreamSrc.src = 0;
    BRDC_printf(_BRDC("SETSRC\n"));
    nStatus = BRD_ctrl(hStrm, NODE0, BRDctrl_STREAM_SETSRC, &rStreamSrc);
    if (!BRD_errcmp(nStatus, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRDctrl_STREAM_SETSRC error 0x%X\n"), nStatus);
        BRD_cleanup();
        return -1;
    }
    BRDC_printf(_BRDC("RESETFIFO\n"));
    nStatus = BRD_ctrl(hStrm, NODE0, BRDctrl_STREAM_RESETFIFO, NULL);
    if (!BRD_errcmp(nStatus, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRDctrl_STREAM_RESETFIFO error 0x%X\n"), nStatus);
        BRD_cleanup();
        return -1;
    }
    RegPokeInd(hReg, 0, 0, 0x2012);
    RegPokeInd(hReg, 0, 0, 0x2010);
    rStreamStart.isCycle = 1;
    BRDC_printf(_BRDC("CBUF_START\n"));
    nStatus = BRD_ctrl(hStrm, NODE0, BRDctrl_STREAM_CBUF_START, &rStreamStart);
    if (!BRD_errcmp(nStatus, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRDctrl_STREAM_CBUF_START error 0x%X\n"), nStatus);
        BRD_cleanup();
        return -1;
    }
    BRDC_printf(_BRDC("\n0\r"));
    IPC_getTime(&start);
    RegPokeInd(hReg, 0, 0, 0x2038);
    while (true) {
        nStatus = BRD_ctrl(hStrm, NODE0, BRDctrl_STREAM_CBUF_STATE, &rStreamState);
        if (!BRD_errcmp(nStatus, BRDerr_OK)) {
            BRDC_printf(_BRDC("BRDctrl_STREAM_CBUF_STATE error 0x%X\n"), nStatus);
            BRD_cleanup();
            return -1;
        }
        IPC_getTime(&stop);
        dTime = IPC_getDiffTime(&start, &stop);
        dSpeed = ((double)rStreamState.blkNumTotal * (double)rStreamAlloc.blkSize / 1024.0 / 1024.0) / (dTime / 1000.0);
        BRDC_printf(_BRDC("%d\t%f\r"), rStreamState.blkNumTotal, dSpeed);
        IPC_delay(100);
        if (IPC_kbhit())
            break;
    }
    IPC_getch();
    nStatus = BRD_ctrl(hStrm, NODE0, BRDctrl_STREAM_CBUF_STOP, NULL);
    if (!BRD_errcmp(nStatus, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRDctrl_STREAM_CBUF_STOP error 0x%X\n"), nStatus);
        BRD_cleanup();
        return -1;
    }
    nStatus = BRD_ctrl(hStrm, NODE0, BRDctrl_STREAM_RESETFIFO, NULL);
    if (!BRD_errcmp(nStatus, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRDctrl_STREAM_RESETFIFO error 0x%X\n"), nStatus);
        BRD_cleanup();
        return -1;
    }
    nStatus = BRD_ctrl(hStrm, NODE0, BRDctrl_STREAM_CBUF_FREE, NULL);
    if (!BRD_errcmp(nStatus, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRDctrl_STREAM_CBUF_FREE error 0x%X\n"), nStatus);
        BRD_cleanup();
        return -1;
    }
    BRD_cleanup();
    return 0;
}

// получить параметры из ini-файла
void GetOptions()
{
    BRDCHAR Buffer[4096];
    BRDCHAR iniFilePath[MAX_PATH];
    BRDCHAR* endptr;
    IPC_getCurrentDir(iniFilePath, sizeof(iniFilePath) / sizeof(BRDCHAR));
    BRDC_strcat(iniFilePath, g_iniFileName);
    // GetPrivateProfileString("Option", "AdcServiceName", "ADC212X200M", g_SrvName, sizeof(g_SrvName), iniFilePath);
    GetInifileString(iniFilePath, _BRDC("Option"), _BRDC("AdcServiceName"), _BRDC("ADC212X200M"), g_SrvName, sizeof(g_SrvName));
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("AdcServiceNum"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    g_AdcSrvNum = BRDC_atoi(Buffer);
    if (g_subNo >= 0)
        g_AdcSrvNum = g_subNo;
    BRDC_sprintf(g_AdcSrvName, _BRDC("%s%d"), g_SrvName, g_AdcSrvNum);

    GetInifileString(iniFilePath, _BRDC("Option"), _BRDC("PldFileName"), _BRDC("ambpcd_v10_adm212x200m.mcs"), g_pldFileName, sizeof(g_pldFileName));
    // GetPrivateProfileString("Option", "PldFileName", "ambpcd_v10_adm212x200m.mcs", g_pldFileName, sizeof(g_pldFileName), iniFilePath);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("isPldLoadAlways"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    g_isPldLoadAlways = BRDC_atoi(Buffer);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("BusMasterEnable"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
    g_DmaOn = BRDC_atoi(Buffer);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("Cycle"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    g_Cycle = BRDC_atoi(Buffer);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("Pause"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    g_Pause = BRDC_atoi(Buffer);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("DaqIntoMemory"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    g_MemOn = BRDC_atoi(Buffer);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("IsWriteFile"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    g_IsWriteFile = BRDC_atoi(Buffer);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("SamplesPerChannel"), _BRDC("16384"), Buffer, sizeof(Buffer), iniFilePath);
    g_samplesOfChannel = BRDC_atoi64(Buffer);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("MemSamplesPerChan"), _BRDC("16384"), Buffer, sizeof(Buffer), iniFilePath);
    g_memorySamplesOfChannel = BRDC_atoi64(Buffer);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("IsSystemMemory"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    g_IsSysMem = BRDC_atoi(Buffer);
    if (g_IsSysMem == 1 && g_info.busType == BRDbus_ETHERNET) {
        BRDC_printf(_BRDC("Ethernet: IsSystemMemory=0 or IsSystemMemory=2 (NOT EQUAL 1)!!!\n"));
        g_IsSysMem = 0;
    }
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("DirFileBufSize"), _BRDC("64"), Buffer, sizeof(Buffer), iniFilePath); // KBytes
    g_FileBufSize = BRDC_atoi(Buffer) * 1024;
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("DirNumBufWrite"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath); // KBytes
    g_DirWriteFile = BRDC_atoi(Buffer);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("DirNumBlock"), _BRDC("2"), Buffer, sizeof(Buffer), iniFilePath); // KBytes
    g_FileBlkNum = BRDC_atoi(Buffer);
    if (g_FileBlkNum < 2)
        g_FileBlkNum = 2;
    GetInifileString(iniFilePath, _BRDC("Option"), _BRDC("DirFileName"), _BRDC("adcdir.bin"), g_dirFileName, sizeof(g_dirFileName));

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("AdcDrqFlag"), _BRDC("2"), Buffer, sizeof(Buffer), iniFilePath);
    g_AdcDrqFlag = BRDC_atoi(Buffer);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("MemDrqFlag"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    g_MemDrqFlag = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Debug"), _BRDC("IoDelay"), _BRDC("128"), Buffer, sizeof(Buffer), iniFilePath);
    g_IoDelay = BRDC_atoi(Buffer);

    if (g_IsWriteFile == 4) {
        BRDC_printf(_BRDC("File mapping mode: IsWriteFile=0, IsSystemMemory=2 !!!\n"));
        g_IsWriteFile = 0;
        g_fileMap = 1;
        g_IsSysMem = 2;
    }
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("RateRate"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    g_transRate = BRDC_atoi(Buffer);
    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("TimeoutSec"), _BRDC("5"), Buffer, sizeof(Buffer), iniFilePath); // sec
    g_MsTimeout = BRDC_atoi(Buffer) * 1000;

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("SwitchOutMask"), _BRDC("0x18"), Buffer, sizeof(Buffer), iniFilePath);
    g_SwitchOutMask = BRDC_strtol(Buffer, &endptr, 0);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("RegDbg"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    g_regdbg = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("AdjustMode"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    g_adjust_mode = BRDC_atoi(Buffer);

    IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("QuickQuit"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    g_quick_quit = BRDC_atoi(Buffer);
}
