#include "main_trd.h"
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

main_trd::main_trd(fmcx_dev_t hw, int idx) : _hw(hw), _idx(idx)
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

main_trd::main_trd(fmcx_dev_t hw, trd_id id, int start) : _hw(hw), _id(id)
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

main_trd::~main_trd()
{
}

//-----------------------------------------------------------------------------

uint32_t main_trd::read_trd_reg(uint32_t offset)
{
    if(_bar) {
        return _bar[offset>>2];
    }
    return 0;
}

//-----------------------------------------------------------------------------

void main_trd::write_trd_reg(uint32_t offset, uint32_t val)
{
    if(_bar) {
        _bar[offset>>2] = val;
    }
}

//-----------------------------------------------------------------------------

int	main_trd::spd_read(int spdType, uint32_t regAdr, uint32_t *pRegVal)
{
    return -1;
}

//-----------------------------------------------------------------------------

int	main_trd::spd_write(int spdType, uint32_t regAdr,  uint32_t RegVal)
{
    return -1;
}

//-----------------------------------------------------------------------------
