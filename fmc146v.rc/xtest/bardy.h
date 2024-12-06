#ifndef BARDY_H
#define BARDY_H

#include "gipcy.h"
#include "brd.h"
#include "ctrlreg.h"
#include "extn.h"
#include "extn_do.h"
#include "extn_dex.h"
#include "exceptinfo.h"
#include "ctrlstrm.h"
#include "ctrlsdram.h"

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <string>
#include <memory>


//-----------------------------------------------------------------------------

class Bardy
{
private:
    Bardy() {
        _num = 0;
        _ok = false;
        char path[1024];
        getcwd(path, 1024);
        char fname[2048];
        snprintf(fname, 2048, "%s/%s", path, "brd.ini");
        S32 err = BRD_init(fname, &_num);
        if((err < 0) || (_num == 0)) {
            fprintf(stdout,"%s, %d: %s() - Erorr in BRD_init()!\n", __FILE__, __LINE__, __FUNCTION__);
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

#endif // BARDY_H
