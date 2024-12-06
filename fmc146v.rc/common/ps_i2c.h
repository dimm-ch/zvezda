
#ifndef __I2C_H__
#define __I2C_H__

//-----------------------------------------------------------------------------

#include <stdint.h>
#include <vector>
#include <string>
#include <sstream>

//-----------------------------------------------------------------------------

class i2c {

public:
    i2c(const char *name, int addr, bool addr_8bit = true);
    virtual ~i2c();

    bool write(const uint8_t i2c_addr, const std::vector<uint8_t> &data);
    bool read(const uint8_t i2c_addr, std::vector<uint8_t> &data, size_t N);

    bool read_byte(uint16_t addr, uint8_t& data);
    bool read_word(uint16_t addr, uint16_t& data);
    bool write_byte(uint16_t addr, uint8_t data);
    bool read_byte(int bus_addr, uint16_t addr, uint8_t& data);
    bool write_byte(int bus_addr, uint16_t addr, uint8_t data);

    bool configure();

private:
    int  m_fd;
    bool m_8bit;

    int i2c_write_1b(uint8_t buf[1]);
    int i2c_write_2b(uint8_t buf[2]);
    int i2c_write_3b(uint8_t buf[3]);
};

//-----------------------------------------------------------------------------

using ps_iic_t = std::shared_ptr<i2c>;

//-----------------------------------------------------------------------------

#define CHECK_I2C_FUNC( var, label ) \
    do { 	if(0 == (var & label)) { \
        fprintf(stderr, "\nError: " \
            #label " function is required. Program halted.\n\n"); \
        throw; } \
    } while(0);

//-----------------------------------------------------------------------------

#endif //__I2C_H__
