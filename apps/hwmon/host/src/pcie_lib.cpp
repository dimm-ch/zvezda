//********************************************************
//
// Пример приложения, 
//   получающего информацию о модуле, 
//   программируемых устройствах на нем (ПЛИС, ППЗУ...)
//   и BARDY-службах (стримы, АЦП, ЦАП...)
//
// (C) InSys, 2007-2015
//
//********************************************************

#include "gipcy.h"
#include "strconv.h"
#include "pcie_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>

//-----------------------------------------------------------------------------

using namespace std;

//-----------------------------------------------------------------------------
// Адреса в памяти Zynq для обмена данными с HOST через PCIe для программы emhwmon
const uint32_t ZYNQ_BASE_ADDR            = 0x20000000;
const uint32_t HOST_DATA_SIZE            = 0x4000000;
const uint32_t CARD_DATA_SIZE            = 0x4000000;
const uint32_t HOST_CMD_SIZE             = 0x100000;
const uint32_t CARD_CMD_SIZE             = 0x100000;
const uint32_t CARD_STATUS_SIZE          = 0x100000;
const uint32_t HOST_WINDOW_SIZE          = 0x100000;

//-----------------------------------------------------------------------------

const uint32_t HOST_DATA          = 0x0;
const uint32_t CARD_DATA          = HOST_DATA + HOST_DATA_SIZE;
const uint32_t HOST_CMD           = CARD_DATA + CARD_DATA_SIZE;
const uint32_t CARD_CMD           = HOST_CMD + HOST_CMD_SIZE;
const uint32_t CARD_STATUS        = CARD_CMD + CARD_CMD_SIZE;

//-----------------------------------------------------------------------------
// Адреса в памяти Zynq для обмена данными с HOST через PCIe
const uint32_t H2C_SIZE			  = 0x100000;
const uint32_t C2H_SIZE			  = 0x100000;
const uint32_t H2C_DATA			  = 0x30400000;
const uint32_t C2H_DATA			  = H2C_DATA + H2C_SIZE;

//-----------------------------------------------------------------------------

enum {
    host_data_id = 0,
    card_data_id,
    host_cmd_id,
    card_cmd_id,
    card_status_id,
    h2c_data_id,
    c2h_data_id,
    total_windows,
};

//-----------------------------------------------------------------------------

uint32_t windows_id[] = {
    ZYNQ_BASE_ADDR + HOST_DATA,
    ZYNQ_BASE_ADDR + CARD_DATA,
    ZYNQ_BASE_ADDR + HOST_CMD,
    ZYNQ_BASE_ADDR + CARD_CMD,
    ZYNQ_BASE_ADDR + CARD_STATUS,
    H2C_DATA,
    C2H_DATA,
};

//-----------------------------------------------------------------------------

enum {
    cmd_list = 77,
    cmd_read,
    cmd_write,
    cmd_clear,
};

//-----------------------------------------------------------------------------
/*
 Поддерживаемые команды монитора
 1) Формат команды для записи файла:
    CMD_CODE:WRITE\0
    CMD_NAME:<filename>\0
    CMD_SIZE:<size>\0
    Формат ответа на команду записи файла:
    CMD_INFO:WRITE:<filename>:SIZE:<size>:[SUCCESS|ERROR]\0
 2) Формат команды для чтения файла:
    CMD_CODE:READ\0
    CMD_NAME:<filename>\0
    Формат ответа на команду чтения файла:
    CMD_INFO:READ:<filename>:SIZE:<size>:[SUCCESS|ERROR]\0
 3) Формат команды для чтения списка файлов в директории:
    CMD_CODE:LIST\0
    Формат ответа на команду чтения списка файлов в директории:
    CMD_INFO:LIST:SIZE:<size>:[SUCCESS|ERROR]\0
 4) Формат команды для удаления файлов в директории:
    CMD_CODE:CLEAR\0
    Формат ответа на команду удаления файлов в директории:
    CMD_INFO:CLEAR:[SUCCESS|ERROR]\0

    В случае ошибки, в строке CMD_INFO: возвращается
    дополнительная информация с комментарием.

    Рабочий каталог для сохранения файлов:
    "/run/media/mmcblk0p1/"
    Желательно не использовать команду CLEAR
*/
//-----------------------------------------------------------------------------

void hostmon_pcie::write_h2c_data(uint32_t *data, uint32_t size, bool isRawData)
{
    // настроим смещение окна pcie2axi на базовый адрес блока данных - h2c
    uint32_t window = windows_id[h2c_data_id];

    uint32_t n_words = (size >> 2);
    uint32_t n_window = (HOST_WINDOW_SIZE >> 2);
    // выберем меньший из размеров (окно или рамер буфера данных)
    uint32_t n_write_size = (n_window > n_words) ? n_words : n_window;

    uint32_t n_step = (n_words / n_window);
    n_step += (((size%HOST_WINDOW_SIZE)) ? 1 : 0);

    uint32_t offset = 0;
    uint32_t nHeaderShift = 0;
    if (!isRawData) {
        nHeaderShift = 4;
    }

    for (uint32_t jj = 0; jj<n_step; jj++) {

        int32_t zynq_addr = window + (HOST_WINDOW_SIZE * jj);
        m_pcie_axi_block[AXI_ADDR_HIGH] = zynq_addr;

        for (uint32_t ii = 0; ii<n_write_size; ii++, offset++) {
            m_pcie2axi[ii] = data[offset /*+ nHeaderShift*/];
            if (offset >= n_words) {
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------

void hostmon_pcie::read_c2h_data(uint32_t *data, uint32_t size, bool isRawData)
{
    // настроим смещение окна pcie2axi на базовый адрес блока данных - h2c
    uint32_t window = windows_id[c2h_data_id];

    uint32_t n_words = (size >> 2);
    uint32_t n_window = (HOST_WINDOW_SIZE >> 2);

    // выберем меньший из размеров (окно или рамер буфера данных)
    uint32_t n_read_size = (n_window > n_words) ? n_words : n_window;

    uint32_t n_step = (n_words / n_window);
    n_step += (((size%HOST_WINDOW_SIZE)) ? 1 : 0);

    uint32_t offset = 0;
    uint32_t nHeaderShift = 0;
    if (!isRawData) {
        nHeaderShift = 4;
    }

    for (uint32_t jj = 0; jj<n_step; jj++) {

        int32_t zynq_addr = window + (HOST_WINDOW_SIZE * jj);
        m_pcie_axi_block[AXI_ADDR_HIGH] = zynq_addr;

        for (uint32_t ii = 0; ii<n_read_size; ii++, offset++) {
            data[offset] = m_pcie2axi[ii + nHeaderShift];
            if (offset >= n_words) {
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------

void hostmon_pcie::write_host_data(uint32_t *data, uint32_t size)
{
    // настроим смещение окна pcie2axi на базовый адрес блока данных - h2c
    uint32_t window = windows_id[host_data_id];

    uint32_t n_words = (size >> 2);
    uint32_t n_window = (HOST_WINDOW_SIZE >> 2);
    // выберем меньший из размеров (окно или рамер буфера данных)
    uint32_t n_write_size = (n_window > n_words) ? n_words : n_window;

    uint32_t n_step = (n_words/n_window);
    n_step += (((size%HOST_WINDOW_SIZE)) ? 1 : 0);

    uint32_t offset = 0;

    for(uint32_t jj=0; jj<n_step; jj++) {

        int32_t zynq_addr = window + (HOST_WINDOW_SIZE * jj);
        m_pcie_axi_block[AXI_ADDR_HIGH] = zynq_addr;

        for(uint32_t ii=0; ii<n_write_size; ii++, offset++) {
            m_pcie2axi[ii] = data[offset];
            if(offset >= n_words) {
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------

void hostmon_pcie::read_card_data(uint32_t *data, uint32_t size)
{
    // настроим смещение окна pcie2axi на базовый адрес блока данных - h2c
    uint32_t window = windows_id[card_data_id];

    uint32_t n_words = (size >> 2);
    uint32_t n_window = (HOST_WINDOW_SIZE >> 2);

    // выберем меньший из размеров (окно или рамер буфера данных)
    uint32_t n_read_size = (n_window > n_words) ? n_words : n_window;

    uint32_t n_step = (n_words/n_window);
    n_step += (((size%HOST_WINDOW_SIZE)) ? 1 : 0);

    uint32_t offset = 0;

    for(uint32_t jj=0; jj<n_step; jj++) {

        int32_t zynq_addr = window + (HOST_WINDOW_SIZE * jj);
        m_pcie_axi_block[AXI_ADDR_HIGH] = zynq_addr;

        for(uint32_t ii=0; ii<n_read_size; ii++, offset++) {
            data[offset] = m_pcie2axi[ii];
            if(offset >= n_words) {
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------

uint32_t hostmon_pcie::hertbeat_counter()
{
    // настроим смещение окна pcie2axi на базовый адрес блока данных - h2c
    m_pcie_axi_block[AXI_ADDR_HIGH] = windows_id[card_status_id];
    return m_pcie2axi[0];
}

//-----------------------------------------------------------------------------

void hostmon_pcie::clear_card_ans()
{
    // настроим смещение окна pcie2axi на базовый адрес блока команд c2h
    uint32_t window = windows_id[card_cmd_id];
    m_pcie_axi_block[AXI_ADDR_HIGH] = window;

    for(int ii=0; ii<(1024>>2); ii++) {
        m_pcie2axi[ii] = '\0';
    }
}

//-----------------------------------------------------------------------------

void hostmon_pcie::clear_card_data()
{
    // настроим смещение окна pcie2axi на базовый адрес блока команд c2h
    uint32_t window = windows_id[card_data_id];
    m_pcie_axi_block[AXI_ADDR_HIGH] = window;

    for(int ii=0; ii<(1024>>2); ii++) {
        m_pcie2axi[ii] = '\0';
    }
}

//-----------------------------------------------------------------------------

void hostmon_pcie::write_cmd(const char* cmd, const char* fname, const char* size)
{
    // перед запиьсю команды очистим поле ответа
    clear_card_ans();

    // перед запиьсю команды очистим поле данных
    clear_card_data();

    // настроим смещение окна pcie2axi на базовый адрес блока данных - h2c
    uint32_t window = windows_id[host_cmd_id];
    m_pcie_axi_block[AXI_ADDR_HIGH] = window;

    size_t cmdlen = strlen(cmd) + 1;
    size_t taglen = strlen(fname) + 1;
    size_t sizelen = strlen(size) + 1;

    // размер временного буфера делаем выровненным на 32 бита
    size_t total = BRD_ALIGN_UP(sizelen + taglen + cmdlen, 4);
    uint32_t *cmd_buf = (uint32_t*)malloc(total);
    if(!cmd_buf) {
        return;
    }
    memset(cmd_buf, 0, total);

    // скопируем в буфер параметры из аргументов
    char *pstr = (char*)cmd_buf;
    strcpy(pstr, cmd);
    pstr += cmdlen;
    strcpy(pstr, fname);
    pstr += taglen;
    strcpy(pstr, size);

    size_t N = (total>>2);
    for(size_t ii=0; ii<N; ii++) {
        m_pcie2axi[ii] = cmd_buf[ii];
    }

    free(cmd_buf);
}

//-----------------------------------------------------------------------------

bool hostmon_pcie::wait_card_ans(int timeout, std::string& cmd_info)
{
    // настроим смещение окна pcie2axi на базовый адрес блока команд c2h
    uint32_t window = windows_id[card_cmd_id];
    m_pcie_axi_block[AXI_ADDR_HIGH] = window;

    // считываем данные из PCIe окна во временный буфер
    // т.к. читать можно только по 32-бита
    uint32_t *cmd_buf = (uint32_t*)malloc(1024);
    if(!cmd_buf) {
        return false;
    }

    char *ptag = 0;
    int pass_count = timeout;
    bool status = false;
    char *pstr = (char *)cmd_buf;

    while(pass_count > 0) {

        // определим есть ли ответ в буфере
        if(!ptag) {

            for(int ii=0; ii<(1024>>2); ii++) {
                cmd_buf[ii] = m_pcie2axi[ii];
            }

            if(strncmp(pstr, "CMD_INFO:", 9) == 0) {
                ptag = pstr;
                pstr += 9;
            }
        }

        if(ptag) {
            // получили завершающий символ ответа "]"
            if(strstr(pstr, "]")) {
                cmd_info = ptag;
            }
        }

        // получим код команды
        if(!cmd_info.empty()) {
            status = true;
            break;
        }

        IPC_delay(1);
        --pass_count;
    }

    free(cmd_buf);

    return status;
}

//-----------------------------------------------------------------------------

bool hostmon_pcie::parse_card_ans(const std::string& cmd_info, int& data_size)
{
    if(cmd_info.empty())
        return false;

    //"CMD_INFO:LIST:/run/media/mmcblk0p2/storage/:SIZE:18:[SUCCESS]"

    size_t status_start = cmd_info.find("[SUCCESS]");
    if(status_start == std::string::npos) {
        return false;
    }

    size_t size_start = cmd_info.find(":SIZE:");
    if(size_start == std::string::npos) {
        return false;
    }

    size_t size_val_start = size_start+6;
    size_t size_val_end = cmd_info.find_first_of(':', size_val_start);
    if(size_val_end == std::string::npos) {
        return false;
    }

    std::string size_string = cmd_info.substr(size_val_start, size_val_end-size_val_start);
    if(size_string.empty()) {
        return false;
    }

    data_size = fromString<int>(size_string);

    return true;
}

//-----------------------------------------------------------------------------

void hostmon_pcie::show_card_info()
{
    // Данные со стороны target находятся в блоке CARD_STATUS, за счетчиком
    // тиков. Настроим смещение окна pcie2axi на базовый адрес блока данных - h2c
    // Реализовано в FMC131V.
    m_pcie_axi_block[AXI_ADDR_HIGH] = windows_id[card_status_id];

    fprintf(stderr, "Timestamp counter : %d\n", m_pcie2axi[0]);

    float t_zynq = (float(m_pcie2axi[1]) * 503.975 / 4096.) - 273.15;
    fprintf(stderr, "%s:\t\t[%.1f] C,\t<%d>\n", "ZYNQ t", t_zynq, m_pcie2axi[1]);

    float t_fpga1 = ((float(m_pcie2axi[18])/4.0f) - 273.15f);
    fprintf(stderr, "%s:\t[%.1f] C,\t<%d>\n", "FPGA1 t", t_fpga1, m_pcie2axi[18]);

    float t_fpga2 = ((float(m_pcie2axi[20])/4.) - 273.15);
    fprintf(stderr, "%s:\t[%.1f] C,\t<%d>\n", "FPGA2 t", t_fpga2, m_pcie2axi[20]);

    float vccint_val = (float(m_pcie2axi[2]) / 4096.) * 3.0f;
    fprintf(stderr, "%s:\t[%.1f] V,\t<%d>\n", "ZYNQ VCCINT", vccint_val, m_pcie2axi[2]);

    float _fval =  float(m_pcie2axi[4])/0.05f;
    fprintf(stderr, "%s:\t[%.1f] mA,\t<%d>\n", "+12V_shunt", _fval, m_pcie2axi[4]);

    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%d>\n", "+0.9V", float(m_pcie2axi[5]), m_pcie2axi[5]);

    fprintf(stderr, "%s:\t[%.1f] mV,\t<%d>\n", "+1.0AVCC", float(m_pcie2axi[6]), m_pcie2axi[6]);

    fprintf(stderr, "%s:\t[%.1f] mV,\t<%d>\n", "FMC2_VADJ", float(m_pcie2axi[7]), m_pcie2axi[7]);

    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%d>\n", "+2.5V", float(m_pcie2axi[8]), m_pcie2axi[8]);

    fprintf(stderr, "%s:\t[%.1f] mV,\t<%d>\n", "+0.95V_1", float(m_pcie2axi[9]), m_pcie2axi[9]);

    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%d>\n", "+1.2V", float(m_pcie2axi[10]), m_pcie2axi[10]);

    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%d>\n", "+1.5V", float(m_pcie2axi[11]), m_pcie2axi[11]);

    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%d>\n", "+1.8V", float(m_pcie2axi[12]), m_pcie2axi[12]);

    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%d>\n", "+12V", float(m_pcie2axi[13]), m_pcie2axi[13]);

    fprintf(stderr, "%s:\t[%.1f] mV,\t<%d>\n", "+3.3V_Zynq", float(m_pcie2axi[14]), m_pcie2axi[14]);

    fprintf(stderr, "%s:\t[%.1f] mV,\t<%d> \n", "FMC1_VADJ", float(m_pcie2axi[15]), m_pcie2axi[15]);

    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%d>\n", "+3.3V", float(m_pcie2axi[16]), m_pcie2axi[16]);

    fprintf(stderr, "%s:\t[%.1f] mV,\t<%d>\n", "+3.3V_CLK", float(m_pcie2axi[17]), m_pcie2axi[17]);

    fprintf(stderr, "%s:\t[%.1f] mV,\t<%d>\n", "+0.95V_2", float(m_pcie2axi[19]), m_pcie2axi[19]);
}

//-----------------------------------------------------------------------------
