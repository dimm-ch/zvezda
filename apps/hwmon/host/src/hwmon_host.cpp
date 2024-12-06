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

#include "brd.h"
#include "gipcy.h"
#include "extn.h"
#include "strconv.h"
#include "pcie_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <string>
//#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

#define		MAX_DEV		12		// считаем, что модулей может быть не больше MAX_DEV
#define		MAX_PU		8		// считаем, что PU-устройств (ПЛИС, ППЗУ) на одном модуле может быть не больше MAX_PU
#define		MAX_SRV		16		// считаем, что служб на одном модуле может быть не больше MAX_SRV

//-----------------------------------------------------------------------------
std::string path = "./1/";
//-----------------------------------------------------------------------------

void print_help()
{
    BRDC_printf(_BRDC(" Command line options:\n"));
    BRDC_printf(_BRDC(" -h print this message\n"));
    BRDC_printf(_BRDC(" -v print debug messages\n"));
    BRDC_printf(_BRDC(" -i print data from card\n"));
    BRDC_printf(_BRDC(" -b <lid> select specific board from brd.ini\n"));
    BRDC_printf(_BRDC(" -t read timestamp area and show counter\n"));
    BRDC_printf(_BRDC(" -l list content of working directory: /run/media/mmcblk0p1\n"));
    BRDC_printf(_BRDC(" -r <remote file name> read file from working directory and save it in local drive\n"));
    BRDC_printf(_BRDC(" -w <local file name> write file to working directory from local drive\n"));
    BRDC_printf(_BRDC(" -c clear all files in working directory: /run/media/mmcblk0p1\n"));
    BRDC_printf(_BRDC(" -o <offset in hexademical> remove <offset> bytes from written file. Use with -w option\n"));
    BRDC_printf(_BRDC(" -f <name> write file to working directory from local drive with specific name. Use with -w option\n"));
}

//-----------------------------------------------------------------------------

//=************************* main *************************
//=********************************************************
int BRDC_main( int argc, BRDCHAR *argv[] )
{
    S32		status;

    // чтобы все печати сразу выводились на экран
    fflush(stdout);
    setbuf(stdout, NULL);

    bool is_syscmd = false;
    bool is_verbose = false;
    bool is_write = false;
    bool is_read = false;
    bool is_list = false;
    bool is_clear = false;
    bool is_timestamp = false;
    bool is_offset = false;
    long tail_offset = 0L;
    bool is_remote_name = false;
    bool is_help = false;
    bool is_info = false;
    U32 lid = 1;
    BRDCHAR* wfname;
    BRDCHAR* rfname;
    BRDCHAR* remote_fname;
    BRDCHAR* syscmd;

    for(int ii=1; ii<argc; ii++) {

        // отобразить справку
        if(BRDC_strcmp(argv[ii], _BRDC("-h")) == 0) {
            is_help = true;
            break;
        }

        if(BRDC_strcmp(argv[ii], _BRDC("-i")) == 0) {
            is_info = true;
            break;
        }

        // отображать дополнительную информацию
        if(BRDC_strcmp(argv[ii], _BRDC("-v")) == 0) {
            is_verbose = true;
        }

        // работать с заднным LID устройства
        if(BRDC_strcmp(argv[ii], _BRDC("-b")) == 0) {
            lid = BRDC_strtol(argv[++ii], 0, 10);
            if(is_verbose) BRDC_printf( _BRDC(" Working with board LID: %d\n"), lid );
        }

        // выполнить системную команду
        if(BRDC_strcmp(argv[ii], _BRDC("-s")) == 0) {
            is_syscmd = true;
            syscmd = argv[++ii];
        }

        // записать файл в рабочий каталог на удаленном устройстве
        if(BRDC_strcmp(argv[ii], _BRDC("-w")) == 0) {
            is_write = true;
            wfname = argv[++ii];
        }

        // прочитать файл из рабочего каталога на удаленном устройстве
        if(BRDC_strcmp(argv[ii], _BRDC("-r")) == 0) {
            is_read = true;
            rfname = argv[++ii];
        }

        // список файлов рабочего каталога на удаленном устройстве
        if(BRDC_strcmp(argv[ii], _BRDC("-l")) == 0) {
            is_list = true;
        }

        // очистить рабочий каталог на удаленном устройстве
        if(BRDC_strcmp(argv[ii], _BRDC("-c")) == 0) {
            is_clear = true;
        }

        // получить счетчик тактов timestamp
        if (BRDC_strcmp(argv[ii], _BRDC("-t")) == 0) {
            is_timestamp = true;
        }

        // если передана опция -o <offset> то от конца файла
        // будет отброшен фрагмент равный <offset> байт
        if (BRDC_strcmp(argv[ii], _BRDC("-o")) == 0) {
            is_offset = true;
            tail_offset = BRDC_strtol(argv[++ii], 0, 16);
        }

        // если передана опция -f <remote_fname> то файл на удаленном
        // устройстве будет сохранен под именем <remote_fname>
        if (BRDC_strcmp(argv[ii], _BRDC("-f")) == 0) {
            is_remote_name = true;
            remote_fname = argv[++ii];
        }
    }

    if(is_help) {
        BRDC_printf( _BRDC(" usage: %s [options]\n"), argv[0] );
        print_help();
        return 0;
    }

    BRD_displayMode(BRDdm_VISIBLE | BRDdm_CONSOLE); // режим вывода информационных сообщений : отображать все уровни на консоле

    S32	DevNum;
    status = BRD_init(_BRDC("brd.ini"), &DevNum); // инициализировать библиотеку
    if(!BRD_errcmp(status, BRDerr_OK))
    {
        BRDC_printf( _BRDC("ERROR: BARDY Initialization = 0x%X\n"), status );
        return -1;
    }
    if(is_verbose) BRDC_printf(_BRDC("BRD_init: OK. Number of devices = %d\n"), DevNum);

    // получить список LID (каждая запись соответствует устройству)
    BRD_LidList lidList;
    lidList.item = MAX_DEV;
    lidList.pLID = new U32[MAX_DEV];
    status = BRD_lidList(lidList.pLID, lidList.item, &lidList.itemReal);


    BRD_Info	info;
    info.size = sizeof(info);

    ULONG iDev = 0;
    // отображаем информацию об всех устройствах, указанных в ini-файле
    for(iDev = 0; iDev < lidList.itemReal; iDev++)
    {
        // Найдем плату с заданным LID
        if(lidList.pLID[iDev] != lid)
            continue;

        if(is_verbose) BRDC_printf(_BRDC("\n"));

        ULONG dev_id = 0;
        BRD_getInfo(lidList.pLID[iDev], &info); // получить информацию об устройстве
        if(info.busType == BRDbus_ETHERNET) {

            if(is_verbose) {
                BRDC_printf(_BRDC("%s: DevID = 0x%x, RevID = 0x%x, IP %u.%u.%u.%u, Port %u, PID = %d.\n"),
                            info.name, info.boardType >> 16, info.boardType & 0xff,
                            (UCHAR)info.bus, (UCHAR)(info.bus >> 8), (UCHAR)(info.bus >> 16), (UCHAR)(info.bus >> 24),
                            info.dev, info.pid);
            }
        } else {

            dev_id = info.boardType >> 16;
            if( dev_id == 0x53B1 || dev_id == 0x53B3) { // FMC115cP or FMC117cP
                if(is_verbose) {
                    BRDC_printf(_BRDC("%s: DevID = 0x%x, RevID = 0x%x, Bus = %d, Dev = %d, G.adr = %d, Order = %d, PID = %d.\n"),
                                info.name, info.boardType >> 16, info.boardType & 0xff, info.bus, info.dev,
                                info.slot & 0xffff, info.pid >> 28, info.pid & 0xfffffff);
                }
            } else {
                if(is_verbose) {
                    BRDC_printf(_BRDC("%s: DevID = 0x%x, RevID = 0x%x, Bus = %d, Dev = %d, Slot = %d, PID = %d.\n"),
                                info.name, info.boardType >> 16, info.boardType & 0xff, info.bus, info.dev, info.slot, info.pid);
                }
            }
        }

        uint32_t* host_vadr = NULL;
        memcpy(&host_vadr, &info.base[6], sizeof(void*));
        U16 blk_cnt = host_vadr[5 * 2] - 1;
        bool get_axi_block = false;
        U16 axi_blk_id = 0;
        U16 axi_blk_index = 0;
        uint32_t *axi_blk_addr = 0;
        uint32_t *zynq_window_addr = 0;
        for (U16 iBlk = 1; iBlk < blk_cnt; iBlk++) {
            axi_blk_id = host_vadr[iBlk * 64];
            if (0x0020 == (axi_blk_id & 0x00FF)) {
                get_axi_block = true;
                axi_blk_index = iBlk;
                axi_blk_addr = (uint32_t*)(host_vadr + axi_blk_index * 64);
                zynq_window_addr = (uint32_t*)(host_vadr + 1024*1024 / 4);
                break;
            }
        }

        // создадим экземпляр класса для работы с RFG2
        // через окно в BAR0 на шине PCIe
        hostmon_pcie hmon(axi_blk_addr, zynq_window_addr);

        if(is_verbose) BRDC_printf(_BRDC("BAR0 = %p\n"), host_vadr);
        if(is_verbose) BRDC_printf(get_axi_block ? _BRDC("BLOCK_AXI = 0x%04X\n") : _BRDC("BLOCK_AXI NOT FOUND\n"), axi_blk_id);

        if(get_axi_block && (axi_blk_index != 0xFFFF)) {

            if(axi_blk_addr) {

                if(is_verbose) BRDC_printf(_BRDC("BLOCK_AXI_ADDR = %p, INDEX = %d\n"), axi_blk_addr, axi_blk_index);
                if(is_verbose) BRDC_printf(_BRDC("BLOCK_AXI_ID = 0x%04X\n"), hmon.block_id());  //AXI ID
                if(is_verbose) BRDC_printf(_BRDC("BLOCK_AXI_VER = 0x%04X\n"), hmon.block_version()); //AXI VER

                //---------------------------------------------------------------------
                // проверка команды WRITE
                if(is_write) {
                    const IPC_str* fname = wfname;
                    IPC_handle fd = IPC_openFile(fname, IPC_OPEN_FILE | IPC_FILE_RDONLY);
                    if (fd) {
                        long long fileSize = 0ULL;
                        IPC_getFileSize(fd, &fileSize);
                        if (is_offset) {
                            fileSize -= tail_offset;
                        }
                        std::string cmd = "CMD_CODE:";
                        std::string name = "CMD_NAME:";
                        std::string size = "CMD_SIZE:";
#ifdef __linux__
                        std::string wfn = is_remote_name ? remote_fname : wfname;
#else
#ifdef _WIN64
                        std::wstring wfn = is_remote_name ? remote_fname : wfname;
#else
                        std::string wfn = wfname;
#endif // _WIN64
#endif
                        std::string fn(wfn.begin(), wfn.end());
                        if(fileSize) {
                            cmd += "WRITE";
                            name += fn;
                            size += toString<long long>(fileSize);
                        }
                        int alignedSize = BRD_ALIGN_UP((int)fileSize,4);
                        void *fileData = malloc(alignedSize);
                        if(fileData) {
                            memset(fileData,0,alignedSize);
                            int res = IPC_readFile(fd,fileData,(int)fileSize);
                            if(res == fileSize) {
                                hmon.write_host_data((uint32_t*)fileData,alignedSize);
                                hmon.write_cmd(cmd.c_str(), name.c_str(), size.c_str());
                                std::string cmd_info;
                                bool status = hmon.wait_card_ans(2000, cmd_info);
                                if(status) {
                                    fprintf(stderr, "%s\n", cmd_info.c_str());
                                } else {
                                    fprintf(stderr, "%s\n", "Error - timeout of waiting card answer!");
                                }
                            }
                            free(fileData);
                        }
                        IPC_closeFile(fd);
                    }
                }

                //---------------------------------------------------------------------
                // проверка команды READ
                if(is_read) {
                    const IPC_str* fname = rfname;
                    std::string cmd = "CMD_CODE:";
                    std::string name = "CMD_NAME:";
#ifdef __linux__
                    std::string rfn = rfname;
#else
#ifdef _WIN64
                    std::wstring rfn = rfname;
#else
                    std::string rfn = rfname;
#endif // _WIN64
#endif
                    std::string fn(rfn.begin(), rfn.end());
                    std::string size = "";
                    cmd += "READ";
                    name += fn;
                    hmon.write_cmd(cmd.c_str(), name.c_str(), size.c_str());
                    std::string cmd_info;
                    bool status = hmon.wait_card_ans(2000, cmd_info);
                    if(status) {
                        fprintf(stderr, "%s\n", cmd_info.c_str());
                    } else {
                        fprintf(stderr, "%s\n", "Error - timeout of waiting card answer!");
                    }
                    if(status) {
                        int cmd_ans_data_size = 0;
                        if(hmon.parse_card_ans(cmd_info, cmd_ans_data_size)) {
                            int card_data_size_align = BRD_ALIGN_UP(cmd_ans_data_size,4);
                            uint32_t *data_buf = (uint32_t*)malloc(card_data_size_align);
                            memset((char*)data_buf, '\0', card_data_size_align);
                            if(data_buf) {
                                hmon.read_card_data(data_buf, card_data_size_align);
                                IPC_handle fd = IPC_openFile(fname, IPC_CREATE_FILE | IPC_FILE_WRONLY);
                                if(fd) {
                                    IPC_writeFile(fd, data_buf, cmd_ans_data_size);
                                    IPC_closeFile(fd);
                                }
                            }
                            free(data_buf);
                        }
                    }
                }

                //---------------------------------------------------------------------
                // проверка команды LIST
                if(is_list) {
                    std::string cmd = "CMD_CODE:";
                    std::string name = "";
                    std::string size = "";
                    std::vector<std::string> file_list;
                    cmd += "LIST";
                    hmon.write_cmd(cmd.c_str(), name.c_str(), size.c_str());
                    std::string cmd_info;
                    bool status = hmon.wait_card_ans(50, cmd_info);
                    if(status) {
                        fprintf(stderr, "%s\n", cmd_info.c_str());
                    } else {
                        fprintf(stderr, "%s\n", "Error - timeout of waiting card answer!");
                    }
                    if(status) {
                        int cmd_ans_data_size = 0;
                        if(hmon.parse_card_ans(cmd_info, cmd_ans_data_size)) {
                            int card_data_size_align = BRD_ALIGN_UP(cmd_ans_data_size,4);
                            uint32_t *data_buf = (uint32_t*)malloc(card_data_size_align);
                            memset((char*)data_buf, '\0', card_data_size_align);
                            if(data_buf) {
                                hmon.read_card_data(data_buf, card_data_size_align);
                                //fprintf(stderr, "FILES: %s\n", (char*)data_buf);
                                const char *token = std::strtok((char*)data_buf, ":");
                                while(token) {
                                    std::string name = token;
                                    file_list.push_back(name);
                                    token = std::strtok(0, ":");
                                };
                            }
                            free(data_buf);
                        }
                    }
                    if(!file_list.empty()) {
                        fprintf(stderr, "------- FILE LIST ---------\n");
                        for(unsigned ii=0; ii<file_list.size(); ii++) {
                            fprintf(stderr, "%s\n", file_list[ii].c_str());
                        }
                        fprintf(stderr, "---------------------------\n");
                    }
                }

                //---------------------------------------------------------------------
                // проверка команды CLEAR
                if(is_clear) {
                    std::string cmd = "CMD_CODE:";
                    std::string name = "";
                    std::string size = "";
                    cmd += "CLEAR";
                    hmon.write_cmd(cmd.c_str(), name.c_str(), size.c_str());
                    std::string cmd_info;
                    bool status = hmon.wait_card_ans(2000, cmd_info);
                    if(status) {
                        fprintf(stderr, "%s\n", cmd_info.c_str());
                    } else {
                        fprintf(stderr, "%s\n", "Error - timeout of waiting card answer!");
                    }
                }

                //---------------------------------------------------------------------
                // проверка команды SYSTEM
                if(is_syscmd) {

                    std::string code = "CMD_CODE:";
                    std::string name = "CMD_NAME:";
                    std::string size = "";
#ifdef __linux__
                    std::string cmdname = syscmd;
#else
#ifdef _WIN64
                    std::wstring cmdname = syscmd;
#else
                    std::string cmdname = syscmd;
#endif // _WIN64
#endif
                    std::string cn(cmdname.begin(), cmdname.end());
                    if(!cn.empty()) {
                        code += "SYSTEM";
                        name += cn;
                        hmon.write_cmd(code.c_str(), name.c_str(), size.c_str());
                        std::string cmd_info;
                        bool status = hmon.wait_card_ans(2000, cmd_info);
                        if(status) {
                            fprintf(stderr, "%s\n", cmd_info.c_str());
                        } else {
                            fprintf(stderr, "%s\n", "Error - timeout of waiting card answer!");
                        }
                    }
                }

                //---------------------------------------------------------------------
                // чтение счетчика
                if (is_timestamp) {
                    for (int jj = 0; jj < 5; jj++) {
                        fprintf(stderr, "0x%X\n", hmon.hertbeat_counter());
                        IPC_delay(1000);
                    }
                }

                //---------------------------------------------------------------------
                // статусная информация из Zynq на устройстве
                if(is_info) {
                    hmon.show_card_info();
                }
            }
        }

    }
    delete lidList.pLID;

    if(is_verbose) BRDC_printf(_BRDC("\n"));

    status = BRD_cleanup();
    if(is_verbose) BRDC_printf(_BRDC("BRD_cleanup: OK\n"));

    return 0;
}

//-----------------------------------------------------------------------------


