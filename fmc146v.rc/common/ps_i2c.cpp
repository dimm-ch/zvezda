// ***************************************************************
//    За основу взят код из проекта http://www.codesink.org
// ***************************************************************

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#include <unistd.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <getopt.h>
#include <errno.h>
#endif
#include <fcntl.h>
#include <signal.h>

#include <vector>
#include <string>
#include <iostream>
#include <memory>

#ifdef __linux__
extern "C" {
#include <fcntl.h>
#include <i2c/smbus.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
}
#endif

#include "ps_i2c.h"

//-----------------------------------------------------------------------------
using namespace std;
//-----------------------------------------------------------------------------

i2c::i2c(const char *name, int i2c_addr, bool addr_8bit)
{
    m_8bit = addr_8bit;
    m_fd = open(name, O_RDWR);
    if(m_fd < 0) {
        fprintf(stderr, "Error open: %s\n", strerror(errno));
        throw;
    }

    int funcs = 0;
    if(ioctl(m_fd, I2C_FUNCS, &funcs) < 0) {
        fprintf(stderr, "Error get function list: %s\n", strerror(errno));
        throw;
    }

    // check for req funcs
    CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_BYTE );
    CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_BYTE );
    CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_BYTE_DATA );
    CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_BYTE_DATA );
    CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_WORD_DATA );
    CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_WORD_DATA );

    // set working device
    if(ioctl(m_fd, I2C_SLAVE, i2c_addr) < 0) {
        fprintf(stderr, "Error set slave address: %s\n", strerror(errno));
        throw;
    }
}

//-----------------------------------------------------------------------------

i2c::~i2c()
{
    close(m_fd);
}

//-----------------------------------------------------------------------------

int i2c::i2c_write_1b(uint8_t buf[1])
{
    int r;
    // we must simulate a plain I2C byte write with SMBus functions
    r = i2c_smbus_write_byte(m_fd, buf[0]);
    if(r < 0)
        fprintf(stderr, "Error i2c_write_1b: %s\n", strerror(errno));
    usleep(10);
    return r;
}

//-----------------------------------------------------------------------------

int i2c::i2c_write_2b(uint8_t buf[2])
{
    int r;
    // we must simulate a plain I2C byte write with SMBus functions
    r = i2c_smbus_write_byte_data(m_fd, buf[0], buf[1]);
    if(r < 0)
        fprintf(stderr, "Error i2c_write_2b: %s\n", strerror(errno));
    usleep(10);
    return r;
}

//-----------------------------------------------------------------------------

int i2c::i2c_write_3b(uint8_t buf[3])
{
    int r;
    // we must simulate a plain I2C byte write with SMBus functions
    // the uint16_t data field will be byte swapped by the SMBus protocol
    r = i2c_smbus_write_word_data(m_fd, buf[0], buf[2] << 8 | buf[1]);
    if(r < 0)
        fprintf(stderr, "Error i2c_write_3b: %s\n", strerror(errno));
    usleep(10);
    return r;
}

//-----------------------------------------------------------------------------

bool i2c::read_byte(uint16_t addr, uint8_t& data)
{
    int r;
    ioctl(m_fd, BLKFLSBUF); // clear kernel read buffer
    if(m_8bit) {
        uint8_t buf[1] = { uint8_t(addr & 0xff) };
        r = i2c_write_1b(buf);
    } else {
        uint8_t buf[2] = { uint8_t((addr >> 8) & 0xff), uint8_t(addr & 0xff) };
        r = i2c_write_2b(buf);
    }
    if (r < 0)
        return false;
    data = i2c_smbus_read_byte(m_fd);
    return true;
}

//-----------------------------------------------------------------------------

bool i2c::write_byte(uint16_t addr, uint8_t data)
{
    int r;
    if(m_8bit) {
        uint8_t buf[2] = { uint8_t(addr & 0xff), data };
        r = i2c_write_2b(buf);
    } else {
        uint8_t buf[3] = { uint8_t((addr >> 8) & 0xff), uint8_t(addr & 0xff), data };
        r = i2c_write_3b(buf);
    }
    if (r < 0)
        return false;
    return true;
}

//-----------------------------------------------------------------------------

bool i2c::read_byte(int bus_addr, uint16_t addr, uint8_t& data)
{
    // set working device
    if(ioctl(m_fd, I2C_SLAVE, bus_addr) < 0) {
        fprintf(stderr, "Error set slave address: %s\n", strerror(errno));
        return false;
    }

    return read_byte(addr, data);
}

//-----------------------------------------------------------------------------

bool i2c::read_word(uint16_t addr, uint16_t& data)
{
    ioctl(m_fd, BLKFLSBUF); // clear kernel read buffer
    int val = i2c_smbus_read_word_data(m_fd, addr);
    if(val < 0) {
        return false;
    }
    //fprintf(stderr, "0x%04x\n", val);
    data =  (int16_t)val;
    return true;
}

//-----------------------------------------------------------------------------

bool i2c::write_byte(int bus_addr, uint16_t addr, uint8_t data)
{
    // set working device
    if(ioctl(m_fd, I2C_SLAVE, bus_addr) < 0) {
        fprintf(stderr, "Error set slave address: %s\n", strerror(errno));
        return false;
    }

    return write_byte(addr, data);
}

//-----------------------------------------------------------------------------

bool i2c::write(const uint8_t i2c_addr, const std::vector<uint8_t> &data)
{
    if(ioctl(m_fd, I2C_SLAVE, i2c_addr) < 0) {
        fprintf(stderr, "Error set slave address: %s\n", strerror(errno));
        return false;
    }

    ssize_t res = ::write(m_fd, data.data(), data.size());
    if((res < 0) || (res != data.size())) {
        fprintf(stderr, "Error write data: %s\n", strerror(errno));
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------

bool i2c::read(const uint8_t i2c_addr, std::vector<uint8_t> &data, size_t N)
{
    if(ioctl(m_fd, I2C_SLAVE, i2c_addr) < 0) {
        fprintf(stderr, "Error set slave address: %s\n", strerror(errno));
        return false;
    }
    data.clear();
    data.resize(N);
    ssize_t res = ::read(m_fd, data.data(), N);
    if((res <= 0) || (res != N)) {
        fprintf(stderr, "%s(): res = %d, data.size() = %d\n", __func__, res, data.size());
        return false;
    }
    data.resize(res);
    return true;
}

//-----------------------------------------------------------------------------
