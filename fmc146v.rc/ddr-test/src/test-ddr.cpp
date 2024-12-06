#include "strconv.h"
#include "test_buf.h"
#include "time_ipc.h"
#include <signal.h>
using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

static volatile int exit_flag = 0;
void signal_handler(int signo)
{
    signo = signo;
    exit_flag = 1;
    fprintf(stderr, "\n");
}

const double MB = (1024. * 1024.);

#define TRDIND_MODE1 0x9
#define TRDIND_MODE2 0xA
#define TRDIND_MODE3 0xB
#define TRDIND_TESTSEQUENCE 0xC
#define TRDIND_SPD_DEVICE 0x203
#define TRDIND_SPD_CTRL 0x204
#define TRDIND_SPD_ADDR 0x205
#define TRDIND_SPD_DATA 0x206
#define TRDIND_SPD_DATAH 0x206

using job_t = std::shared_ptr<std::thread>;

void check_thread(stream_t stream, U32 buffer, U32 block_in_buffer, U32 block_size_words, bool agree_mode, BRDstrm_Stub* pStub)
{
    S32 index_pc = -1, index_dma = -1, status = 0;
    U32 *buf, *block;
    while (!exit_flag) {
        if (index_pc == index_dma) {
            index_dma = pStub->lastBlock;
        }
        if (index_pc != index_dma) {
            if (index_pc + 1 >= buffer)
                index_pc = 0;
            else
                index_pc++;
            buf = (U32*)stream->block(index_pc);
            chn_start();
            for (int ii = 0; ii < block_in_buffer; ii++) {
                block = buf + ii * block_size_words;
                for(int ii=0; ii<8; ++ii) {
                    buf_check_psd512(block, block_size_words, ii);
                }
            }
            chn_step();
            if (agree_mode) {
                ipc_delay(150);
                //fprintf(stderr, "\ndma %d pc %d\n", index_dma, index_pc);
                status = stream->done(index_pc);
            }
        }
    }
}

void show_help(const std::string& program)
{
    fprintf(stderr, "usage: %s <options>\n", program.c_str());
    fprintf(stderr, "Options description:\n");
    fprintf(stderr, "-b <lid> - LID of board\n");
    fprintf(stderr, "-t <tetrad> - ddr tetrad number\n");
    fprintf(stderr, "-a - agree mode\n");
    // fprintf(stderr "-f - fifo mode"\n");
    fprintf(stderr, "-azb <addr> - active zone base addr\n");
    fprintf(stderr, "-az <size> - active zone size in Kb\n");
    fprintf(stderr, "-blocks <numb> - number of blocks for dma\n");
    fprintf(stderr, "-bsize <size> - size of block in Kb\n");
    fprintf(stderr, "-h - Print this message\n");
}

int main(int argc, char* argv[])
{
    fprintf(stderr, "---------------------------\n");
    fprintf(stderr, " TEST: %s\n", argv[0]);
    fprintf(stderr, "---------------------------\n");

    if (argc == 1) {
        show_help(argv[0]);
        return 0;
    }

    setlocale(LC_ALL, "Russian");
    signal(SIGINT, signal_handler);

    bool is_help = is_option(argc, argv, "-h");
    if (is_help) {
        show_help(argv[0]);
        return 0;
    }
    S32 ulid = get_from_cmdline(argc, argv, "-b", -1);
    S32 ddr_tetr = get_from_cmdline(argc, argv, "-t", -1);
    U32 az_base = get_from_cmdline(argc, argv, "-azb", 0);
    U32 az_size_kb = get_from_cmdline(argc, argv, "-az", 32768);
    U32 blocks_numb = get_from_cmdline(argc, argv, "-blocks", 8);
    U32 block_size_kb = get_from_cmdline(argc, argv, "-bsize", 256);
    bool is_fifo = is_option(argc, argv, "-f");
    bool agree = is_option(argc, argv, "-a");
    if (agree)
        fprintf(stderr, "agree mode ON\n");
    S32 brd_count = 0;
    try {
        if (Bardy::initBardy(brd_count)) {
            std::vector<U32> lids;
            if (!Bardy::boardsLID(lids)) {
                fprintf(stderr, "Can't get board LIDs\n");
                return -1;
            }
            brd_dev_t dev = nullptr;
            for (const auto& id : lids) {
                if (ulid == id) {
                    dev = get_device(id);
                    break;
                }
            }
            if(!dev) {
                fprintf(stderr, "Can't get device with LID\n");
                return -1;
            }
            dev->show_services();
            dev->lock_services();
            S32 maintetrad = -1;
            dev->get_trd_number(1, maintetrad);
            dev->get_trd_number(1, maintetrad);
            dev->RegPokeInd(maintetrad, 0, 1);
            ipc_delay(100);
            dev->RegPokeInd(maintetrad, 0, 0);
            ipc_delay(100);
            for (int i = 0; i < 0x30; i++)
                dev->RegPokeInd(maintetrad, i, 0);
            if (ddr_tetr == -1)
                dev->get_trd_number(0x9B, ddr_tetr);
            if (ddr_tetr == -1)
                dev->get_trd_number(0xE7, ddr_tetr);
            if (ddr_tetr < 0) {
                fprintf(stderr, "Please set DDR tetrade number in device.\n");
                return -1;
            }
            S32 val = 0;
            dev->get_trd_id(ddr_tetr, val);
            if (val <= 0) {
                fprintf(stderr, "no DDR tetrad found\n");
                return -1;
            }
            fprintf(stderr, "DDR TRD: 0x%04X\n", ddr_tetr);

            // prepare ddr4
            sdram_t sdram = get_sdram(dev, 0);
            U32 mode = is_fifo ? 1 : 2;
            sdram->prepare(az_size_kb, mode);
            sdram->set_full_rate();
            dev->RegPokeInd(ddr_tetr, TRDIND_TESTSEQUENCE, 0x100);

            if (1) {
                U32 spd_addr[] = { 2, 3, 4, 5, 12, 13 };
                U32 spd_data[sizeof(spd_addr) / sizeof(U32)] = { 0 };
                dev->RegPokeInd(ddr_tetr, 0, 1);
                dev->RegPokeInd(ddr_tetr, 0, 0);
                for (int ii = 0; ii < sizeof(spd_addr) / sizeof(U32); ii++) {
                    dev->RegPokeInd(ddr_tetr, TRDIND_SPD_ADDR, spd_addr[ii]);
                    dev->RegPokeInd(ddr_tetr, TRDIND_SPD_CTRL, 2);
                    spd_data[ii] = dev->RegPeekInd(ddr_tetr, TRDIND_SPD_DATA);
                }
                spd_data[0] &= 0xFF;
                if (spd_data[0] != 0xC) {
                    fprintf(stderr, "Wrong memory type 0x%X. Must be 0xC\n", spd_data[0]);
                    return -1;
                }
                spd_data[1] &= 0x7;
                if (spd_data[1] != 0x3) {
                    switch (spd_data[1]) {
                    case 0x0:
                        fprintf(stderr, "DDR module type \"UNDEFINED\"\r\n");
                        break;
                    case 0x1:
                        fprintf(stderr, "DDR module type \"RDIMM\"\r\n");
                        break;
                    case 0x2:
                        fprintf(stderr, "DDR module type \"UDIMM\"\r\n");
                        break;
                    case 0x3: // тип модуля (SO-DIMM - 0x3)
                        fprintf(stderr, "DDR module type \"SODIMM\"\r\n");
                        break;
                    case 0x4:
                        fprintf(stderr, "DDR module type \"LRDIMM\"\r\n");
                        break;
                    default:
                        break;
                    }
                }
                U32 sdram_capacity = 0;
                switch (spd_data[2] & 0xF) {
                case 0:
                    sdram_capacity = 256;
                    break;
                case 1:
                    sdram_capacity = 512;
                    break;
                case 2:
                    sdram_capacity = 1024;
                    break;
                case 3:
                    sdram_capacity = 2048;
                    break;
                case 4:
                    sdram_capacity = 4096;
                    break;
                case 5:
                    sdram_capacity = 8198;
                    break;
                case 6:
                    sdram_capacity = 16384;
                    break;
                case 7:
                    sdram_capacity = 32768;
                    break;
                case 8:
                    sdram_capacity = 12288; // 12Gb
                    break;
                case 9:
                    sdram_capacity = 24576; // 24Gb
                    break;
                default:;
                }
                fprintf(stderr, "DDR4 SDRAM %d Mbit\n", sdram_capacity);
                U32 bank_addr_bits = (spd_data[2] >> 4) & 0x3;
                bank_addr_bits += 2;
                fprintf(stderr, "DDR4 SDRAM %d bit address\n", bank_addr_bits);
                U32 bank_group_bits = (spd_data[2] >> 6) & 0x3;
                fprintf(stderr, "DDR4 SDRAM %d banks\n", bank_group_bits);
                U32 col_addr_bits = spd_data[3] & 0x7;
                col_addr_bits += 9;
                fprintf(stderr, "DDR4 SDRAM %d column address bits\n", col_addr_bits);
                U32 row_addr_bits = (spd_data[3] >> 3) & 0x7;
                row_addr_bits += 12;
                fprintf(stderr, "DDR4 SDRAM %d row address bits\n", row_addr_bits);
                U32 sdram_width = 0;
                switch (spd_data[4] & 0x7) {
                case 0:
                    sdram_width = 4;
                    break;
                case 1:
                    sdram_width = 8;
                    break;
                case 2:
                    sdram_width = 16;
                    break;
                case 3:
                    sdram_width = 32;
                    break;
                default:;
                }
                fprintf(stderr, "DDR4 SDRAM memory bus width %d\n", sdram_width);
                U32 ranks = (spd_data[4] >> 3) & 0x7;
                ranks += 1;
                fprintf(stderr, "DDR4 SDRAM %d ranks\n", ranks);
                U32 primary_bus_width = 0;
                switch (spd_data[5] & 0x7) {
                case 0:
                    primary_bus_width = 8;
                    break;
                case 1:
                    primary_bus_width = 16;
                    break;
                case 2:
                    primary_bus_width = 32;
                    break;
                case 3:
                    primary_bus_width = 64;
                    break;
                default:;
                }
                fprintf(stderr, "DDR4 SDRAM module bus width %d\n", primary_bus_width);
                U32 module_capacity = (sdram_capacity / 8) * (primary_bus_width / sdram_width) * ranks;
                fprintf(stderr, "DDR4 SDRAM capacity %d Mbyte\n", module_capacity);

                U32 mode1 = 0;
                mode1 |= ((row_addr_bits - 8) & 0x7) << 2;
                mode1 |= ((col_addr_bits - 8) & 0x7) << 5;
                mode1 |= (ranks == 1) ? 0 : 0x100;
                mode1 |= 0x2200;

                dev->RegPokeInd(ddr_tetr, TRDIND_MODE1, mode1);
                fprintf(stderr, "Waiting for DDR init\n");
                int attempt;
                int attempt_max = 100;
                U32 status;
                for (attempt = 0; attempt < attempt_max; attempt++) {
                    status = dev->RegPeekDir(ddr_tetr, 0);
                    if (status & 0x800)
                        break;
                    else
                        ipc_delay(100);
                }
                if (attempt == attempt_max) {
                    fprintf(stderr, "ERROR: DDR not initialized\n");
                    return -1;
                }

                if (is_fifo) {
                    U32 mode2 = 0x10; // FIFO_MODE
                    dev->RegPokeInd(ddr_tetr, TRDIND_MODE2, mode2);
                    fprintf(stderr, "FIFO mode\n");
                } else {
                    dev->RegPokeInd(ddr_tetr, TRDIND_MODE2, 0);
                    U32 azbase_reg_l = az_base & 0xFFFF;
                    U32 azbase_reg_h = (az_base >> 16) & 0xFFFF;
                    U32 AzEnd = az_base + az_size_kb * (1024 / 4) - 1;
                    U32 SdramAzSizeOfKb = (AzEnd + 1 - az_base) / (256);
                    fprintf(stderr, "Active zone start 0x%08X\n", az_base);
                    fprintf(stderr, "Active zone end 0x%08X\n", AzEnd);
                    fprintf(stderr, "Active zone size %d Mbyte\n", SdramAzSizeOfKb / 1024);
                    dev->RegPokeInd(ddr_tetr, 0x10, az_base & 0xFFFF);
                    dev->RegPokeInd(ddr_tetr, 0x11, az_base >> 16);
                    dev->RegPokeInd(ddr_tetr, 0x0E, AzEnd & 0xFFFF);
                    dev->RegPokeInd(ddr_tetr, 0x0F, AzEnd >> 16);
                }
                dev->RegPokeInd(ddr_tetr, TRDIND_MODE3, 1 << 5);
                dev->RegPokeInd(ddr_tetr, TRDIND_TESTSEQUENCE, 0x100);
            }

            block_size_kb *= 1024;

            stream_t stream = get_stream(dev, 0, BRDstrm_DIR_IN, ddr_tetr);
            stream->prepare(blocks_numb, block_size_kb);
            stream->adjust(agree);
            BRDstrm_Stub* pStub = nullptr;
            stream->stub(&pStub);
            buf_check_start(32, 64);
            sdram->reset_fifo();
            sdram->enable();
            stream->reset_fifo();
            stream->start(true);
            fprintf(stderr, "press Esc to exit (linux)\n");
            fprintf(stderr, "press Ctrl+[ or to exit (windows + ssh)\n");
            U32 block_ok = 0, block_err = 0;
            ipc_time_t start, current;
            std::vector<job_t> jobs;
            jobs.push_back(std::make_shared<std::thread>(check_thread, stream, blocks_numb, 1, block_size_kb >> 2, agree, pStub));
            int c = 0;
            IPC_init();
            IPC_initKeyboard();
            start = ipc_get_time();

            auto start_prog = high_resolution_clock::now();
            while (!exit_flag) {

            if(IPC_kbhit()){
			    int ch = 0 | IPC_getch();
			    if(ch == 0xe0 && IPC_kbhit()){ // extended character (0xe0, xxx)
				    ch = (ch<<8) | IPC_getch(); // get extended xharaxter info
			}
            switch(ch){
				case 0x1b:
                    //cleanup
                    fprintf(stderr,"\nESC pressed,exiting\n");
                    exit_flag = 1;
                    for (auto& job : jobs) {
                    job->join();
                     }
                    stream->stop();
                    sdram->disable();
                    if (block_err != 0) {
                    for(int ii=0; ii<8; ++ii) {
                        fprintf(stderr, _BRDC("\nСписок ошибок %d: \n"), ii);
                        fprintf(stderr, _BRDC("%s\n"), report_word_error(ii));
                }
            }
            stream->cleanup();
                    return 0;
                    break;
				break;
			}
			fprintf(stderr,"\n");
            }



                check_result(&block_ok, &block_err, nullptr, nullptr, nullptr);
                current = ipc_get_time();
                double time_ms = ipc_get_difftime(start, current);
                double total_MBytes = (double(pStub->totalCounter) * double(block_size_kb)) / MB;
                double total_Rate = 1000.0 * (total_MBytes / time_ms);
                auto current_cycle = high_resolution_clock::now();
                auto duration = duration_cast<seconds>(current_cycle - start_prog);
                fprintf(stderr, "time=%lld, %.3d total blocks %.3d \tOK %.3d \tERR %.2d \tspeed %.2f Mb/sec\r",duration, c, pStub->totalCounter, block_ok, block_err, total_Rate);
                c++;
                ipc_delay(250);
            }
            fprintf(stderr, "\n");
            //cleanup
            for (auto& job : jobs) {
                job->join();
            }
            stream->stop();
            sdram->disable();
            if (block_err != 0) {
                for(int ii=0; ii<8; ++ii) {
                    fprintf(stderr, _BRDC("\nСписок ошибок %d: \n"), ii);
                    fprintf(stderr, _BRDC("%s\n"), report_word_error(ii));
                }
            }
            stream->cleanup();
        }
    } catch (const except_info_t& errInfo) {
        fprintf(stderr, "%s", errInfo.info.c_str());
    } catch (...) {
        fprintf(stderr, "%s", "Unknown exception in the program!");
    }
}
