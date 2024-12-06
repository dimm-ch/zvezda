#include "fid_blk.h"
#include "strconv.h"
#include "exceptinfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <thread>
#include <chrono>

//-----------------------------------------------------------------------------
using namespace std;
//-----------------------------------------------------------------------------

fid_blk::fid_blk(fmcx_dev_t hw, int idx) : _hw(hw), _idx(idx)
{
    _valid = false;

    // Проверим все узлы, которые описывают блоки (должен быть один узел блоков на модуль)
    for(unsigned ii=0; ii<_hw->blk_nodes_total(); ++ii) {

        // Узел описывающий текущее пространство блоков
        const user_node_t& node = _hw->get_blk_node(ii);
        if(!node.bar)
            continue;

        size_t range = idx*PE_BLK_SIZE;
        if(range >= node.sz) {
            fprintf(stderr, " Block index out of range!\n");
            continue;
        }

        // Проверим блок c заданным индексом
        _bar = reinterpret_cast<uint32_t*>((uint8_t*)node.bar + idx*PE_BLK_SIZE);

        //fprintf(stderr, " Node name: %s, address: %p, size: 0x%x\n", node.name.c_str(), node.bar, node.sz);

        uint32_t blkid = rd(0x0) & 0xfff;
        if(blkid == 0xfff) {
            fprintf(stderr, " Invalid BLK_ID = 0x%04X\n", blkid);
        } else {
            _id = blk_id(blkid);
            _valid = true;
            _name = node.name;
            break;
        }
    }
}

//-----------------------------------------------------------------------------

fid_blk::fid_blk(fmcx_dev_t hw, blk_id id, int start) : _hw(hw), _id(id)
{
    _valid = false;

    // Проверим все узлы, которые описывают блоки (должен быть один узел блоков на модуль)
    for(unsigned ii=0; ii<_hw->blk_nodes_total(); ++ii) {

        // Узел описывающий текущее пространство блоков
        const user_node_t& node = _hw->get_blk_node(ii);
        if(!node.bar)
            continue;

        // Максимальное число блоков в текущем узле
        unsigned max_block_counter = 8;//(node.sz / PE_BLK_SIZE);

        // Проверим все блоки
        for(unsigned jj=start; jj<max_block_counter; ++jj) {

            _bar = reinterpret_cast<uint32_t*>((uint8_t*)node.bar + jj*PE_BLK_SIZE);

            uint32_t blkid = rd(0x0) & 0xfff;

            //fprintf(stderr, "0x%04x: BLK_ID = 0x%04X\n", jj*PE_BLK_SIZE, blkid);

            if((blkid == 0xfff) || (blkid == blk_id(0))) {
                continue;
            } else {
                if(blk_id(blkid) == id) {
                    //fprintf(stderr, "FOUND BLK_ID = 0x%04X\n", blkid);
                    _idx = jj;
                    _id = blk_id(blkid);
                    _valid = true;
                    _name = node.name;
                    return;
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------

fid_blk::~fid_blk()
{
}

//-----------------------------------------------------------------------------

uint32_t fid_blk::rd(uint32_t reg)
{
    return read_blk_reg(reg >> 2);
}

//-----------------------------------------------------------------------------

void fid_blk::wd(uint32_t reg, uint32_t val)
{
    write_blk_reg(reg >> 2, val);
}

//-----------------------------------------------------------------------------

uint32_t fid_blk::read_blk_reg(uint32_t reg)
{
    if(_bar) {
        return _bar[reg];
    }
    return 0;
}

//-----------------------------------------------------------------------------

void fid_blk::write_blk_reg(uint32_t reg, uint32_t val)
{
    if(_bar) {
        _bar[reg] = val;
    }
}

//-----------------------------------------------------------------------------

bool fid_blk::WaitReady()
{
    uint32_t SpdControl = 0;
    uint32_t timeout = 10;
    uint32_t pass = 0;

    while(1) {

        SpdControl = rd(PEFIDadr_SPD_CTRL);
        if(SpdControl & 0x8000)
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));

        pass++;

        if(pass > 100) {
            fprintf(stderr, "%s(%s): TIMEOUT\n", __func__, _name.c_str());
            return false;
        }

    }

    return true;
}

//-----------------------------------------------------------------------------
//  WriteFmcEeprom - writing data to I2C device
//  Input:  devid - device identificator (0 - I2C FMC, 1 - source of power)
//			devadr - device address (0x50 - I2C FMC, 0 - source of power)
//			buf - write data buffer
//			offset - offset within EEPROM
//			size - size of data buffer
//  Output:
//  Notes:
//-----------------------------------------------------------------------------
int fid_blk::WriteFmcEeprom(int devid, int devadr, uint8_t *buf, uint16_t Offset, long Length)
{
    int i = 0;
    FMC_SPD_CTRL SpdControl;

    wd(PEFIDadr_SPD_DEVICE, devid);
    SpdControl.AsWhole = 0;
    wd(PEFIDadr_SPD_CTRL, SpdControl.AsWhole);

    if(!WaitReady())
        return -ETIMEDOUT;

    for(i = 0; i < Length; i++)
    {
        int sector = Offset / 256;
        wd(PEFIDadr_SPD_ADDR, Offset); // write address
        wd(PEFIDadr_SPD_DATAL, buf[i]); // write data
        SpdControl.ByBits.Addr = devadr + sector;
        SpdControl.ByBits.WriteOp = 1;	// type operation - write
        wd(PEFIDadr_SPD_CTRL, SpdControl.AsWhole);
        //fprintf(stderr, "%s(): FMC EEPROM - sector = %d, address = 0x%X, offset = %d, data = %x\n",
        //        __func__, sector, SpdControl.ByBits.Addr, Offset, buf[i]);
        if(!WaitReady())
            return -ETIMEDOUT;
        Offset++;
    }
    SpdControl.AsWhole = 0;
    wd(PEFIDadr_SPD_CTRL, SpdControl.AsWhole);

    return 0;
}

//-----------------------------------------------------------------------------
//  ReadFmcEeprom - reading data from I2C device
//  Input:  devid - device identificator (0 - I2C FMC, 1 - source of power)
//			devadr - device address (0x50 - I2C FMC, 0 - source of power)
//			buf - reading data buffer
//			offset - offset within EEPROM
//			Length - size of data buffer
//  Output: buf - read data buffer
//  Notes:
//-----------------------------------------------------------------------------
int fid_blk::ReadFmcEeprom(int devid, int devadr, uint8_t *buf, uint16_t Offset, long Length)
{
    int i = 0;
    FMC_SPD_CTRL SpdControl;

    wd(PEFIDadr_SPD_DEVICE, devid);

    SpdControl.AsWhole = 0;
    wd(PEFIDadr_SPD_CTRL, SpdControl.AsWhole);

    if(!WaitReady())
        return -ETIMEDOUT;

    for(i = 0; i < Length; i++)
    {
        int sector = Offset / 256;
        wd(PEFIDadr_SPD_ADDR, Offset); // write address
        SpdControl.ByBits.Addr = devadr + sector;
        SpdControl.ByBits.ReadOp = 1;	// type operation - read
        wd(PEFIDadr_SPD_CTRL, SpdControl.AsWhole);
        if(!WaitReady())
            return -ETIMEDOUT;
        buf[i] = rd(PEFIDadr_SPD_DATAL) & 0xFF; // read data
        //fprintf(stderr, "%s(): FMC EEPROM - sector = %d, address = 0x%X, offset = %d, data = %x\n",
        //        __FUNCTION__,sector, SpdControl.ByBits.Addr, Offset, buf[i]);
        Offset++;
    }
    SpdControl.AsWhole = 0;
    wd(PEFIDadr_SPD_CTRL, SpdControl.AsWhole);
    return 0;
}
