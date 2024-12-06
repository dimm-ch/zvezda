
#include <fstream>
#include <iostream>
#include <string>

#include "brd.h"
#include "ctrl.h"
#include "ctrladc.h"
#include "ctrldac.h"
#include "ctrlddc.h"
#include "ctrlstrm.h"
#include "ctrltest.h"
#include "icr.h"
// #include "ctrldspnode.h"
#include "ctrlgc5016.h"
#include "ctrlreg.h"
#include "ctrlsdram.h"
#include "gipcy.h"

//=************************* main *************************
//    Вывод всей информации об обнаруженных устройствах
//=********************************************************

void printPUs(U32* pLID);
void findTetrads(BRD_Handle dev, BRDCHAR* name);

int BRDC_main(int argc, BRDCHAR* argv[])
{
    BRD_Handle hDev = -1; //, hStrm = -1, hReg = -1;
    BRD_Handle hService = -1;
    BRD_ServList aServices[32] = { 0 };

    U32 nStatus = BRDerr_OK, aLidList[16] = { 0 }, nMode = 0;
    S32 nDevs = 0;

    IPC_initKeyboard();

    setlocale(LC_ALL, "RUSSIAN");
    fflush(stdout);
    setbuf(stdout, NULL);

    BRD_displayMode(BRDdm_FATAL); // BRDdm_VISIBLE | BRDdm_CONSOLE);

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

#define MAX_LID 20
    BRD_LidList lidList;
    lidList.item = MAX_LID;
    lidList.pLID = new U32[MAX_LID];
    BRD_shell(BRDshl_LID_LIST, &lidList);

    BRD_Info info;
    info.size = sizeof(info);

    BRDC_printf("\n       *** LID device info ***  \n");
    for (ULONG i = 0; i < lidList.itemReal; i++) {
        BRD_getInfo(lidList.pLID[i], &info);
        BRDC_printf("\n *** LID # %d  \n", i + 1);
        BRDC_printf(_BRDC(" ** Info: Board: %s, DevID = 0x%x, RevID = 0x%x, Bus = %d, Dev = %d, Slot = %d.\n"),
            info.name, info.boardType >> 16, info.boardType & 0xff, info.bus, info.dev, info.slot);

        printPUs(&lidList.pLID[i]);
    }

    BRD_cleanup();

    return 0;
}

#define MAX_PUs 12
#define MAX_SERVICEs 24

void printPUs(U32* pLID)
{
    U32 open_mode;
    U32 ItemReal;
    BRD_Handle handle;
    handle = BRD_open(*pLID, BRDopen_SHARED, &open_mode);
    BRD_Version ver;
    BRD_version(handle, &ver);
    BRDC_printf(_BRDC("-- Version: Shell v.%d.%d, Driver v.%d.%d\n"), ver.brdMajor, ver.brdMinor, ver.drvMajor, ver.drvMinor);
    if (handle > 0 && BRDopen_SHARED == open_mode) {

        BRD_PuList PuList[MAX_PUs];
        BRD_puList(handle, PuList, MAX_PUs, &ItemReal);
        if (ItemReal <= 8) {
            BRDC_printf("\n * PU's : \n");
            for (U32 j = 0; j < ItemReal; j++) {
                BRDC_printf("----------------------------------------------------------\n");
                BRDC_printf(_BRDC("* PU # %d: %s, Id = %d, Code = %x, Attr = %x \n"),
                    j, PuList[j].puDescription, PuList[j].puId, PuList[j].puCode, PuList[j].puAttr);
                if (PuList[j].puCode == PLD_CFG_TAG && PuList[j].puId == 0x100) {
                    U32 PldState;
                    BRD_puState(handle, PuList[j].puId, &PldState);
                    BRDC_printf(_BRDC("  PU state: ADM PLD State = %d\n"), PldState);
                    if (!PldState) {
                        BRDC_printf(_BRDC("  This PU don't loaded ..\n"));
                    }
                }
                BRDC_printf("----------------------------------------------------------\n");
            }
        }
        // services
        BRDC_printf("\n * Services : \n");
        BRDC_printf("----------------------------------------------------------\n");
        BRD_ServList srvList[MAX_SERVICEs];
        BRD_serviceList(handle, 0, srvList, MAX_SERVICEs, &ItemReal);

        for (U32 j = 0; j < ItemReal && j < MAX_SERVICEs; j++) {
            BRDC_printf(_BRDC(" [%d] service: %s, attributes = 0x%X.\n"),
                j, srvList[j].name, srvList[j].attr);
            findTetrads(handle, srvList[j].name);
        }
        BRDC_printf("----------------------------------------------------------\n");
    }
    BRD_close(handle);
}

void findTetrads(BRD_Handle dev, BRDCHAR* name)
{
    // сначала находим тетраду памяти
    U32 mode = BRDcapt_EXCLUSIVE;
    BRD_Handle hndl = BRD_capture(dev, 0, &mode, name, 10000);
    if (hndl < 0) {
        BRDC_printf(" ERR> Service %s don't captured\n", name);
        return;
    }
    U32 status;
    BRD_Reg regdata;
    regdata.reg = 0x100;
    S32 iTetr = 0;
    for (iTetr = 0; iTetr < 14; iTetr++) {
        regdata.tetr = iTetr;
        status = BRD_ctrl(hReg, 0, BRDctrl_REG_READIND, &regdata);
        if (!BRD_errcmp(status, BRDerr_OK))
            BRDC_printf(_BRDC(" ERR: Tetr = 0x%04X, Reg = 0x%04X\n"), regdata.tetr, regdata.reg);
        else {
            BRDC_printf(_BRDC(" >Tetrada = 0x%04X, Reg = 0x%04X  value = 0x%04X\n"),
                regdata.tetr, regdata.reg, regdata.value);
        }
    }
}
