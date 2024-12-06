#include "fmcx_dma.h"
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

//-----------------------------------------------------------------------------
using namespace std;
//-----------------------------------------------------------------------------

fmcx_dma::fmcx_dma(fmcx_dev_t hw, chan_dir dir, chan_id id) : _hw(hw), _dir(dir)
{
    _stub = nullptr;
    _dma_buf = nullptr;    _channel = id;
    _page_size = sysconf(_SC_PAGESIZE);
    _mm.init(_hw->fd());
    _name = (dir == chan_dir::DIR_DMA_MM2S) ? "MM2S" : "S2MM";
}

//-----------------------------------------------------------------------------

fmcx_dma::~fmcx_dma()
{
    cleanup();
}

//-----------------------------------------------------------------------------

void fmcx_dma::cleanup()
{
}

//-----------------------------------------------------------------------------

bool fmcx_dma::dma_prepare(unsigned block_count, unsigned block_size, dma_memory_type block_type, uint64_t phys_base, uint64_t phys_area)
{
    if(!_dma_blocks.empty())
        return true;

    int bufsize = sizeof(struct dma_alloc_t) + (block_count-1)*sizeof(void*);
    _dma_buf = (struct dma_alloc_t *)malloc(bufsize);
    if(!_dma_buf) {
        return false;
    }

    memset(_dma_buf, 0, bufsize);

    _dma_buf->direction = _dir;
    _dma_buf->channel = _channel;
    _dma_buf->blocks_type = block_type;
    _dma_buf->blocks_count = block_count;
    _dma_buf->blocks_size = block_size;
    _dma_buf->stub[0] = nullptr;

    if(_dma_buf->blocks_type == dma_memory_type::BOOT_MEMORY_DMA) {
        if(phys_base == 0 || phys_area == 0 || (block_count*block_size > phys_area) || (block_size > 128*1024*1024)) {
            fprintf(stderr, "%s: ERROR - invalid parameters in dma_memory_type::PHYS_MEMORY_DMA mode\n", __func__);
            goto free_blocks;
        }
    }

    for(unsigned jj=0; jj<_dma_buf->blocks_count; jj++) {
        void *buf = 0;
        if(_dma_buf->blocks_type == USER_MEMORY_DMA) {
            int res = posix_memalign(&buf, _page_size, _dma_buf->blocks_size);
            if(res < 0 || !buf) {
                fprintf(stderr, "%s: ERROR - %s\n", __func__, strerror(errno));
                goto free_blocks;
            }
        } else if(_dma_buf->blocks_type == BOOT_MEMORY_DMA) {
            buf = reinterpret_cast<void*>(phys_base + jj * block_size);
        }
        _dma_buf->blocks_addr[jj] = buf;
        _dma_blocks.push_back(buf);
    }

    if (ioctl(_hw->fd(), DMA_BUFFER_LOCK, _dma_buf) < 0) {
        fprintf(stderr, "%s(): DEV_DMA_BUFFER_LOCK failed\n", __func__);
        goto free_blocks;
    }

    if(_dma_buf->blocks_type == KERNEL_MEMORY_DMA || _dma_buf->blocks_type == BOOT_MEMORY_DMA) {
        for(unsigned j=0; j<_dma_buf->blocks_count; j++) {
            void *buf = _dma_buf->blocks_addr[j];
            try {
                _dma_buf->blocks_addr[j] = _mm.map(buf, _dma_buf->blocks_size);
                _dma_blocks[j] = _dma_buf->blocks_addr[j];
                fprintf(stderr, "%s(): Map block: %p --> %p\n", __func__, buf, _dma_buf->blocks_addr[j]);
            } catch (const except_info_t& errInfo) {
                fprintf(stderr, "%s(): Error map block. %s\n", __func__, errInfo.info.c_str());
                goto unmap_blocks;
            }
        }
    }

    if(_dma_buf->stub[0]) {
        try {
            _stub = (struct dma_stub_t*)_mm.map(_dma_buf->stub[0], sizeof(struct dma_stub_t));
            fprintf(stderr, "%s(): Map stub: %p --> %p\n", __func__, _dma_buf->stub[0], _stub);
        } catch (const except_info_t& errInfo) {
            fprintf(stderr, "%s(): Error map stub. %s\n", __func__, errInfo.info.c_str());
            goto unmap_blocks;
        }
    }

    return true;

unmap_blocks:
    if(_dma_buf->blocks_type == KERNEL_MEMORY_DMA || _dma_buf->blocks_type == BOOT_MEMORY_DMA) {
        _mm.unmap();
    }

    struct dma_free_t param;
    param.channel = _channel;
    ioctl(_hw->fd(), DMA_BUFFER_UNLOCK, &param);

free_blocks:
    if(_dma_buf->blocks_type == USER_MEMORY_DMA) {
        for(unsigned j=0; j<_dma_buf->blocks_count; j++) {
            if(_dma_buf->blocks_addr[j]) {
                free(_dma_buf->blocks_addr[j]);
            }
        }
    }

    _dma_blocks.clear();
    free(_dma_buf);
    _dma_buf = nullptr;
    _stub = nullptr;

    return false;
}

//-----------------------------------------------------------------------------

bool fmcx_dma::dma_start(bool cycle)
{
    //struct usr_irq_enable_t irqen = { _dma_buf->channel };
    //if (ioctl(_hw->fd(), USR_IRQ_ENABLE, &irqen) < 0) {
    //    fprintf(stderr, "%s(): DEV_DMA_BUFFER_START failed\n", __func__);
    //    return false;
    //}

    struct dma_start_t start;
    start.direction = _dma_buf->direction;
    start.channel = _dma_buf->channel;
    start.cycle = cycle ? 1 : 0;

    if (ioctl(_hw->fd(), DMA_BUFFER_START, &start) < 0) {
        fprintf(stderr, "%s(): DEV_DMA_BUFFER_START failed\n", __func__);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------

bool fmcx_dma::dma_wait_block(int timeout)
{
    struct dma_wait_block_t wait;
    wait.direction = _dma_buf->direction;
    wait.channel = _dma_buf->channel;
    wait.timeout = timeout;
    wait.completed_block_counter = 0;
    wait.completed_desc_counter = 0;
    if (ioctl(_hw->fd(), DMA_BLOCK_WAIT, &wait) < 0) {
        fprintf(stderr, "%s(): DEV_DMA_BLOCK_WAIT failed\n", __func__);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool fmcx_dma::dma_wait_buffer(int timeout)
{
    struct dma_wait_block_t wait;
    wait.direction = _dma_buf->direction;
    wait.channel = _dma_buf->channel;
    wait.timeout = timeout;
    wait.completed_block_counter = 0;
    wait.completed_desc_counter = 0;
    if (ioctl(_hw->fd(), DMA_BUFFER_WAIT, &wait) < 0) {
        fprintf(stderr, "%s(): DEV_DMA_BUFFER_WAIT failed\n", __func__);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool fmcx_dma::dma_state(struct dma_state_t& state)
{
    struct dma_state_t s;
    s.direction = _dma_buf->direction;
    s.channel = _dma_buf->channel;
    s.status = 0;
    s.completed_desc_counter = 0;
    s.completed_block_counter = 0;
    if (ioctl(_hw->fd(), DMA_STATE, &s) < 0) {
        fprintf(stderr, "%s(): DEV_DMA_BUFFER_WAIT failed\n", __func__);
        return false;
    }
    state = s;
    return true;
}

//-----------------------------------------------------------------------------

bool fmcx_dma::dma_stop()
{
    struct dma_stop_t stop;
    stop.direction = _dma_buf->direction;
    stop.channel = _dma_buf->channel;
    if (ioctl(_hw->fd(), DMA_BUFFER_STOP, &stop) < 0) {
        fprintf(stderr, "%s(): DEV_DMA_BUFFER_STOP failed\n", __func__);
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------

bool fmcx_dma::dma_free()
{
    if(!_dma_buf) {
        return false;
    }

    if(_dma_buf->blocks_type == KERNEL_MEMORY_DMA || _dma_buf->blocks_type == BOOT_MEMORY_DMA) {
        _mm.unmap();
    }

    struct dma_free_t param;
    param.direction = _dma_buf->direction;
    param.channel = _dma_buf->channel;
    ioctl(_hw->fd(), DMA_BUFFER_UNLOCK, &param);

    if(_dma_buf->blocks_type == USER_MEMORY_DMA) {
        for(unsigned j=0; j<_dma_buf->blocks_count; j++) {
            if(_dma_buf->blocks_addr[j]) {
                free(_dma_buf->blocks_addr[j]);
            }
        }
    }

    _dma_blocks.clear();
    free(_dma_buf);
    _dma_buf = nullptr;
    _stub = nullptr;

    return true;
}

//-----------------------------------------------------------------------------

void fmcx_dma::dma_clear()
{
    dma_stop();
    dma_free();
}

//-----------------------------------------------------------------------------

void* fmcx_dma::dma_block(unsigned block_index)
{
    if(_dma_blocks.empty())
        return 0;

    if(block_index >= _dma_blocks.size()) {
        return 0;
    }

    return _dma_blocks.at(block_index);
}

//-----------------------------------------------------------------------------

bool fmcx_dma::dma_blocks(std::vector<void*>& data_blocks)
{
    if(_dma_blocks.empty())
        return false;

    data_blocks.clear();
    data_blocks = _dma_blocks;
/*
    for(unsigned j=0; j<data_blocks.size(); j++) {

        uint8_t *bd = (uint8_t*)data_blocks.at(j);

        if(!bd)
            continue;

        fprintf(stderr, "Block address: %p\n", bd);
        fprintf(stderr, "%d: ", j);

        for(unsigned i=0; i<16; i++) {
            fprintf(stderr, " %02X", bd[i]);
        }

        fprintf(stderr, "%s","\n");
    }
*/
    return true;
}

//-----------------------------------------------------------------------------

std::vector<void*>& fmcx_dma::dma_blocks()
{
    return _dma_blocks;
}

//-----------------------------------------------------------------------------
