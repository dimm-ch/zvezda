
#ifndef __MAPPER_H__
#define __MAPPER_H__

#include <stdint.h>
#include <vector>
#include <string>
#include <sstream>

//-----------------------------------------------------------------------------

struct map_addr_t {
    void *virtualAddress;
    size_t physicalAddress;
    size_t areaSize;
};

//-----------------------------------------------------------------------------

class Mapper {

public:
    Mapper();
    Mapper(int handle);
    virtual ~Mapper();

    void  init(int handle);
    void* map(size_t pa, uint32_t size);
    void* map(void*  pa, uint32_t size);
    void  unmap(void* va);
    void  unmap();

private:
    int fd;
    bool extHandle;

    std::vector<struct map_addr_t> mappedList;

    int openDevMem();
    void closeDevMem();
};

//-----------------------------------------------------------------------------

#endif //__MAPPER_H__
