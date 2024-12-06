#ifndef TRD_BASE_H
#define TRD_BASE_H

#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <chrono>

//-----------------------------------------------------------------------------

enum trd_id {
    trd_main_id     = 0x0001,
    trd_dac_id      = 0x00E0,
    trd_dddr3_id    = 0x00E5,
};

//-----------------------------------------------------------------------------
// Номера специфических косвенных регистров тетрады DAC
enum trd_common_regs {
        DACnr_SPD_DEVICE        = 0x203,
        DACnr_SPD_CTRL          = 0x204,
        DACnr_SPD_ADDR          = 0x205,
        DACnr_SPD_DATA          = 0x206,
        DACnr_SPD_DATAH         = 0x207,
};

//-----------------------------------------------------------------------------
// Numbers of Tetrad Registers
enum trd_direct_regs {
		TRDnr_STATUS = 0,    // (0x00) Status register
		TRDnr_DATA = 1,    // (0x02) Data register
		TRDnr_CMDADR = 2,    // (0x04) Address command register
		TRDnr_CMDDATA = 3,    // (0x06) Data command register
};

//-----------------------------------------------------------------------------

class trd_base
{
public:
    trd_base() {}
    virtual ~trd_base() {}

    virtual uint32_t rd(uint32_t reg);
    virtual void wd(uint32_t reg, uint32_t val);
    virtual uint32_t ri(uint32_t reg);
    virtual void wi(uint32_t reg, uint32_t val);

    virtual bool is_valid() = 0;
    virtual const std::string& name() = 0;

protected:

    virtual int spd_read(int spdType, uint32_t regAdr, uint32_t *pRegVal) = 0;
    virtual int spd_write(int spdType, uint32_t regAdr,  uint32_t RegVal) = 0;
    virtual uint32_t read_trd_reg(uint32_t offset) = 0;
    virtual void write_trd_reg(uint32_t offset, uint32_t val) = 0;
    virtual bool wait_trd_status(int timeout_10ms, uint16_t mask = 0x1);
};

//-----------------------------------------------------------------------------

#endif // TRD_BASE_H
