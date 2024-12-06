#ifndef FMCX_DMA_H
#define FMCX_DMA_H

#include "fmcx_dev.h"
#include "ddaxizynq.h"
#include "mapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <memory>

//-----------------------------------------------------------------------------

class fmcx_dma
{
public:
    fmcx_dma(fmcx_dev_t hw, chan_dir dir, chan_id id);
    virtual ~fmcx_dma();

    bool dma_prepare(unsigned block_count, unsigned block_size, dma_memory_type block_type, uint64_t phys_base = 0, uint64_t phys_area = 0);
    bool dma_start(bool cycle = false);
    bool dma_wait_block(int timeout);
    bool dma_wait_buffer(int timeout);
    bool dma_state(struct dma_state_t& state);
    bool dma_stop();
    bool dma_free();
    void dma_clear();
    bool dma_blocks(std::vector<void*>& dma_blocks);
    std::vector<void*>& dma_blocks();
    void* dma_block(unsigned block_index);
    unsigned dma_block_size() { return _dma_buf->blocks_size; }
    struct dma_stub_t* dma_stub() { return _stub; }
    const std::string& dma_name() { return _name; }
    chan_dir dma_direction() { return _dir; }
    chan_id dma_id() { return _channel; }

private:
    fmcx_dev_t _hw;
    chan_dir _dir;
    chan_id _channel;
    unsigned _page_size;
    Mapper _mm;
    struct dma_alloc_t* _dma_buf;
    std::vector<void*>  _dma_blocks;
    struct dma_stub_t* _stub;
    std::string _name;
    void cleanup();
};

//-----------------------------------------------------------------------------

typedef std::shared_ptr<fmcx_dma> fmcx_dma_t;
inline fmcx_dma_t get_channel(fmcx_dev_t hw, chan_dir dir, chan_id id) { return std::make_shared<fmcx_dma>(hw, dir, id); }

//-----------------------------------------------------------------------------

#endif // FMCX_DMA_H
