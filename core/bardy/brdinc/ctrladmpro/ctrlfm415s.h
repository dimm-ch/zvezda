#ifndef _CTRL_FM415S_H
	#define _CTRL_FM415S_H

#include "ctrladmpro.h"
enum {
	BRDctrl_FM415S_SETCLKMODE = BRDctrl_FM402S + 64,
	BRDctrl_FM415S_GETCLKMODE = BRDctrl_FM402S + 65,
	BRDctrl_FM415S_GETSTATUS = BRDctrl_FM402S + 66,
	BRDctrl_FM415S_SETTESTMODE = BRDctrl_FM402S + 67,
	BRDctrl_FM415S_GETTESTMODE = BRDctrl_FM402S + 68,
	BRDctrl_FM415S_START = BRDctrl_FM402S + 69,
	BRDctrl_FM415S_STOP = BRDctrl_FM402S + 70,
	BRDctrl_FM415S_GETTESTRESULT = BRDctrl_FM402S + 71,
	BRDctrl_FM415S_SETDIR = BRDctrl_FM402S + 72,
	BRDctrl_FM415S_GETDIR = BRDctrl_FM402S + 73,
	BRDctrl_FM415S_SETCHANMASK = BRDctrl_FM402S + 74,
	BRDctrl_FM415S_GETCHANMASK = BRDctrl_FM402S + 75,
	BRDctrl_FM415S_GETRXTXSTATUS = BRDctrl_FM402S + 76,
	BRDctrl_FM415S_SETCLKSRC = BRDctrl_FM402S + 77,
	BRDctrl_FM415S_GETCLKSRC = BRDctrl_FM402S + 78,
	BRDctrl_FM415S_SETCLKVALUE = BRDctrl_FM402S + 79,
	BRDctrl_FM415S_GETCLKVALUE = BRDctrl_FM402S + 80,
	BRDctrl_FM415S_PREPARE = BRDctrl_FM402S + 81,
	BRDctrl_FM415S_SETDECIM = BRDctrl_FM402S + 82,
	BRDctrl_FM415S_GETDECIM = BRDctrl_FM402S + 83,
	BRDctrl_FM415S_ILLEGAL = BRDctrl_FM402S + 84,
};

//typedef struct _FM415S_CHANMASK {
//	U32 nChanMask;
//	U32 nStreamChan;
//} FM415S_CHANMASK, * PFM415S_CHANMASK;

typedef struct _FM415S_DIR {
	U08 nChan[4]; //0-Rx, 1-Tx
} FM415S_DIR, * PFM415S_DIR;

typedef struct _FM415S_GETTESTRESULT {
	U32					nChan;
	unsigned long long	lReadWord[32];
	unsigned long long	lExpectWord[32];
	U32					nIndex[32];
	U32					nBlock[32];
	U32					nTotalError;
} FM415S_GETTESTRESULT, * PFM415S_GETTESTRESULT;

typedef struct _FM415S_GETSTATUS {
	U08 isLaneUp[4];
	U08 isPLL_Lock[4];
	U08 isLineUp[4];
	U08 isLinkUp[4];
	U08 isData[4];
	U16 wLinkErrCorr[4];
} FM415S_GETSTATUS, *PFM415S_GETSTATUS;

typedef struct _FM415S_GETRXTXSTATUS {
	U32 nBlockRead[4];
	U32 nBlockOk[4];
	U32 nBlockErr[4];
	U32 nBlockWrite[4];
} FM415S_GETRXTXSTATUS, * PFM415S_GETRXTXSTATUS;

typedef struct _FM415S_TESTMODE {
	U08 isGenEnable;
	U08 isCntEnable;
}FM415S_TESTMODE, * PFM415S_TESTMODE;

#endif