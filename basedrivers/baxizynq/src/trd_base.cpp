#include "trd_base.h"
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

uint32_t trd_base::rd(uint32_t regnum)
{
    uint32_t offset = regnum*0x1000;
    return read_trd_reg(offset);
}

//-----------------------------------------------------------------------------

void trd_base::wd(uint32_t regnum, uint32_t val)
{
    uint32_t offset = regnum*0x1000;
    write_trd_reg(offset, val);
}

//-----------------------------------------------------------------------------

uint32_t trd_base::ri(uint32_t regnum)
{
    uint32_t CmdAdr  = 0x2000;
    uint32_t CmdData = 0x3000;
    uint32_t ret;

    write_trd_reg(CmdAdr, regnum);

    if(!wait_trd_status(100))
        return 0xFFFF;

    ret = read_trd_reg(CmdData);
    ret &= 0xFFFF;

    return ret;
}

//-----------------------------------------------------------------------------

void trd_base::wi(uint32_t regnum, uint32_t val)
{
    uint32_t CmdAdr  = 0x2000;
    uint32_t CmdData = 0x3000;

    write_trd_reg(CmdAdr, regnum);

    if(!wait_trd_status(100))
        return;

    write_trd_reg(CmdData, val);
}

//-----------------------------------------------------------------------------

bool trd_base::wait_trd_status(int timeout_10ms, uint16_t mask)
{
    int ii=0;
    bool res = true;

    while(1)  {

        uint32_t status = rd(TRDnr_STATUS);
        if(status & mask) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if(++ii > timeout_10ms) {
            fprintf(stderr, "%s(%s) - TIMEOUT. STATUS = 0x%04X\n", __func__, name().c_str(), status);
            res = false;
            break;
        }
    }
    return res;
}

//-----------------------------------------------------------------------------
