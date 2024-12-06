#include "dev_util.h"

S32 RegPokeInd(BRD_Handle hSrv, S32 trdNo, S32 rgnum, U32 val)
{
    BRD_Reg params;
    params.bytes = sizeof(U32);
    params.reg = rgnum;
    params.tetr = trdNo;
    params.val = val;

    return BRD_ctrl(hSrv, NODE0, BRDctrl_REG_WRITEIND, &params);
}

U32 RegPeekInd(BRD_Handle hSrv, S32 trdNo, S32 rgnum, S32& status)
{
    BRD_Reg params;
    U32 ret;

    params.bytes = sizeof(U32);
    params.reg = rgnum;
    params.tetr = trdNo;
    params.val = 0;

    status = BRD_ctrl(hSrv, NODE0, BRDctrl_REG_READIND, &params);
    ret = params.val;
    ret &= 0xFFFF;
    return ret;
}

S32 RegPokeDir(BRD_Handle hSrv, S32 trdNo, S32 rgnum, U32 val)
{
    BRD_Reg params;
    params.bytes = sizeof(U32);
    params.reg = rgnum;
    params.tetr = trdNo;
    params.val = val;

    return BRD_ctrl(hSrv, NODE0, BRDctrl_REG_WRITEDIR, &params);
}

U32 RegPeekDir(BRD_Handle hSrv, S32 trdNo, S32 rgnum, S32& status)
{
    BRD_Reg params;
    U32 ret;

    params.bytes = sizeof(U32);
    params.reg = rgnum;
    params.tetr = trdNo;
    params.val = 0;

    status = BRD_ctrl(hSrv, NODE0, BRDctrl_REG_READDIR, &params);
    //        ret=params.val & 0xFFFF;
    ret = params.val;
    return ret;
}