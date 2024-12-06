#ifndef MAIN_TRD_H
#define MAIN_TRD_H

#include "fmcx_dev.h"
#include "trd_base.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <memory>

//-----------------------------------------------------------------------------

enum main_trd_spd {
    main_spd_gen = 0,           //! генератор
    main_spd_clkswitch = 1,     //! коммутатор тактовых частот в версии, совместимой с FMC106P и т.д.
    main_spd_power = 8,         //! узел  контроля питания
    main_spd_eeprom = 16,       //! I2C  EEPROM
};

//-----------------------------------------------------------------------------

class main_trd : public trd_base
{
public:
    main_trd(fmcx_dev_t hw, int idx);                //! Создаем тетраду по номеру
    main_trd(fmcx_dev_t hw, trd_id id, int start = 0);     //! Создаем тетраду по ID начиная с номера
    virtual ~main_trd();
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

typedef std::shared_ptr<main_trd> main_trd_t;
template <class T> main_trd_t get_main_trd(fmcx_dev_t hw, T val) { return std::make_shared<main_trd>(hw, val); }

//-----------------------------------------------------------------------------

#endif // MAIN_TRD_H
