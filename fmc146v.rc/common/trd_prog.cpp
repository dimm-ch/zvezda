#include "trd_prog.h"
#include "ctrlreg.h"
#include "gipcy.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#include <vector>
#include <iostream>
#include <fstream>

//-----------------------------------------------------------------------------

using namespace std;

//-----------------------------------------------------------------------------

trd_prog::trd_prog(brd_dev_t dev, std::string fileName) : m_dev(dev), m_configFile(fileName)
{
    std::vector<std::string> optionsList;
    get_options(optionsList);
    process_options(optionsList);
}

//-----------------------------------------------------------------------------

trd_prog::~trd_prog()
{
}

//-----------------------------------------------------------------------------

size_t trd_prog::get_options(std::vector<std::string>& optionsList)
{
    std::string     str;
    fstream         ifs;

    ifs.open(m_configFile.c_str(), ios::in);
    if (!ifs.is_open()) {
        return -2;
    }

    optionsList.clear();

    while(!ifs.eof()) {

        getline( ifs, str );

        if(!str.length())
            continue;

        if(strstr(str.c_str(),"#")) {
            continue;
        }

        if(strstr(str.c_str(),"TRD_")) {
            optionsList.push_back(str);
        }

        if (strstr(str.c_str(), "BLK_")) {
            optionsList.push_back(str);
        }

        if (strstr(str.c_str(), "SPD_")) {
            optionsList.push_back(str);
        }
    }

    //fprintf(stderr, "optionListSize = %d\n", optionsList.size());

    return optionsList.size();
}

//-----------------------------------------------------------------------------

int trd_prog::process_options(std::vector<std::string>& optionsList)
{
    if(optionsList.empty())
        return -1;

    vector<unsigned> argList;

    for(unsigned i=0; i<optionsList.size(); i++) {

        string& str = optionsList.at(i);

        get_args(str, argList);

        size_t begin = 0;
        size_t end = str.find_first_of("(",0);
        if(end < 0)
            continue;

        string opt = str.substr(begin, (end-begin));

        if (strcmp(opt.c_str(), "BLK_READ") == 0) {
            process_blk_read(argList);
        } else if (strcmp(opt.c_str(), "BLK_WRITE") == 0) {
            process_blk_write(argList);
        } else if(strcmp(opt.c_str(),"TRD_READ") == 0) {
            process_trd_read(argList);
        } else if(strcmp(opt.c_str(),"TRD_WRITE") == 0) {
            process_trd_write(argList);
        } else if(strcmp(opt.c_str(),"TRD_SET_REG_BIT") == 0) {
            process_trd_set_bit(argList);
        } else if(strcmp(opt.c_str(),"TRD_CLR_REG_BIT") == 0) {
            process_trd_clr_bit(argList);
        } else if(strcmp(opt.c_str(),"TRD_WAIT_REG_VAL") == 0) {
            process_trd_wait_val(argList);
        } else if(strcmp(opt.c_str(),"SPD_WRITE") == 0) {
            process_trd_write_spd(argList);
        } else if(strcmp(opt.c_str(),"SPD_READ") == 0) {
            process_trd_read_spd(argList);
        } else if(strcmp(opt.c_str(),"SPD_WAIT_VAL") == 0) {
            process_trd_wait_spd_val(argList);
        } else if(strcmp(opt.c_str(),"TRD_PAUSE") == 0) {
            process_trd_pause(argList);
        }

        argList.clear();
    }

    return 0;
}

//-----------------------------------------------------------------------------

bool trd_prog::get_args(string& str, std::vector<unsigned>& argList)
{
    int totalParam = 1;
    size_t begin = 0;
    size_t end = 0;

    argList.clear();

    end = str.find_first_of("(",begin);
    if(end < 0)
        return false;

    begin = end;
    begin++;

    for(size_t i=0; i<str.length(); i=end) {

        end = str.find_first_of(",",begin);
        if(end < 0) {

            end = str.find_last_of(")",str.length());
            string value = str.substr(begin, (end-begin));
            argList.push_back(get_value(value));
            break;
        }

        string value = str.substr(begin, (end-begin));
        argList.push_back(get_value(value));

        totalParam += 1;
        begin = end;
        begin++;
    }

    return !argList.empty();
}

//-----------------------------------------------------------------------------

unsigned trd_prog::get_value(std::string& value)
{
    size_t begin = 0;
    size_t end = 0;
    unsigned val = 0;

    end = value.find_first_of("x",begin);
    if(end < 0) {
        val = strtoul(value.c_str(), 0, 10);
    } else {
        val = strtoul(value.c_str(), 0, 16);
    }

    return val;
}

//-----------------------------------------------------------------------------

int trd_prog::process_blk_read(std::vector<unsigned>& argList)
{
    if (argList.empty())
        return -1;

    if (!m_dev->is_valid())
        return -2;

    U32 regVal = m_dev->rh(argList.at(0), argList.at(1));

    fprintf(stderr, "BLK_READ( %d, 0x%X ) : VAL: 0x%X\n", argList.at(0), argList.at(1), regVal);

    return 0;
}

//-----------------------------------------------------------------------------

int trd_prog::process_blk_write(std::vector<unsigned>& argList)
{
    if (argList.empty())
        return -1;

    if (!m_dev->is_valid())
        return -2;

    m_dev->wh(argList.at(0), argList.at(1), argList.at(2));

    fprintf(stderr, "BLK_WRITE( %d, 0x%X, 0x%X )\n", argList.at(0), argList.at(1), argList.at(2));

    return 0;
}

//-----------------------------------------------------------------------------

int trd_prog::process_trd_read(std::vector<unsigned>& argList)
{
    if(argList.empty())
        return -1;

    if (!m_dev->is_valid())
        return -2;

    U32 regVal = m_dev->ri(argList.at(0), argList.at(1));

    fprintf(stderr, "TRD_READ( %d, 0x%X ) : VAL: 0x%X\n", argList.at(0), argList.at(1), regVal);

    return 0;
}

//-----------------------------------------------------------------------------

int trd_prog::process_trd_write(std::vector<unsigned>& argList)
{
    if(argList.empty())
        return -1;

    if (!m_dev->is_valid())
        return -2;

    m_dev->wi(argList.at(0), argList.at(1), argList.at(2));

    fprintf(stderr, "TRD_WRITE( %d, 0x%X, 0x%X )\n", argList.at(0), argList.at(1), argList.at(2));


    return 0;
}

//-----------------------------------------------------------------------------

int trd_prog::process_trd_set_bit(std::vector<unsigned>& argList)
{
    if(argList.empty())
        return -1;

    if (!m_dev->is_valid())
        return -2;

    U32 trdRegValue = m_dev->ri(argList.at(0), argList.at(1));
    trdRegValue |= (1 << argList.at(2));
    m_dev->wi(argList.at(0), argList.at(1), trdRegValue);

    fprintf(stderr, "TRD_SET_REG_BIT( %d, 0x%X, 0x%X )\n", argList.at(0), argList.at(1), trdRegValue);

    return 0;
}

//-----------------------------------------------------------------------------

int trd_prog::process_trd_clr_bit(std::vector<unsigned>& argList)
{
    if(argList.empty())
        return -1;

    if (!m_dev->is_valid())
        return -2;

    U32 trdRegValue = m_dev->ri(argList.at(0), argList.at(1));
    trdRegValue &= ~(1 << argList.at(2));
    m_dev->wi(argList.at(0), argList.at(1), trdRegValue);

    fprintf(stderr, "TRD_CLR_REG_BIT( %d, 0x%X, 0x%X )\n", argList.at(0), argList.at(1), trdRegValue);

    return 0;
}

//-----------------------------------------------------------------------------

int trd_prog::process_trd_pause(std::vector<unsigned>& argList)
{
    if(argList.empty())
        return -1;

    IPC_delay(argList.at(0));

    return 0;
}

//-----------------------------------------------------------------------------

int trd_prog::process_trd_wait_val(std::vector<unsigned>& argList)
{
    if(argList.empty())
        return -1;

    if (!m_dev->is_valid())
        return -2;

    unsigned attempt = argList.at(3);
    unsigned pass = 0;
    U32 trdRegValue = 0;

    while(1) {

        trdRegValue = m_dev->ri(argList.at(0), argList.at(1));
        if(trdRegValue == argList.at(2)) {
            break;
        }

        ++pass;

        if(pass >= attempt) {
            fprintf(stderr, "TRD_WAIT_REG_VAL( %d, 0x%X ): TIMEOUT\n", argList.at(0), argList.at(1));
            return -1;
        }

        IPC_delay(1);
    }

    fprintf(stderr, "TRD_WAIT_REG_VAL( %d, 0x%X ): VAL 0x%X\n", argList.at(0), argList.at(1), trdRegValue);

    return 0;
}

//-----------------------------------------------------------------------------

int trd_prog::process_trd_write_spd(std::vector<unsigned>& argList)
{
    if(argList.empty())
        return -1;

    if (!m_dev->is_valid())
        return -2;

    //writeSpdDev(argList.at(0), argList.at(1), argList.at(2), argList.at(3), 0);
    //m_dev->ws();

    fprintf(stderr, "TRD_WRITE_SPD( %d, 0x%X, 0x%X, 0x%X )\n",
            argList.at(0), argList.at(1), argList.at(2), argList.at(3));

    return 0;
}

//-----------------------------------------------------------------------------

int trd_prog::process_trd_read_spd(std::vector<unsigned>& argList)
{
    if(argList.empty())
        return -1;

    if (!m_dev->is_valid())
        return -2;

    U32 val = 0;
    //m_dev->rs();

    fprintf(stderr, "TRD_READ_SPD( %d, 0x%X, 0x%X ): VAL 0x%X\n",
            argList.at(0), argList.at(1), argList.at(2), val & 0xFF);

    return 0;
}

//-----------------------------------------------------------------------------

int trd_prog::process_trd_wait_spd_val(std::vector<unsigned>& argList)
{
    if(argList.empty())
        return -1;

    if (!m_dev->is_valid())
        return -2;

    unsigned attempt = argList.at(4);
    unsigned pass = 0;
    U32 spdRegValue = 0;

    while(1) {

        //readSpdDev(argList.at(0), argList.at(1), argList.at(2), 0, spdRegValue);
        if((spdRegValue & 0xff) == argList.at(3)) {
            break;
        }

        ++pass;

        if(pass >= attempt) {
            fprintf(stderr, "TRD_WAIT_SPD_VAL( %d, 0x%X ): --- TIMEOUT! VAL = 0x%x\n", argList.at(0), argList.at(1), (spdRegValue & 0xff));
            return -1;
        }

        IPC_delay(1);
    }

    fprintf(stderr, "TRD_WAIT_SPD_VAL( %d, 0x%X ): OK! --- VAL 0x%X\n", argList.at(0), argList.at(1), (spdRegValue & 0xff));

    return 0;
}

//-----------------------------------------------------------------------------

