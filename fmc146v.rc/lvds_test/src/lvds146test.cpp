#include "lvds146test.h"

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

void show_help(const std::string& program)
{
    fprintf(stderr, "usage: %s <options>\n", program.c_str());
    fprintf(stderr, "Options description:\n");
    fprintf(stderr, "-b <lid> - LID of test FPGA\n");
    fprintf(stderr, "-t <test> - test type: 0 - LVDS\n\t1 - LVCMOS VPX\n\t2 - LVCMOS FPGA\n");
    fprintf(stderr, "-r <lid> - LID of second test FPGA\n");
    fprintf(stderr, "-h - Print this message\n");
}

int main(int argc, char* argv[])
{
    fprintf(stderr, "---------------------------\n");
    fprintf(stderr, " TEST: %s\n", argv[0]);
    fprintf(stderr, "---------------------------\n");

    setlocale(LC_ALL, "Russian");
    signal(SIGINT, signal_handler);

    S32 ulid = get_from_cmdline(argc, argv, "-b", -1);
    S32 rlid = get_from_cmdline(argc, argv, "-r", -1);
    U32 ntest_type = get_from_cmdline(argc, argv, "-t", 10);
    bool is_help = is_option(argc, argv, "-h");
    if( (ntest_type == 10) || is_help )
    {
        show_help(argv[0]);
        return 0;
    }

    if( ((ntest_type == 2)||(ntest_type == 0)) && ((rlid == -1) || (rlid == ulid)) )
    {
        fprintf(stderr, "ERROR: need 2 different LIDs for test\n");
        return -1;
    }

    try 
    {
        S32 brd_count = 0;
        brd_dev_t fpga = nullptr;
        if( Bardy::initBardy(brd_count) ) 
        {
            std::vector<U32> lids;
            if( Bardy::boardsLID(lids) ) 
            {
                for( const auto& lid : lids ) 
                {
                    BRD_Info info;
                    if( Bardy::boardInfo(lid, info) ) 
                    {

                        // Получим PCIe Device ID
                        uint16_t devid = ((info.boardType >> 16) & 0xffff);
                        if( (devid == 0x5534) )
                            if( ulid == -1 )
                                ulid = lid;
                    }
                }
                if( ulid != -1 )
                    fpga = get_device(ulid);
                else
                {
                    fprintf(stderr, "ERROR: no lid found\n");
                }
                S32 main_trd = -1;
                if( fpga->get_trd_number(1, main_trd) ) 
                {
                    fprintf(stderr, "FPGA%d: TRD ID: 1 - OK, NUMBER: 0x%04X\n", ulid, main_trd);
                }
                else
                    fprintf(stderr, "FPGA%d: TRD ID: 1 - not found\n", ulid);
                REG_TST_OUT reg_out = { 0 }, reg_out2 = { 0 };
                REG_TST_IN reg_in = { 0 }, reg_in2 = { 0 };
                fpga->RegPokeInd(main_trd, MAIN_REG_TST_OUT, 0);
                fpga->RegPokeInd(main_trd, MAIN_REG_TST_IN, 0);
                switch( ntest_type )
                {
                case 0://LVDS VPX
                    fprintf(stderr, "LVDS VPX TEST\n");
                    if( ulid != rlid )
                    {
                        brd_dev_t fpga2 = get_device(rlid);
                        fpga2->RegPokeInd(main_trd, MAIN_REG_TST_OUT, 0);
                        fpga2->RegPokeInd(main_trd, MAIN_REG_TST_IN, 0);
                        reg_out.asWhole = 0;
                        reg_in.asWhole = 0;
                        reg_out.byBits.LVDS_VPX1 = 1;
                        for( int i = 0; i < 6; i++ )
                        {
                            fpga->RegPokeInd(main_trd, MAIN_REG_TST_OUT, reg_out.asWhole);
                            ipc_delay(100);
                            reg_in.asWhole = fpga->RegPeekInd(main_trd, MAIN_REG_TST_IN);
                            if( (reg_in.asWhole & 0x7E) != (reg_out.asWhole & 0x7E) )
                            {
                                fprintf(stderr, "ERROR:LID%d LVDS VPX%d (OUT=0x%X IN=0x%X)\n", ulid, i + 1, reg_out.asWhole & 0x7E, reg_in.asWhole & 0x7E);
                            }
                            reg_in.asWhole = fpga2->RegPeekInd(main_trd, MAIN_REG_TST_IN);
                            if( (reg_in.asWhole & 0x7E) != (reg_out.asWhole & 0x7E) )
                            {
                                fprintf(stderr, "ERROR:LID%d LVDS VPX%d (OUT=0x%X IN=0x%X)\n", rlid, i + 1, reg_out.asWhole & 0x7E, reg_in.asWhole & 0x7E);
                            }
                            reg_out.asWhole = reg_out.asWhole << 1;
                        }
                        fprintf(stderr, "\ntest finished\n");
                    }
                    break;
                case 1://LVCMOS VPX
                    fprintf(stderr, "LVCMOS VPX TEST\n");
                    reg_out.asWhole = 0;
                    reg_in.asWhole = 0;
                    reg_out.byBits.LVCMOS_VPX1 = 1;
                    for( int i = 0; i < 4; i++ )
                    {
                        fpga->RegPokeInd(main_trd, MAIN_REG_TST_OUT, reg_out.asWhole);
                        ipc_delay(100);
                        reg_in.asWhole = fpga->RegPeekInd(main_trd, MAIN_REG_TST_IN);
                        if( (reg_in.asWhole & 0xF00) != (reg_out.asWhole & 0xF00) )
                        {
                            fprintf(stderr, "ERROR: LVCMOS VPX%d (OUT=0x%X IN=0x%X)\n", i + 1, reg_out.asWhole & 0xF00, reg_in.asWhole & 0xF00);
                        }
                        reg_out.asWhole = reg_out.asWhole << 1;
                    }
                    fprintf(stderr, "\ntest finished\n");
                    break;
                case 2://LVCMOS FPGA
                    fprintf(stderr, "LVCMOS FPGA TEST\n");
                    if( ulid != rlid )
                    {
                        brd_dev_t fpga2 = get_device(rlid);
                        fpga2->RegPokeInd(main_trd, MAIN_REG_TST_OUT, 0);
                        fpga2->RegPokeInd(main_trd, MAIN_REG_TST_IN, 0);
                        reg_out.asWhole = 0;
                        reg_in.asWhole = 0;
                        reg_out2.asWhole = 0;
                        reg_in2.asWhole = 0;
                        reg_out.byBits.LVCMOS_FPGA0 = 1;
                        for( int i = 0; i < 4; i++ )
                        {
                            fpga->RegPokeInd(main_trd, MAIN_REG_TST_OUT, reg_out.asWhole);
                            ipc_delay(100);
                            reg_in2.asWhole = fpga2->RegPeekInd(main_trd, MAIN_REG_TST_IN);
                            if ((reg_in2.asWhole & 0xF000) != (reg_out.asWhole & 0xF000))
                                fprintf(stderr, "ERROR: LVCMOS FPGA%d (OUT%d=0x%X IN%d=0x%X)\n", ulid, i + 1, reg_out.asWhole & 0xF000, rlid, reg_in2.asWhole & 0xF000);
                            reg_out2.asWhole = reg_out.asWhole;
                            fpga2->RegPokeInd(main_trd, MAIN_REG_TST_OUT, reg_out.asWhole);
                            ipc_delay(100);
                            reg_in.asWhole = fpga->RegPeekInd(main_trd, MAIN_REG_TST_IN);
                            if( (reg_in.asWhole & 0xF000) != (reg_out2.asWhole & 0xF000) )
                                fprintf(stderr, "ERROR: LVCMOS FPGA%d (OUT%d=0x%X IN%d=0x%X)\n", rlid, i + 1, reg_out2.asWhole & 0xF000, ulid, reg_in.asWhole & 0xF000);
                            reg_out.asWhole = reg_out.asWhole << 1;
                        }
                        fprintf(stderr, "\ntest finished\n");
                    }
                    else
                        fprintf(stderr, "ERROR: need 2 different LIDs for LVCMOS FPGA test\n");
                    break;
                default:
                    break;
                }
            }
        }
    }
    catch( const except_info_t& errInfo ) {

        fprintf(stderr, "%s", errInfo.info.c_str());

    }
    catch( ... ) {

        fprintf(stderr, "%s", "Unknown exception in the program!");
    }

    return 0;
}