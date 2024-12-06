#include "bardy.h"
#include "brd_dev.h"
#include "strconv.h"
#include "exceptinfo.h"

#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <memory>
#include <mutex>
#include <csignal>
#include <bitset>

//------------------------------------------------------------------------------

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

//-----------------------------------------------------------------------------

static volatile int exit_flag = 0;
void signal_handler(int signo) {
    signo = signo;
    exit_flag = 1;
    fprintf(stderr, "\n");
}

const double MB = (1024.*1024.);

//------------------------------------------------------------------------------

uint8_t reverse_bits(uint8_t n)
{
    uint8_t ans = 0;
    for(int i = 7; i >= 0; i--) {
        ans |= (n & 1) <<i;
        n >>= 1;
    }
    return ans;
}

//-----------------------------------------------------------------------------

template <typename T>
bool read_bitstream_file(const std::string& fname, bool is_swap, std::vector<T>& data)
{
    fprintf(stderr, "File: %s\n", fname.c_str());

    std::fstream ifs(fname.c_str(), std::ios_base::binary | std::ios::in);
    if(!ifs.is_open()) {
        throw except_info("%s, %d: %s() Can't open configuration file: %s\n", __FILE__, __LINE__, __func__, fname.c_str());
    }

    ifs.seekg(0, ios::end);
    ssize_t fsize = ifs.tellg();
    ifs.seekg(0, ios::beg);

    fprintf(stderr, "File size: %ld\n", fsize);

    data.clear();
    while(!ifs.eof()) {
        T value = 0;
        ifs.read(reinterpret_cast<char*>(&value), sizeof(T));
        if(is_swap) {
            uint8_t lsb = reverse_bits(uint8_t(value));
            uint8_t msb = reverse_bits(value >> 8);
            uint16_t reverted_val = msb << 8 | lsb;
            data.push_back(reverted_val);
        } else {
            data.push_back(value);
        }
    }

    return !data.empty();
}

const uint32_t FPGA_CONFIG_TRD_ID = 0x12C;

//-----------------------------------------------------------------------------
// Регистра статус тетрады
const uint32_t TRD_STATUS_REG = 0x0;

// Биты регистра статус у тетрады
const uint32_t STATUS_CMD_RDY = 0x1;
const uint32_t STATUS_DONE1   = 0x2;
const uint32_t STATUS_DONE2   = 0x4;
const uint32_t STATUS_INIT1   = 0x8;
const uint32_t STATUS_INIT2   = 0x10;

// Регистры конфигурации тетрады
const uint32_t TRD_CFG_MODE  = 0x9;
const uint32_t TRD_CFG_MODE_FPGA1  = 0x2;
const uint32_t TRD_CFG_MODE_FPGA2  = 0x4;

const uint32_t TRD_CFG_REG   = 0xB;
const uint32_t TRD_CFG_REG_PROG1   = 0x2;
const uint32_t TRD_CFG_REG_PROG2   = 0x4;

const uint32_t TRD_CFG_DATA  = 0xA;
//-----------------------------------------------------------------------------

bool wait_status_bits_set(brd_dev_t dev, uint32_t mask, uint32_t trd_idx, uint32_t timeout)
{
    time_point<high_resolution_clock> current_time, start_time;
    start_time = high_resolution_clock::now();
    U32 status = dev->rd(trd_idx, TRD_STATUS_REG);
    while (!(status & mask)) {
        status = dev->rd(trd_idx, TRD_STATUS_REG);
        current_time = high_resolution_clock::now();
        double time_ms = duration_cast<milliseconds>(current_time - start_time).count();
        if(time_ms > timeout) {
            return false;
        }
    }
    return true;
}

//-----------------------------------------------------------------------------

bool wait_status_bits_clear(brd_dev_t dev, uint32_t mask, uint32_t trd_idx, uint32_t timeout)
{
    time_point<high_resolution_clock> current_time, start_time;
    start_time = high_resolution_clock::now();
    U32 status = dev->rd(trd_idx, TRD_STATUS_REG);
    while (status & mask) {
        status = dev->rd(trd_idx, TRD_STATUS_REG);
        current_time = high_resolution_clock::now();
        double time_ms = duration_cast<milliseconds>(current_time - start_time).count();
        if(time_ms > timeout) {
            return false;
        }
    }
    return true;
}

//-----------------------------------------------------------------------------

bool write_bitstream_data(brd_dev_t dev, S32 trd_idx, U32 fpga_mask, std::vector<uint16_t>& data)
{
    U32 mask = 0;
    U32 done_mask = 0;
    U32 init_mask = 0;

    if(fpga_mask & 0x1) {
        mask |= TRD_CFG_REG_PROG1;
        done_mask |= STATUS_DONE1;
        init_mask |= STATUS_INIT1;
    }

    if(fpga_mask & 0x2) {
        mask |= TRD_CFG_REG_PROG2;
        done_mask |= STATUS_DONE2;
        init_mask |= STATUS_INIT2;
    }
fprintf(stderr, "Error wait TRD_STATUS_REG[CMD_RDY]!\n");
    // 1. Ожидаем готовность бита CMD_RDY
    bool status = wait_status_bits_set(dev, fpga_mask, idx, 1000);
    if(!status) {
        fprintf(stderr, "Error wait TRD_STATUS_REG[CMD_RDY]!\n");
        return false;
    }

    // 2. Записываем в регистр TRD_CFG_REG
    dev->wi(trd_idx, TRD_CFG_REG, 0);

    // 3. Записываем в регистр TRD_CFG_MODE
    dev->wi(trd_idx, TRD_CFG_MODE, mask);

    // 4. Ожидаем готовность битов STATUS_DONEx | STATUS_INITx
    status = wait_status_bits_clear(dev, (done_mask|init_mask), trd_idx, 1000);
    if(!status) {
        fprintf(stderr, "Error wait TRD_STATUS_REG[STATUS_DONEx | STATUS_INITx]!\n");
        return false;
    }

    // 5. Записываем в регистр TRD_CFG_REG
    dev->wi(trd_idx, TRD_CFG_REG, mask);

    // 6. Ожидаем готовность бита STATUS_INITx
    status = wait_status_bits_set(dev, init_mask, trd_idx, 1000);
    if(!status) {
        fprintf(stderr, "Error wait TRD_STATUS_REG[STATUS_INITx]!\n");
        return false;
    }

    // Записываем данные в регистр TRD_CFG_DATA по 16 бит
    double step = 1.0/float(data.size());
    double percent = 0;
    double time_s = 0;
    uint32_t idx = 0;
    time_point<high_resolution_clock> start_time = high_resolution_clock::now();
    for(const auto& value : data) {

        // 8. Ожидаем готовность бита CMD_RDY
        bool status = wait_status_bits_set(dev, 0x1, trd_idx, 1000);
        if(!status) {
            fprintf(stderr, "Error wait TRD_STATUS_REG[CMD_RDY]!\n");
            return false;
        }

        ++idx;
        percent = 100.0 * idx * step;

        // 7. Записываем в регистр TRD_CFG_DATA, меняя биты местами (побайтно)
        dev->wi(trd_idx, TRD_CFG_DATA, value);

        if((idx%4096) == 0) {
            time_point<high_resolution_clock> current_time = high_resolution_clock::now();
            time_s = duration_cast<seconds>(current_time - start_time).count();
            fprintf(stderr, "Status: [ %.2f %% ] -- Total bytes: [ %d ] -- Time: %.2f s\r", percent, 2*idx, time_s);
        }

        if(exit_flag) {
            break;
        }
    }

    time_point<high_resolution_clock> current_time = high_resolution_clock::now();
    time_s = duration_cast<seconds>(current_time - start_time).count();
    fprintf(stderr, "Status: [ %.2f %% ] -- Total bytes: [ %d ] -- Time: %.2f s\r", percent, 2*(idx-1), time_s);

    if(!exit_flag) {

        // 9. Ожидаем готовность битов STATUS_DONEx | STATUS_INITx
        status = wait_status_bits_set(dev, (done_mask|init_mask), trd_idx, 1000);
        if(!status) {
            fprintf(stderr, "Error wait TRD_STATUS_REG[DONEx:INITx]!\n");
            return false;
        }

        // 10. Записываем в регистр TRD_CFG_MODE запрет конфигурации
        dev->wi(trd_idx, TRD_CFG_MODE, 0);
    }

    fprintf(stderr, exit_flag ? "\nStatus: INTERRUPTED\n" : "\nStatus: SUCCESS\n");

    return true;
}

//-----------------------------------------------------------------------------

bool fpga_load_bit(brd_dev_t dev, S32 trd_idx, U32 fpga_mask, bool is_swap, const std::string& fname)
{
    bool status = false;
    try {
        std::vector<uint16_t> file_data;
        status = read_bitstream_file(fname, is_swap, file_data);
        if(status && !file_data.empty()) {
           status = write_bitstream_data(dev, trd_idx, fpga_mask, file_data);
        }
    } catch(const except_info_t& e) {
        fprintf(stderr, "%s(): %s\n", __func__, e.info.c_str());
    } catch(...) {
        fprintf(stderr, "%s(): Unknown exception in the program!\n", __func__);
    }
    return status;
}

//-----------------------------------------------------------------------------

using job_t = std::shared_ptr<std::thread>;

//-----------------------------------------------------------------------------

void show_help(const std::string& program)
{
    fprintf(stderr, "usage: %s <options>\n", program.c_str());
    fprintf(stderr, "Options description:\n");
    fprintf(stderr, "-b <lid> - LID of board for programming\n");
    fprintf(stderr, "-mask <0x1 | 0x2 | 0x3> - Mask of FPGA for configuration\n");
    fprintf(stderr, "-swap <0 | 1> - Reverse bits in configuration file data\n");
    fprintf(stderr, "-f <filename> - Name of FPGA configuration file\n");
    fprintf(stderr, "-h - Print this message\n");
}

//-----------------------------------------------------------------------------

int BRDC_main(int argc, char *argv[])
{
    fprintf(stderr, "---------------------------\n");
    fprintf(stderr, " TEST: %s\n", argv[0]);
    fprintf(stderr, "---------------------------\n");

    //setlocale(LC_ALL, "Russian");
    signal(SIGINT, signal_handler);

    if(argc == 1) {
        show_help(argv[0]);
        return 0;
    }

    // Пользователь может указать LID модуля который выполняет конфигурацию
    bool is_help = is_option(argc, argv, "-h");
    if(is_help) {
        show_help(argv[0]);
    }

    // Пользователь может указать LID модуля который выполняет конфигурацию
    U32 ulid = get_from_cmdline(argc, argv, "-b", -1);

    // Пользователь может указать маску FPGA которые программировать одним файлом
    U32 fpga_mask = get_from_cmdline(argc, argv, "-mask", 0x1);

    // Пользователь может указать LID модуля
    U32 uswap = get_from_cmdline(argc, argv, "-swap", 1);
    bool is_swap = uswap ? true : false;

    // Имя файла конфигурации
    std::string fname;
    bool is_bitstream = get_from_cmdline<std::string>(argc, argv, "-f", "", fname);
    if(!is_bitstream) {
        fprintf(stderr, "Please, specify bitstream file name: -f data.bit\n");
        return -1;
    }

    try {

        S32 brd_count = 0;
        if (Bardy::initBardy(brd_count)) {

            std::vector<U32> lids;
            if(!Bardy::boardsLID(lids)) {
                fprintf(stderr, "Can't get board LIDs\n");
                return -1;
            }

            S32 trd_idx = -1;
            brd_dev_t dev = nullptr;
            for(const auto& id : lids) {
                dev = get_device(id);
                if(!dev->get_trd_number(0x12C, trd_idx)) {
                    //fprintf(stderr, "Board LID%d: FPGA_CONFIG_TRD: 0x%0X not fuond!\n", id, FPGA_CONFIG_TRD_ID);
                    continue;
                } else {
                    // Пользователь не указал LID модуля, берем первый найденный
                    if(ulid < 0) {
                        break;
                    } else {
                        // Нашли плату заданную пользователем
                        if(ulid == id) {
                            break;
                        }
                    }
                }
            }

            fprintf(stderr, "LID%d: FPGA_CONFIG_TRD: 0x%0X, TRD_IDX: 0x%X\n", dev->board_lid(), FPGA_CONFIG_TRD_ID, trd_idx);

            if(!dev) {
                fprintf(stderr, "FPGA_CONFIG_TRD: 0x%0X not fuond on any board\n", FPGA_CONFIG_TRD_ID);
                return -1;
            }

            std::vector<job_t> jobs;
            if(is_bitstream) {
                jobs.push_back( std::make_shared<std::thread>(fpga_load_bit, dev, trd_idx, fpga_mask, is_swap, fname) );
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

        unsigned last_block = 0, total_block = 0;
        bool ok = dma->wait_block(last_block, total_block, 1000);
        if (ok) {

            current_time = high_resolution_clock::now();
            double time_ms = duration_cast<milliseconds>(current_time - start_time).count();

            double total_MBytes = (double(total_block) * double(block_size)) / MB;
            double total_Rate = 1000.0*(total_MBytes/time_ms);

            fprintf(stderr, " CHAN%d: [%d :", dma->id(), total_block);
            for (unsigned kk = 0; kk < block_count; kk++) {
                fprintf(stderr, " %08x", *((uint32_t*) dma->block(kk)));
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

        unsigned last_block = 0, total_block = 0;
        bool ok = dma->wait_block(last_block, total_block, 1000);
        if (ok) {

            current_time = high_resolution_clock::now();
            double time_ms = duration_cast<milliseconds>(current_time - start_time).count();

            double total_MBytes = (double(total_block) * double(block_size)) / MB;
            double total_Rate = 1000.0*(total_MBytes/time_ms);

            fprintf(stderr, " CHAN%d: [%d :", dma->id(), total_block);
            for (unsigned kk = 0; kk < block_count; kk++) {
                fprintf(stderr, " %08x", *((uint32_t*) dma->block(kk)));
            }
            fprintf(stderr, "] t %.02f Rate [MBytes/s]: %.02f, time [s]: %.02f", dev->board_temperature(), total_Rate, time_ms/1000.);
        }
        fprintf(stderr, "\r");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    dma->stop();

    fprintf(stderr, "\n");
}

//-----------------------------------------------------------------------------
