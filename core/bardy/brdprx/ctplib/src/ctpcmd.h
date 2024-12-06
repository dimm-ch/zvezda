/********************************************************************
 *  (C) Copyright 1992-2005 InSys Corp.  All rights reserved.
 *
 *  ======== ccpcmd.h ========
 *  Universal test command definition:
 *
 *  version 1.0
 *
 ********************************************************************/

#ifndef __CTPCMD_H
#define __CTPCMD_H

// Command Codes

#define DEF_CTPSTRING 64
#define DEF_DATA 12
#define DEF_RESULT (DEF_DATA - 1)

typedef struct CTP_COMMAND {
    U32 tag; // tag (0x1124BB66)
    U32 cnt; // message counter
    U32 md5; // checking sum
    U32 type; // type of command
    U32 data[DEF_DATA];
} CTP_COMMAND, *PCTP_COMMAND;

#define CTP_CMD_TAG 0x1124BB66

//Types
#define CTP_CMD_TYPE 0x01

//Commands
enum {
    CTP_UPLOAD,
    CTP_DISCONNECT,

    CTP_BRD_OPEN,
    CTP_BRD_CLOSE,
    CTP_BRD_GETINFO,
    CTP_BRD_LOAD,
    CTP_BRD_PULOAD,
    CTP_BRD_PUSTATE,
    CTP_BRD_PULIST,
    CTP_BRD_RESET,
    CTP_BRD_START,
    CTP_BRD_STOP,
    CTP_BRD_SYMBOL,
    CTP_BRD_VERSION,
    CTP_BRD_CAPTURE,
    CTP_BRD_RELEASE,
    CTP_BRD_CTRL,
    CTP_BRD_EXTENSION,
    CTP_BRD_PEEK,
    CTP_BRD_POKE,
    CTP_BRD_READRAM,
    CTP_BRD_WRITERAM,
    CTP_BRD_READFIFO,
    CTP_BRD_WRITEFIFO,
    CTP_BRD_READDPRAM,
    CTP_BRD_WRITEDPRAM,
    CTP_BRD_READ,
    CTP_BRD_WRITE,
    CTP_BRD_GETMSG,
    CTP_BRD_PUTMSG,
    CTP_BRD_SIGNAL,
    CTP_BRD_SIGNALGRAB,
    CTP_BRD_SIGNALWAIT,
    CTP_BRD_SIGNALIACK,
    CTP_BRD_SIGNALFRESH,
    CTP_BRD_SIGNALINFO,
    CTP_BRD_SIGNALLIST,
    CTP_BRD_DEVHARDWAREINIT,
    CTP_BRD_DEVHARDWARECLEANUP,
    CTP_BRD_SIDEHARDWAREINIT,
    CTP_BRD_SIDEHARDWARECLEANUP,
    CTP_BRD_GETSERVLIST,
    CTP_BRD_PUREAD,
    CTP_BRD_PUWRITE,
    CTP_BRD_SERVCMDQUERY
};

enum {

    CTP_BRD_STARTSTREAM = 0xAA00,
    CTP_BRD_STOPSTREAM,
    CTP_BRD_READSTREAM,
    CTP_BRD_WRITESTREAM,
    CTP_BRD_RESETFIFOSTREAM,
    CTP_BRD_CBUFALLOCSTREAM,
    CTP_BRD_CBUFFREESTREAM

};

enum {
    CTP_BRD_OK = 0, //Ok
    CTP_BRD_ERROR
};

#endif // __CTPCMD_H
