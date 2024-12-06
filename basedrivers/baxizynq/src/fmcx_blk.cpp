#include "fmcx_blk.h"
//#include "ambpexregs.h"
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

fmcx_blk::fmcx_blk(fmcx_dev_t hw, int idx) : _hw(hw), _idx(idx)
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

fmcx_blk::fmcx_blk(fmcx_dev_t hw, blk_id id, int start) : _hw(hw), _id(id)
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

fmcx_blk::~fmcx_blk()
{
}

//-----------------------------------------------------------------------------

uint32_t fmcx_blk::rd(uint32_t reg)
{
    return read_blk_reg(reg >> 2);
}

//-----------------------------------------------------------------------------

void fmcx_blk::wd(uint32_t reg, uint32_t val)
{
    write_blk_reg(reg >> 2, val);
}

//-----------------------------------------------------------------------------

uint32_t fmcx_blk::read_blk_reg(uint32_t reg)
{
    if(_bar) {
        return _bar[reg];
    }
    return 0;
}

//-----------------------------------------------------------------------------

void fmcx_blk::write_blk_reg(uint32_t reg, uint32_t val)
{
    if(_bar) {
        _bar[reg] = val;
    }
}

//-----------------------------------------------------------------------------
//  WaitSemaReady - wait semaphor flag or data ready flag (1)
//  Input:	SemaOrReady = 1 sema, 0 ready
//  Output:
//  Notes: при SemaOrReady = 1 ожидаем срабатывание флага семафора
//		   при SemaOrReady = 0 ожидаем готовности данных
//-----------------------------------------------------------------------------

int fmcx_blk::WaitSemaReady(int SemaOrReady)
{
    SPD_CTRL SpdControl;
    int stat;
    uint32_t timeout = 10;
    uint32_t pass = 0;

    SpdControl.AsWhole = rd(PEMAINadr_SPD_CTRL);

    while(1) {

        SpdControl.AsWhole = rd(PEMAINadr_SPD_CTRL);

        if(SemaOrReady)
            stat = SpdControl.ByBits.Sema;
        else
            stat = SpdControl.ByBits.Ready;

        if(stat)
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));

        pass++;

        if(pass > 100) {
            fprintf(stderr, "%s(%s): TIMEOUT\n", __func__, _name.c_str());
            return -ETIMEDOUT;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
//  WriteEEPROM - writing data to serial EEPROM
//  Input:  idRom - identificator of EEPROM (0 - ADM, 1 - base ICR)
//			buf - write data buffer
//			offset - offset within EEPROM
//			size - size of data buffer
//  Output:
//  Notes:
//-----------------------------------------------------------------------------

int fmcx_blk::WriteEEPROM(int idRom, const uint16_t* buf, uint16_t Offset, uint32_t Length)
{
    int Status = 0;
    SPD_CTRL SpdControl;
    uint32_t i=0;

    SpdControl.AsWhole = 0;
    SpdControl.ByBits.Sema = 1;
    SpdControl.ByBits.SpdId = idRom; // 0 - submodule EEPROM, 1 - base EEPROM

    wd(PEMAINadr_SPD_CTRL, SpdControl.AsWhole);
    WaitSemaReady(1); // waiting SEMA

    SpdControl.ByBits.WriteEn = 1; // write enable
    wd(PEMAINadr_SPD_CTRL, SpdControl.AsWhole);
    WaitSemaReady(0); // waiting READY

    for(i = 0; i < Length; i++)
    {
        wd(PEMAINadr_SPD_ADDR, Offset); // write address
        wd(PEMAINadr_SPD_DATAL, buf[i]); // write data
        SpdControl.ByBits.WriteOp = 1;	// type operation - write
        //fprintf(stderr, "SPD_ADDR: 0x%x, SPD_DATAL: 0x%x, SPD_CTRL: 0x%x\n", Offset, buf[i], SpdControl.AsWhole);
        wd(PEMAINadr_SPD_CTRL, SpdControl.AsWhole);
        WaitSemaReady(0); // waiting READY
        Offset++;
    }

    SpdControl.ByBits.WriteEn = 0;
    SpdControl.ByBits.WriteOp = 0;
    SpdControl.ByBits.WriteDis = 1;  // write disable
    wd(PEMAINadr_SPD_CTRL, SpdControl.AsWhole);
    SpdControl.AsWhole = 0;
    wd(PEMAINadr_SPD_CTRL, SpdControl.AsWhole);

    return Status;
}

//-----------------------------------------------------------------------------
//  ReadEEPROM - reading data from serial EEPROM
//  Input:  idRom - identificator of EEPROM (0 - ADM, 1 - base ICR)
//			buf - reading data buffer
//			offset - offset within EEPROM
//			size - size of data buffer
//  Output: buf - read data buffer
//  Notes:
//-----------------------------------------------------------------------------
int fmcx_blk::ReadEEPROM(int idRom, uint16_t* buf, uint16_t Offset, uint32_t Length)
{
    int Status = 0;
    SPD_CTRL SpdControl;
    uint32_t i=0;

    SpdControl.AsWhole = 0;
    SpdControl.ByBits.Sema = 1;
    SpdControl.ByBits.SpdId = idRom; // 0 - submodule EEPROM, 1 - base EEPROM

    wd(PEMAINadr_SPD_CTRL, SpdControl.AsWhole);
    WaitSemaReady(1); // waiting SEMA

    for(i = 0; i < Length; i++)
    {
        wd(PEMAINadr_SPD_ADDR, Offset); // write address
        SpdControl.ByBits.ReadOp = 1;	// type operation - read
        wd(PEMAINadr_SPD_CTRL, SpdControl.AsWhole);
        WaitSemaReady(0); // waiting READY
        uint16_t val = rd(PEMAINadr_SPD_DATAL); // read data
        buf[i] = val;
        //fprintf(stderr, "SPD_ADDR: 0x%x, SPD_DATAL: 0x%x, SPD_CTRL: 0x%x\n", Offset, val, SpdControl.AsWhole);
        Offset++;
    }
    SpdControl.AsWhole = 0;
    wd(PEMAINadr_SPD_CTRL, SpdControl.AsWhole);

    return Status;
}

//-----------------------------------------------------------------------------
/*
int WaitReady(struct CWambpex *brd)
{
    uint32_t SpdControl = 0;
    int stat = 0;
    int i = 0;

    fprintf(stderr, "%s()\n", __FUNCTION__);

    for(i = 0; i < 1000; i++) // wait 10 msec
    {
        SpdControl = ReadOperationReg(brd, PEFIDadr_SPD_CTRL + brd->m_BlockFidAddr);
        stat = SpdControl & 0x8000;
        fprintf(stderr, "%s(): FMC EEPROM - stat = 0x%X\n", __FUNCTION__, stat);
        if(stat)
            break;
        udelay(10);
    }
    if(!stat)
        return -1;
    return 0;
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
int WriteFmcEeprom(struct CWambpex *brd, int devid, int devadr, u8 *buf, u16 Offset, long Length)
{
    int Status = 0;
    int i = 0;
    FMC_SPD_CTRL SpdControl;

    fprintf(stderr, "%s(): FMC EEPROM - devid = 0x%X, devadr = 0x%X, offset = %d, length = %ld\n", __FUNCTION__, devid, devadr, Offset, Length);

    WriteOperationReg(brd, PEFIDadr_SPD_DEVICE + brd->m_BlockFidAddr, devid);
    SpdControl.AsWhole = 0;
    WriteOperationReg(brd, PEFIDadr_SPD_CTRL + brd->m_BlockFidAddr, SpdControl.AsWhole);
    ToTimeOut(1000);
    //ToPauseEx(1);
    Status = WaitReady(brd); // waiting READY
    if(Status < 0)
        return Status;

    for(i = 0; i < Length; i++)
    {
        int sector = Offset / 256;
        WriteOperationReg(brd, PEFIDadr_SPD_ADDR + brd->m_BlockFidAddr, Offset); // write address
        WriteOperationReg(brd, PEFIDadr_SPD_DATAL + brd->m_BlockFidAddr, buf[i]); // write data
        SpdControl.ByBits.Addr = devadr + sector;
        SpdControl.ByBits.WriteOp = 1;	// type operation - write
        WriteOperationReg(brd, PEFIDadr_SPD_CTRL + brd->m_BlockFidAddr, SpdControl.AsWhole);
        ToTimeOut(10000);
        Status = WaitReady(brd); // waiting READY
        fprintf(stderr, "%s(): FMC EEPROM - sector = %d, address = 0x%X, offset = %d, data = %x, Status = %x\n",
                __FUNCTION__,sector, SpdControl.ByBits.Addr, Offset, buf[i], Status);
        if(Status < 0)
            break;
        Offset++;
    }
    SpdControl.AsWhole = 0;
    WriteOperationReg(brd, PEFIDadr_SPD_CTRL + brd->m_BlockFidAddr, SpdControl.AsWhole);
    return Status;
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
int ReadFmcEeprom(struct CWambpex *brd, int devid, int devadr, u8 *buf, u16 Offset, long Length)
{
    int Status = 0;
    int i = 0;
    FMC_SPD_CTRL SpdControl;

    fprintf(stderr, "%s(): FMC EEPROM - devid = 0x%X, devadr = 0x%X, offset = %d, length = %ld\n", __FUNCTION__, devid, devadr, Offset, Length);

    WriteOperationReg(brd, PEFIDadr_SPD_DEVICE + brd->m_BlockFidAddr, devid);

    SpdControl.AsWhole = 0;
    WriteOperationReg(brd, PEFIDadr_SPD_CTRL + brd->m_BlockFidAddr, SpdControl.AsWhole);
    ToTimeOut(1000);
    //ToPauseEx(1);
    Status = WaitReady(brd); // waiting READY
    if(Status < 0)
        return Status;

    for(i = 0; i < Length; i++)
    {
        int sector = Offset / 256;
        WriteOperationReg(brd, PEFIDadr_SPD_ADDR + brd->m_BlockFidAddr, Offset); // write address
        SpdControl.ByBits.Addr = devadr + sector;
        SpdControl.ByBits.ReadOp = 1;	// type operation - read
//	    KdPrint(("CWambpex::Reading MFC EEPROM - sector = %d, address = 0x%X, offset = %d\n", sector, SpdControl.ByBits.Addr, Offset));
        WriteOperationReg(brd,PEFIDadr_SPD_CTRL + brd->m_BlockFidAddr, SpdControl.AsWhole);
//		ToPauseEx(1);
        Status = WaitReady(brd); // waiting READY
        //USHORT ubuf = ReadOperationReg(PEFIDadr_SPD_DATAL + m_BlockFidAddr); // read data
        //KdPrint(("CWambpex::Reading FMC EEPROM - ubuf = 0x%X, offset = %d\n", ubuf, Offset));
        buf[i] = ReadOperationReg(brd,PEFIDadr_SPD_DATAL + brd->m_BlockFidAddr) & 0xFF; // read data
        fprintf(stderr, "%s(): FMC EEPROM - sector = %d, address = 0x%X, offset = %d, data = %x, Status = %x\n",
                __FUNCTION__,sector, SpdControl.ByBits.Addr, Offset, buf[i], Status);
        if(Status < 0)
            break;
        Offset++;
    }
    SpdControl.AsWhole = 0;
    WriteOperationReg(brd,PEFIDadr_SPD_CTRL + brd->m_BlockFidAddr, SpdControl.AsWhole);
    return Status;
}
*/
//-----------------------------------------------------------------------------
// Base module identification
//
typedef struct _ICR_Id4953 {
    uint16_t wTag;           // tag of structure (BASE_ID_TAG)
    uint16_t wSize;          // size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
    uint16_t wSizeAll;       // size of all data, writing into base module EPROM
    uint32_t dSerialNum;     // serial number ( () )
    uint16_t wDeviceId;      // base module type (  )
    uint8_t  bVersion;       // base module version (  )
    uint8_t  bDay;           // day of Data (      )
    uint8_t  bMon;           // montag of Data (      )
    uint16_t wYear;          // year of Data (      )
} __attribute__((packed)) ICR_Id4953, *PICR_Id4953, ICR_IdBase, *PICR_IdBase;

//-----------------------------------------------------------------------------

uint32_t fmcx_blk::GetPID()
{
    ICR_IdBase Info;
    int offset = (0x80/2);
    int size = (sizeof(Info)/2);
    int res = ReadEEPROM(1, (uint16_t*)&Info, offset, size);
    if(res < 0) {
        fprintf(stderr, "%s(): Error in ReadEEPROM()\n", __FUNCTION__);
        return ~0x0;
    }

    return Info.dSerialNum;
}

//-----------------------------------------------------------------------------

uint32_t fmcx_blk::GetTAG()
{
    ICR_IdBase Info;
    int offset = (0x80/2);
    int size = (sizeof(Info)/2);
    int res = ReadEEPROM(1, (uint16_t*)&Info, offset, size);
    if(res < 0) {
        fprintf(stderr, "%s(): Error in ReadEEPROM()\n", __FUNCTION__);
        return ~0x0;
    }

    return Info.wTag;
}

//-----------------------------------------------------------------------------
