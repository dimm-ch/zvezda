#ifndef FMCX_GPIO_H
#define FMCX_GPIO_H

#include "mem_dev.h"
#include "mapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>

//-----------------------------------------------------------------------------

enum axi_gpio_regs {
    GPIO_DATA = 0x0,
    GPIO_TRI = 0x4,
    GPIO2_DATA = 0x8,
    GPIO2_TRI = 0xC,
    GIER = 0x11C,
    IPER = 0x128,
    IPSR = 0x120,
};

//-----------------------------------------------------------------------------

enum axi_gpio_id {
    GPIO_ID_0 = 0x0,
    GPIO_ID_1 = 0x1,
};

//-----------------------------------------------------------------------------

class fmcx_gpio
{
public:
    fmcx_gpio(mem_dev_t hw, uint32_t id);
    virtual ~fmcx_gpio();

    void enable_out(uint32_t mask, axi_gpio_id gpio_id)
    {
        _hw->write_axi(_id + (gpio_id == GPIO_ID_0 ? GPIO_TRI : GPIO2_TRI), ~mask);
    }

    void enable_in(uint32_t mask, axi_gpio_id gpio_id)
    {
        _hw->write_axi(_id + (gpio_id == GPIO_ID_0 ? GPIO_TRI : GPIO2_TRI), mask);
    }

    uint32_t read(axi_gpio_id gpio_id)
    {
        return _hw->read_axi(_id + (gpio_id == GPIO_ID_0 ? GPIO_DATA : GPIO2_DATA));
    }

    void write(axi_gpio_id gpio_id, uint32_t mask)
    {
        _hw->write_axi(_id + (gpio_id == GPIO_ID_0 ? GPIO_DATA : GPIO2_DATA), mask);
    }

    void set_bit(uint32_t bitmask, axi_gpio_id gpio_id)
    {
        uint32_t val = 0;
        _hw->read_axi(_id + (gpio_id == GPIO_ID_0 ? GPIO_DATA : GPIO2_DATA), val);
        val |= bitmask;
        _hw->write_axi(_id + (gpio_id == GPIO_ID_0 ? GPIO_DATA : GPIO2_DATA), val);
    }

    void clear_bit(uint32_t bitmask, axi_gpio_id gpio_id)
    {
        uint32_t val = 0;
        _hw->read_axi(_id + (gpio_id == GPIO_ID_0 ? GPIO_DATA : GPIO2_DATA), val);
        val &= ~bitmask;
        _hw->write_axi(_id + (gpio_id == GPIO_ID_0 ? GPIO_DATA : GPIO2_DATA), val);
    }

    void pulse_bit(uint32_t bitmask, axi_gpio_id gpio_id)
    {
        set_bit(bitmask, gpio_id);
        clear_bit(bitmask, gpio_id);
    }

private:
    mem_dev_t _hw;
    uint32_t _id;
};

//-----------------------------------------------------------------------------

typedef std::shared_ptr<fmcx_gpio> fmcx_gpio_t;

//-----------------------------------------------------------------------------

#endif // FMCX_GPIO_H
