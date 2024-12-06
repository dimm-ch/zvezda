/********************************************************************
 *  (C) Copyright 1992-2005 InSys Corp.  All rights reserved.
 *
 *  ======== ccpapi.h ========
 *  CCP API functions definition:
 *
 *  version 2.0
 *
 *	NOTE: all data transfered in multibyte strings
 *
 ********************************************************************/

#ifndef __CTPAPI_H
#define __CTPAPI_H

#include "brd.h"
#include "brdapi.h"
#include "brderr.h"
#include "ctpcmd.h"
#include "utypes.h"

#include "gipcy.h"

#define CTP_VERSION 150

#define ENCODE_TETR_REG(a, b) ((b << 8) | a)

enum {
    CTP_IO_NULL = 0,
    CTP_IO_MSG = 1,
    CTP_IO_FIFO = 2,
    CTP_IO_RAM = 3,
    CTP_IO_DPRAM = 4,

    CTP_ADM_DIR = 5,
    CTP_ADM_IND = 6,

    CTP_IO_PU = 7,

    CTP_IO_STREAM = 8,

    CTP_IO_MAX
};

enum {
    CTP_SIGNAL_NULL = 0,
    CTP_SIGNAL_SEND = 1,
    CTP_SIGNAL_GRAB,
    CTP_SIGNAL_IACK,
    CTP_SIGNAL_WAIT
};

typedef struct {
    IPC_handle s;
} CTP_HOST;

typedef struct {
    CTP_HOST* host;
    S32 devid;
} CTP_DEV;

typedef struct
{
    S32 code; // reserved
    U32 boardType; // Board Type
    char name[128]; // Device Name ASCII
    U32 pid; // Board Phisical ID
    U32 verMajor; // Driver Version
    U32 verMinor; // Driver Version
    U32 subunitType[16]; // Subunit Type Code
} TCTP_GetInfo;

typedef struct
{
    U32 size;
    U32 running;
    U32 exitCode;

    U32 errCnt;
    U32 warnCnt;
} TCTP_GetState;

typedef struct
{
    U32 puId; // Programmable Unit ID
    U32 puCode; // Programmable Unit Code
    U32 puAttr; // Programmable Unit Attribute
    char puDescription[128]; // Programmable Unit Description Text
} TCTP_PuList;

#define DEF_CTPPNAME _BRDC("ctp://")
#define DEF_CTPPLEN ((sizeof(DEF_CTPPNAME) / sizeof(BRDCHAR)) - 1)

#ifdef __cplusplus
extern "C" {
#endif

CTP_HOST* CTP_Connect(BRDCHAR* adress, U32 port, U32 params);
S32 CTP_Disconnect(CTP_HOST* handle);

#if _WIN64

#define CTP_mbstobcs(dst, src, maxcnt) mbstowcs(dst, src, maxcnt)
#define CTP_bcstombs(dst, src, maxcnt) wcstombs(dst, src, maxcnt)

#else

#define CTP_mbstobcs(dst, src, maxcnt) strncpy(dst, src, maxcnt)
#define CTP_bcstombs(dst, src, maxcnt) strncpy(dst, src, maxcnt)

#endif

//
//
//

CTP_DEV* CTP_Open(CTP_HOST* host, U32 brdid);
S32 CTP_Close(CTP_DEV* dev);
S32 CTP_GetInfo(CTP_HOST* host, U32 brdid, TCTP_GetInfo* info);

S32 CTP_UploadFile(CTP_DEV* dev, BRDCHAR* filename, void* buffer, U32 size);

S32 CTP_LoadDSP(CTP_DEV* dev, U32 nodeId, BRDCHAR** argv, U32 argc);
S32 CTP_Start(CTP_DEV* dev, U32 nodeId);
S32 CTP_Stop(CTP_DEV* dev, U32 nodeId);

S32 CTP_PULoad(CTP_DEV* dev, BRDCHAR* filename, U32 puID);
S32 CTP_PUState(CTP_DEV* dev, U32 puID);
S32 CTP_PUList(CTP_DEV* dev, TCTP_PuList* list, U32 nItems);

S32 CTP_Read(CTP_DEV* dev, U32 nodeID, U32 type, U32 offset, void* data, U32 size, S32 timeout = 0);
S32 CTP_Write(CTP_DEV* dev, U32 nodeID, U32 type, U32 offset, void* data, U32 size, S32 timeout = 0);

S32 CTP_GetMsg(CTP_DEV* dev, U32 nodeId, void* data, U32 size);
S32 CTP_PutMsg(CTP_DEV* dev, U32 nodeId, void* data, U32 size);

S32 CTP_Signal(CTP_DEV* dev, U32 nodeID, U32 type, int sigId, U32* param);

S32 CTP_ReadEx(CTP_DEV* dev, U32 nodeID, U32 type, U32 devID, U32 offset, void* data, U32 size, S32 timeout);

S32 CTP_StartStream(CTP_DEV* s, U32 dir, U32 fifoId, S32 size, S32 blknum, U32 isCycle, U32 drq);
S32 CTP_StopStream(CTP_DEV* s, U32 dir, U32 fifoId, S32 size, S32 blknum, U32 isCycle);

S32 CTP_ReadStream(CTP_DEV* s, CTP_HOST* host, int src, void* data, U32 size);
S32 CTP_WriteStream(CTP_DEV* s, CTP_HOST* host, int src, void* data, U32 size, S32 blknum);

S32 CTP_ResetFifoStream(CTP_DEV* s, U32 dir, U32 fifoId);

S32 CTP_CBufAllocStream(CTP_DEV* s, U32 dir, U32 fifoId, S32 size, S32 blknum, U32 isCycle);

S32 CTP_CBufFreeStream(CTP_DEV* s, U32 dir, U32 fifoId);

#ifdef __cplusplus
}; //extern "C"
#endif

#endif // __CTPAPI_H
