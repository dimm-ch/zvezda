#ifndef FID_BLK_H
#define FID_BLK_H

#include "blk_base.h"
#include "fmcx_dev.h"
#include "utypes_linux.h"
#include "axizynqregs.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <memory>

//-----------------------------------------------------------------------------

class fid_blk : public blk_base
{
public:
    fid_blk(fmcx_dev_t hw, int idx);                //! Создаем тетраду по номеру
    fid_blk(fmcx_dev_t hw, blk_id id, int start = 0);     //! Создаем тетраду по ID начиная с номера
    virtual ~fid_blk();

    bool is_valid() { return _valid; }
    uint32_t rd(uint32_t reg);              //!< Чтение регистра блока
    void wd(uint32_t reg, uint32_t val);    //!< Запись регистра блока
    const std::string& name() { return _name; }

    int WriteFmcEeprom(int devid, int devadr, uint8_t *buf, uint16_t Offset, long Length);
    int ReadFmcEeprom(int devid, int devadr, uint8_t *buf, uint16_t Offset, long Length);

private:
    fmcx_dev_t _hw;
    bool _valid;
    bool _direct;
    int  _idx;
    blk_id _id;
    uint32_t* _bar;
    std::string _name;

    uint32_t read_blk_reg(uint32_t regnum);
    void write_blk_reg(uint32_t regnum, uint32_t val);
    bool WaitReady();
    //int WriteEEPROM(int idRom, const uint16_t* buf, uint16_t Offset, uint32_t Length);
    //int ReadEEPROM(int idRom, uint16_t* buf, uint16_t Offset, uint32_t Length);
};

//-----------------------------------------------------------------------------

typedef std::shared_ptr<fid_blk> fid_blk_t;
template <class T> fid_blk_t get_fid_blk(fmcx_dev_t hw, T val) { return std::make_shared<fid_blk>(hw, val); }
//-----------------------------------------------------------------------------


#endif // FID_BLK_H
