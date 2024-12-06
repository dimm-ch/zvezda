#include "fmcx_dev.h"
#include "strconv.h"
#include "exceptinfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>

//-----------------------------------------------------------------------------
using namespace std;
//-----------------------------------------------------------------------------

fmcx_dev::fmcx_dev(const std::string& devname) : _name(devname)
{
    try {
        _fd = open(devname.c_str(), O_RDWR, 0666);
        if (_fd < 0) {
            throw except_info("%s, %d: %s() - Error open device: %s\n", __FILE__, __LINE__, __FUNCTION__, devname.c_str());
        }

        for(int ii=0; ii<16; ++ii) {
            struct node_info_t node;
            node.idx = ii;
            int res = ioctl(_fd, BAR_INFO, &node);
            if(res < 0) {
                //int err = errno;
                //fprintf(stderr, "%s, %d: %s() - %s errno = %d\n", __FILE__, __LINE__, __func__, strerror(errno), err);
                if(errno != ENODEV) {
                    throw except_info("%s, %d: %s() - Error get device node info: %s\n", __FILE__, __LINE__, __FUNCTION__, devname.c_str());
                }
                break;
            } else {
                _nodes_info.push_back(node);
            }
        }

        _mm.init(_fd);

        for(unsigned ii=0; ii<_nodes_info.size(); ++ii) {

            struct node_info_t& info = _nodes_info.at(ii);
            if(!info.pa)
                continue;

            struct user_node_t node;
            node.bar = reinterpret_cast<size_t*>(_mm.map(info.pa, info.sz));
            if(!node.bar) {
                throw except_info("%s, %d: %s() - Can't map node %s into application.\n", __FILE__, __LINE__, __FUNCTION__, info.name);
            }

            node.sz = info.sz;
            node.name = info.name;

            if(strstr(node.name.c_str(), "fmc")) {
                msg("%s(): Map FMC %s: 0x%lx --> %p : [0x%lx]\n", __func__, node.name.c_str(), info.pa, node.bar, node.sz);
                _blk_list.push_back(node);
            }
            if(strstr(node.name.c_str(), "axis_switch")) {
                msg("%s(): Map SWITCH %s: 0x%lx --> %p : [0x%lx]\n", __func__, node.name.c_str(), info.pa, node.bar, node.sz);
                _blk_list.push_back(node);
            }
            if(strstr(node.name.c_str(), "trd")) {
                msg("%s(): Map TRD %s: 0x%lx --> %p : [0x%lx]\n", __func__, node.name.c_str(), info.pa, node.bar, node.sz);
                _trd_list.push_back(node);
            }
        }

        msg("%s(): BLK_SIZE [0x%d]\n", __func__, _blk_list.size());
        msg("%s(): TRD_SIZE [0x%d]\n", __func__, _trd_list.size());

        // DMA specific parameters
        _page_size = sysconf(_SC_PAGESIZE);

    } catch(const except_info_t& errInfo) {
        cleanup();
        throw errInfo;
    } catch(...) {
        cleanup();
    }
}

//-----------------------------------------------------------------------------

fmcx_dev::~fmcx_dev()
{
    cleanup();
}

//-----------------------------------------------------------------------------

void fmcx_dev::cleanup()
{
    if(_fd > 0) close(_fd);
}

//-----------------------------------------------------------------------------

const user_node_t& fmcx_dev::get_trd_node(uint32_t idx)
{
    if(idx >= _trd_list.size())
        return _zero_node;
    return _trd_list.at(idx);
}

//-----------------------------------------------------------------------------

const user_node_t& fmcx_dev::get_blk_node(uint32_t idx)
{
    if(idx >= _blk_list.size())
        return _zero_node;
    return _blk_list.at(idx);
}

//-----------------------------------------------------------------------------
/*
bool fmcx_dev::write_node(uint32_t node, uint32_t regnum, uint32_t val)
{
    if(node >= _nodes.size())
        return false;

    volatile uint32_t *pmem = (volatile uint32_t*)_nodes.at(node).bar;
    if(!pmem)
        return false;

    pmem[regnum] = val;

    return true;
}

//-----------------------------------------------------------------------------

bool fmcx_dev::read_node(uint32_t node, uint32_t regnum, uint32_t& val)
{
    if(node >= _nodes.size())
        return false;

    volatile uint32_t *pmem = (volatile uint32_t*)_nodes.at(node).bar;
    if(!pmem)
        return false;

    val = pmem[regnum];

    return true;
}
*/
//-----------------------------------------------------------------------------

void fmcx_dev::delay(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

//-----------------------------------------------------------------------------

void fmcx_dev::msg(const char *fmt, ...)
{
    if(!_verbose)
        return;

    va_list argptr;
    va_start(argptr, fmt);
    char msg[256];
    vsprintf(msg, fmt, argptr);
    fprintf(stderr, "%s", msg);
}

//-----------------------------------------------------------------------------

bool fmcx_dev::device_exist(unsigned id)
{
    std::string name = "/dev/fast_" + toString<int>(id);
    int fd = open(name.c_str(), O_RDWR, 0666);
    if(fd < 0)
        return false;
    close(fd);
    return true;
}

//-----------------------------------------------------------------------------

bool fmcx_dev::device_exist(const std::string& name)
{
    int fd = open(name.c_str(), O_RDWR, 0666);
    if(fd < 0)
        return false;
    close(fd);
    return true;
}

//-----------------------------------------------------------------------------

