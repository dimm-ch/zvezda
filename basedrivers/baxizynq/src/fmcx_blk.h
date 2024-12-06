#ifndef FMCX_BLK_H
#define FMCX_BLK_H

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

class fmcx_blk : public blk_base
{
public:
    fmcx_blk(fmcx_dev_t hw, int idx);                //! Создаем тетраду по номеру
    fmcx_blk(fmcx_dev_t hw, blk_id id, int start = 0);     //! Создаем тетраду по ID начиная с номера
    virtual ~fmcx_blk();

    bool is_valid() { return _valid; }
    uint32_t rd(uint32_t reg);              //!< Чтение регистра блока
    void wd(uint32_t reg, uint32_t val);    //!< Запись регистра блока

    uint32_t GetPID();
    uint32_t GetTAG();
	int WriteEEPROM(int idRom, const uint16_t* buf, uint16_t Offset, uint32_t Length);
	int ReadEEPROM(int idRom, uint16_t* buf, uint16_t Offset, uint32_t Length);

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
    int WaitSemaReady(int SemaOrReady);
    //int WriteEEPROM(int idRom, const uint16_t* buf, uint16_t Offset, uint32_t Length);
    //int ReadEEPROM(int idRom, uint16_t* buf, uint16_t Offset, uint32_t Length);
};

//-----------------------------------------------------------------------------

typedef std::shared_ptr<fmcx_blk> fmcx_blk_t;
template <class T> fmcx_blk_t get_blk(fmcx_dev_t hw, T val) { return std::make_shared<fmcx_blk>(hw, val); }
//-----------------------------------------------------------------------------


#endif // FMCX_BLK_H
