#include "dev_util.h"

#include <fstream>
#include <iostream>
#include <string>

//=************************* main *************************
//=********************************************************

struct mainUtilParams_ {
    int numLid;
    int mode;
    BRDCHAR serviceName[16];
    int timeout;
} mainUtilParams;

struct commandLineParams_ {
    BRDCHAR service[16];
    bool write;
    S32 tetrad;
    S32 reg;
    U32 value;
    bool indirect;
} commandLineParams;

void DisplayHelp(void);
void ParseCommandLine(int argc, BRDCHAR* argv[]);
void GetOptions(void);
void retServiceName();

int BRDC_main(int argc, BRDCHAR* argv[])
{
    BRD_Handle hDev = -1; //, hStrm = -1, hReg = -1;
    BRD_Handle hService = -1;
    BRD_ServList aServices[32] = { 0 };
    BRDCHAR* findService = "not find";
    U32 nStatus = BRDerr_OK, aLidList[16] = { 0 }, nMode = 0;
    S32 nDevs = 0;

    GetOptions();
    ParseCommandLine(argc, argv);

    U08* pBuf[128] = { 0 };
    double dSpeed = 0.0, dTime = 0.0;
    IPC_TIMEVAL start = { 0 }, stop = { 0 };
    IPC_initKeyboard();
    setlocale(LC_ALL, "RUSSIAN");
    fflush(stdout);
    setbuf(stdout, NULL);
    // BRDdm_FATAL); // BRDdm_VISIBLE | BRDdm_CONSOLE);
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

    U32 frmList = aLidList[mainUtilParams.numLid];
    hDev = BRD_open(frmList, BRDopen_SHARED, &nMode);
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

    BRD_displayMode(BRDdm_VISIBLE | BRDdm_CONSOLE);

    // Services
    BRDC_printf(_BRDC("Services:\n"));
    for (int i = 0; i < nDevs; i++) {
        nMode = 0;
        // BRDC_printf(_BRDC("Srv %d: %s"), i, aServices[i].name);
        if (BRDC_strstr(aServices[i].name, commandLineParams.service) != nullptr) {
            hService = BRD_capture(hDev, 0, &nMode, aServices[i].name, mainUtilParams.timeout);
            findService = aServices[i].name;
            // BRDC_printf(_BRDC(" captured (mode %d)"), nMode);
        }
        BRDC_printf(_BRDC("\n"));
    }
    if (hService <= 0) {
        BRDC_printf(_BRDC("Selected service not found\n"));
        BRD_cleanup();
        return -1;
    } else
        BRDC_printf("Captured service name - %s in mode %d\n\n", findService, nMode);

    int retVal = 0;
    S32 status = 0;
    if (commandLineParams.indirect) {
        if (commandLineParams.write) {
            status = RegPokeInd(hService, commandLineParams.tetrad, commandLineParams.reg, commandLineParams.value);
            printf("Service %s : Write - tetrada=0x%X:register=0x%X value=0x%X\n", findService, commandLineParams.tetrad,
                commandLineParams.reg, commandLineParams.value);
        } else {
            U32 v = RegPeekInd(hService, commandLineParams.tetrad, commandLineParams.reg, status);
            printf("Service %s : Read - tetrada=0x%X:register=0x%X value=0x%X\n", findService, commandLineParams.tetrad,
                commandLineParams.reg, v);
            retVal = v;
        }
    } else {
        if (commandLineParams.write) {
            status = RegPokeDir(hService, commandLineParams.tetrad, commandLineParams.reg, commandLineParams.value);
            printf("Service %s : Write - tetrada=0x%X:register=0x%X value=0x%X\n", findService, commandLineParams.tetrad,
                commandLineParams.reg, commandLineParams.value);
        } else {
            U32 v = RegPeekDir(hService, commandLineParams.tetrad, commandLineParams.reg, status);
            printf("Service %s : Read - tetrada=0x%X:register=0x%X value=0x%X\n", findService, commandLineParams.tetrad,
                commandLineParams.reg, v);
            retVal = v;
        }
    }
    printf("%s \n", (status >= BRDerr_OK) ? " - Ok!" : " -!!! Function error return !!!");

    BRD_release(hService, 0);
    BRD_close(hDev);
    BRD_cleanup();

    return retVal;
}

// получить параметры из ini-файла
void GetOptions()
{
    BRDCHAR Buffer[4096];
    BRDCHAR iniFilePath[MAX_PATH];
    BRDCHAR* endptr;
    IPC_getCurrentDir(iniFilePath, sizeof(iniFilePath) / sizeof(BRDCHAR));
    BRDC_strcat(iniFilePath, "/example.ini");
    BRDC_printf("Load from - %s : \n", iniFilePath);

    IPC_getPrivateProfileString(_BRDC("Params"), _BRDC("lid"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    mainUtilParams.numLid = BRDC_atoi(Buffer);
    BRDC_printf(_BRDC("-- lid : %d \n"), mainUtilParams.numLid);

    IPC_getPrivateProfileString(_BRDC("Params"), _BRDC("mode"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
    mainUtilParams.mode = BRDC_atoi(Buffer);
    BRDC_printf(_BRDC("-- mode : %d \n"), mainUtilParams.mode);

    IPC_getPrivateProfileString(_BRDC("Params"), _BRDC("serviceName"), _BRDC("REG"), Buffer, sizeof(Buffer), iniFilePath);
    strcpy(mainUtilParams.serviceName, Buffer);
    BRDC_printf(_BRDC("-- serviceName : %s \n"), mainUtilParams.serviceName);

    IPC_getPrivateProfileString(_BRDC("Params"), _BRDC("timeoutServiceWait"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
    mainUtilParams.timeout = BRDC_atoi(Buffer);
    BRDC_printf(_BRDC("-- timeout : %d \n"), mainUtilParams.timeout);

    // GetInifileString(iniFilePath, _BRDC("Params"), _BRDC("serviceName"), _BRDC("REG"), mainUtilParams.serviceName, sizeof(mainUtilParams.serviceName));
}

// разобрать командную строку
void ParseCommandLine(int argc, BRDCHAR* argv[])
{
    std::string ts;
    printf("Input parameters: \n");
    commandLineParams.write = false;
    commandLineParams.indirect = true;

    size_t i;
    for (i = 1; i < argc; ++i) {

        if ((tolower(argv[i][1]) == _BRDC('h')) || (argv[i][1] == _BRDC('?'))) {
            DisplayHelp();
            exit(1);
        }
        if ((!strcmp(argv[i], "-w"))) {
            commandLineParams.write = true;
        }
        if (((strcmp(argv[i], "-dir") == 0) || (strcmp(argv[i], "-d") == 0))) {
            ts = argv[i];
            commandLineParams.indirect = false;
        }
        if (((strcmp(argv[i], "-serv") == 0) || (strcmp(argv[i], "-s") == 0)) && (i + 1 < argc)) {
            strcpy(commandLineParams.service, argv[i + 1]);
            retServiceName();
            printf(" - service = %s  (sr=%s)\n", commandLineParams.service, argv[i + 1]);
        }
        if (((strcmp(argv[i], "-tetr") == 0) || (strcmp(argv[i], "-t") == 0)) && (i + 1 < argc)) {
            ts = argv[i + 1];
            commandLineParams.tetrad = atoi(ts.c_str());
            printf(" - tetrada = %d  (sr=%s)\n", commandLineParams.tetrad, ts.c_str());
        }
        if (((!strcmp(argv[i], "-reg")) || (!strcmp(argv[i], "-r"))) && (i + 1 < argc)) {
            ts = argv[i + 1];
            commandLineParams.reg = atoi(ts.c_str());
            printf(" - register = 0x%X  (sr=%s)\n", commandLineParams.reg, ts.c_str());
        }
        if (((!strcmp(argv[i], "-val")) || (!strcmp(argv[i], "-v"))) && (i + 1 < argc)) {
            ts = argv[i + 1];
            // commandLineParams.value = atoi(ts.c_str());
            commandLineParams.value = strtoull(ts.c_str(), NULL, 0);
            printf(" - value = 0x%X (%d) (sr=%s)\n", commandLineParams.value, commandLineParams.value, ts.c_str());
        }
    }
    printf(" %s\n", commandLineParams.write ? " - write" : " - read");
    printf(" %s\n", commandLineParams.indirect ? " - indirect" : " - direct");
}

//////////////////////////////////////////////////////////////////////////
void DisplayHelp()
{
    BRDC_printf(_BRDC("Usage:\n"));
    BRDC_printf(_BRDC("\n"));
    BRDC_printf(_BRDC("acs [ini-file] [options]\n"));
    BRDC_printf(_BRDC("\n"));
    BRDC_printf(_BRDC("[ini-file] - optional name of ini-file (only default name - 'example.ini'\n"));
    BRDC_printf(_BRDC("[options] :\n"));
    BRDC_printf(_BRDC(" [-s (or -serv)]  - using sevice (reg, mon, adc, dac), default in ini-file\n"));
    BRDC_printf(_BRDC(" -w         - write\n"));
    BRDC_printf(_BRDC(" [-dir (or -d)] - direct register operation (default - indirect)\n"));
    BRDC_printf(_BRDC(" [-r] - read\n"));
    BRDC_printf(_BRDC(" -t (or -tetr) <tetrada>   - tetrada - number of tetrada\n"));
    BRDC_printf(_BRDC(" -r (or -reg) <regNo> - regNo -number of register\n"));
    BRDC_printf(_BRDC(" -v (or -val) <value> - value is value for write\n"));
    BRDC_printf(_BRDC("\n"));
}

void retServiceName()
{
    if ((strcmp(commandLineParams.service, "reg") == 0))
        strcpy(commandLineParams.service, "REG0");
    else if ((strcmp(commandLineParams.service, "mon") == 0))
        strcpy(commandLineParams.service, "SYSMON0");
    else if ((strcmp(commandLineParams.service, "adc") == 0))
        strcpy(commandLineParams.service, "ADC214x3GDA0");
    else if ((strcmp(commandLineParams.service, "dac") == 0))
        strcpy(commandLineParams.service, "DAC214x3GDA0");
}
