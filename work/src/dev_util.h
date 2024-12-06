#pragma once;

#include "brd.h"
#include "ctrlreg.h"
#include "ctrlstrm.h"
#include "gipcy.h"
#include "locale.h"

#ifdef __IPC_WIN__
#include "utypes.h"
#else
#include "utypes_linux.h"
#endif

S32 RegPokeInd(BRD_Handle hSrv, S32 trdNo, S32 rgnum, U32 val);
U32 RegPeekInd(BRD_Handle hSrv, S32 trdNo, S32 rgnum, S32& status);
S32 RegPokeDir(BRD_Handle hSrv, S32 trdNo, S32 rgnum, U32 val);
U32 RegPeekDir(BRD_Handle hSrv, S32 trdNo, S32 rgnum, S32& status);
