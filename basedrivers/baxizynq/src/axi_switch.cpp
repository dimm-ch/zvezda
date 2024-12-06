#include "axi_switch.h"
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

axi_switch::axi_switch(fmcx_dev_t hw, int start) : _hw(hw)
{
    _valid = false;
    for(unsigned ii=start; ii<_hw->blk_nodes_total(); ++ii) {

        const user_node_t& node = _hw->get_blk_node(ii);
        if(!node.bar)
            continue;

        if(!strstr(node.name.c_str(), "axis_switch"))
            continue;

        _bar = reinterpret_cast<uint32_t*>(node.bar);
        _valid = true;
        _name = node.name;
        _idx = ii;
        break;
    }
}

//-----------------------------------------------------------------------------

axi_switch::~axi_switch()
{
}

//-----------------------------------------------------------------------------

uint32_t axi_switch::rd(uint32_t offset)
{
    if(_bar) {
        return _bar[offset>>2];
    }
    return 0;
}

//-----------------------------------------------------------------------------

void axi_switch::wd(uint32_t offset, uint32_t val)
{
    if(_bar) {
        _bar[offset>>2] = val;
    }
}

//-----------------------------------------------------------------------------
