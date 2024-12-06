#ifndef BARDY_H
#define BARDY_H

#include "gipcy.h"
#include "brd.h"
#include "ctrlreg.h"
#include "extn.h"
#include "exceptinfo.h"
#include "ctrlstrm.h"
#include "ctrlsdram.h"
#include "string_convert.h"

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <unistd.h>

//-----------------------------------------------------------------------------

class Bardy
{
private:
    Bardy() {
        _num = 0;
        _ok = false;
        BRD_displayMode(BRDdm_VISIBLE | BRDdm_CONSOLE);
        char path[1024];
        getcwd(path, 1024);
        char fname[2048];
        snprintf(fname, 2048, "%s/%s", path, "brd.ini");
        S32 err = BRD_init(fname, &_num);
        //InSys::brd_string iniFile{_BRDC("brd.ini")};
        //S32 err = BRD_init(iniFile.c_str(), &_num);
        if((err < 0) || (_num == 0)) {
            //BRDC_fprintf(stdout, _BRDC("path: %s\n"), iniFile.c_str());
            //BRDC_fprintf(stdout, _BRDC("path: %s\n"), path);
            //BRDC_fprintf(stdout,_BRDC("%s() - Erorr in BRD_init(): 0x%X\n"), __func__, err);
            _ok = false;
        } else {
            _ok = true;
            //fprintf(stdout,"%s, %d: %s() - BRD_init() - OK\n", __FILE__, __LINE__, __FUNCTION__);
        }
    }
    Bardy(const Bardy&);
    Bardy& operator=(Bardy&);
    S32 _num;
    bool _ok;
public:
    static bool initBardy(S32& count) {
        static Bardy bardy;
        return bardy.init_ok(count);
    }

    static bool boardsLID(std::vector<U32>& lids) {
        U32 lid[128] = {0};
        BRD_LidList lidList;
        lidList.item = 128;
        lidList.pLID = lid;
        S32 status = BRD_lidList(lidList.pLID, lidList.item, &lidList.itemReal);
        if(status < 0) {
            fprintf(stdout,"%s, %d: %s() - Erorr in BRD_lidList()!\n", __FILE__, __LINE__, __FUNCTION__);
            return false;
        }
        for(U32 ii=0; ii<lidList.itemReal; ++ii) {
            lids.push_back(lid[ii]);
        }
        return true;
    }
    static bool boardInfo(U32 lid, BRD_Info& info) {
        info.size = sizeof(BRD_Info);
        S32 err = BRD_getInfo(lid, &info);
        if (err < 0) {
            fprintf(stderr, "%s, %d: %s() - Error in BRD_getInfo()!\n", __FILE__, __LINE__, __FUNCTION__);
            return false;
        }
        return true;
    }
    virtual ~Bardy() {
        BRD_cleanup();
        //fprintf(stdout,"%s, %d: %s() - BRD_cleanup() - OK\n", __FILE__, __LINE__, __FUNCTION__);
    }
    bool init_ok(S32& count) {
        count = _num;
        return _ok;
    }
};

//-----------------------------------------------------------------------------

class Gipcy {
public:
    Gipcy() {
        int res = IPC_init();
        if(res != IPC_OK) {
            ok = false;
        } else {
            ok = true;
        }
    }
    virtual ~Gipcy() {
        IPC_cleanup();
    }
    bool is_ok() { return ok; }
private:
    bool ok;
};

//--------------------------------------------------------------------

#endif // BARDY_H
