
#include "strconv.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>

//-----------------------------------------------------------------------------

int version_major = 1;
int version_minor = 4;

//-----------------------------------------------------------------------------

using namespace std;

//-----------------------------------------------------------------------------

static const uint64_t MAP_SIZE = 256*1024*1024ULL;

//-----------------------------------------------------------------------------

const uint64_t ZYNQ_BASE_ADDR            = 0x20000000ULL;
const uint64_t HOST_DATA_SIZE            = 0x4000000ULL;
const uint64_t CARD_DATA_SIZE            = 0x4000000ULL;
const uint64_t HOST_CMD_SIZE             = 0x100000ULL;
const uint64_t CARD_CMD_SIZE             = 0x100000ULL;
const uint64_t CARD_STATUS_SIZE          = 0x100000ULL;
const uint64_t HOST_WINDOW_SIZE          = 0x100000ULL;

//-----------------------------------------------------------------------------

const uint64_t HOST_DATA          = 0x0ULL;
const uint64_t CARD_DATA          = HOST_DATA + HOST_DATA_SIZE;
const uint64_t HOST_CMD           = CARD_DATA + CARD_DATA_SIZE;
const uint64_t CARD_CMD           = HOST_CMD + HOST_CMD_SIZE;
const uint64_t CARD_STATUS        = CARD_CMD + CARD_CMD_SIZE;

//-----------------------------------------------------------------------------

void* heartbeat_thread(void *param);
void* command_thread(void *param);

// -------------------------------------------------------------------

enum {
    cmd_list = 77,
    cmd_read,
    cmd_write,
    cmd_clear,
    cmd_system,
};

//-----------------------------------------------------------------------------

#define BRD_ALIGN_UP(value, align)  (((value) & (align-1)) ? \
    (((value) + (align-1)) & ~(align-1)) : \
    (value))

#define BRD_ALIGN_DOWN(value, align) ((value) & ~(align-1))

//-----------------------------------------------------------------------------
static int g_StopFlag = 0;
void stop_monitor(int sig)
{
    fprintf(stderr, "\n");
    fprintf(stderr, "%s: SIGNAL = %d\n", __func__, sig);
    g_StopFlag = 1;
}

//-----------------------------------------------------------------------------

void mdelay(int ms)
{
    struct timeval tv = {0, 0};
    tv.tv_usec = 1000*ms;

    select(0,NULL,NULL,NULL,&tv);
}

//-----------------------------------------------------------------------------
std::string path = "/run/media/mmcblk0p1/";
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    argc = argc; argv = argv;

    fprintf(stderr, "%s(): Start monitor. Version: %d.%d  Build date: %s - %s\n", __FUNCTION__, version_major, version_minor, __DATE__, __TIME__);
    fprintf(stderr, "%s(): Working path: %s\n", __FUNCTION__, path.c_str());

    signal(SIGINT, stop_monitor);

    // -------------------------------------------------------------------

    int m_fd = -1;
    if((m_fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        fprintf(stderr, "Error open device /dev/mem\n");
        return -1;
    }

    volatile uint8_t* base = (uint8_t*)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, ZYNQ_BASE_ADDR);
    if(base == MAP_FAILED) {
        close(m_fd);
        fprintf(stderr, "Error map device memory\n");
        return -1;
    }

    // -------------------------------------------------------------------
    // Поток сердцебиения
    pthread_t heartbeat_thread_id;
    pthread_attr_t heartbeat_attr;
    int res = pthread_attr_init(&heartbeat_attr);
    if(res != 0) {
        fprintf(stderr, "%s(): %s\n", __FUNCTION__, strerror(errno));
        goto err_exit;
    }

    res = pthread_attr_setdetachstate(&heartbeat_attr, PTHREAD_CREATE_DETACHED);
    if(res != 0) {
        fprintf(stderr, "%s(): Error set thre server_main_thread()\n", __FUNCTION__);
        pthread_attr_destroy(&heartbeat_attr);
        goto err_exit;
    }

    res = pthread_create(&heartbeat_thread_id, &heartbeat_attr, heartbeat_thread, (void*)base);
    if(res != 0) {
        fprintf(stderr, "%s(): Error start heartbeat_thread()\n", __FUNCTION__);
        goto err_exit;
    }

    // -------------------------------------------------------------------
    // Поток обработки команд
    pthread_t cmd_thread_id;
    pthread_attr_t cmd_attr;
    res = pthread_attr_init(&cmd_attr);
    if(res != 0) {
        fprintf(stderr, "%s(): %s\n", __FUNCTION__, strerror(errno));
        goto err_exit;
    }

    res = pthread_attr_setdetachstate(&cmd_attr, PTHREAD_CREATE_DETACHED);
    if(res != 0) {
        fprintf(stderr, "%s(): Error set thre server_main_thread()\n", __FUNCTION__);
        pthread_attr_destroy(&heartbeat_attr);
        pthread_attr_destroy(&cmd_attr);
        goto err_exit;
    }

    res = pthread_create(&cmd_thread_id, &cmd_attr, command_thread, (void*)base);
    if(res != 0) {
        fprintf(stderr, "%s(): Error start command_thread()\n", __FUNCTION__);
        goto err_exit;
    }

    // -------------------------------------------------------------------

    while(!g_StopFlag) {
        sched_yield();
        mdelay(1000);
    }

err_exit:
    g_StopFlag = 1;
    mdelay(1000);
    pthread_attr_destroy(&cmd_attr);
    pthread_attr_destroy(&heartbeat_attr);
    munmap((void*)base, MAP_SIZE);
    fprintf(stderr, "%s() FINISHED\n", __FUNCTION__);

    return 0;
}

//-----------------------------------------------------------------------------
// Поток приема/передачи данных

void* heartbeat_thread(void *param)
{
    fprintf(stderr, "%s(%p) STARTED\n", __FUNCTION__, param);

    uint8_t *card_status = (uint8_t*)param;
    if(!card_status) {
        fprintf(stderr, "%s() INVALID MAAPPED ADDRESS\n", __FUNCTION__);
        return 0;
    }

    fprintf(stderr, "%s(): base = %p\n", __FUNCTION__, card_status);
    fprintf(stderr, "%s(): offset = 0x%08lX  \n", __FUNCTION__, CARD_STATUS);

    card_status += CARD_STATUS;

    uint32_t *status = (uint32_t*)card_status;
    uint32_t heartbit_counter = 0;

    fprintf(stderr, "%s(): status = %p\n", __FUNCTION__, status);

    while(!g_StopFlag) {
        sched_yield();
        mdelay(250);
        status[0] = ++heartbit_counter;
        //fprintf(stderr, "%s(): addr: %p, counter = %d \r", __FUNCTION__, status, status[0]);
    }

    fprintf(stderr, "%s() FINISHED\n", __FUNCTION__);

    return 0;
}

// -------------------------------------------------------------------
// Поток обработки команд

void* command_thread(void *param)
{
    fprintf(stderr, "%s(%p) STARTED\n", __FUNCTION__, param);

    if(!param) {
        fprintf(stderr, "%s() INVALID MAAPPED ADDRESS\n", __FUNCTION__);
        return 0;
    }

    const int BSIZE = 1024;
    volatile uint8_t* hcmd = (uint8_t*)param+HOST_CMD;
    volatile uint8_t* ccmd = (uint8_t*)param+CARD_CMD;
    volatile uint8_t* hdat = (uint8_t*)param+HOST_DATA;
    volatile uint8_t* cdat = (uint8_t*)param+CARD_DATA;

    volatile uint8_t* local_hcmd = new uint8_t[BSIZE];
    volatile uint8_t* local_ccmd = new uint8_t[BSIZE];

    fprintf(stderr, "%s(): base = %p\n", __FUNCTION__, param);
    fprintf(stderr, "%s(): hcmd = %p\n", __FUNCTION__, hcmd);

    memset((void*)local_hcmd, '\0', BSIZE);
    memset((void*)local_ccmd, '\0', BSIZE);

    while(!g_StopFlag) {

        char *ptag = 0;
        std::string code;
        std::string name;
        std::string size;
        int   cmd_id = 0;
        bool do_processing = false;
        volatile uint32_t *local_hcmd32 =  (uint32_t*)local_hcmd;
        volatile uint32_t *hcmd32 =  (uint32_t*)hcmd;
        volatile uint32_t *local_ccmd32 =  (uint32_t*)local_ccmd;
        volatile uint32_t *ccmd32 =  (uint32_t*)ccmd;

        while(!g_StopFlag) {

            if(!do_processing) {
                for(int jj=0; jj<1024/4; jj++) {
                    local_hcmd32[jj] = hcmd32[jj];
                }
                //fprintf(stderr, "%x %x %x\n", local_hcmd32[0], local_hcmd32[1], local_hcmd32[2]);
            }

            // определим есть ли команда в буфере
            char *pstr = (char*)local_hcmd;
            if(strncmp(pstr, "CMD_CODE:", 9) == 0) {

                ptag = pstr;
                pstr += 9;
            }

            // получим код команды
            if(!ptag)
                continue;

            //fprintf(stderr, "%s\n", ptag);

            if(strncmp(pstr, "LIST", 4) == 0) {
                code = pstr;
                pstr += 5;
                cmd_id = cmd_list;
                //fprintf(stderr, "%s\n", code.c_str());
            }

            if(strncmp(pstr, "READ", 4) == 0) {
                code = pstr;
                pstr += 5;
                cmd_id = cmd_read;
            }

            if(strncmp(pstr, "WRITE", 5) == 0) {
                code = pstr;
                pstr += 6;
                cmd_id = cmd_write;
            }

            if(strncmp(pstr, "CLEAR", 5) == 0) {
                code = pstr;
                pstr += 6;
                cmd_id = cmd_clear;
            }

            if(strncmp(pstr, "SYSTEM", 6) == 0) {
                code = pstr;
                pstr += 7;
                cmd_id = cmd_system;
            }

            //fprintf(stderr, "CODE = %s\n", code.c_str());

            switch(cmd_id) {

            case cmd_list: { do_processing = true; } break;

            case cmd_clear: { do_processing = true; } break;

            case cmd_read: {

                // получим параметры для команды чтения и стирания
                if(strncmp(pstr, "CMD_NAME:", 9) == 0) {

                    pstr += 9;
                    name = pstr;

                    if(!name.empty())
                        do_processing = true;
                }

            } break;

            case cmd_write: {

                // получим параметры для команды записи
                if(strncmp(pstr, "CMD_NAME:", 9) == 0) {

                    pstr += 9;
                    name = pstr;
                    pstr += name.size()+1;

                    if(strncmp(pstr, "CMD_SIZE:", 9) == 0) {

                        pstr += 9;
                        size = pstr;

                        if(!name.empty() && !size.empty())
                            do_processing = true;
                    }
                }

            } break;

            case cmd_system: {

                // получим имя системной команды для выполнения
                if(strncmp(pstr, "CMD_NAME:", 9) == 0) {

                    pstr += 9;
                    name = pstr;
                    pstr += name.size()+1;

                    if(!name.empty())
                        do_processing = true;
                }
            } break;

            }

            // получили все параметрs для команды
            if(do_processing) {
                break;
            }
        }

        if(g_StopFlag)
            break;

        //fprintf(stderr, "CMD: %s, NAME = %s, CODE: %s\n", ptag, code.c_str(), name.c_str());

        // строка с ответом для HOST (отправляется в любом случае)
        std::string ans_info = "CMD_INFO:";

        // код и имя команды сформируем сразу
        ans_info += code;
        ans_info += ":";

        // обработаем команду записи
        if(cmd_id == cmd_write) {

            ans_info += name;
            ans_info += ":";

            std::string fname = path;
            fname += name;

            int fd = open(fname.c_str(), O_CREAT|O_TRUNC|O_RDWR, 0666);
            if(fd < 0) {

                fprintf(stderr, "%s(): Error open file: %s. (%s)\n", __FUNCTION__, fname.c_str(), strerror(errno));
                ans_info += "Open File:[ERROR]";

            } else {

                size_t dataSize = fromString<size_t>(size.c_str());
                ssize_t written = write(fd, (const void*)hdat, dataSize);
                if( written != (ssize_t)dataSize ) {
                    ans_info += "Write File:[ERROR]";
                } else {
                    // размер в команде записи известен сразу
                    ans_info += "SIZE:";
                    ans_info += size;
                    ans_info += ":";
                    ans_info += "[SUCCESS]";
                }

                close(fd);
            }
        }

        // обработаем системную команду
        if(cmd_id == cmd_system) {

            ans_info += name;
            ans_info += ":";

            int res = system(name.c_str());
            if(res < 0) {

                fprintf(stderr, "%s(): Error in system(%s) - (%s)\n", __FUNCTION__, name.c_str(), strerror(errno));
                ans_info += "Error in system():[ERROR]";

            } else {
                ans_info += "[SUCCESS]";
            }
        }

        // обработаем команду чтения
        if(cmd_id == cmd_read) {

            ans_info += name;
            ans_info += ":";

            size_t dataSize = 0;
            std::string fname = path;
            fname += name;

            int fd = open(fname.c_str(), O_RDONLY, 0666);
            if(fd < 0) {
                ans_info += "Open File:[ERROR]";
            } else {

                // получим информацию о файле
                struct stat finfo;
                int res = fstat(fd, &finfo);
                if(res < 0) {
                    fprintf(stderr, "%s(): %s\n", __FUNCTION__, strerror(errno));
                    ans_info += "Info File:[ERROR]";
                } else {

                    // скопируем информацию о размере файла в ответ серверу
                    dataSize = finfo.st_size;
                    size = toString<size_t>(dataSize);
                    ans_info += "SIZE:";
                    ans_info += size;
                    ans_info += ":";

                    // считаем данные из файла сразу в буфер данных модуля
                    ssize_t readed = read(fd, (void*)cdat, dataSize);
                    if( readed != (ssize_t)dataSize ) {
                        ans_info += "Read File:[ERROR]";
                    } else {
                        ans_info += "[SUCCESS]";
                    }
                }
                close(fd);
            }
        }


        if(cmd_id == cmd_list) {

            std::string list;
            DIR *dir = 0;
            struct dirent *ent = 0;
            if ((dir = opendir(path.c_str())) != NULL) {

                while ((ent = readdir(dir)) != NULL) {

                    if(strcmp(ent->d_name, "..") == 0) {
                        continue;
                    }

                    if(strcmp(ent->d_name, ".") == 0) {
                        continue;
                    }

                    if(!list.empty())
                        list += ":";

                    list += ent->d_name;
                }

                // открыли директорию, но в ней нет файлов
                // и доавим завершающий \0.
                list += '\0';

                closedir (dir);

            } else {

                fprintf(stderr, "%s(): Error open directory: %s. (%s)\n", __FUNCTION__, path.c_str(), strerror(errno));
                ans_info += "Open Directory:[ERROR]";
            }

            if(!list.empty()) {

                // подготовим информацию о размере списка файлов директории
                size = toString<size_t>(list.size());
                ans_info += "SIZE:";
                ans_info += size;
                ans_info += ":";
                ans_info += "[SUCCESS]";

                // скопируем список в область данных модуля предварительно сделав размер кратным 4-байта
                int tail  = (list.size() % 4);
                for(int ii=0; ii<tail; ii++) {
                    list.push_back('\0');
                }
                uint32_t *pstr = (uint32_t*)list.c_str();
                for(unsigned ii=0; ii<list.size()/4; ii++) {
                    *((uint32_t*)cdat + ii) = pstr[ii];
                }
            }
        }
        /*
        if(cmd_id == cmd_clear) {

            DIR *dir = 0;
            struct dirent *ent = 0;
            if ((dir = opendir(path.c_str())) != NULL) {

                while ((ent = readdir(dir)) != NULL) {

                    if(strcmp(ent->d_name, "..") == 0)
                        continue;

                    if(strcmp(ent->d_name, ".") == 0)
                        continue;

                    // сформируем имя файла
                    std::string filepath = path;
                    filepath += ent->d_name;

                    remove(filepath.c_str());
                }

                ans_info += "[SUCCESS]";
                closedir (dir);

            } else {

                fprintf(stderr, "%s(): Error open directory: %s. (%s)\n", __FUNCTION__, name.c_str(), strerror(errno));
                ans_info += "Open Directory:[ERROR]";
            }
        }
*/

        fprintf(stderr, "%s\n", ans_info.c_str());

        // Zero host command line area
        for(int ii=0; ii<BSIZE/4; ii++) {
            hcmd32[ii] = 0;
            ccmd32[ii] = 0;
            local_ccmd32[ii] = 0;
            local_hcmd32[ii] = 0;
        }

        // Write answer to host data area
        ans_info += '\0';
        int tail  = (ans_info.size() % 4);
        for(int ii=0; ii<tail; ii++) {
            ans_info.push_back('\0');
        }
        uint32_t *pstr = (uint32_t*)ans_info.c_str();
        for(unsigned ii=0; ii<ans_info.size()/4; ii++) {
            *((uint32_t*)ccmd + ii) = pstr[ii];
        }
        /*
        memcpy((void*)local_ccmd, ans_info.c_str(), ans_info.size());

        int align_size = BRD_ALIGN_UP(ans_info.size(),4);
        for(int jj=0; jj<align_size/4; jj++) {
            ccmd32[jj] = local_ccmd32[jj];
        }
*/
    }

    if(local_hcmd) delete[] local_hcmd;
    if(local_ccmd) delete[] local_ccmd;

    fprintf(stderr, "%s() FINISHED\n", __FUNCTION__);

    return 0;
}

//-----------------------------------------------------------------------------
