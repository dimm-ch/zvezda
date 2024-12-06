#include "../../common/dev_util.h"

#include <fstream>
#include <iostream>
#include <string>

//=************************* main *************************
//=********************************************************
#define VER_HI 1
#define VER_LOW 1

using namespace std;

struct mainUtilParams_ {
    int numLid;
    int mode;
    BRDCHAR serviceName[16];
    BRD_Handle hService;
    int timeout;
} mainUtilParams;

void DisplayHelp(void);
void ParseCommandLine(int argc, BRDCHAR* argv[]);
// void GetOptions(void);
void retServiceName();
bool isExit(std::string line);
bool isCommand(std::string& line, commandLineParams& params, BRD_Handle hDev);
bool parsingLine(std::string& line, commandLineParams& params);
S32 printLidsList(U32 aLidLis[16], U32 nDevs);
S32 printServiceList(BRD_Handle& hndlDev);
void getServiceHandle(BRD_Handle& hndlDev, int numServ);
int findNumService(BRDCHAR* nameService, BRD_Handle hndlDev);
int retModeDeviceOpen(std::string sMode);

bool isError(bool check, BRDCHAR* pMess, bool cleanup = false)
{
    if (check)
        return false;
    if (pMess)
        BRDC_printf("ERROR: %ы", pMess);
    if (cleanup)
        BRD_cleanup();
    return true;
}

int BRDC_main(int argc, BRDCHAR* argv[])
{
    BRD_Handle hDev = -1; //, hStrm = -1, hReg = -1;
    BRD_ServList aServices[32] = { 0 };
    BRDCHAR* findService = "not find";
    U32 nStatus = BRDerr_OK, aLidList[16] = { 0 };
    S32 nDevs = 0;

    // GetOptions();
    mainUtilParams.mode = 1;
    mainUtilParams.numLid = 0;
    strcpy(mainUtilParams.serviceName, "REG0");
    mainUtilParams.timeout = 5000;
    ParseCommandLine(argc, argv);

    U08* pBuf[128] = { 0 };
    double dSpeed = 0.0, dTime = 0.0;
    IPC_TIMEVAL start = { 0 }, stop = { 0 };
    setlocale(LC_ALL, "RUSSIAN");

    nStatus = BRD_init(_BRDC("brd.ini"), &nDevs);
    if (!BRD_errcmp(nStatus, BRDerr_OK) || nDevs <= 0) {
        BRDC_printf(_BRDC("BRD_init error 0x%X, or device not found ..\n"), nStatus);
        return -1;
    }

    nStatus = BRD_lidList(aLidList, 16, (U32*)&nDevs);
    if (!BRD_errcmp(nStatus, BRDerr_OK) || nDevs <= 0) {
        BRDC_printf(_BRDC("BRD_lidList error 0x%X, or LIDs not found ..\n"), nStatus);
        BRD_cleanup();
        return -1;
    }
    printLidsList(aLidList, (U32)nDevs);

    U32 frmList = aLidList[mainUtilParams.numLid];

    hDev = BRD_open(frmList, BRDopen_EXCLUSIVE, &mainUtilParams.mode);
    if (hDev <= 0) {
        BRDC_printf(_BRDC("BRD_open error 0x%X\n"), hDev);
        BRD_cleanup();
        return -1;
    }

    std::string s = getStrOpenModeDevice(mainUtilParams.mode);
    BRDC_printf(_BRDC("Board %d (lid=%d) opened in %s"), frmList, mainUtilParams.numLid, s.c_str());

    getServiceHandle(hDev, findNumService(mainUtilParams.serviceName, hDev));

    // Цикл обработки
    std::cout << std::endl
              << "Begin work, type: TETR:REG=VALUE (for write) & TETR:REG (for read) !" << std::endl;
    commandLineParams params;
    params.hService = mainUtilParams.hService;

    while (1) {

        std::string line;
        std::cout << std::endl
                  << "<" << mainUtilParams.serviceName << "> ";
        getline(std::cin, line);
        if (isExit(line))
            break;
        if (isCommand(line, params, hDev))
            continue;
        if (parsingLine(line, params)) {
            U32 stst = processReg(params);
            printf("%s: %s tetrada=%d register=0x%X value=0x%X\n",
                (stst > 0) ? "Ok" : "Err", params.write ? "Write" : "Read", params.tetrad,
                params.reg, params.value);
        } else {
            printf(" .. the input >%s< is not recognized, try again ..\n", line.c_str());
        }
    }

    BRD_release(mainUtilParams.hService, 0);
    BRD_close(hDev);
    BRD_cleanup();
}

bool isCommand(std::string& line, commandLineParams& params, BRD_Handle hDev)
{
    if (line.empty())
        return false;
    char id = line.at(0);
    if (id != '%')
        return false;
    string com = line.substr(0, 2);
    if (com == "%l") { // сменить lid

    } else if (com == "%s") { // сменить service
        S32 ns = printServiceList(hDev);
        if (ns < 1) {
            // printf("");
            return true;
        }
        printf("Select service number (in [x]) .. ");
        std::string com;
        getline(std::cin, com);
        int numServ = atoi(com.c_str());
        getServiceHandle(hDev, numServ);
        params.hService = mainUtilParams.hService;

    } else if (com == "%i") { // info
        printInfo(params, hDev);
    } else if (com == "%b") { // доступ через SPD
        spdAccess(params, hDev);
    } else {
        printf(" .. the input >%s< is not recognized, try again ..\n", line.c_str());
        printf("Note: %l - change LID,%/s - change service, %/i - info, %/b - access to SPD.\n ");
    }
    return true;
}

bool parsingLine(std::string& line, commandLineParams& params)
{
    if (line.empty())
        return false;
    bool write = false;
    string s, tetr, reg, val;
    uint32_t t, r, v;
    int n = 0;
    s = line;
    n = s.find(':');
    if (n < 0)
        return false;
    tetr = s.substr(0, n);
    s = s.substr(n + 1);
    n = s.find('=');
    reg = s.substr(0, n);
    if (n > 0) {

        write = true;
        val = s.substr(n + 1);
    }

    // cout << "REco line =" << line.c_str() << "  tetr=" << tetr.c_str() << "  reg=" << reg.c_str() << "   val=" << val.c_str() << endl;

    params.write = write;
    params.tetrad = strtoull(tetr.c_str(), NULL, 0);
    params.reg = strtoull(reg.c_str(), NULL, 0);
    params.value = strtoull(val.c_str(), NULL, 0);

    // cout << "Convert  tetr=0x" << hex << params.tetrad << "  reg=" << params.reg << "   val=" << params.value << endl;

    return true;
}

bool isExit(std::string line)
{
    if (line == "Q" || line == "q")
        return true;
    return false;
}

// разобрать командную строку
void ParseCommandLine(int argc, BRDCHAR* argv[])
{
    std::string ts;
    printf("Input parameters: \n");

    size_t i;
    for (i = 1; i < argc; ++i) {

        if ((!strcmp(argv[i], "-h")) || (!strcmp(argv[i], "-help"))) {
            DisplayHelp();
            exit(1);
        }
        if ((!strcmp(argv[i], "-ver")) || (!strcmp(argv[i], "-version"))) {
            printf(" - Version %d.%d\n", VER_HI, VER_LOW);
            exit(1);
        }
        if (((!strcmp(argv[i], "-lid")) || (!strcmp(argv[i], "-l"))) && (i + 1 < argc)) {
            ts = argv[i + 1];
            mainUtilParams.numLid = atoi(ts.c_str());
            printf(" - lid = 0x%X  (arg=%s)\n", mainUtilParams.numLid, ts.c_str());
        }
        if (((strcmp(argv[i], "-serv") == 0) || (strcmp(argv[i], "-s") == 0)) && (i + 1 < argc)) {
            strcpy(mainUtilParams.serviceName, argv[i + 1]);
            retServiceName();
            printf(" - service = %s  (arg=%s)\n", mainUtilParams.serviceName, argv[i + 1]);
        }
        if (((strcmp(argv[i], "-mode") == 0) || (strcmp(argv[i], "-m") == 0)) && (i + 1 < argc)) {
            ts = argv[i + 1];
            mainUtilParams.mode = retModeDeviceOpen(ts);
            printf(" - mode = %s  \n", getStrOpenModeDevice(mainUtilParams.mode).c_str());
        }
        if (((strcmp(argv[i], "-timeout") == 0) || (strcmp(argv[i], "-t") == 0)) && (i + 1 < argc)) {
            ts = argv[i + 1];
            mainUtilParams.timeout = atoi(ts.c_str());
            printf(" - timeout = %d  (sr=%s)\n", mainUtilParams.timeout, ts.c_str());
        }
    }
}

//////////////////////////////////////////////////////////////////////////
void DisplayHelp()
{
    BRDC_printf(_BRDC("Usage:\n"));
    BRDC_printf(_BRDC("\n"));
    BRDC_printf(_BRDC("regio [options]\n"));
    BRDC_printf(_BRDC("\n"));
    BRDC_printf(_BRDC("[options] :\n"));
    BRDC_printf(_BRDC(" [-l (or -lid)]  - logicalID namber\n"));
    BRDC_printf(_BRDC(" [-s (or -serv)]  - using sevice? one of :\n"));
    BRDC_printf(_BRDC("                  - reg (default),\n"));
    BRDC_printf(_BRDC("                  - mon,\n"));
    BRDC_printf(_BRDC("                  - adc,\n"));
    BRDC_printf(_BRDC("                  - dca,\n"));
    BRDC_printf(_BRDC(" [-m (or -mode)]  - opening mode, one of: \n"));
    BRDC_printf(_BRDC("                  - excl (is exclusive),\n"));
    BRDC_printf(_BRDC("                  - shared (default),\n"));
    BRDC_printf(_BRDC("                  - spy, \n"));
    BRDC_printf(_BRDC("                  - hi (is highest = shared + spy).\n"));
    BRDC_printf(_BRDC(" [-t (or -timeout)]  - opening timeout, default 5000\n"));
    BRDC_printf(_BRDC("\n"));
}

void retServiceName()
{
    if ((strcmp(mainUtilParams.serviceName, "reg") == 0))
        strcpy(mainUtilParams.serviceName, "REG0");
    else if ((strcmp(mainUtilParams.serviceName, "mon") == 0))
        strcpy(mainUtilParams.serviceName, "SYSMON0");
    else if ((strcmp(mainUtilParams.serviceName, "adc") == 0))
        strcpy(mainUtilParams.serviceName, "ADC214x3GDA0");
    else if ((strcmp(mainUtilParams.serviceName, "dac") == 0))
        strcpy(mainUtilParams.serviceName, "DAC214x3GDA0");
}

int retModeDeviceOpen(std::string sMode)
{
    if (sMode == "excl")
        return BRDopen_EXCLUSIVE;
    else if (sMode == "shared")
        return BRDopen_SHARED;
    else if (sMode == "spy")
        return BRDopen_SPY;
    else if (sMode == "hi")
        return BRDopen_HIGHEST;
    else
        return BRDopen_SHARED;
}

int findNumService(BRDCHAR* nameService, BRD_Handle hndlDev)
{
    BRD_ServList aServices[32] = { 0 };
    U32 nStatus = BRDerr_OK, aLidList[16] = { 0 }, nMode = 0;
    U32 nDevs = 0;

    nStatus = BRD_serviceList(hndlDev, NODE0, aServices, 32, &nDevs);
    if (!BRD_errcmp(nStatus, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRD_serviceList error 0x%X\n"), nStatus);
        BRD_cleanup();
        return -1;
    }
    for (int i = 0; i < nDevs; i++) {
        if (BRDC_strstr(aServices[i].name, mainUtilParams.serviceName) != nullptr)
            return i;
    }
    return -1;
}

void getServiceHandle(BRD_Handle& hndlDev, int numServ)
{
    BRD_ServList aServices[32] = { 0 };
    U32 nStatus = BRDerr_OK, aLidList[16] = { 0 }, nMode = 0;
    S32 nDevs = 0;

    nStatus = BRD_serviceList(hndlDev, NODE0, aServices, 32, (U32*)&nDevs);
    if (!BRD_errcmp(nStatus, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRD_serviceList error 0x%X\n"), nStatus);
        BRD_cleanup();
        exit(1);
    }

    S32 stat = BRD_release(mainUtilParams.hService, 0);
    if (stat < 0) {
        BRDC_printf("Service  not released (error=0x%X)\n", nStatus);
    }
    mainUtilParams.hService = BRD_capture(hndlDev, 0, &nMode, aServices[numServ].name, mainUtilParams.timeout);
    if (mainUtilParams.hService <= 0) {
        BRDC_printf("Service %s not found\n", aServices[numServ].name);
        BRD_cleanup();
        exit(1);
    } else {
        BRDC_printf("\nCaptured service name - %s in mode %s\n", aServices[numServ].name, getStrCaptureModeService(nMode).c_str());
    }
    strcpy(mainUtilParams.serviceName, aServices[numServ].name);
}

S32 printLidsList(U32 aLidLis[16], U32 nDevs)
{
    BRD_Info info;
    info.size = sizeof(BRD_Info);
    printf("***** DEVICEs LIST : \n");
    for (size_t i = 0; i < nDevs; i++) {
        S32 err = BRD_getInfo(aLidLis[i], &info);
        printf("* LID=%d - %s (ver.%d.%d) pid=%d(0x%X) type=%d(0x%X)\n", i, info.name, info.verMajor, info.verMinor,
            info.pid, info.pid, info.boardType, info.boardType);
    }
    printf("***** \n");
    return nDevs;
}

S32 printServiceList(BRD_Handle& hndlDev)
{
    BRD_ServList aServices[32] = { 0 };
    U32 nStatus = BRDerr_OK, aLidList[16] = { 0 }, nMode = 0;
    S32 nDevs = 0;

    nStatus = BRD_serviceList(hndlDev, NODE0, aServices, 32, (U32*)&nDevs);
    if (!BRD_errcmp(nStatus, BRDerr_OK)) {
        BRDC_printf(_BRDC("BRD_serviceList error 0x%X\n"), nStatus);
        BRD_cleanup();
        exit(1);
    }

    printf("***** SERVICEs LIST :\n");
    for (size_t i = 0; i < nDevs; i++) {
        BRDC_printf(_BRDC("* [%d] %s  attrib=%d(0x%X)\n"), i, aServices[i].name, aServices[i].attr, aServices[i].attr);
    }
    printf("***** \n");
    return nDevs;
}

/*
char* translateAttrServices()
{

)
*/
