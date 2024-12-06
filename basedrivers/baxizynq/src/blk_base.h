#ifndef BLK_BASE_H
#define BLK_BASE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>

//-----------------------------------------------------------------------------

class blk_base
{
public:
    blk_base() {}
    virtual ~blk_base() {}

    virtual bool is_valid() = 0;
    virtual uint32_t rd(uint32_t reg) = 0;
    virtual void wd(uint32_t reg, uint32_t val) = 0;
    virtual const std::string& name() = 0;
};

//-----------------------------------------------------------------------------

enum blk_id {
    blk_main_id = 0x0013,
    blk_fid_id =  0x0019,
};

//-----------------------------------------------------------------------------

#endif // BLK_BASE_H
