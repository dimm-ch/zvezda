#include "gpio146test.h"

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
    fprintf(stderr, "-b <lid> - LID of test device\n");
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
    bool is_help = is_option(argc, argv, "-h");
    if (is_help )
    {
        show_help(argv[0]);
        return 0;
    }
    try {
        S32 brd_count = 0;
        brd_dev_t fpga = nullptr;
        if( Bardy::initBardy(brd_count) ) {
            std::vector<U32> lids;
            if( Bardy::boardsLID(lids) ) {
                for( const auto& lid : lids ) {
                    BRD_Info info;
                    if( Bardy::boardInfo(lid, info) ) {

                        // ������� PCIe Device ID
                        uint16_t devid = ((info.boardType >> 16) & 0xffff);
                        if( (devid == 0x5533) )
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
                S32 fmc146_trd = -1;
                if( fpga->get_trd_number(TETR_ID, fmc146_trd) ) {
                    fprintf(stderr, "dev%d: TRD ID: 0x%X - OK, NUMBER: 0x%04X\n", ulid,TETR_ID, fmc146_trd);
                }
                else
                    fprintf(stderr, "dev%d: TRD ID: 0x%X - not found\n", ulid, TETR_ID);
                REG_GPIO reg_gpio = { 0 };           
                U32 nVal = 0;
                fprintf(stderr, "GPIO TEST\n");
                //-------------------------------------------------------
                nVal = fpga->RegPeekInd(fmc146_trd, VPX_GA);
                fprintf(stderr, "GEOGRAPHICAL ADDRESS : 0x%.4X\n",nVal );
                //-------------------------------------------------------
                nVal = fpga->RegPeekInd(fmc146_trd, VPX_DISC);
                fprintf(stderr, "NVMRO, GDISCRETE SIGNALS : 0x%.4X\n",nVal );  
                //-------------------------------------------------------  
                //-------------------------------------------------------
                //-------------------------------------------------------
                //-------------------------------------------------------
                // TEST DIRECTION GPIO1 -> GPIO3
                // TEST DIRECTION GPIO2 -> GPIO4
                reg_gpio.asWhole = 0;
                reg_gpio.byBits.GPIO3_T = 1;
                reg_gpio.byBits.GPIO4_T = 1;
                // copmare 5:4 and 11:10
                fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg_gpio.asWhole);
                IPC_delay(100);
                nVal = fpga->RegPeekInd(fmc146_trd, GPIO_REG);
                if( ((nVal >> 10) & 0xF) != ((reg_gpio.asWhole >> 4) & 0xF) )
                    fprintf(stderr, "1 ERROR: write 0x%X read 0x%X\n", (reg_gpio.asWhole >> 4) & 0xF, (nVal >> 8) & 0xF);
                reg_gpio.byBits.GPIO1_O = 1;
                reg_gpio.byBits.GPIO2_O = 0;
                //write to reg
                fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg_gpio.asWhole);
                IPC_delay(100);
                //read from reg
                nVal = fpga->RegPeekInd(fmc146_trd, GPIO_REG);
                //compare readed to written
                if( ((nVal >> 10) & 0xF) != ((reg_gpio.asWhole >> 4) & 0xF) )
                    fprintf(stderr, "2 ERROR: write 0x%X read 0x%X\n", (reg_gpio.asWhole >> 4 ) & 0xF, (nVal >> 8) & 0xF );
                reg_gpio.byBits.GPIO1_O = 0;
                reg_gpio.byBits.GPIO2_O = 1;
                fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg_gpio.asWhole);
                IPC_delay(100);
                nVal = fpga->RegPeekInd(fmc146_trd, GPIO_REG);
                if( ((nVal >> 10) & 0xF) != ((reg_gpio.asWhole >> 4) & 0xF) )
                    fprintf(stderr, "3 ERROR: write 0x%X read 0x%X\n", (reg_gpio.asWhole >> 4 ) & 0xF, (nVal >> 8) & 0xF);
                reg_gpio.byBits.GPIO1_O = 1;
                reg_gpio.byBits.GPIO2_O = 1;
                fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg_gpio.asWhole);
                IPC_delay(100);
                nVal = fpga->RegPeekInd(fmc146_trd, GPIO_REG);
                if( ((nVal >> 10) & 0xF) != ((reg_gpio.asWhole >> 4) & 0xF) )
                    fprintf(stderr, "4 ERROR: write 0x%X read 0x%X\n", (reg_gpio.asWhole >> 4) & 0xF, (nVal >> 8) & 0xF);
                //---------------------------------
                // write 0xf and the end of the test
                fpga->RegPokeInd(fmc146_trd, GPIO_REG, 0xf);
                fprintf(stderr, "\ntest one direction finished\n");  
                //-------------------------------------------------------
                //-------------------------------------------------------
                //------------------------------------------------------- 
                // TEST DIRECTION GPIO1 <- GPIO3
                // TEST DIRECTION GPIO2 <- GPIO4
                reg_gpio.asWhole = {0};
                reg_gpio.byBits.GPIO1_T = 1;
                reg_gpio.byBits.GPIO2_T = 1;
                // copmare 7:6 and 9:8
                fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg_gpio.asWhole);
                IPC_delay(100);
                uint shiftvalg0 = 6;
                uint shiftvalg1 = 8;
                //unsigned = 0xC;
                nVal = fpga->RegPeekInd(fmc146_trd, GPIO_REG);
                if( ((nVal >> shiftvalg1) & 0x3) != ((reg_gpio.asWhole >> shiftvalg0) & 0x3) )
                    fprintf(stderr, "1 ERROR: write 0x%X read 0x%X\n", (reg_gpio.asWhole >> shiftvalg0) & 0x3, (nVal >> shiftvalg1) & 0x3);
                reg_gpio.byBits.GPIO1_O = 1;
                reg_gpio.byBits.GPIO2_O = 0;
                //write to reg
                fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg_gpio.asWhole);
                IPC_delay(100);
                //read from reg
                nVal = fpga->RegPeekInd(fmc146_trd, GPIO_REG);
                //compare readed to written
                if( ((nVal >> shiftvalg1) & 0xF) != ((reg_gpio.asWhole >> shiftvalg0) & 0xF) )
                    fprintf(stderr, "2 ERROR: write 0x%X read 0x%X\n", (reg_gpio.asWhole >> shiftvalg0 ) & 0xF, (nVal >> shiftvalg1) & 0xF );
                reg_gpio.byBits.GPIO1_O = 0;
                reg_gpio.byBits.GPIO2_O = 1;
                fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg_gpio.asWhole);
                IPC_delay(100);
                nVal = fpga->RegPeekInd(fmc146_trd, GPIO_REG);
                if( ((nVal >> shiftvalg1) & 0xF) != ((reg_gpio.asWhole >> shiftvalg0) & 0xF) )
                    fprintf(stderr, "3 ERROR: write 0x%X read 0x%X\n", (reg_gpio.asWhole >> shiftvalg0 ) & 0xF, (nVal >>shiftvalg1) & 0xF);
                reg_gpio.byBits.GPIO1_O = 1;
                reg_gpio.byBits.GPIO2_O = 1;
                fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg_gpio.asWhole);
                IPC_delay(100);
                nVal = fpga->RegPeekInd(fmc146_trd, GPIO_REG);
                if( ((nVal >> shiftvalg1) & 0xF) != ((reg_gpio.asWhole >> shiftvalg0) & 0xF) )
                    fprintf(stderr, "4 ERROR: write 0x%X read 0x%X\n", (reg_gpio.asWhole >> shiftvalg0) & 0xF, (nVal >> shiftvalg1) & 0xF);
                //---------------------------------
                // write 0xf and the end of the test
                fpga->RegPokeInd(fmc146_trd, GPIO_REG, 0xf); 
                fprintf(stderr, "\ntest other direction finished\n");                   
            }
        }
        return 0;
    }
    catch( const except_info_t& errInfo ) {

        fprintf(stderr, "%s", errInfo.info.c_str());

    }
    catch( ... ) {

        fprintf(stderr, "%s", "Unknown exception in the program!");
    }

    return 0;
}