#include "ddr3_trd.h"
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

ddr3_trd::ddr3_trd(fmcx_dev_t hw, int idx) : _hw(hw), _idx(idx)
{
    _valid = false;
    const user_node_t& node = _hw->get_trd_node(idx);
    if(!node.bar) {
        fprintf(stderr, " Invalid TRD index: 0x%04X\n", idx);
        return;
    }

    _bar = reinterpret_cast<uint32_t*>(node.bar);

    //fprintf(stderr, " Node name: %s, address: %p, size: 0x%x\n", node.name.c_str(), node.bar, node.sz);

    uint32_t trdid = ri(0x100) & 0xffff;
    if(trdid == 0xffff || trdid == 0x0) {
        fprintf(stderr, " Invalid TRD_ID = 0x%04X\n", trdid);
    } else {
        _id = trd_id(trdid);
        _valid = true;
    }
}

//-----------------------------------------------------------------------------

ddr3_trd::ddr3_trd(fmcx_dev_t hw, trd_id id, int start) : _hw(hw), _id(id)
{
    _valid = false;
    for(unsigned ii=start; ii<_hw->trd_nodes_total(); ++ii) {

        const user_node_t& node = _hw->get_trd_node(ii);
        if(!node.bar)
            continue;

        _bar = reinterpret_cast<uint32_t*>(node.bar);

        uint32_t trdid = ri(0x100) & 0xffff;
        if(trdid == 0xffff || trdid == 0x0) {
            fprintf(stderr, " Invalid TRD_ID = 0x%04X\n", trdid);
        } else {
            if(id == trdid) {
                _idx = ii;
                _id = trd_id(trdid);
                _valid = true;
                _name = node.name;
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------

ddr3_trd::~ddr3_trd()
{
}

//-----------------------------------------------------------------------------

uint32_t ddr3_trd::read_trd_reg(uint32_t offset)
{
    if(_bar) {
        return _bar[offset>>2];
    }
    return 0;
}

//-----------------------------------------------------------------------------

void ddr3_trd::write_trd_reg(uint32_t offset, uint32_t val)
{
    if(_bar) {
        _bar[offset>>2] = val;
    }
}

//-----------------------------------------------------------------------------

int	ddr3_trd::spd_read(int spdType, uint32_t regAdr, uint32_t *pRegVal)
{
    return -1;
}

//-----------------------------------------------------------------------------

int	ddr3_trd::spd_write(int spdType, uint32_t regAdr,  uint32_t RegVal)
{
    return -1;
}

//-----------------------------------------------------------------------------

#define BUFSIZEPKG 62
#define TRDIND_MODE0					0x0
#define TRDIND_STMODE					0x5
#define TRDIND_MODE1					0x9
#define TRDIND_MODE2					0xA
#define TRDIND_TESTSEQUENCE				0xC
#define TRDIND_SPD_DEVICE				0x203
#define TRDIND_SPD_CTRL					0x204
#define TRDIND_SPD_ADDR					0x205
#define TRDIND_SPD_DATA					0x206
#define TRDIND_SPD_DATAH				0x206
#define TRD_CTRL                        1
#define REG_MUX_CTRL                    0x0F
#define REG_GEN_CNT1                    0x1A
#define REG_GEN_CNT2                    0x1B
#define REG_GEN_CTRL                    0x1E
#define REG_GEN_SIZE                    0x1F
#define TRD_DIO_IN                      6
#define TRD_CTRL                        1

bool ddr3_trd::prepare_ddr3(uint32_t SdramFifoMode, uint32_t SdramAzBase, uint32_t SdramAzSize)
{
    //fprintf(stderr, "%s() (START)\n", __FUNCTION__);

    uint32_t ii;
    uint32_t spd_addr[]={ 4, 5, 7, 8 };
    uint32_t spd_data[sizeof(spd_addr)/sizeof(uint32_t)];

    wi(0x0, 0x3);	// MODE0
    wi(0x0, 0x0);	// MODE0

    for(ii=0;ii<sizeof(spd_addr)/sizeof(uint32_t);ii++) {
        wi(TRDIND_SPD_ADDR, spd_addr[ii]);
        wi(TRDIND_SPD_CTRL, 2);
        spd_data[ii] = ri(TRDIND_SPD_DATA);
    }

    //----------------------------------------------------------------------------

    uint32_t columns;
    uint32_t rows;
    uint32_t banks;
    uint32_t ranks;
    uint32_t size;
    uint32_t primary_bus;
    uint32_t sdram_widht;

    switch( (spd_data[0]>>4) & 0x7 ) {
    case 0: banks = 3; break;
    case 1: banks = 4; break;
    case 2: banks = 5; break;
    case 3: banks = 6; break;
    default: banks=0; break;
    }

    switch(spd_data[0] & 0xF ) {
    case 0:	size=256; break;
    case 1:	size=512; break;
    case 2:	size=1024; break;
    case 3:	size=2048; break;
    case 4:	size=4096; break;
    case 5:	size=8192; break;
    case 6:	size=16384; break;
    default: size=0;
    }

    switch( (spd_data[1]>>3) & 0x7 ) {
    case 0: rows=12; break;
    case 1: rows=13; break;
    case 2: rows=14; break;
    case 3: rows=15; break;
    case 4: rows=16; break;
    default: rows=0; break;
    }

    switch( (spd_data[1] & 0x7 ) ) {
    case 0: columns=9; break;
    case 1: columns=10; break;
    case 2: columns=11; break;
    case 3: columns=12; break;
    default: columns=0; break;
    }

    switch( (spd_data[2]>>3) & 0x7 ) {
    case 0: ranks=1; break;
    case 1: ranks=2; break;
    case 2: ranks=3; break;
    case 3: ranks=4; break;
    default: ranks=0; break;
    }

    switch( (spd_data[2]) & 0x7 ) {
    case 0: sdram_widht=4; break;
    case 1: sdram_widht=8; break;
    case 2: sdram_widht=16; break;
    case 3: sdram_widht=32; break;
    default: sdram_widht=0; break;
    }

    switch( (spd_data[3]) & 0x7 ) {
    case 0: primary_bus=8; break;
    case 1: primary_bus=16; break;
    case 2: primary_bus=32; break;
    case 3: primary_bus=64; break;
    default: primary_bus=0; break;
    }

    uint32_t mode1=0;
    mode1  = ((rows-8)<<2)&0x1C;
    mode1 |= ((columns-8)<<5) & 0xE0;
    mode1 |= ((ranks-1)<<8) ;
    mode1 |= (1<<9);
    mode1 |= 0x2000;

    wi(TRDIND_MODE1, mode1);	// CONF_REG

    uint32_t total_size = size / 8 * primary_bus / sdram_widht * ranks;
    fprintf(stderr, "\nCOLUMNS \t\t%d\nROWS    \t\t%d\nBANKS   \t\t%d\nRANKS   \t\t%d\nSDRAM SIZE   \t\t%d \nSDRAM WITH \t\t%d\nPRIMARY BUS \t\t%d\nTOTAL SIZE \t\t%d [MB]\n",
            columns, rows, banks, ranks, size, sdram_widht, primary_bus, total_size );
    //----------------------------------------------------------------------------

    uint32_t mode3 = 0;
    wi(0x0B, mode3);                // управление источником

    //-----------------------------------------------------------------------------
    fprintf(stderr, "Waiting for Memory Initialization  ... ");
    if(!wait_trd_status(100, 0x800)) {
        fprintf( stderr, "ERROR\n");
        return false;
    }
    fprintf( stderr, "DONE\n");

    if(1 == SdramFifoMode) { // Режим FIFO

        wi(0x10, 0 );
        wi(0x11, 0 );

        wi(0x0E, 0 );
        wi(0x0F, 0 );

        wi(0x14, 0 );
        wi(0x15, 0 );

        wi(TRDIND_MODE2, 0x14 ); // Режим FIFO
        fprintf(stderr, "Memory mode: FIFO\n");

    } else { // Режим с автоматическим чтением

        // Начало активной зоны
        wi(0x10, SdramAzBase & 0xFFFF );
        wi(0x11, SdramAzBase >> 16 );

        uint32_t size = SdramAzSize/4; // общий размер буфера в 32-х разрядных словах
        uint32_t AzEnd = SdramAzBase + size - 1;

        fprintf(stderr, "Memory mode: SDRAM\n");
        //BRDC_printf( _BRDC( "\r\nРежим с автоматическим чтением\r\n" ) );
        //BRDC_printf( _BRDC( "\r\nНачало активной зоны: 0x%.8X\r\n" ), SdramAzBase );
        //BRDC_printf( _BRDC( "\r\nКонец  активной зоны: 0x%.8X\r\n" ), AzEnd );
        //BRDC_printf( _BRDC( "\r\nРазмер активной зоны: 0x%.8X [MB]\r\n\n" ), size / ( 256 * 1024 ) );

        wi(0x0E, AzEnd & 0xFFFF );
        wi(0x0F, AzEnd >> 16 );
    }


    wi(TRDIND_MODE0, 2);  // Сброс FIFO

    //if( SdramFifoOutRestart )
    //{
    //	uint32_t mode3=0;
    //	mode3=pBrd->RegPeekInd( DDR2_trdNo, 0x0B );
    //	wi(0x0B, mode3 | 0x200 );    // сброс входного FIFO
    //	wi(0x0B, mode3 );
    //}

    wi(TRDIND_MODE0, 0 );

    //fprintf(stderr, "%s() (EXIT)\n", __FUNCTION__);

    return true;
}

void ddr3_trd::set_active_area(uint32_t SdramAzBase, uint32_t SdramAzSize)
{
    // Начало активной зоны
    wi(0x10, SdramAzBase & 0xFFFF );
    wi(0x11, SdramAzBase >> 16 );

    uint32_t size = SdramAzSize/4; // общий размер буфера в 32-х разрядных словах
    uint32_t AzEnd = SdramAzBase + size - 1;

    wi(0x0E, AzEnd & 0xFFFF );
    wi(0x0F, AzEnd >> 16 );
}

