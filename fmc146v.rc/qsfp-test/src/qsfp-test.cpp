#include "bardy.h"
#include "brd_dev.h"
#include "ctrlfm402s.h"
#include "exceptinfo.h"
#include "strconv.h"
#include "table_engine_console.h"
#include "time_ipc.h"
#include <iostream>
#include <memory>
#include <csignal>

using namespace std;

static volatile int exit_flag = 0;
void signal_handler(int signo) {
    signo = signo;
    exit_flag = 1;
    fprintf(stderr, "\n");
}

int main(int argc, char* argv[])
{
    fprintf(stderr, "---------------------------\n");
    fprintf(stderr, " TEST: %s\n", argv[0]);
    fprintf(stderr, "---------------------------\n");

    setlocale(LC_ALL, "Russian");
    signal(SIGINT, signal_handler);

    U32 utest_type = get_from_cmdline(argc, argv, "-t", 0); // 0 - psd, 1 - counter
    U32 ulid = get_from_cmdline(argc, argv, "-b", -1);
    if(ulid < 0) {
        fprintf(stderr, "Please specify board LID!\n");
        return -1;
    }
    S32 brd_count = 0;
    BRD_Handle h402s[2] = { 0 };
    FM402S_TESTMODE rTest = { 0 };
    FM402S_GETSTATUS rStat = { 0 };
    FM402S_GETRXTXSTATUS rRxTx[2] = { 0 };
    if (Bardy::initBardy(brd_count)) {
        std::vector<U32> lids;
        if (!Bardy::boardsLID(lids)) {
            fprintf(stderr, "Can't get board LIDs\n");
            return -1;
        }
        brd_dev_t dev = nullptr;
        for (const auto& id : lids) {
            if(id == ulid) {
                dev = get_device(id);
                break;
            }
        }
        if(!dev) {
            fprintf(stderr, "Can't get board LIDs\n");
            return -1;
        }
        dev->show_services();
        dev->lock_services();
        try {
            h402s[0] = dev->lock_service(std::string("FM402S0"), 0);
            h402s[1] = dev->lock_service(std::string("FM402S1"), 0);
        } catch (const except_info_t& errInfo) {
            fprintf(stderr, "%s", errInfo.info.c_str());
            return -1;
        } catch (...) {
            fprintf(stderr, "%s", "Unknown exception in the program!");
            return -1;
        }
        S32 maintetrad = -1;
        dev->get_trd_number(1, maintetrad);
        dev->RegPokeInd(maintetrad, 0, 1);
        ipc_delay(100);
        dev->RegPokeInd(maintetrad, 0, 0);
        ipc_delay(100);
        for (int i = 0; i < 0x30; i++)
            dev->RegPokeInd(maintetrad, i, 0);
        rTest.isGenEnable = 1;
        if (utest_type == 1) {
            rTest.isCntEnable = 1;
        }
        dev->brdctrl(h402s[0], BRDctrl_FM402S_SETTESTMODE, (void*)&rTest);
        dev->brdctrl(h402s[1], BRDctrl_FM402S_SETTESTMODE, (void*)&rTest);

        dev->brdctrl(h402s[0], BRDctrl_FM402S_PREPARE, nullptr);
        dev->brdctrl(h402s[1], BRDctrl_FM402S_PREPARE, nullptr);
        ipc_delay(250);

        BRDC_printf(_BRDC("service #0\n"));
        for (int i = 0; i < 15; i++) {
            dev->brdctrl(h402s[0], BRDctrl_FM402S_GETSTATUS, (void*)&rStat);
            if ((rStat.isChanUp == 1) && (rStat.isPLL_Lock == 1) && (rStat.isQSFPLineUp[0] == 1) && (rStat.isQSFPLineUp[1] == 1) && (rStat.isQSFPLineUp[2] == 1) && (rStat.isQSFPLineUp[3] == 1))
                break;
            BRDC_printf(_BRDC("retry #%d wait %d ms\r"), i + 1, 250);
            ipc_delay(250);
        }
        BRDC_printf(_BRDC("\n"));
        if (rStat.isPLL_Lock == 0) // PLL+LOCK
        {
            BRDC_printf(_BRDC("ERROR: no Pll lock in service #0\n"));
            // BRD_cleanup();
            // return TEST_ERR_NO_PLL_LOCK;
        } else
            BRDC_printf(_BRDC("Pll lock in service #0\n"));
        if ((rStat.isQSFPLineUp[0] != 1) || (rStat.isQSFPLineUp[1] != 1) || (rStat.isQSFPLineUp[2] != 1) || (rStat.isQSFPLineUp[3] != 1)) // LINE_UP
        {
            BRDC_printf(_BRDC("ERROR: no Line up in service #0 (%d %d %d %d)\n"), rStat.isQSFPLineUp[0], rStat.isQSFPLineUp[1], rStat.isQSFPLineUp[2], rStat.isQSFPLineUp[3]);
            // BRD_cleanup();
            // return TEST_ERR_NO_LINE_UP;
        } else
            BRDC_printf(_BRDC("Line up in service #0\n"));
        if (rStat.isChanUp == 0) // CHAN_UP
        {
            BRDC_printf(_BRDC("ERROR: no QSFP present in service #0\n"));
            // BRD_cleanup();
            // return TEST_ERR_NO_CHAN_UP;
        } else
            BRDC_printf(_BRDC("QSFP present in service #0\n"));

        BRDC_printf(_BRDC("service #1\n"));
        for (int i = 0; i < 15; i++) {
            dev->brdctrl(h402s[1], BRDctrl_FM402S_GETSTATUS, (void*)&rStat);
            if ((rStat.isChanUp == 1) && (rStat.isPLL_Lock == 1) && (rStat.isQSFPLineUp[0] == 1) && (rStat.isQSFPLineUp[1] == 1) && (rStat.isQSFPLineUp[2] == 1) && (rStat.isQSFPLineUp[3] == 1))
                break;
            BRDC_printf(_BRDC("retry #%d wait %d ms\r"), i + 1, 250);
            ipc_delay(250);
        }
        BRDC_printf(_BRDC("\n"));
        if (rStat.isPLL_Lock == 0) // PLL+LOCK
        {
            BRDC_printf(_BRDC("ERROR: no Pll lock in service #1\n"));
            // BRD_cleanup();
            // return TEST_ERR_NO_PLL_LOCK;
        } else
            BRDC_printf(_BRDC("Pll lock in service #1\n"));
        if ((rStat.isQSFPLineUp[0] != 1) || (rStat.isQSFPLineUp[1] != 1) || (rStat.isQSFPLineUp[2] != 1) || (rStat.isQSFPLineUp[3] != 1)) // LINE_UP
        {
            BRDC_printf(_BRDC("ERROR: no Line up in service #1 (%d %d %d %d)\n"), rStat.isQSFPLineUp[0], rStat.isQSFPLineUp[1], rStat.isQSFPLineUp[2], rStat.isQSFPLineUp[3]);
            // BRD_cleanup();
            // return TEST_ERR_NO_LINE_UP;
        } else
            BRDC_printf(_BRDC("Line up in service #1\n"));
        if (rStat.isChanUp == 0) // CHAN_UP
        {
            BRDC_printf(_BRDC("ERROR: no QSFP present in service #1\n"));
            // BRD_cleanup();
            // return TEST_ERR_NO_CHAN_UP;
        } else
            BRDC_printf(_BRDC("QSFP present in service #1\n"));

        dev->brdctrl(h402s[0], BRDctrl_FM402S_START, nullptr);
        dev->brdctrl(h402s[1], BRDctrl_FM402S_START, nullptr);

        // char* asColumns[4] = { (char*)" service  ", (char*)" sent      ", (char*)" get       ", (char*)" err       " };

        const std::vector<std::string> columns { " service  ", " sent      ", " get       ", " err       " };
        std::shared_ptr<TableEngine> console = std::make_shared<TableEngineConsole>();
        console->CreateTable(columns);
        int row0 = console->AddRowTable();
        int row1 = console->AddRowTable();
        console->SetValueTable(row0, 0, "%d", row0);
        console->SetValueTable(row1, 0, "%d", row1);

        while (!exit_flag) {
            dev->brdctrl(h402s[0], BRDctrl_FM402S_GETRXTXSTATUS, &(rRxTx[0]));
            dev->brdctrl(h402s[1], BRDctrl_FM402S_GETRXTXSTATUS, &(rRxTx[1]));

            console->SetValueTable(row0, 1, "0x%X", rRxTx[0].nBlockWrite);
            console->SetValueTable(row0, 2, "0x%X", rRxTx[0].nBlockRead);
            console->SetValueTable(row0, 3, "0x%X", rRxTx[0].nBlockErr);
            console->SetValueTable(row1, 1, "0x%X", rRxTx[1].nBlockWrite);
            console->SetValueTable(row1, 2, "0x%X", rRxTx[1].nBlockRead);
            console->SetValueTable(row1, 3, "0x%X", rRxTx[1].nBlockErr);

            ipc_delay(500);
        }
        if (rRxTx[0].nBlockErr != 0) {
            BRDC_printf(_BRDC("ERROR:\nService #0\n"));
            FM402S_GETTESTRESULT rTestRes = { 0 };
            for (int i = 0; i < 4; i++) {
                rTestRes.nChan = i;
                dev->brdctrl(h402s[0], BRDctrl_FM402S_GETTESTRESULT, &rTestRes);
                if (rTestRes.nTotalError != 0)
                    BRDC_printf(_BRDC("channel %d\n"), i);
                for (int j = 0; j < rTestRes.nTotalError; j++) {
                    BRDC_printf(_BRDC("block %d index %d read %llu expected %llu xor %llu\n"), rTestRes.nBlock[j], rTestRes.nIndex[j], rTestRes.lReadWord[j], rTestRes.lExpectWord[j], rTestRes.lReadWord[j] ^ rTestRes.lExpectWord[j]);
                }
            }
        }
        if (rRxTx[1].nBlockErr != 0) {
            BRDC_printf(_BRDC("ERROR:\nService #1\n"));
            FM402S_GETTESTRESULT rTestRes = { 0 };
            for (int i = 0; i < 4; i++) {
                rTestRes.nChan = i;
                dev->brdctrl(h402s[1], BRDctrl_FM402S_GETTESTRESULT, &rTestRes);
                if (rTestRes.nTotalError != 0)
                    BRDC_printf(_BRDC("channel %d\n"), i);
                for (int j = 0; j < rTestRes.nTotalError; j++) {
                    BRDC_printf(_BRDC("block %d index %d read %llu expected %llu xor %llu\n"), rTestRes.nBlock[j], rTestRes.nIndex[j], rTestRes.lReadWord[j], rTestRes.lExpectWord[j], rTestRes.lReadWord[j] ^ rTestRes.lExpectWord[j]);
                }
            }
        }
        BRDC_printf(_BRDC("\n"));
    }
}
