#ifndef MEM_DEV_H
#define MEM_DEV_H

#include "mapper.h"

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <memory>

//-----------------------------------------------------------------------------

class mem_dev
{
public:
    mem_dev(uint32_t phys_address, uint32_t phys_size);
    virtual ~mem_dev();

    //-----------------------------------------------------------------------------

    template <typename T> bool mem_write(uint32_t offset, T val)
    {
        volatile T *pmem = (volatile T*)_bar;
        if(!pmem)
            return false;
        pmem[offset >> 2] = val;
        return true;
    }

    //-----------------------------------------------------------------------------

    //bool mem_write(uint32_t offset, uint32_t val);
    bool mem_read(uint32_t offset, uint32_t& val);
    uint32_t mem_read(uint32_t offset);
    bool write_axi(uint32_t offset, uint32_t val) { return mem_write(offset, val); }
    uint32_t read_axi(uint32_t offset) { return mem_read(offset); }
    bool read_axi(uint32_t offset, uint32_t& val) { return mem_read(offset, val); }

protected:
    Mapper _map;
    unsigned _page_size = {4096};
    uint32_t* _bar;
    uint32_t _size;
};

//-----------------------------------------------------------------------------
using mem_dev_t = std::shared_ptr<mem_dev>;
//-----------------------------------------------------------------------------

#endif // MEM_DEV_H
