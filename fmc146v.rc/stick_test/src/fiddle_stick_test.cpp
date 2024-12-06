#include "bardy.h"
#include "brd_dev.h"
#include "exceptinfo.h"
#include "strconv.h"
#include "test_buf.h"
#include "time_ipc.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <math.h>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <string>
#include <sys/time.h>
#include <thread>
#include <unistd.h>

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

#define TETR_ID 0x333

#define IZA_REG 0x10 //OUT
#define MOSI_REG 0x11 //OUT
#define LVDS_FPGA_REG 0x1F
#define BK_IN_REG 0x210 //IN
#define MISO_REG 0x211 //IN
#define LVDS_FPGA_DATA_REG 0x21F

typedef union
{
    U16 asWhole;
    struct {
        U08 IZA : 1;
        U08 IPA : 1;
        U08 BLABK : 1;
        U08 BK_OUT0 : 1;
        U08 BK_OUT1 : 1;
        U08 BK_OUT2 : 1;
        U08 BK_OUT3 : 1;
        U08 DS : 1;
        U08 IBP : 1;
        U08 D0 : 1;
        U08 LIZA : 1;
        U08 SPI_CLK0 : 1;
        U08 SPI_CLK1 : 1;
        U08 SPI_SS0 : 1;
        U08 SPI_SS1 : 1;
        U08 res : 1;
    }byBits;
} REG_IZA;

typedef union
{
    U16 asWhole;
    struct {
        U08 SPI_MOSI0 : 1;
        U08 SPI_MOSI1 : 1;
        U08 SPI_MOSI2 : 1;
        U08 SPI_MOSI3 : 1;
        U08 SPI_MOSI4 : 1;
        U08 SPI_MOSI5 : 1;
        U08 SPI_MOSI6 : 1;
        U08 SPI_MOSI7 : 1;
        U08 res;
    }byBits;
} REG_MOSI;

typedef union
{
    U16 asWhole;
    struct {
        U08 LVDS_FPGA0 : 1;
        U08 LVDS_FPGA1 : 1;
        U08 LVDS_FPGA2 : 1;
        U08 LVDS_FPGA3 : 1;
        U16 res : 12;
    }byBits;
} REG_LVDS_FPGA;

typedef union
{
    U16 asWhole;
    struct {
        U08 res : 3;
        U08 BK_IN0 : 1;
        U08 BK_IN1 : 1;
        U08 BK_IN2 : 1;
        U08 BK_IN3 : 1;
        U08 TI : 1;
        U08 res2;
    }byBits;
} REG_BKIN;

typedef union
{
    U16 asWhole;
    struct {
        U08 res : 6;
        U08 SPI_MISO0 : 1;
        U08 SPI_MISO1 : 1;
        U08 SPI_MISO2 : 1;
        U08 SPI_MISO3 : 1;
        U08 SPI_MISO4 : 1;
        U08 SPI_MISO5 : 1;
        U08 SPI_MISO6 : 1;
        U08 SPI_MISO7 : 1;
        U08 res2 : 2;
    }byBits;
} REG_MISO;

typedef union
{
    U16 asWhole;
    struct {
        U08 LVDS_FPGA0 : 1;
        U08 LVDS_FPGA1 : 1;
        U08 LVDS_FPGA2 : 1;
        U08 LVDS_FPGA3 : 1;
        U16 res : 12;
    }byBits;
} REG_LVDS_FPGA_DATA;

int main(int argc, char* argv[])
{
    fprintf(stderr, "---------------------------\n");
    fprintf(stderr, " TEST: %s\n", argv[0]);
    fprintf(stderr, "---------------------------\n");

    setlocale(LC_ALL, "Russian");
    signal(SIGINT, signal_handler);

    try {
        S32 brd_count = 0;
        brd_dev_t fpga = nullptr;
        if( Bardy::initBardy(brd_count) ) {
            std::vector<U32> lids;
            if( Bardy::boardsLID(lids) ) {
                for( const auto& lid : lids ) {
                    BRD_Info info;
                    if( Bardy::boardInfo(lid, info) ) {

                        // Получим PCIe Device ID
                        uint16_t devid = ((info.boardType >> 16) & 0xffff);
                        if( (devid == 0x5531) || (devid == 0x5532) )
                            fpga = get_device(lid);
                    }
                }
                S32 fiddle_trd = -1;
                if( fpga->get_trd_number(0x333, fiddle_trd) ) {
                    fprintf(stderr, "FPGA1: TRD ID: 0x%04X - OK, NUMBER: 0x%04X\n", TETR_ID, fiddle_trd);
                }
                else {
                    fprintf(stderr, "FPGA1: TRD ID 0x%04X - NOT FOUND.\n", TETR_ID);
                }
                //cable SPI#1->X6 SPI0_MISO <- SPI0_MOSI; SPI1_MISO <- SPI1_MOSI; SPI2_MISO <- SPI2_MOSI 
                // SPI3_MISO <- SPI3_MOSI
                bool is_error = false;
                REG_MISO rMISO = { 0 };
                REG_MOSI rMOSI = { 0 };
                REG_IZA rIZA = { 0 };
                REG_BKIN rBKIN = { 0 };
                fprintf(stderr, "Установите кабель SPI в разъем X6\n");
                IPC_getch();
                rMOSI.byBits.SPI_MOSI0 = 1;
                rMOSI.byBits.SPI_MOSI1 = 1;
                rMOSI.byBits.SPI_MOSI2 = 1;
                rMOSI.byBits.SPI_MOSI3 = 1;                
                fpga->wi(fiddle_trd, MOSI_REG, rMOSI.asWhole);
                rMISO.asWhole = fpga->ri(fiddle_trd, MISO_REG);
                if( rMISO.byBits.SPI_MISO0 != 1 )
                {
                    fprintf(stderr, "ERROR SPI_MOSI0 -> SPI_MISO0\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO1 != 1 )
                {
                    fprintf(stderr, "ERROR SPI_MOSI1 -> SPI_MISO1\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO2 != 1 )
                {
                    fprintf(stderr, "ERROR SPI_MOSI2 -> SPI_MISO2\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO3 != 1 )
                {
                    fprintf(stderr, "ERROR SPI_MOSI3 -> SPI_MISO3\n");
                    is_error = true;
                }
                if( !is_error )
                    fprintf(stderr, "Проверка прошла успешно, ошибок нет\n");

                // cable SPI#2->X6 SPI0_CLK -> SPI0_MISO; SPI0_FSS -> SPI1_MISO
                is_error = false;
                fprintf(stderr, "Установите кабель SPI в разъем X6\n");
                IPC_getch();
                rIZA.byBits.SPI_CLK0 = 1;
                rIZA.byBits.SPI_SS0 = 1;
                fpga->wi(fiddle_trd, IZA_REG, rIZA.asWhole);
                rMISO.asWhole = fpga->ri(fiddle_trd, MISO_REG);
                if( rMISO.byBits.SPI_MISO0 != 1 )
                {
                    fprintf(stderr, "ERROR SPI_CLK0 -> SPI_MISO0\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO1 != 1 )
                {
                    fprintf(stderr, "ERROR SPI_SS0 -> SPI_MISO1\n");
                    is_error = true;
                }
                if( !is_error )
                    fprintf(stderr, "Проверка прошла успешно, ошибок нет\n");

                //cable SPI#1->X5 SPI4_MISO <- SPI4_MOSI; SPI5_MISO <- SPI5_MOSI; SPI6_MISO <- SPI6_MOSI 
                // SPI7_MISO <- SPI7_MOSI
                is_error = false;
                fprintf(stderr, "Установите кабель SPI в разъем X5\n");
                IPC_getch();
                rMOSI.byBits.SPI_MOSI4 = 1;
                rMOSI.byBits.SPI_MOSI5 = 1;
                rMOSI.byBits.SPI_MOSI6 = 1;
                rMOSI.byBits.SPI_MOSI7 = 1;
                fpga->wi(fiddle_trd, MOSI_REG, rMOSI.asWhole);
                rMISO.asWhole = fpga->ri(fiddle_trd, MISO_REG);
                if( rMISO.byBits.SPI_MISO4 != 1 )
                {
                    fprintf(stderr, "ERROR SPI4_MOSI -> SPI4_MISO\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO5 != 1 )
                {
                    fprintf(stderr, "ERROR SPI5_MOSI -> SPI5_MISO\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO6 != 1 )
                {
                    fprintf(stderr, "ERROR SPI6_MOSI -> SPI6_MISO\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO7 != 1 )
                {
                    fprintf(stderr, "ERROR SPI7_MOSI -> SPI7_MISO\n");
                    is_error = true;
                }
                if( !is_error )
                    fprintf(stderr, "Проверка прошла успешно, ошибок нет\n");

                // cable SPI#2->X5 SPI4_CLK -> SPI4_MISO; SPI4_FSS -> SPI5_MISO
                is_error = false;
                fprintf(stderr, "Установите кабель SPI в разъем X5\n");
                IPC_getch();
                rIZA.byBits.SPI_CLK1 = 1;
                rIZA.byBits.SPI_SS1 = 1;
                fpga->wi(fiddle_trd, IZA_REG, rIZA.asWhole);
                rMISO.asWhole = fpga->ri(fiddle_trd, MISO_REG);
                if( rMISO.byBits.SPI_MISO4 != 1 )
                {
                    fprintf(stderr, "ERROR SPI_CLK4 -> SPI_MISO4\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO5 != 1 )
                {
                    fprintf(stderr, "ERROR SPI_SS4 -> SPI_MISO5\n");
                    is_error = true;
                }
                if( !is_error )
                    fprintf(stderr, "Проверка прошла успешно, ошибок нет\n");

                //cable GPIO#1 BK_OUT0 -> BK_IN0; BK_IN1 <- BK_OUT1; BK_IN2 <- BK_OUT2; BK_IN3 <- BK_OUT3
                is_error = false;
                fprintf(stderr, "Установите кабель GPIO в разъем X7\n");
                IPC_getch();
                rIZA.asWhole = 0;
                rIZA.byBits.BK_OUT0 = 1;
                rIZA.byBits.BK_OUT1 = 1;
                rIZA.byBits.BK_OUT2 = 1;
                rIZA.byBits.BK_OUT3 = 1;
                fpga->wi(fiddle_trd, IZA_REG, rIZA.asWhole);
                rBKIN.asWhole = fpga->ri(fiddle_trd, BK_IN_REG);
                if( rBKIN.byBits.BK_IN0 != 1 )
                {
                    fprintf(stderr, "ERROR БК_Выход0 -> БК_Вход0\n");
                    is_error = true;
                }
                if( rBKIN.byBits.BK_IN1 != 1 )
                {
                    fprintf(stderr, "ERROR БК_Выход1 -> БК_Вход1\n");
                    is_error = true;
                }
                if( rBKIN.byBits.BK_IN2 != 1 )
                {
                    fprintf(stderr, "ERROR БК_Выход2 -> БК_Вход2\n");
                    is_error = true;
                }
                if( rBKIN.byBits.BK_IN3 != 1 )
                {
                    fprintf(stderr, "ERROR БК_Выход3 -> БК_Вход3\n");
                    is_error = true;
                }
                if( !is_error )
                    fprintf(stderr, "Проверка прошла успешно, ошибок нет\n");

                //cable GPIO#2 BK_IN0 <- IPA; BK_IN1 <- IZA
                is_error = false;
                fprintf(stderr, "Установите кабель GPIO в разъем X7\n");
                IPC_getch();
                rIZA.asWhole = 0;
                rIZA.byBits.IPA = 1;
                rIZA.byBits.IZA = 1;
                fpga->wi(fiddle_trd, IZA_REG, rIZA.asWhole);
                rBKIN.asWhole = fpga->ri(fiddle_trd, BK_IN_REG);
                if( rBKIN.byBits.BK_IN0 != 1 )
                {
                    fprintf(stderr, "ERROR ИПА -> БК_Вход0\n");
                    is_error = true;
                }
                if( rBKIN.byBits.BK_IN1 != 1 )
                {
                    fprintf(stderr, "ERROR ИЗА -> БК_Вход1\n");
                    is_error = true;
                }
                if( !is_error )
                    fprintf(stderr, "Проверка прошла успешно, ошибок нет\n");

                //cable SYNC->X6 SPI0_MISO <- DS; SPI1_MISO <- IBP; SPI2_MISO <- D0; SPI3_MISO <- LIZA; SPI3_MOSI -> TI
                is_error = false;
                fprintf(stderr, "Установите кабель SYNC в разъем X6\n");
                IPC_getch();
                rIZA.asWhole = 0;
                rIZA.byBits.DS = 1;
                rIZA.byBits.IBP = 1;
                rIZA.byBits.D0 = 1;
                rIZA.byBits.LIZA = 1;
                fpga->wi(fiddle_trd, IZA_REG, rIZA.asWhole);
                rMISO.asWhole = fpga->ri(fiddle_trd, MISO_REG);
                if( rMISO.byBits.SPI_MISO0 != 1 )
                {
                    fprintf(stderr, "ERROR D сумма -> SPI0_MISO\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO1 != 1 )
                {
                    fprintf(stderr, "ERROR D сумма -> SPI0_MISO\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO2 != 1 )
                {
                    fprintf(stderr, "ERROR D0 -> SPI2_MISO\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO3 != 1 )
                {
                    fprintf(stderr, "ERROR ИЗА -> SPI3_MISO\n");
                    is_error = true;
                }

                rMOSI.asWhole = 0;
                rMOSI.byBits.SPI_MOSI3 = 1;
                fpga->wi(fiddle_trd, MOSI_REG, rMOSI.asWhole);
                rBKIN.asWhole = fpga->ri(fiddle_trd, BK_IN_REG);
                if( rBKIN.byBits.TI != 1 )
                {
                    fprintf(stderr, "ERROR SPI3_MOSI -> ТИ\n");
                    is_error = true;
                }
                if( !is_error )
                    fprintf(stderr, "Проверка прошла успешно, ошибок нет\n");

                //cable SYNC->X5 SPI4_MISO <- DS; SPI5_MISO <- IBP; SPI6_MISO <- D0; SPI7_MISO <- IZA; SPI7_MOSI -> TI
                is_error = false;
                fprintf(stderr, "Установите кабель SYNC в разъем X5\n");
                IPC_getch();
                rIZA.asWhole = 0;
                rIZA.byBits.DS = 1;
                rIZA.byBits.IBP = 1;
                rIZA.byBits.D0 = 1;
                rIZA.byBits.LIZA = 1;
                fpga->wi(fiddle_trd, IZA_REG, rIZA.asWhole);
                rMISO.asWhole = fpga->ri(fiddle_trd, MISO_REG);
                if( rMISO.byBits.SPI_MISO4 != 1 )
                {
                    fprintf(stderr, "ERROR D сумма -> SPI4_MISO\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO5 != 1 )
                {
                    fprintf(stderr, "ERROR D сумма -> SPI5_MISO\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO6 != 1 )
                {
                    fprintf(stderr, "ERROR D0 -> SPI6_MISO\n");
                    is_error = true;
                }
                if( rMISO.byBits.SPI_MISO7 != 1 )
                {
                    fprintf(stderr, "ERROR ИЗА -> SPI7_MISO\n");
                    is_error = true;
                }

                rMOSI.asWhole = 0;
                rMOSI.byBits.SPI_MOSI7 = 1;
                fpga->wi(fiddle_trd, MOSI_REG, rMOSI.asWhole);
                rBKIN.asWhole = fpga->ri(fiddle_trd, BK_IN_REG);
                if( rBKIN.byBits.TI != 1 )
                {
                    fprintf(stderr, "ERROR SPI7_MOSI -> ТИ\n");
                    is_error = true;
                }
                if( !is_error )
                    fprintf(stderr, "Проверка прошла успешно, ошибок нет\n");
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