#include "bardy.h"
#include "brd_dev.h"
#include "exceptinfo.h"
#include "strconv.h"
#include "test_buf.h"
#include "time_ipc.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <math.h>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <string>
#include <sys/time.h>
#include <thread>
#include <unistd.h>

//------------------------------------------------------------------------------

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

//-----------------------------------------------------------------------------

using namespace std;

//-----------------------------------------------------------------------------

static volatile int exit_flag = 0;
void signal_handler(int signo)
{
    signo = signo;
    exit_flag = 1;
    fprintf(stderr, "\n");
}

const double MB = (1024. * 1024.);

//------------------------------------------------------------------------------

void dma_s2mm_rate(brd_dev_t dev, unsigned block_count, unsigned block_size)
{
    ipc_time_t current_time, start_time;
    U32 total_blocks = 0;

    fprintf(stderr, "Block count: %d\n", block_count);
    fprintf(stderr, "Block  size: %d\n", block_size);
    fprintf(stderr, "\n");

    stream_t dma = get_stream(dev, 0, BRDstrm_DIR_IN, 0);

    dma->prepare(block_count, block_size);

    BRDctrl_StreamStub* stub = dma->stub();
    if (!stub) {
        exit_flag = 1;
        return;
    }

    dma->adjust(false);

    dma_blocks_t data_blocks = dma->blocks();

    dma->start(true);

    // запись в регистры тетрады MAIN
    dev->wi(0x0, 0xC, 1);
    dev->wi(0x0, 0x0, 2);
    ipc_delay(1);
    dev->wi(0x0, 0x0, 0);
    ipc_delay(1);
    dev->wi(0x0, 0xC, 1 << 9);
    dev->wi(0x0, 0x0, 0x2038);

    start_time = ipc_get_time();

    while (!exit_flag) {

        total_blocks = dma->stub()->totalCounter;

        current_time = ipc_get_time();
        double time_ms = ipc_get_difftime(start_time, current_time);

        double total_MBytes = (double(total_blocks) * double(block_size)) / MB;
        double total_Rate = 1000.0 * (total_MBytes / time_ms);

        fprintf(stderr, "STUB [%06d, %06d] t,C = %.2f, Rate [%.2f] MB/s",
            dma->stub()->totalCounter, dma->stub()->lastBlock,
            dev->board_temperature(), total_Rate);

        fprintf(stderr, " - DONE\r");

        ipc_delay(250);
    }

    dma->stop();

    // запись в регистры тетрады MAIN
    dev->wi(0x0, 0xC, 0);
    dev->wi(0x0, 0x0, 0x3);
    dev->wi(0x0, 0x0, 0x0);

    fprintf(stderr, "\n");
}

//------------------------------------------------------------------------------

void dma_s2mm_check(brd_dev_t dev, unsigned block_count, unsigned block_size)
{
    ipc_time_t current_time, start_time;
    U32 block_size_words = block_size >> 2;
    S32 index_pc = -1, index_dma = -1;
    U32 block_ok = 0, block_err = 0, total_blocks = 0;

    fprintf(stderr, "Block count: %d\n", block_count);
    fprintf(stderr, "Block  size: %d\n", block_size);
    fprintf(stderr, "\n");

    buf_check_start(32, 64);

    stream_t dma = get_stream(dev, 0, BRDstrm_DIR_IN, 0);

    dma->prepare(block_count, block_size);

    BRDctrl_StreamStub* stub = dma->stub();
    if (!stub) {
        exit_flag = 1;
        return;
    }

    dma_blocks_t data_blocks = dma->blocks();

    dma->adjust(true);
    dma->start(true);

    // запись в регистры тетрады MAIN
    dev->wi(0x0, 0xC, 1);
    dev->wi(0x0, 0x0, 2);
    ipc_delay(1);
    dev->wi(0x0, 0x0, 0);
    ipc_delay(1);
    dev->wi(0x0, 0xC, 1 << 9);
    dev->wi(0x0, 0x0, 0x2038);

    start_time = ipc_get_time();

    while (!exit_flag) {

        total_blocks = dma->stub()->totalCounter;

        if (index_pc == index_dma) {
            index_dma = stub->lastBlock;
        }

        if (index_pc != index_dma) {

            if (index_pc + 1 >= (S32)block_count)
                index_pc = 0;
            else
                index_pc++;

            U32* data_block = static_cast<U32*>(data_blocks.at(index_pc));
            chn_start();
            buf_check_psd256(data_block, block_size_words, 0);
            buf_check_psd256(data_block, block_size_words, 1);
            buf_check_psd256(data_block, block_size_words, 2);
            buf_check_psd256(data_block, block_size_words, 3);
            chn_step();

            check_result(&block_ok, &block_err, nullptr, nullptr, nullptr);

            current_time = ipc_get_time();
            double time_ms = ipc_get_difftime(start_time, current_time);

            double total_MBytes = (double(total_blocks) * double(block_size)) / MB;
            double total_Rate = 1000.0 * (total_MBytes / time_ms);

            fprintf(stderr, "STUB [%06d, %06d] pc = %06d, dma = %06d, ok: %06d err: %06d, Rate [%.2f] MB/s",
                dma->stub()->totalCounter, dma->stub()->lastBlock,
                index_pc, index_dma, block_ok, block_err, total_Rate);

            ipc_delay(1);

            dma->done(index_pc);

            fprintf(stderr, " - DONE\r");
        }
    }

    dma->stop();

    // запись в регистры тетрады MAIN
    dev->wi(0x0, 0xC, 0);
    dev->wi(0x0, 0x0, 0x3);
    dev->wi(0x0, 0x0, 0x0);

    if (block_err != 0) {
        BRDC_printf(_BRDC("\nСписок ошибок 0: \n"));
        BRDC_printf(_BRDC("%s\n"), report_word_error(0));

        BRDC_printf(_BRDC("\nСписок ошибок 1: \n"));
        BRDC_printf(_BRDC("%s\n"), report_word_error(1));

        BRDC_printf(_BRDC("\nСписок ошибок 2: \n"));
        BRDC_printf(_BRDC("%s\n"), report_word_error(2));

        BRDC_printf(_BRDC("\nСписок ошибок 3: \n"));
        BRDC_printf(_BRDC("%s\n"), report_word_error(3));
    }

    fprintf(stderr, "\n");
}

//-----------------------------------------------------------------------------

using job_t = std::shared_ptr<std::thread>;

//-----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    fprintf(stderr, "---------------------------\n");
    fprintf(stderr, " TEST: %s\n", argv[0]);
    fprintf(stderr, "---------------------------\n");

    setlocale(LC_ALL, "Russian");
    signal(SIGINT, signal_handler);

    int lid = get_from_cmdline(argc, argv, "-b", -1);
    if (lid < 0) {
        fprintf(stderr, "Please, specify board LID!\n");
        return -1;
    }

    unsigned blocks_count = get_from_cmdline(argc, argv, "-n", 4);
    unsigned block_size = get_from_cmdline(argc, argv, "-s", 256);
    block_size *= 1024;

    bool is_check = is_option(argc, argv, "-c");
    bool is_rate = is_option(argc, argv, "-r");

    if (!is_check && !is_rate) {
        fprintf(stderr, "Please, specify stream data rate or check option: -r or -c>\n");
        return -1;
    }

    if (is_check) {
        if (blocks_count < 8) {
            blocks_count = 8;
        }
        is_rate = false;
    }

    try {

        S32 brd_count = 0;
        if (Bardy::initBardy(brd_count)) {

            brd_dev_t dev = get_device(lid);

            std::vector<job_t> jobs;
            if (is_rate) {
                jobs.push_back(std::make_shared<std::thread>(dma_s2mm_rate, dev, blocks_count, block_size));
            }

            if (is_check) {
                jobs.push_back(std::make_shared<std::thread>(dma_s2mm_check, dev, blocks_count, block_size));
            }

            ipc_delay(50);

            for (auto& job : jobs) {
                job->join();
            }
        }

    } catch (const except_info_t& errInfo) {

        fprintf(stderr, "%s", errInfo.info.c_str());

    } catch (...) {

        fprintf(stderr, "%s", "Unknown exception in the program!");
    }

    return 0;
}

//------------------------------------------------------------------------------
