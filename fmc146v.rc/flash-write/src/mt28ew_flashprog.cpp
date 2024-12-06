#include <stdio.h>
#ifndef __linux__
#include <conio.h>
#endif
#include <locale.h>
#include <time.h>

#include "bardy.h"
#include "brd_dev.h"
#include "brderr.h"
#include "ctrlreg.h"
#include "gipcy.h"
#include "mt28ew_flashprog.h"
#include "strconv.h"
#include "time_ipc.h"
#include "utypes.h"

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

//--------------------------------------------------------------------

int MainLoop(brd_dev_t dev);

//--------------------------------------------------------------------

U16 g_BuffIn[MAXBUFSIZE];
U16 g_BuffOut[MAXBUFSIZE];
U32 g_addr = 0;
U32 g_size = 0;
U32 g_mode = 0;
U32 g_nLid = 1;
U32 trd_flash = 0;

BRDCHAR g_FileName[1024];
std::string filename = "";

BRD_Handle m_hReg;
BRD_Handle g_hBrd = 0;
BRD_Info info;

LONG GetMCSDataSize(const BRDCHAR *FileName)
{
	LONG nDataSize = 0;
	FILE *pFile;
	pFile = BRDC_fopen(FileName, _BRDC("rb"));
	if (pFile != 0)
	{
		UINT nOffset = 0;
		UINT nAddr = 0, nType = 0, nLength = 0, nAddrPrev = 0;
		while (!feof(pFile))
		{
			char str[256] = { 0 };
			nAddr = 0; nType = 0; nLength = 0;
			fgets(str, 256, pFile);
			if (feof(pFile))
				break;
			if (str[0] != ':')
			{
				if (feof(pFile))
					break;
				else
					return 0;
			}
			sscanf(str + 1, "%2x", &nLength);
			sscanf(str + 3, "%4x", &nAddr);
			sscanf(str + 7, "%2x", &nType);
			nLength = (nLength & 0xFF);
			nAddr = (nAddr & 0xFFFF);
			nType = (nType & 0xFF);
			if (nType == 4)
			{
				continue;
			}
			if (nType == 1)
			{
				break;
			}
			if ((nAddr != nAddrPrev) && (nAddrPrev != 0))
				nAddrPrev = nAddr;
			nAddrPrev = nAddr + nLength;
			nDataSize += nLength;
			if (feof(pFile))
				break;
		}
		BRDC_fclose(pFile);
	}
	return nDataSize;
}


//--------------------------------------------------------------------

int BRDC_main(int argc, BRDCHAR* argv[])
{
    Gipcy G;
    if (!G.is_ok()) {
        fprintf(stderr, "GIPCY library not loaded!\n");
        return -1;
    }

    BRDC_printf(_BRDC("MT28EW_FLASHPROG.\n"));

    if (argc == 1) {
        DisplayUsageMsg();
        return 0;
    };

    if (is_option(argc, argv, "-i")) {
        g_mode |= IS_ID;
    }

    if (is_option(argc, argv, "-r")) {
        g_mode |= IS_RD;
    }

    if (is_option(argc, argv, "-w")) {
        g_mode |= IS_WR;
    }

    if (is_option(argc, argv, "-fe")) {
        g_mode |= IS_FE;
    }

    if (is_option(argc, argv, "-be")) {
        g_mode |= IS_BE;
    }

    if (is_option(argc, argv, "-bc")) {
        g_mode |= IS_BC;
    }

    g_size = get_from_cmdline<U32>(argc, argv, "-s", 0);

    filename = get_from_cmdline<std::string>(argc, argv, "-f", "");
    if (filename.empty()) {
        return -1;
    }

    g_addr = get_from_cmdline<U32>(argc, argv, "-a", 0);

    g_nLid = get_from_cmdline<U32>(argc, argv, "-b", -1);
    if (g_nLid < 0) {
        return -1;
    }

    if (is_option(argc, argv, "-l")) {
        g_mode |= IS_IPROG;
    }

    S32 brd_count = 0;
    if (!Bardy::initBardy(brd_count)) {
        return -1;
    }

    brd_dev_t dev = get_device(g_nLid);
    if (!dev) {
        BRDC_fprintf(stderr, _BRDC("Ошибка при открытии модуля!\n"));
        return -1;
    }

    info = dev->board_info();
    BRDC_printf(_BRDC("\r\nПараметры модуля:\r\n")
                    _BRDC(" LID:       %d\r\n")
                        _BRDC(" BoardType: %.8X \r\n")
                            _BRDC(" Имя:       %s  v%d.%d\r\n")
                                _BRDC(" Номер:     %d\r\n")
                                    _BRDC(" Bus:       %d\r\n")
                                        _BRDC(" Dev:       %d\r\n")
                                            _BRDC(" Slot:      %d\r\n\r\n"),
        g_nLid,
        info.boardType,
        info.name,
        (info.boardType >> 4) & 0xF,
        info.boardType & 0xF,
        info.pid,
        info.bus,
        info.dev,
        info.slot);

    return MainLoop(dev);
}

//--------------------------------------------------------------------

int MainLoop(brd_dev_t dev)
{
    int ret;
    // xxxxxxxxxxxxxxxxxxxxxxxxx
    U32 Bytes = 0;
    for (U32 trdNo = 1; trdNo < 16; trdNo++) {
        ret = dev->ri(trdNo, 0x100);
        if (ret == TRD_MT28EW_ID) {
            trd_flash = trdNo;
            break;
        }
    }

    if (trd_flash == 0) {
        BRDC_fprintf(stderr, _BRDC("ОШИБКА: Тетрада (ID : 0x%X) не найдена\n"), TRD_MT28EW_ID);
        return -1;
    }

    ////xxxxxxxxxxxxxxxx Check test Params xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    BRDC_printf(_BRDC("%-24s %d\n"),
        _BRDC("Номер тетрады :"), trd_flash);

    {

        if ((g_mode & IS_RD) || (g_mode & IS_WR)) {
            if (!filename.empty()) {
                BRDC_printf(_BRDC("%-24s 0x%X\n")
                                _BRDC("%-24s %s\n"),
                    _BRDC("Начальный адрес :"), g_addr,
                    _BRDC("Имя файла :"), filename.c_str());

                Bytes = GetFileSize();
                BRDC_printf(_BRDC("Размер файла %s :\t %d (Байт)\n"), filename.c_str(), Bytes);

                U32 block_num = Bytes / 0x20000; // 128 KB block size
                U32 rem = Bytes % 0x20000;

                if (rem != 0)
                    block_num++;

                BRDC_printf(_BRDC("Займет :\t %d/0x%X (Блоков/Слов х16 (*1024))\n"), block_num, block_num * 0x10000);
            } else {
                BRDC_fprintf(stderr, _BRDC("\nОШИБКА : Файл - источник данных не указан.\n"));
                return -1;
            }
        }
    }

    
    BRDC_printf(_BRDC("\n\n"));

    // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    dev->wi(trd_flash, TRDIND_MODE0, 1);
    dev->wi(trd_flash, TRDIND_MODE0, 0);

    // xxxxxxxxxxxxxxxxxxxx Read Identification xxxxxxxxxxxxxxxxxxxxxxxxxx
    if (g_mode & IS_ID) {
        ReadID(dev);
    }

    //	//xxxxxxxxxxxxxxxxxxxxxxxx CHIP ERASE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    if (g_mode & IS_FE) {
        ////The CHIP ERASE(80/10h) command erases the entire chip.
        ////During the CHIP ERASE operation, the device ignores all other commands, including
        ////ERASE SUSPEND.It is not possible to abort the operation.All bus READ operations during
        ////CHIP ERASE output the data polling register on the data I/Os
        fprintf(stderr, "ERASING FLASH..\n");
        ChipErase(dev);
    }

    // xxxxxxxxxxxxxxxxxxxxxx BLOCK ERASE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    if (g_mode & IS_BE) {
        ////The main memory array is divided into 128KB or 64KW uniform blocks
        ////The BLOCK ERASE(80/30h) command erases a list of one or more blocks
        ////Any command except BLOCK ERASE or ERASE SUSPEND during this timeout period resets that block to the read mode.
        ////The system can monitor DQ3 to determine if the block erase timer has timed out.
        ////After the program / erase controller has started, it is not possible to select any more
        ////blocks.Each additional block must therefore be selected within the timeout period of
        ////the last block.The timeout timer restarts when an additional block is selected

        BlockErase(dev);
    }

    // xxxxxxxxxxxxxxxxxxxxxx BLANK CHECK xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    if (g_mode & IS_BC) {
        ////The main memory array is divided into 128KB or 64KW uniform blocks
        ////The BLANK CHECK operation determines whether a specified block is blank(that is,
        ////completely erased).It can also be used to determine whether a previous ERASE operation
        ////was successful, including ERASE operations that might have been interrupted by
        ////power loss.
        fprintf(stderr, "BLANCKECKING FLASH..\n");
        BlankCheck(dev);
    }

    //	//xxxxxxxxxxxxxxxxxxxxxxx Write Data xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    if (g_mode & IS_WR) {
        fprintf(stderr, "WRITING FLASH..\n");
        WriteData(dev,Bytes);
    }
    // xxxxxxxxxxxxxxxxxxxxxxx Read Data xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

    if (g_mode & IS_RD) {
        fprintf(stderr, "VERIFYING FLASH..\n");
        ReadData(dev,Bytes);
    }

    // xxxxxxxxxxxxxxxxxxxxxx IPROG Command xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    if (g_mode & IS_IPROG) {
#ifndef __linux__
        if (g_mode & IS_IPROG) {

            // IprogCmd(trd_flash);
            handle = UniDrv_Open(0);
            if (handle == 0) {
                printf("ISPCICFG ERROR: Can't Open ISDRV\n\n");
            }

            // char* cCmdKey;
            // char* endptr;

            int err = 0;
            // Define PCI Cards
            // if (venID != 0 && devID != 0)
            {
                U32 devID = (info.boardType >> 16) & 0xFFFF;
                int bus = FindDevice(devID, 0);
                while (bus) {
                    bus = FindBridge(bus);
                    if (bus < 0) {
                        err = 1;
                        break;
                    }
                }
                IprogCmd(trd_flash);
                Sleep(500);

                if (!err) {
                    OffAndOn();
                }
            }
            // else
            //{
            //	printf("ISPCICFG ERROR: Specify VendorID & DeviceID in command line\n\n");
            //	_getch();
            // }

            for (int i = 0; i < dev_num; i++)
                delete pDevices[i];

            UniDrv_Close(handle);
        }
#endif
    }

    BRDC_printf(_BRDC("\nПриложение завершено...\n"));

    return 0;
}

void DisplayUsageMsg(void)
{
    BRDC_printf(_BRDC("%s\n"),
        _BRDC(" Программирование памяти FLASH.\n")
            _BRDC(" Использование:\n")
                _BRDC("\tflashprog [-fe] [-be] [-bc] [-a <начальный адрес>] [-s<размер>] [-i] [-r] [-w] [-f <имя файла>] [-b<lid>] [-o <экземпляр>]\n")
                    _BRDC("\t-с\tПрограмирование со стороны несущей платы\n")
                        _BRDC("\t-i\tЧтение информации FLASH\n")
                            _BRDC("\t-fe\tCтирание всей памяти\n")
                                _BRDC("\t-r\tОперация записи\n")
                                    _BRDC("\t-w\tОперация чтения\n")
                                        _BRDC("\t-f\tИмя файла с данными для программирования\n")
                                            _BRDC("\t-bn\tn - Lid - идентификатор модуля\n")
                                                _BRDC("\t-bc\tПроверка блока (стерт/записан)\n")
                                                    _BRDC("\t-a \tНачальный адрес\n")
                                                        _BRDC("\t-s \tРазмер (в словах х16)\n")

    );
}

// xxxxxxxxxxx Crc xxxxxxxxxxxxxxxxxxxxx
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
U08 Crc(U08* pcBlock, U08 len)
{
    U08 crc = 0x00;

    while (len--)
        crc += *pcBlock++;

    crc = ~crc;
    crc += 1;
    crc &= 0xFF;
    return crc;
}

// xxxxxxxxxxxx GetFileSize xxxxxxxxxxxxxxxxxxxxxxx
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
U32 GetFileSize(void)
{
    bool do_cycle_again = true;
    FILE* stream;
    char str[261];
    U32 reclen;
    U32 offset;
    U32 rectype;
    U32 Bytes = 0;

    U08 byte[261];
    U32 data;
    U32 AddrUpperTwoBytes;
    U32 AddrWords;

    stream = BRDC_fopen(filename.c_str(), _BRDC("rt"));

    if (stream == NULL) {
        BRDC_fprintf(stderr, _BRDC("\n*** Can't find file \"%s\"\n"), filename.c_str());
        return -1;
    }

    do {
        fgets(str, 261, stream);

        if (str[0] != ':') {
            BRDC_fprintf(stderr, _BRDC("\nWrong format!\n"));
            return -1;
        }

        sscanf(str + 1, "%2x", &reclen);
        sscanf(str + 3, "%4x", &offset);
        sscanf(str + 7, "%2x", &rectype);

        if (rectype == 0x1) // End-of-file record
            do_cycle_again = false;

        byte[0] = (U08)reclen;
        byte[1] = (U08)(offset & 0xFF);
        byte[2] = (U08)(offset >> 8) & 0xFF;
        byte[3] = (U08)rectype;

        if (rectype == 0x4) // extended linear address record
        {
            for (int j = 0; j < reclen; j++) {
                sscanf(str + 9 + 2 * j, "%2x", &data);
                byte[4 + j] = (U08)data;
            }

            AddrUpperTwoBytes = (byte[4] << 8) | byte[5];
            Bytes = 0;
            continue;
        } else if (rectype == 0) // data record
                                 //		if (rectype == 0) // data record
        {
            AddrWords = AddrUpperTwoBytes;
            AddrWords <<= 16;
            AddrWords &= 0xFFFF0000;
            AddrWords |= offset & 0xFFFF;
            Bytes += reclen; // number of data bytes
        }

    } while (do_cycle_again);

    fclose(stream);

    return AddrWords + Bytes;
}

// xxxxxxxxxxxxxxxxxxxx Read Identification xxxxxxxxxxxxxxxxxxxxxxxxxx

void ReadID(brd_dev_t dev)
{
    U32 size_byte = 3;

    // READ CFI(98h)
    // CMD(98h)
    dev->wi(trd_flash, TRDIND_DATA, 0x98);
    dev->wi(trd_flash, TRDIND_ADDR_H, 0x0);
    // dev->wi(trd_flash, 16#201#, x"0555"); //(Из документации)
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x55); //(По модели)

    // Read CFI data
    //  addr = 1Bh (VCC logic supply minimum program/erase voltage)
    // Bits[7:4] BCD value in volts
    // Bits[3:0] BCD value in 100mV
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x1B);
    U32 ret = dev->ri(trd_flash, TRDIND_DATA);

    float Vcc_min = (ret >> 4) & 0xF;
    Vcc_min += (ret & 0xF) * 0.1;

    // addr = 1Ch (VCC logic supply maximum program / erase voltage)
    // Bits[7:4] BCD value in volts
    // Bits[3:0] BCD value in 100mV
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x1C);
    ret = dev->ri(trd_flash, TRDIND_DATA);

    float Vcc_max = (ret >> 4) & 0xF;
    Vcc_max += (ret & 0xF) * 0.1;

    // addr = 27h (Device size = 2^n in number of bytes)
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x27);
    ret = dev->ri(trd_flash, TRDIND_DATA);

    // EXIT READ CFI (F0h)
    dev->wi(trd_flash, TRDIND_DATA, 0xF0);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x0);

    U32 size_bytes = 1 << (ret & 0xFF);
    U32 size_Mb = size_bytes >> 17;
    U32 size_MB = size_Mb >> 3;
    BRDC_printf(_BRDC("%-40s %.1f (V)\n")
                    _BRDC("%-40s %.1f (V)\n")
                        _BRDC("%-40s %d (MB)/%d (Mb)\n"),
        _BRDC("Минимальное напряжение питания :"), Vcc_min,
        _BRDC("Максимальное напряжение питания :"), Vcc_max,
        _BRDC("Объем памяти :"), size_MB, size_Mb);

    if (g_size == 0)
        g_size = size_bytes >> 1; // Words
}

//	//xxxxxxxxxxxxxxxxxxxxxxxx CHIP ERASE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void ChipErase(brd_dev_t dev)
{
    // The CHIP ERASE(80/10h) command erases the entire chip.
    // During the CHIP ERASE operation, the device ignores all other commands, including
    // ERASE SUSPEND.It is not possible to abort the operation.All bus READ operations during
    // CHIP ERASE output the data polling register on the data I/Os

    ipc_time_t t0;
    ipc_time_t t1;

    U32 hour, min, sec;
    U32 _min, _sec;

    BRDC_printf(_BRDC("Объемное стирание ... "));

    t0 = ipc_get_time();

    // CHIP ERASE(80h / 10h)

    // TWO UNLOCK CYCLES
    dev->wi(trd_flash, TRDIND_DATA, 0xAA);
    dev->wi(trd_flash, TRDIND_ADDR_H, 0x0);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x555);

    dev->wi(trd_flash, TRDIND_DATA, 0x55);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x2AA);

    // CMD(80h)
    dev->wi(trd_flash, TRDIND_DATA, 0x80);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x555);

    // TWO UNLOCK CYCLES
    dev->wi(trd_flash, TRDIND_DATA, 0xAA);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x555);

    dev->wi(trd_flash, TRDIND_DATA, 0x55);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x2AA);

    // CMD(10h)
    dev->wi(trd_flash, TRDIND_DATA, 0x10);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x555);

    // Polling
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x0);

    U32 ret = dev->ri(trd_flash, TRDIND_DATA);
    U32 toggle_bit = ret & 0x40; // Mask 6-th bit

    for (;;) {
        dev->wi(trd_flash, TRDIND_ADDR_L, 0x0);

        ret = dev->ri(trd_flash, TRDIND_DATA);
        ret &= 0x40; // Mask 6-th
        if (toggle_bit == ret)
            break;
        else
            toggle_bit = ret;
    }

    // After the CHIP ERASE operation completes, the device returns to read mode, unless an
    // error has occurred.If an error occurs, the device will continue to output the data polling
    // register.

    // When the operation fails, a READ / RESET command must be issued to reset the error
    // condition and return to read mode.The status of the array must be confirmed through
    // the BLANK CHECK operation and the BLOCK ERASE command re - issued to the failed
    // block.

    t1 = ipc_get_time();
    double time_ms = ipc_get_difftime(t0, t1);
    _sec = time_ms / 1000;
    sec = _sec % 60;
    _min = _sec / 60;
    min = _min % 60;
    hour = _min / 60;

    BRDC_printf(_BRDC("Завершено. Время операции: %1d:%.2d:%.2d\n"), hour, min, sec);
}

// xxxxxxxxxxxxxxxxxxxxxx BLOCK ERASE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void BlockErase(brd_dev_t dev)
{
    // The main memory array is divided into 128KB or 64KW uniform blocks
    // The BLOCK ERASE(80/30h) command erases a list of one or more blocks
    // Any command except BLOCK ERASE or ERASE SUSPEND during this timeout period resets that block to the read mode.
    // The system can monitor DQ3 to determine if the block erase timer has timed out.
    // After the program / erase controller has started, it is not possible to select any more
    // blocks.Each additional block must therefore be selected within the timeout period of
    // the last block.The timeout timer restarts when an additional block is selected

    AddrUnion Addr;
    Addr.x32 = g_addr & (~0xFFFF); // кратность 128 кБ (64 KW)
    AddrUnion CurrAddr = Addr;
    U32 block_num = g_size / 0x10000;
    U32 rem = g_size % 0x10000;

    if (rem != 0)
        block_num++;

    // BRDC_printf(_BRDC("Cтирание сектора 0x%X. Нажмите ESC, чтобы прервать ожидание ...\n"), g_addr);

    // xxxxxxxxxxxxx UNLOCK BYPASS(20h) xxxxxxxxxxxxxxxxxxxxx
    dev->wi(trd_flash, TRDIND_DATA, 0xAA);
    dev->wi(trd_flash, TRDIND_ADDR_H, 0x0);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x555);

    dev->wi(trd_flash, TRDIND_DATA, 0x55);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x2AA);

    // CMD(20h)
    dev->wi(trd_flash, TRDIND_DATA, 0x20);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x555);

    for (int ii = 0; ii < block_num; ii++) {
        BRDC_printf(_BRDC("Cтирание блока ...%d/%d\r"), ii, block_num);
        // UNLOCK BYPASS BLOCK ERASE (80h/30h)
        dev->wi(trd_flash, TRDIND_DATA, 0x80);
        dev->wi(trd_flash, TRDIND_ADDR_H, Addr.x16[1]); // Don't Care Address
        dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]); // Don't Care Address

        dev->wi(trd_flash, TRDIND_DATA, 0x30);
        if (CurrAddr.x16[1] != Addr.x16[1])
            dev->wi(trd_flash, TRDIND_ADDR_H, Addr.x16[1]); // Block Address

        dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]); // Block Address

        // Polling
        dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]);

        U32 ret = dev->ri(trd_flash, TRDIND_DATA);
        U32 toggle_bit = ret & 0x40; // Mask 6-th bit

        for (;;) {
            dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]);

            ret = dev->ri(trd_flash, TRDIND_DATA);
            ret &= 0x40; // Mask 6-th
            if (toggle_bit == ret)
                break;
            else
                toggle_bit = ret;
        }
        CurrAddr = Addr;
        Addr.x32 += 0x10000;
    }

    // xxxxxxxxx UNLOCK BYPASS RESET (90h/00h)
    //  The UNLOCK BYPASS RESET command is used to return to read/reset mode
    //  from unlock bypass mode
    dev->wi(trd_flash, TRDIND_DATA, 0x90);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0); // Don't Care Address

    dev->wi(trd_flash, TRDIND_DATA, 0x0);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0); // Don't Care Address
}

// xxxxxxxxxxxxxxxxxxxxxx BLANK CHECK xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void BlankCheck(brd_dev_t dev)
{
    // The main memory array is divided into 128KB or 64KW uniform blocks
    // The BLANK CHECK operation determines whether a specified block is blank(that is,
    // completely erased).It can also be used to determine whether a previous ERASE operation
    // was successful, including ERASE operations that might have been interrupted by
    // power loss.

    AddrUnion Addr;
    U32 start_block = g_addr >> 16;
    U32 block_num = g_size / 0x10000; // кратность 128 кБ (64 KW)
    block_num += start_block;

    U32 is_err = 0;

    for (U32 ii = start_block; ii < block_num; ii++) {
        BRDC_printf(_BRDC("Blank Check ... Block %d/%d\r"), ii, block_num);

        Addr.x32 = ii * 0x10000; // 0x10000 - размер одного блока

        dev->wi(trd_flash, TRDIND_DATA, 0xAA);
        dev->wi(trd_flash, TRDIND_ADDR_H, 0x0);
        dev->wi(trd_flash, TRDIND_ADDR_L, 0x555);

        dev->wi(trd_flash, TRDIND_DATA, 0x55);
        dev->wi(trd_flash, TRDIND_ADDR_L, 0x2AA);

        // CMD (EBh)
        dev->wi(trd_flash, TRDIND_DATA, 0xEB);
        dev->wi(trd_flash, TRDIND_ADDR_H, Addr.x16[1]); // Block Address
        dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]); // Block Address

        // CMD (76h)
        dev->wi(trd_flash, TRDIND_DATA, 0x76);
        dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]); // Block Address

        dev->wi(trd_flash, TRDIND_DATA, 0x00);
        dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]); // Block Address

        dev->wi(trd_flash, TRDIND_DATA, 0x00);
        dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]); // Block Address

        // xxxxxxxxxxx BLANK CHECK CONFIRM AND READ (29h) xxxxxxxxxxxxxxxxxxxxxx
        dev->wi(trd_flash, TRDIND_DATA, 0x29);
        dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]); // Block Address

        // Polling
        dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]);

        U32 ret = dev->ri(trd_flash, TRDIND_DATA);
        U32 toggle_bit = ret & 0x40; // Mask 6-th bit

        for (;;) {
            dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]);

            ret = dev->ri(trd_flash, TRDIND_DATA);
            // ret &= 0x40; // Mask 6-th
            if (toggle_bit == (ret & 0x40))
                break;
            else {
                toggle_bit = ret & 0x40;
                if (ret & 0x20) // Check 5-th bit
                {
                    dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]);
                    ret = dev->ri(trd_flash, TRDIND_DATA);
                    if (toggle_bit != (ret & 0x40)) {
                        is_err = 1;
                        break;
                    }
                }
            }
        }

        if (is_err) {
            BRDC_printf(_BRDC("Blank Check ... Part is NOT Blank\n"));
            // After the BLANK CHECK operation has completed, the device returns to read mode unless
            // an error has occurred.When an error occurs, the device continues to output data
            // polling register data.A READ / RESET command must be issued to reset the error condition
            // and return the device to read mode.

            // xxxxxxxxxxx READ/RESET (F0h) xxxxxxxxxxxxxxxxx
            dev->wi(trd_flash, TRDIND_DATA, 0xF0);
            dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]); // Don't Care Address

            // control read
            dev->wi(trd_flash, TRDIND_ADDR_L, Addr.x16[0]);
            ret = dev->ri(trd_flash, TRDIND_DATA);
            break;
        }
    }

    if (is_err == 0)
        BRDC_printf(_BRDC("Blank Check ... Part is Blank\n"));
}

// xxxxxxxxxxxxxxxxxxxxxxx Write Data xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void WriteData(brd_dev_t dev,U32 fsize)
{
    U32 AddrUpperTwoBytes;
    DataParam g_DataParam[2];
    U32 Param = 0;
    U32 reclen_words;
    FILE* stream;
    int j;
    char str[261];
    char* pstr = str;
    U32 reclen;
    U32 offset;
    U32 rectype;
    U32 data;
    U32 chksum;
    U08 crc_ret;
    U08 byte[261];

    g_DataParam[0].Words = 0;
    g_DataParam[1].Words = 0;

    ipc_time_t old_time;
    ipc_time_t curr_time;

    U32 hour, min, sec;
    U32 _min, _sec;

    BRDC_printf(_BRDC("Запись данных ... \n"));

    stream = BRDC_fopen(filename.c_str(), _BRDC("rt"));

    if (stream == NULL) {
        BRDC_fprintf(stderr, _BRDC("\n*** Can't find file \"%s\"\n"), filename.c_str());
        return;
    }

    old_time = ipc_get_time();

    bool do_cycle_again = true;

    //// RESET FIFO_OUT
    // dev->wi(trd_flash, TRDIND_MODE0, 2);
    // dev->wi(trd_flash, TRDIND_MODE0, 0);

    // xxxxxxxxxxxxx UNLOCK BYPASS(20h) xxxxxxxxxxxxxxxxxxxxx
    dev->wi(trd_flash, TRDIND_DATA, 0xAA);
    dev->wi(trd_flash, TRDIND_ADDR_H, 0x0);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x555);

    dev->wi(trd_flash, TRDIND_DATA, 0x55);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x2AA);

    // CMD(20h)
    dev->wi(trd_flash, TRDIND_DATA, 0x20);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0x555);

    AddrUnion StartAddr;
    AddrUnion CurrentAddr;
    StartAddr.x32 = -1;
    long int total = (long int) fsize;
    time_point<high_resolution_clock> start_time = high_resolution_clock::now();
    long int total_bytes=0;
    double time_s = 0;
    unsigned long long counter = 0;
    do {
        fgets(str, 261, stream);

        if (str[0] != ':') {
            BRDC_fprintf(stderr, _BRDC("\nWrong format!\n"));
            return;
        }

        sscanf(str + 1, "%2x", &reclen);
        sscanf(str + 3, "%4x", &offset);
        sscanf(str + 7, "%2x", &rectype);

        if (rectype == 0x1) // End-of-file record
        {
            do_cycle_again = false;
            // break;
        }
        // else if (rectype!=0)
        //	continue;

        byte[0] = (U08)reclen;
        byte[1] = (U08)(offset & 0xFF);
        byte[2] = (U08)(offset >> 8) & 0xFF;
        byte[3] = (U08)rectype;
        //fprintf(stderr, "---->  %d / %d \n", current,writes);

        
        //fprintf(stderr, "WORDS: %d\n",g_DataParam[Param].Words);


        for (j = 0; j < reclen; j++) {
            sscanf(str + 9 + 2 * j, "%2x", &data);
            byte[4 + j] = (U08)data;
        }

        crc_ret = Crc(byte, reclen + 4);

        sscanf(str + 9 + 2 * reclen, "%2x", &chksum);

        if ((U08)chksum == crc_ret) {

            if (rectype == 0x4) // Extended linear address record
            {
                AddrUpperTwoBytes = (byte[4] << 8) | byte[5];
                continue;
            } else if (rectype == 0) // Data record
            {
                g_DataParam[Param].Addr.x16[1] = AddrUpperTwoBytes;
                g_DataParam[Param].Addr.x16[0] = offset;
                g_DataParam[Param].Addr.x32 >>= 1; // Convert from byte address to word address
                g_DataParam[Param].Addr.x32 += g_addr;

                if (StartAddr.x32 == -1)
                    StartAddr.x32 = g_DataParam[Param].Addr.x32;
            }

            reclen_words = reclen >> 1;
            // Пишем блоками. Размер блока 512 слов х16.

            if (((g_DataParam[Param].Words + reclen_words) <= 512) && // Не превышен размер буфера
                (g_DataParam[Param].Addr.x32 == (StartAddr.x32 + g_DataParam[Param].Words)) && // Не изменился начальный адрес
                (((StartAddr.x32 + g_DataParam[Param].Words) & 0xFFF0000) == (g_DataParam[Param].Addr.x32 & 0xFFF0000)) && // Не вышли за границу блока
                (rectype != 0x1)) // Не конец файла
            {
                memcpy(&g_DataParam[Param].WordsArr[g_DataParam[Param].Words], &byte[4], reclen);
                g_DataParam[Param].Words += reclen_words;
                continue;
            } else {
                // xxxxxxxxxxxxx UNLOCK BYPASS WRITE TO BUFFER(25h) xxxxxxxxxxxxxxxxxxxxx
                // CMD(25h)
                dev->wi(trd_flash, TRDIND_DATA, 0x25);
                dev->wi(trd_flash, TRDIND_ADDR_H, StartAddr.x16[1]); // Block Address
                dev->wi(trd_flash, TRDIND_ADDR_L, StartAddr.x16[0]); // Block Address

                // N(1Fh) where N + 1 is the number of words / bytes to be programmed.
                dev->wi(trd_flash, TRDIND_DATA, g_DataParam[Param].Words - 1);
                dev->wi(trd_flash, TRDIND_ADDR_L, StartAddr.x16[0]); // Block Address
                U32 tst = 0;
                CurrentAddr = StartAddr;

            
                for (U32 ii = 0; ii < g_DataParam[Param].Words; ii++) {
                    dev->wi(trd_flash, TRDIND_DATA, g_DataParam[Param].WordsArr[ii]);
                    if (CurrentAddr.x16[1] != StartAddr.x16[1])
                        // tst = 1;
                        dev->wi(trd_flash, TRDIND_ADDR_H, StartAddr.x16[1]);
                    dev->wi(trd_flash, TRDIND_ADDR_L, StartAddr.x16[0]);
                    CurrentAddr = StartAddr;
                    StartAddr.x32++;
                }

                time_point<high_resolution_clock> current_time = high_resolution_clock::now();
                time_s = duration_cast<seconds>(current_time - start_time).count();
                total_bytes+= (g_DataParam[Param].Words * 2.0) ; // increment
                //total_bytes+= (512.0) ; // increment
                double percent = ( (  (double) (total_bytes)  ) / (double) total) * 100.0;
                if (!(counter % 250))
                    fprintf(stderr, "Status: [ %.2f %% ] -- Total bytes: [ %ld ] -- Time: %.2f s\r", percent, total_bytes, time_s);
                counter+=1;
                // To program the content of the program buffer, this command must be followed by a
                //  WRITE TO BUFFER PROGRAM CONFIRM command(29h)
                dev->wi(trd_flash, TRDIND_DATA, 0x29);
                dev->wi(trd_flash, TRDIND_ADDR_L, CurrentAddr.x16[0]); // Block Address

                // Polling
                dev->wi(trd_flash, TRDIND_ADDR_L, CurrentAddr.x16[0]);

                U32 ret = dev->ri(trd_flash, TRDIND_DATA);
                U32 toggle_bit = ret & 0x40; // Mask 6-th bit

                for (;;) {
                    dev->wi(trd_flash, TRDIND_ADDR_L, CurrentAddr.x16[0]);

                    ret = dev->ri(trd_flash, TRDIND_DATA);
                    ret &= 0x40; // Mask 6-th
                    if (toggle_bit == ret)
                        break;
                    else
                        toggle_bit = ret;
                }

                StartAddr.x32 = g_DataParam[Param].Addr.x32;
                Param ^= 1;
                g_DataParam[Param].Addr.x32 = StartAddr.x32;
                memcpy(g_DataParam[Param].WordsArr, &byte[4], reclen);
                g_DataParam[Param].Words = reclen_words;
            }
        } else {
            BRDC_fprintf(stderr, _BRDC("CRC_ERROR \n"));
            return;
        }
        
    } while (do_cycle_again);

    // xxxxxxxxx UNLOCK BYPASS RESET (90h/00h)
    //  The UNLOCK BYPASS RESET command is used to return to read/reset mode
    //  from unlock bypass mode
    dev->wi(trd_flash, TRDIND_DATA, 0x90);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0); // Don't Care Address

    dev->wi(trd_flash, TRDIND_DATA, 0x0);
    dev->wi(trd_flash, TRDIND_ADDR_L, 0); // Don't Care Address

    fclose(stream);

    curr_time = ipc_get_time();
    double time_ms = ipc_get_difftime(old_time, curr_time);
    _sec = time_ms / 1000;
    sec = _sec % 60;
    _min = _sec / 60;
    min = _min % 60;
    hour = _min / 60;

    BRDC_printf(_BRDC("Завершено. Время операции: %1d:%.2d:%.2d\n"), hour, min, sec);
}

// xxxxxxxxxxxxxxxxxxxxxxx Read Data xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void ReadData(brd_dev_t dev,U32 fsize)
{
    FILE* stream;
    int j;
    char str[261];
    char* pstr = str;
    U32 reclen;
    U32 offset;
    U32 rectype;
    U32 data;
    U32 chksum;
    U08 crc_ret;
    U08 byte[261];
    ipc_time_t old_time;
    ipc_time_t curr_time;

    U32 hour, min, sec;
    U32 _min, _sec;

    DataParam g_DataParam;
    U32 AddrUpperTwoBytes;
    AddrUnion AddrLast;
    U32 err_cntr;
    U32 err_indx = 0;
    AddrLast.x16[1] = -1;
    double time_s = 0;
    long int total = (long int) fsize;
    BRDC_printf(_BRDC("Чтение данных ... "));

    stream = BRDC_fopen(filename.c_str(), _BRDC("rt"));
    unsigned long long counter = 0;
    if (stream == NULL) {
        BRDC_fprintf(stderr, _BRDC("\n*** Can't find file \"%s\"\n"), filename.c_str());
        return;
    }

    old_time = ipc_get_time();

    time_point<high_resolution_clock> start_time = high_resolution_clock::now();

    bool do_cycle_again = true;
    long long total_bytes = 0;
    do {
        fgets(str, 261, stream);

        if (str[0] != ':') {
            BRDC_fprintf(stderr, _BRDC("\nWrong format!\n"));
            return;
        }

        sscanf(str + 1, "%2x", &reclen);
        sscanf(str + 3, "%4x", &offset);
        sscanf(str + 7, "%2x", &rectype);

        if (rectype == 0x1) // rectype=1 - End of File Record (запись, сигнализирующая о конце файла)
        {
            do_cycle_again = false;
            break;
        }

        byte[0] = (U08)reclen;
        byte[1] = (U08)(offset & 0xFF);
        byte[2] = (U08)(offset >> 8) & 0xFF;
        byte[3] = (U08)rectype;

        // if (reclen % 2)
        //	U32 odd_reclen = 1;

        for (j = 0; j < reclen; j++) {
            sscanf(str + 9 + 2 * j, "%2x", &data);
            byte[4 + j] = (U08)data;

        }

        crc_ret = Crc(byte, reclen + 4);

        sscanf(str + 9 + 2 * reclen, "%2x", &chksum);

        if ((U08)chksum == crc_ret) {

            if (rectype == 0x4) //  rectype=0x4 - Extended Linear Address Record (запись расширенного линейного адреса)
            {
                AddrUpperTwoBytes = (byte[4] << 8) | byte[5];
                continue;
            } else if (rectype == 0) // rectype=0 -  Data Record (запись, содержащая данные)
            {
                g_DataParam.Addr.x16[1] = AddrUpperTwoBytes;
                g_DataParam.Addr.x16[0] = offset;
                g_DataParam.Addr.x32 >>= 1; // Convert from byte address to word address
                g_DataParam.Addr.x32 += g_addr;
            }

            memcpy(&g_DataParam.WordsArr[0], &byte[4], reclen);

            U16* pBuffIn = &g_BuffIn[0];
            err_cntr = 0;
            U32 words = reclen >> 1;

            g_ErrorParam[err_indx].Addr = g_DataParam.Addr.x32;
            g_ErrorParam[err_indx].Words = words; // g_DataParam.Words;

            for (U32 ii = 0; ii < words; ii++) {
                if (AddrLast.x16[1] != g_DataParam.Addr.x16[1])
                    dev->wi(trd_flash, TRDIND_ADDR_H, g_DataParam.Addr.x16[1]);

                dev->wi(trd_flash, TRDIND_ADDR_L, g_DataParam.Addr.x16[0]);
                *(pBuffIn + ii) = dev->ri(trd_flash, TRDIND_DATA);
                AddrLast.x16[1] = g_DataParam.Addr.x16[1];
                g_DataParam.Addr.x32++;

                g_ErrorParam[err_indx].isErr[ii] = false;
                g_ErrorParam[err_indx].exp_val[ii] = g_DataParam.WordsArr[ii];
                g_ErrorParam[err_indx].rec_val[ii] = pBuffIn[ii]; // pBuffByte[ii];

                if (pBuffIn[ii] != g_DataParam.WordsArr[ii]) {
                    err_cntr++;

                    if (err_indx < 16) {
                        g_ErrorParam[err_indx].isErr[ii] = true;
                    } else {
                        do_cycle_again = false;
                        break;
                    }
                }
            }
            time_point<high_resolution_clock> current_time = high_resolution_clock::now();
            time_s = duration_cast<seconds>(current_time - start_time).count();
            total_bytes+= ((double)words * 2.0) ; // increment
            double percent = ( (  (double) (total_bytes)  ) / (double) total) * 100.0;
            if (!(counter % 5000))
                fprintf(stderr, "Status: [ %.2f %% ] -- Time: %.2f s\r", percent, time_s);
            counter+=1;
            if (err_cntr != 0)
                err_indx++;
        } else {
            BRDC_fprintf(stderr, _BRDC("CRC_ERROR \n"));
            return;
        }
    } while (do_cycle_again);

    curr_time = ipc_get_time();
    double time_ms = ipc_get_difftime(old_time, curr_time);
    _sec = time_ms / 1000;
    sec = _sec % 60;
    _min = _sec / 60;
    min = _min % 60;
    hour = _min / 60;

    BRDC_printf(_BRDC("Завершено. Время операции: %1d:%.2d:%.2d\n"), hour, min, sec);

    if (!filename.empty())
        fclose(stream);

    if (err_indx > 0) {
        err_indx = (err_indx < 16) ? err_indx : 16;

        for (U32 jj = 0; jj < err_indx; jj++) {
            BRDC_printf(_BRDC("\n%-10s 0x%08X\n"), _BRDC("АДРЕС: "), g_ErrorParam[jj].Addr);
            BRDC_printf(_BRDC("%-10s"), _BRDC("ОЖИДАЕТСЯ: "));
            for (U32 kk = 0; kk < g_ErrorParam[jj].Words; kk++)
                BRDC_printf(_BRDC("%04X"), g_ErrorParam[jj].exp_val[kk]);

            BRDC_printf(_BRDC("\n%-10s"), _BRDC("ПРОЧИТАНО: "));
            for (U32 kk = 0; kk < g_ErrorParam[jj].Words; kk++) {
#ifndef __linux__
                if (g_ErrorParam[jj].isErr[kk])
                    SetConsoleTextAttribute(hConsoleOut, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
                BRDC_printf(_BRDC("%04X"), g_ErrorParam[jj].rec_val[kk]);
#ifndef __linux__
                SetConsoleTextAttribute(hConsoleOut, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
#endif
            }
        }
    }
}

// xxxxxxxxxxxxxxxxxxxxxx IPROG Command xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void IprogCmd(brd_dev_t dev, U32 trd_flash)
{
    BRDC_printf(_BRDC("Загрузка прошивки из ПЗУ ...\n"));

    dev->wi(trd_flash, TRDIND_MODE2, 1); // START IPROG
    dev->wi(trd_flash, TRDIND_ADDR_H, (g_addr >> 16) & 0xFFFF); // 0x201
    dev->wi(trd_flash, TRDIND_ADDR_L, g_addr & 0xFFFF); // 0x200
    //		dev->wi(trd_flash, TRDIND_MODE2, 1); // START IPROG
}

#ifndef __linux__

//= ********************** FindPciCard *********************
//=********************************************************
int FindDevice(U32 devID, U32 inst)
{
    int ret;
    PCI_DEVICE pciDev;
    PCI_HEADER pciHdr;
    ULONG* ptr = (ULONG*)&pciHdr;
    USHORT venID = 0x4953;

    ret = UniDrv_PCI_FindDevice(handle, (USHORT)devID, (USHORT)venID, (USHORT)inst, &pciDev);
    if (ret != 0) {
        printf("\nISPCICFG ERROR: Can't Find Device with VendorID=%X, DeviceID=%X\n\n", venID, devID);
        _getch();
        return 0;
    }
    ret = UniDrv_PCI_ReadCfgSpace(handle, &pciDev, &pciHdr);
    if (ret != 0) {
        printf("\nISPCICFG ERROR: Can't Read PCI Device Configuration Space\n\n");
        _getch();
        return 0;
    }

    printf("Device: VendorID=%04X, DeviceID=%04X bus=%02X, dev=%02X\n", venID, devID, pciDev.bus, pciDev.tar.ByBits.dno);

    pDevices[dev_num] = new DEV_DESCR;
    pDevices[dev_num]->pciDev = pciDev;

    ret = UniDrv_PCI_ReadCfgData(handle, &pciDev, 0, 256, pDevices[dev_num]->cfg_data);
    dev_num++;

    return pciDev.bus;
}

// получает secondary bus
// возвращает primery bus
int FindBridge(int sec_bus)
{
    int nbus = 0, ndev = 0, nfunc = 0;
    int ret;
    PCI_DEVICE pciDev;
    PCI_HEADER pciHdr;

    int fl_break = 0;
    for (nbus = 0; nbus < 32; nbus++) {
        for (ndev = 0; ndev < 32; ndev++) {
            for (nfunc = 0; nfunc < 8; nfunc++) {
                pciDev.bus = nbus;
                pciDev.tar.ByBits.fno = nfunc;
                pciDev.tar.ByBits.dno = ndev;
                ret = UniDrv_PCI_ReadCfgSpace(handle, &pciDev, &pciHdr);
                if (ret != 0)
                    continue;
                if ((pciHdr.VID == 0) || (pciHdr.VID == 0xFFFF))
                    continue;
                //				printf("Thing: bus=%02d, dev=%02d, func=%02d DID=%04X, VID=%04X bclass=%02x, subclass=%02x pribus=%02x, secbus=%02x\n",
                //							nbus, ndev, nfunc, pciHdr.DID, pciHdr.VID, pciHdr.CLCD.base, pciHdr.CLCD.sub, pciHdr.Hdr.Type01h.PRINUM, pciHdr.Hdr.Type01h.SECNUM);
                if ((pciHdr.CLCD.base != 6) || (pciHdr.CLCD.sub != 4))
                    continue;
                if (pciHdr.Hdr.Type01h.SECNUM == sec_bus) {
                    fl_break = 1;
                    printf("Bridge: bus=%02d, dev=%02d, func=%02d\n", nbus, ndev, nfunc);
                    printf("        DeviceID = %04X, VendorID = %04X\n", pciHdr.DID, pciHdr.VID);
                    printf("        base class=%02x, subclass=%02x\n", pciHdr.CLCD.base, pciHdr.CLCD.sub);
                    printf("        primary bus =%02x, secondary bus=%02x\n", pciHdr.Hdr.Type01h.PRINUM, pciHdr.Hdr.Type01h.SECNUM);

                    if (nbus) {
                        pDevices[dev_num] = new DEV_DESCR;
                        pDevices[dev_num]->pciDev = pciDev;

                        ret = UniDrv_PCI_ReadCfgData(handle, &pciDev, 0, 256, pDevices[dev_num]->cfg_data);
                        dev_num++;
                    }
                    break;
                }
                if ((pciHdr.HDR & 0x80) == 0 && nfunc == 0)
                    break;
            }
            if (fl_break == 1)
                break;
        }
        if (fl_break == 1)
            break;
    }
    if (nbus == 16) {
        printf("\n Bridge is not found!!!\n");
        return -1;
    }
    return pciHdr.Hdr.Type01h.PRINUM;
}

int OffAndOn(void)
{
    int ret;
    ULONG cmd_reg = 0;
    for (int iDev = 0; iDev < dev_num; iDev++) {
        ret = UniDrv_PCI_ReadCfgData(handle, &(pDevices[iDev]->pciDev), 4, 4, &cmd_reg);

        // Запрет работы PCI устройства
        cmd_reg &= (~0x0000000e);
        ret = UniDrv_PCI_WriteCfgData(handle, &(pDevices[iDev]->pciDev), 4, 4, &cmd_reg);
    }

    // if (set_mode == 2)
    //{
    //	//printf("\n TURN ON DEVICE AND PRESS ANY KEY !!!\n", venID, devID );
    // }
    // else
    //{
    //	printf("\n TURN OFF AND CHANGE DEVICE (OR RELOAD PLD)!!!\n");
    //	printf(" PRESS ANY KEY AFTER ON\n");
    //	_getch();
    // }
    ////_getch();

    //	for(int iDev=0; iDev<dev_num; iDev++)
    for (int iDev = dev_num - 1; iDev >= 0; iDev--) {
        ret = UniDrv_PCI_ReadCfgData(handle, &(pDevices[iDev]->pciDev), 0, 256, cfg_data);
        if (cfg_data[0] != pDevices[iDev]->cfg_data[0])
            ret = UniDrv_PCI_WriteCfgData(handle, &(pDevices[iDev]->pciDev), 0, 4, &(pDevices[iDev]->cfg_data[0]));
        for (int i = 2; i < 64; i++) {
            if (cfg_data[i] != pDevices[iDev]->cfg_data[i])
                ret = UniDrv_PCI_WriteCfgData(handle, &(pDevices[iDev]->pciDev), i * 4, 4, &(pDevices[iDev]->cfg_data[i]));
        }
        // ret = UniDrv_PCI_WriteCfgData( handle, &(pDevices[iDev]->pciDev), 8, 248, pDevices[iDev]->cfg_data+8);

        // Разрешение работы PCI устройства
        ret = UniDrv_PCI_WriteCfgData(handle, &(pDevices[iDev]->pciDev), 4, 4, &(pDevices[iDev]->cfg_data[1]));
    }
    return ret;
}

#endif
