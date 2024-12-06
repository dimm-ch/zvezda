#ifndef AXI_SWITCH_H
#define AXI_SWITCH_H

#include "fmcx_dev.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <memory>

//-----------------------------------------------------------------------------

enum axi_switch_spd {
    ctrl_reg = 0,           //! Регистр управления
    mi_mux_reg = 0x40,      //! Смещение регистров мультиплексора
};

//-----------------------------------------------------------------------------

class axi_switch
{
public:
    axi_switch(fmcx_dev_t hw, int start);
    virtual ~axi_switch();

    bool is_valid() { return _valid; }
    uint32_t rd(uint32_t offset);              //!< Чтение регистра
    void wd(uint32_t offset, uint32_t val);    //!< Запись регистра
    const std::string& name() { return _name; }

private:
    fmcx_dev_t _hw;
    bool _valid;
    int _idx;
    uint32_t* _bar;
    std::string _name;
};

//-----------------------------------------------------------------------------

typedef std::shared_ptr<axi_switch> axi_switch_t;
template <class T> axi_switch_t get_axi_switch(fmcx_dev_t hw, T val) { return std::make_shared<axi_switch>(hw, val); }

//-----------------------------------------------------------------------------

#endif // #define AXI_SWITCH_H
