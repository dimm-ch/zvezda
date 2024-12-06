#ifndef _PCIE_LIB_H_
#define _PCIE_LIB_H_

//-----------------------------------------------------------------------------

#include <stdint.h>
#include <string>

//-----------------------------------------------------------------------------

/*
 Поддерживаемые команды
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
 5) Формат команды для выполнения системных команд на удаленном устройстве:
    CMD_CODE:SYSTEM\0
    CMD_NAME:<cmdname>\0
    Формат ответа на команду записи файла:
    CMD_INFO:SYSTEM:<cmdname>:[SUCCESS|ERROR]\0

    В случае ошибки, в строке CMD_INFO: возвращается
    дополнительная информация с комментарием.

    Рабочий каталог для сохранения файлов:
    "/run/media/mmcblk0p2/storage/"
    Желательно не использовать команду CLEAR
*/

//-----------------------------------------------------------------------------

#define BRD_ALIGN_UP(value, align)  (((value) & (align-1)) ? \
                                    (((value) + (align-1)) & ~(align-1)) : \
                                    (value))

#define BRD_ALIGN_DOWN(value, align) ((value) & ~(align-1))

//-----------------------------------------------------------------------------

#define AXI_BLOCK_ID    0x0
#define AXI_BLOCK_VER   0x1<<1
#define AXI_CTRL        0x8<<1
#define AXI_ADDR_HIGH   0xA<<1

//-----------------------------------------------------------------------------

class hostmon_pcie {

private:
    uint32_t *m_pcie_axi_block;
    int32_t *m_pcie2axi;
	hostmon_pcie() { m_pcie_axi_block = 0; m_pcie2axi = 0; }
    void clear_card_ans();
    void clear_card_data();

public:
	hostmon_pcie(uint32_t *pcie_axi_block, uint32_t *pcie2axi) {
        m_pcie_axi_block = pcie_axi_block;
        m_pcie2axi = (int32_t*)pcie2axi;
        m_pcie_axi_block[AXI_CTRL] = 0x1;
    }
    ~hostmon_pcie() { m_pcie_axi_block[AXI_CTRL] = 0x0; }

    void write_host_data(uint32_t *data, uint32_t size);
    void read_card_data(uint32_t *data, uint32_t size);
    uint32_t hertbeat_counter();
    void write_cmd(const char* cmd, const char* fname, const char* size);
    bool wait_card_ans(int timeout, std::string& cmd_info);
    bool parse_card_ans(const std::string& cmd_info, int& data_size);
    uint16_t block_id() { return m_pcie_axi_block[AXI_BLOCK_ID] & 0xFFFF; }
    uint16_t block_version() { return m_pcie_axi_block[AXI_BLOCK_VER] & 0xFFFF; }
    uint32_t dbg_read(uint32_t zynq_addr)
    {
        // настроим смещение окна pcie2axi на базовый адрес блока данных - h2c
        m_pcie_axi_block[AXI_ADDR_HIGH] = zynq_addr;
        return m_pcie2axi[(zynq_addr & 0xFFFFF) >> 2];
    }
    void dbg_write(uint32_t zynq_addr, uint32_t zynq_val)
    {
        // настроим смещение окна pcie2axi на базовый адрес блока данных - h2c
        m_pcie_axi_block[AXI_ADDR_HIGH] = zynq_addr;
        m_pcie2axi[(zynq_addr & 0xFFFFF) >> 2] = zynq_val;
    }

    //-----------------------------------------------------------------------------
    void write_h2c_data(uint32_t *data, uint32_t size, bool isRawData = true);
    void read_c2h_data(uint32_t *data, uint32_t size, bool isRawData = true);
	virtual void show_card_info();
};

//-----------------------------------------------------------------------------

#endif //_PCIE_LIB_H_
