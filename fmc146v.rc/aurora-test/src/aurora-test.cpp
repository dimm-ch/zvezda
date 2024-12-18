#include "bardy.h"
#include "brd_dev.h"
#include "strconv.h"
#include "exceptinfo.h"
#include <cstdio>
#include <iostream>
#include "time_ipc.h"
#include <csignal>
#include <chrono>

#define TRD_AURORA_ID_VPXx4 0x10F 
#define TRD_AURORA_ID_FPGAx8 0x115

#define REG_ERR_ADR		0x10
#define REG_ERR_DATA	0x204

// ����� ����������� ������
#define REG_BLOCK_RD_L	0x4000
#define REG_BLOCK_RD_H	0x4001
// ����� ���������� ������
#define REG_BLOCK_OK_L	0x4002
#define REG_BLOCK_OK_H	0x4003
// ����� ������������ ������
#define REG_BLOCK_ERR_L	0x4004
#define REG_BLOCK_ERR_H 0x4005
// ����� ���������� ������
#define REG_BLOCK_WR_L	0x200
#define REG_BLOCK_WR_H	0x201

#define REG_TOTAL_ERR_L 0x100
#define REG_TOTAL_ERR_H 0x101

#define REG_READ_D0 0x0
#define REG_READ_D2 0x2

#define REG_EXPECT_D0 0x4
#define REG_EXPECT_D2 0x6

#define REG_INDEX 0x8

#define REG_BLOCK_D0 0x9

static volatile int exit_flag = 0;
using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

void signal_handler(int signo) {
    signo = signo;
    exit_flag = 1;
    fprintf(stderr, "\n");
}

void show_help(const std::string& program)
{
    fprintf(stderr, "usage: %s <options>\n", program.c_str());
    fprintf(stderr, "Options description:\n");
    fprintf(stderr, "-t <lid> - LID of 1 board\n");
    fprintf(stderr, "-r <lid> - LID of 2 board\n");
    fprintf(stderr, "-a <type> - Aurora type: 0 - FPGA, 1 - VPX\n");
    fprintf(stderr, "-h - Print this message\n");
}

U32 read_blocks_rd(brd_dev_t dev, U32 trd, U32 offset)
{
    U32 val[2] = { 0 };
    dev->RegPokeInd(trd, REG_ERR_ADR, REG_BLOCK_RD_L+offset);
    val[0] = dev->RegPeekInd(trd, REG_ERR_DATA);

    dev->RegPokeInd(trd, REG_ERR_ADR, REG_BLOCK_RD_H+offset);
    val[1] = dev->RegPeekInd(trd, REG_ERR_DATA);

    return ((val[1] << 16) | (val[0] & 0xFFFF));
}

U32 read_blocks_wr(brd_dev_t dev, U32 trd)
{
    U32 val[2] = { 0 };    
    val[0] = dev->RegPeekInd(trd, REG_BLOCK_WR_L);
    val[1] = dev->RegPeekInd(trd, REG_BLOCK_WR_H);

    return ((val[1] << 16) | (val[0] & 0xFFFF));
}

U32 read_blocks_ok(brd_dev_t dev, U32 trd, U32 offset)
{
    U32 val[2] = { 0 };
    dev->RegPokeInd(trd, REG_ERR_ADR, REG_BLOCK_OK_L+offset);
    val[0] = dev->RegPeekInd(trd, REG_ERR_DATA);

    dev->RegPokeInd(trd, REG_ERR_ADR, REG_BLOCK_OK_H+offset);
    val[1] = dev->RegPeekInd(trd, REG_ERR_DATA);

    return ((val[1] << 16) | (val[0] & 0xFFFF));
}

U32 read_blocks_err(brd_dev_t dev, U32 trd, U32 offset)
{
    U32 val[2] = { 0 };
    dev->RegPokeInd(trd, REG_ERR_ADR, REG_BLOCK_ERR_L+offset);
    val[0] = dev->RegPeekInd(trd, REG_ERR_DATA);

    dev->RegPokeInd(trd, REG_ERR_ADR, REG_BLOCK_ERR_H+offset);
    val[1] = dev->RegPeekInd(trd, REG_ERR_DATA);

    return ((val[1] << 16) | (val[0] & 0xFFFF));
}

U32 get_item(brd_dev_t dev, U32 trd, U32 reg)
{
    U32 d, dx, d0, d1, ii;
    for (ii = 0; ii < 100; ii++)
    {
        dev->RegPokeInd(trd, REG_ERR_ADR, reg);
        d0 = dev->RegPeekInd(trd, REG_ERR_DATA) & 0xFFFF;;
        dev->RegPokeInd(trd, REG_ERR_ADR, reg + 1);
        d1 = dev->RegPeekInd(trd, REG_ERR_DATA) & 0xFFFF;;
        d = (d1 << 16) | d0;
        dev->RegPokeInd(trd, REG_ERR_ADR, reg);
        d0 = dev->RegPeekInd(trd, REG_ERR_DATA) & 0xFFFF;;
        dev->RegPokeInd(trd, REG_ERR_ADR, reg + 1);
        d1 = dev->RegPeekInd(trd, REG_ERR_DATA) & 0xFFFF;;
        dx = (d1 << 16) | d0;
        if (d == dx)
            break;
    }
    return dx;
}

int main(int argc, char* argv[])
{
	fprintf(stderr, "---------------------------\n");
	fprintf(stderr, " TEST: %s\n", argv[0]);
	fprintf(stderr, "---------------------------\n");
    fprintf(stderr, "press Esc to exit (linux)\n");
    fprintf(stderr, "press Ctrl+[ or to exit (windows + ssh)\n");
    signal(SIGINT, signal_handler);
	S32 ulid_rx = get_from_cmdline(argc, argv, "-r", -1);
    BRDC_printf(_BRDC("cl RX lid%d\t"), ulid_rx);
	S32 ulid_tx = get_from_cmdline(argc, argv, "-t", -1);
    BRDC_printf(_BRDC("TX lid%d\n"), ulid_tx);
    U32 test_type = get_from_cmdline(argc, argv, "-a", 0);
    U32 is_help = is_option(argc, argv, "-h");

    IPC_init();
    IPC_initKeyboard();

    if( is_help )
    {
        show_help(argv[0]);
        return 0;
    }
    if( test_type == 0 )
        printf("test aurora FPGA\n");
    else
        printf("test aurora VPX\n");
	S32 brd_count = 0, lid = 0;
    if (Bardy::initBardy(brd_count)) {
        std::vector<U32> lids;
        if (!Bardy::boardsLID(lids)) {
            fprintf(stderr, "Can't get board LIDs\n");
            return -1;
        }        
        brd_dev_t dev_rx = nullptr, dev_tx = nullptr;
        for (const auto& id : lids) {
            dev_rx = get_device(id);
            lid = id;
            if (ulid_rx < 0) {
                ulid_rx = id;
                break;
            }
            else
                if (ulid_rx == id)
                    break;
        }
        if( ulid_rx != lid )
        {
            fprintf(stderr, "ERROR: wrong RX lid %d\n", ulid_rx);
            return -1;
        }
        lid = 0;
        BRDC_printf(_BRDC("get RX lid%d\t"), ulid_rx);
        for (const auto& id : lids) {
            if (id != ulid_rx)
            {
                dev_tx = get_device(id);
                lid = id;
                if (ulid_tx < 0) {
                    ulid_tx = lid;
                    break;
                }
                else
                    if (ulid_tx == id)
                        break;
            }
        }
        if (ulid_tx != lid)
        {
            fprintf(stderr, "ERROR: wrong TX lid %d\n", ulid_tx);
            return -1;
        }
        BRDC_printf(_BRDC("TX lid%d\n"), ulid_tx);
        //dev_rx->enum_services();
        //dev_tx->enum_services();
        dev_rx->show_services();
        dev_tx->show_services();
        dev_rx->lock_services();
        dev_tx->lock_services();
        S32 maintetrad = -1;
        dev_rx->get_trd_number(1, maintetrad);
        dev_rx->RegPokeInd(maintetrad, 0, 1);
        ipc_delay(100);
        dev_rx->RegPokeInd(maintetrad, 0, 0);
        ipc_delay(100);
        for (int i = 0; i < 0x30; i++)
            dev_rx->RegPokeInd(maintetrad, i, 0);
        dev_tx->get_trd_number(1, maintetrad);
        dev_tx->RegPokeInd(maintetrad, 0, 1);
        ipc_delay(100);
        dev_tx->RegPokeInd(maintetrad, 0, 0);
        ipc_delay(100);
        for (int i = 0; i < 0x30; i++)
            dev_tx->RegPokeInd(maintetrad, i, 0);
        S32 trd_rx0 = -1, trd_rx1 = -1, trd_tx0 = -1, trd_tx1 = -1;
        if( test_type == 0 )
        {
            dev_rx->get_trd_number(TRD_AURORA_ID_FPGAx8, trd_rx0);
            dev_tx->get_trd_number(TRD_AURORA_ID_FPGAx8, trd_tx0);
        }
        else
        {
            dev_rx->get_trd_number(TRD_AURORA_ID_VPXx4, trd_rx0);
            for( int i = trd_rx0 + 1; i < 16; i++ )
            {
                U32 id = dev_rx->RegPeekInd(i, 0x100);
                if( id == TRD_AURORA_ID_VPXx4 )
                {
                    trd_rx1 = i;
                    break;
                }
            }
            dev_tx->get_trd_number(TRD_AURORA_ID_VPXx4, trd_tx0);
            for( int i = trd_tx0 + 1; i < 16; i++ )
            {
                U32 id = dev_tx->RegPeekInd(i, 0x100);
                if( id == TRD_AURORA_ID_VPXx4 )
                {
                    trd_tx1 = i;
                    break;
                }
            }
        }
        if( (trd_tx0 == -1) || (trd_rx0 == -1) || (((trd_tx1 == -1) ||  (trd_rx1 == -1)) && test_type) )
        {
            if (test_type)
                fprintf(stderr, "ERROR: can't find aurora tetrad (lid%d trd %d; lid %d trd %d)\n", ulid_rx, trd_rx0, ulid_tx, trd_tx0);
            else
                fprintf(stderr, "ERROR: can't find aurora tetrad (lid%d trd %d and %d; lid %d trd %d and %d)\n", ulid_rx, trd_rx0, trd_rx1, ulid_tx, trd_tx0, trd_tx1);
            return -1;
        }
        dev_tx->RegPokeInd(trd_tx0, 0, 0); //stop
        if (test_type != 0) dev_tx->RegPokeInd(trd_tx1, 0, 0);
        dev_rx->RegPokeInd(trd_rx0, 0, 0);
        if( test_type != 0 ) dev_rx->RegPokeInd(trd_rx1, 0, 0);

        dev_rx->RegPokeInd(trd_rx0, 0, 3); // reset
        dev_rx->RegPokeInd(trd_rx0, 0, 0);
        if( test_type != 0 ) 
        {
            dev_rx->RegPokeInd(trd_rx1, 0, 3);
            dev_rx->RegPokeInd(trd_rx1, 0, 0);
        }
        dev_tx->RegPokeInd(trd_tx0, 0, 3);
        dev_tx->RegPokeInd(trd_tx0, 0, 0);
        if( test_type != 0 ) 
        {
            dev_tx->RegPokeInd(trd_tx1, 0, 3);
            dev_tx->RegPokeInd(trd_tx1, 0, 0);
        }
        //printf("reset\n");

        dev_rx->RegPokeInd(trd_rx0, 0xC, 0x3 << 9); // test enable
        dev_tx->RegPokeInd(trd_tx0, 0xC, 0x3 << 9);
        if( test_type != 0 ) 
        {
            dev_rx->RegPokeInd(trd_rx1, 0xC, 0x3 << 9);
            dev_tx->RegPokeInd(trd_tx1, 0xC, 0x3 << 9);
        }
        //printf("test enable\n");
        dev_rx->RegPokeInd(trd_rx0, 0, 0x20); // start rx
        dev_tx->RegPokeInd(trd_tx0, 0, 0x20);
        if( test_type != 0 ) 
        {
            dev_rx->RegPokeInd(trd_rx1, 0, 0x20);
            dev_tx->RegPokeInd(trd_tx1, 0, 0x20);
        }
        //printf("start\n");
        U32 val = 0, status[4] = { 0 };
        bool ready = true;
        for (int i = 0; i < 64; i++)
        {
            ready = true;
            val = dev_rx->RegPeekDir(trd_rx0, 0);
            if ((val >> 10) != 0x3F) 
            {
                ready = false;
                status[0] = 0;
            }
            else
                status[0] = 1;
            if( test_type != 0 )
            {
                val = dev_rx->RegPeekDir(trd_rx1, 0);
                if( (val >> 10) != 0x3F ) 
                {
                    ready = false;
                    status[2] = 0;
                }
                else
                    status[2] = 1;
            }
            val = dev_tx->RegPeekDir(trd_tx0, 0);
            if ((val >> 10) != 0x3F) 
            {
                ready = false;
                status[1] = 0;
            }
            else
                status[1] = 1;
            if( test_type != 0 )
            {
                val = dev_tx->RegPeekDir(trd_tx1, 0);
                if( (val >> 10) != 0x3F ) 
                {
                    ready = false;
                    status[3] = 0;
                }
                else
                    status[3] = 1;
            }            
            ipc_delay(100);
        }
        if (!ready)
        {
            printf("AURORA not ready!\t");
            if( test_type != 0 )
            {
                for( int i = 0; i < 4; i++ )
                    if( status[i] == 0 )
                        printf("%d\t", i);
            }
            else
            {
                for( int i = 0; i < 2; i++ )
                    if( status[i] == 0 )
                        printf("%d\t", i);
            }

            return -1;
        }

        dev_tx->RegPokeInd(trd_tx0, 9, 1); // start tx
        dev_rx->RegPokeInd(trd_rx0, 9, 1);
        if( test_type != 0 ) 
        {
            dev_tx->RegPokeInd(trd_tx1, 9, 1);
            dev_rx->RegPokeInd(trd_rx1, 9, 1);
        }
        U32 wr_blocks=0, rd_blocks=0, err_blocks=0, nOffset = 0x4000;
        if( test_type == 1 )
        {
            //nOffset = 0;
            fprintf(stderr, "lid%d\t\t\t\t\t\t\t\tlid%d\n", ulid_tx, ulid_rx);
        }
        else
            fprintf(stderr, "lid%d\t\t\t\tlid%d\n", ulid_tx, ulid_rx);

        auto start_prog = high_resolution_clock::now();
        while (!exit_flag)
        {
            if(IPC_kbhit()){
			int ch = 0 | IPC_getch();
			if(ch == 0xe0 && IPC_kbhit()){ // extended character (0xe0, xxx)
				ch = (ch<<8) | IPC_getch(); // get extended xharaxter info
			}
            switch(ch){
				case 0x1b:
                    fprintf(stderr,"\nESC pressed,exiting\n");
                    exit_flag = 1;
                    break;
				break;
			}
			fprintf(stderr,"\n");

            }


            wr_blocks = read_blocks_wr(dev_tx, trd_tx0);
            rd_blocks = read_blocks_rd(dev_tx, trd_tx0, nOffset);
            err_blocks = read_blocks_err(dev_tx, trd_tx0, nOffset);
            auto current_cycle = high_resolution_clock::now();
            auto duration = duration_cast<seconds>(current_cycle - start_prog);
            fprintf(stderr, "[%lld]t%d:wr0x%X:rd0x%X:err[0x%X] ",duration, trd_tx0, wr_blocks,rd_blocks,err_blocks);
            if( test_type != 0 )
            {
                wr_blocks = read_blocks_wr(dev_tx, trd_tx1);
                rd_blocks = read_blocks_rd(dev_tx, trd_tx1, nOffset);
                err_blocks = read_blocks_err(dev_tx, trd_tx1, nOffset);
                fprintf(stderr, "t%d:wr0x%X:rd0x%X:err[0x%X] ", trd_tx1, wr_blocks, rd_blocks, err_blocks);
                wr_blocks = read_blocks_wr(dev_rx, trd_rx1);
                rd_blocks = read_blocks_rd(dev_rx, trd_rx1, nOffset);
                err_blocks = read_blocks_err(dev_rx, trd_rx1, nOffset);
                val = read_blocks_ok(dev_rx, trd_rx1, nOffset);
                fprintf(stderr, "t%d:wr0x%X:rd0x%X:err[0x%X] ", trd_rx1, wr_blocks, rd_blocks, err_blocks);
            }
            wr_blocks = read_blocks_wr(dev_rx, trd_rx0);
            rd_blocks = read_blocks_rd(dev_rx, trd_rx0, nOffset);
            err_blocks = read_blocks_err(dev_rx, trd_rx0, nOffset);
            val = read_blocks_ok(dev_rx, trd_rx0, nOffset);
            fprintf(stderr, "t%d:wr0x%X:rd0x%X:err[0x%X] ", trd_rx0, wr_blocks, rd_blocks, err_blocks);
            fprintf(stderr, "\r");
            ipc_delay(500);
        }
        U32 rx_tetr[2] = { (U32)trd_rx0, (U32)trd_rx1 }, trd_num = 2, chan_num = 2;
        for (int trd = 0; trd < 1+test_type; trd++) {
            for (int chan = 0; chan < 8-(4*test_type); chan++)
            {
                U32 base = chan * 0x1000;
                U32 total_err = get_item(dev_rx, rx_tetr[trd], base + REG_TOTAL_ERR_L);
                U32 cnt = total_err, word;
                if (cnt > 16)
                    cnt = 16;
                if (cnt > 0)
                    BRDC_printf(_BRDC("tetrad %d chan %d (total errors %d)\n"), rx_tetr[trd], chan, total_err);
                for (int ii = 0; ii < cnt; ii++)
                {
                    word = base + ii * 0x10;
                    unsigned long long data_rd, data_expect, tmp;
                    U32 index, block;
                    data_rd = get_item(dev_rx, rx_tetr[trd], word + REG_READ_D0);
                    tmp = get_item(dev_rx, rx_tetr[trd], word + REG_READ_D2);
                    data_rd |= tmp << 32;

                    data_expect = get_item(dev_rx, rx_tetr[trd], word + REG_EXPECT_D0);
                    tmp = get_item(dev_rx, rx_tetr[trd], word + REG_EXPECT_D2);
                    data_expect |= tmp << 32;

                    index = get_item(dev_rx, rx_tetr[trd], word + REG_INDEX) & 0xFFFF;

                    block = get_item(dev_rx, rx_tetr[trd], word + REG_BLOCK_D0);

                    tmp = data_expect ^ data_rd;
                    BRDC_printf(_BRDC("ERROR: %4d  block: %-4d  index: %.8X  expect: %.16llX  receive: %.16llX xor: %.16llX \r\n"),
                        ii, block, index, data_expect, data_rd, tmp);
                }
            }
        }
        U32 tx_tetr[2] = { (U32)trd_tx0, (U32)trd_tx1};
        for (int trd = 0; trd < 1+test_type; trd++) {
            for (int chan = 0; chan < 8 - (4 * test_type); chan++)
            {
                U32 base = chan * 0x1000;
                U32 total_err = get_item(dev_tx, tx_tetr[trd], base + REG_TOTAL_ERR_L);
                U32 cnt = total_err, word;
                if (cnt > 16)
                    cnt = 16;
                if (cnt > 0)
                    BRDC_printf(_BRDC("tetrad %d chan %d (total errors %d)\n"), tx_tetr[trd], chan, total_err);
                for (int ii = 0; ii < cnt; ii++)
                {
                    word = base + ii * 0x10;
                    unsigned long long data_rd, data_expect, tmp;
                    U32 index, block;
                    data_rd = get_item(dev_tx, tx_tetr[trd], word + REG_READ_D0);
                    tmp = get_item(dev_tx, tx_tetr[trd], word + REG_READ_D2);
                    data_rd |= tmp << 32;

                    data_expect = get_item(dev_tx, tx_tetr[trd], word + REG_EXPECT_D0);
                    tmp = get_item(dev_tx, tx_tetr[trd], word + REG_EXPECT_D2);
                    data_expect |= tmp << 32;

                    index = get_item(dev_tx, tx_tetr[trd], word + REG_INDEX) & 0xFFFF;

                    block = get_item(dev_tx, tx_tetr[trd], word + REG_BLOCK_D0);

                    tmp = data_expect ^ data_rd;
                    BRDC_printf(_BRDC("ERROR: %4d  block: %-4d  index: %.8X  expect: %.16llX  receive: %.16llX xor: %.16llX \r\n"),
                        ii, block, index, data_expect, data_rd, tmp);
                }
            }
        }
        dev_tx->RegPokeInd(trd_tx0, 0, 0); //stop
        dev_rx->RegPokeInd(trd_rx0, 0, 0);
        if( test_type == 1 )
        {
            dev_tx->RegPokeInd(trd_tx1, 0, 0);
            dev_rx->RegPokeInd(trd_rx1, 0, 0);
        }
    }
    printf("\n");
	return 0;
}
