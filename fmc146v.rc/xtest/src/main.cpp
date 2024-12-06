#include "bardy.h"
#include "brd_dev.h"
#include "strconv.h"
#include "exceptinfo.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>

//------------------------------------------------------------------------------

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

//-----------------------------------------------------------------------------

using namespace std;

//-----------------------------------------------------------------------------

static volatile int exit_flag = 0;
void signal_handler(int signo) {
    signo = signo;
    exit_flag = 1;
    fprintf(stderr, "\n");
}

const double MB = (1024.*1024.);

//------------------------------------------------------------------------------

void dma_s2mm_test(brd_dev_t dev, unsigned block_count, unsigned block_size)
{
    time_point<high_resolution_clock> current_time, start_time;

    fprintf(stderr, "Block count: %d\n", block_count);
    fprintf(stderr, "Block  size: %d\n", block_size);

    stream_t dma = get_stream(dev, 0, BRDstrm_DIR_IN, 0);
    dev->wi(0x0, 0xC, 1 << 9);
    dma->prepare(block_count, block_size);
    dma->adjust(false);
    dma->start(true);
    dev->wi(0x0, 0x0, 0x2038);

    start_time = high_resolution_clock::now();

    while (!exit_flag) {
         if(IPC_kbhit()){
			int ch = 0 | IPC_getch();
			if(ch == 0xe0 && IPC_kbhit()){ // extended character (0xe0, xxx)
				ch = (ch<<8) | IPC_getch(); // get extended xharaxter info
			}
            switch(ch){
				case 0x1b:
                    fprintf(stderr,"ESC pressed\n");
                    return;
                    break;
			}
			fprintf(stderr,"\n");
            }

        unsigned last_block = 0, total_block = 0;
        bool ok = dma->wait_block(last_block, total_block, 1000);
        if (ok) {

            current_time = high_resolution_clock::now();
            double time_ms = duration_cast<milliseconds>(current_time - start_time).count();

            double total_MBytes = (double(total_block) * double(block_size)) / MB;
            double total_Rate = 1000.0*(total_MBytes/time_ms);

            fprintf(stderr, " CHAN%d: [%d :", dma->id(), total_block);
            for (unsigned kk = 0; kk < block_count; kk++) {
                fprintf(stderr, " %08x", *((u32*) dma->block(kk)));
            }
            fprintf(stderr, "] t %.02f Rate [MBytes/s]: %.02f, time [s]: %.02f", dev->board_temperature(), total_Rate, time_ms/1000.);
        }
        fprintf(stderr, "\r");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    dma->stop();

    dev->wi(0x0, 0xC, 0);
    dev->wi(0x0, 0x0, 0x3);
    dev->wi(0x0, 0x0, 0x3);

    fprintf(stderr, "\n");
}

//-----------------------------------------------------------------------------

void dma_mm2s_test(brd_dev_t dev, unsigned block_count, unsigned block_size)
{
    time_point<high_resolution_clock> current_time, start_time;

    fprintf(stderr, "Block count: %d\n", block_count);
    fprintf(stderr, "Block  size: %d\n", block_size);

    stream_t dma = get_stream(dev, 1, BRDstrm_DIR_OUT, 0);
    dma->prepare(block_count, block_size);
    dma->adjust(false);
    dma->start(true);

    start_time = high_resolution_clock::now();

    while (!exit_flag) {
        if(IPC_kbhit()){
			int ch = 0 | IPC_getch();
			if(ch == 0xe0 && IPC_kbhit()){ // extended character (0xe0, xxx)
				ch = (ch<<8) | IPC_getch(); // get extended xharaxter info
			}
            switch(ch){
				case 0x1b:
                    fprintf(stderr,"ESC pressed\n");
                    return;
                    break;
			}
			fprintf(stderr,"\n");
            }
        unsigned last_block = 0, total_block = 0;
        bool ok = dma->wait_block(last_block, total_block, 1000);
        if (ok) {

            current_time = high_resolution_clock::now();
            double time_ms = duration_cast<milliseconds>(current_time - start_time).count();

            double total_MBytes = (double(total_block) * double(block_size)) / MB;
            double total_Rate = 1000.0*(total_MBytes/time_ms);

            fprintf(stderr, " CHAN%d: [%d :", dma->id(), total_block);
            for (unsigned kk = 0; kk < block_count; kk++) {
                fprintf(stderr, " %08x", *((u32*) dma->block(kk)));
            }
            fprintf(stderr, "] [t]: %.02f, Rate: [MBytes/s]: %.02f, time: [s]: %.02f", dev->board_temperature(), total_Rate, time_ms/1000.);
        }
        fprintf(stderr, "\r");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    dma->stop();

    fprintf(stderr, "\n");
}

//-----------------------------------------------------------------------------

using job_t = std::shared_ptr<std::thread>;

//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    fprintf(stderr, "---------------------------\n");
    fprintf(stderr, " TEST: %s\n", argv[0]);
    fprintf(stderr, "---------------------------\n");

    setlocale(LC_ALL, "Russian");
    signal(SIGINT, signal_handler);
    fprintf(stderr, "press Esc to exit (linux)\n");
    fprintf(stderr, "press Ctrl+[ or to exit (windows + ssh)\n");
    IPC_init();
    IPC_initKeyboard();
    int lid = get_from_cmdline(argc, argv, "-b", -1);
    if(lid < 0) {
        fprintf(stderr, "Please, specify board LID!\n");
        return -1;
    }

    int in = get_from_cmdline(argc, argv, "-in", -1);
    if(in < 0) {
        fprintf(stderr, "Please, specify stream direction: -in <1|0>\n");
        return -1;
    }

    int out = get_from_cmdline(argc, argv, "-out", -1);
    if(out < 0) {
        fprintf(stderr, "Please, specify stream direction: -out <1|0>\n");
        return -1;
    }

    unsigned size = get_from_cmdline(argc, argv, "-s", 256);
    size *= 1024;

    try {

        S32 brd_count = 0;
        if (Bardy::initBardy(brd_count)) {

            brd_dev_t dev = get_device(lid);

            std::vector<job_t> jobs;
            if(in) {
                jobs.push_back( std::make_shared<std::thread>(dma_s2mm_test, dev, 4, size) );
            }

            if(out) {
                jobs.push_back( std::make_shared<std::thread>(dma_mm2s_test, dev, 4, size) );
            }

            for(auto& job : jobs) {
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
