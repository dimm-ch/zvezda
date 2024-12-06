#ifndef FMCX_TRD_H
#define FMCX_TRD_H

#include "fmcx_dev.h"
#include "trd_base.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <memory>

//-----------------------------------------------------------------------------

class fmcx_trd : public trd_base
{
public:
    fmcx_trd(fmcx_dev_t hw, int idx);                //! Создаем тетраду по номеру
    fmcx_trd(fmcx_dev_t hw, trd_id id, int start = 0);     //! Создаем тетраду по ID начиная с номера
    virtual ~fmcx_trd();
    bool is_valid() { return _valid; }
    const std::string& name() { return _name; }

private:
    fmcx_dev_t _hw;
    bool _valid;
    bool _direct;
    int  _idx;
    trd_id _id;
    uint32_t* _bar;
    std::string _name;

    int	spd_read(int spdType, uint32_t regAdr, uint32_t *pRegVal);
    int	spd_write(int spdType, uint32_t regAdr,  uint32_t RegVal);
    uint32_t read_trd_reg(uint32_t offset);
    void write_trd_reg(uint32_t offset, uint32_t val);
};

//-----------------------------------------------------------------------------

typedef std::shared_ptr<fmcx_trd> fmcx_trd_t;
template <class T> fmcx_trd_t get_trd(fmcx_dev_t hw, T val) { return std::make_shared<fmcx_trd>(hw, val); }
//-----------------------------------------------------------------------------


#endif // FMCX_TRD_H
