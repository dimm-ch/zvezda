/********************************************************************
 *  (C) Copyright 1992-2005 InSys Corp.  All rights reserved.
 *
 *  ======== ccpapi.cpp ========
 *  CTP Common remote Test Protocole
 *
 *  version 2.0
 *
 *  By DOR	(nov. 2005)
 ********************************************************************/
#include <stdlib.h>

#include "brderr.h"
#include "ctpapi.h"
#include "gipcy.h"
#include "netcmn.h"
#include "utypes.h"

#define MAX_CTPDEVICE 16
#define MAX_TIMEOUT 25

int ctp_cmd_CNT = 0; // счетчик команд CCP

IPC_sockaddr _IPC_resolve(IPC_str* addr)
{
    IPC_sockaddr a;

    int port = INADDR_ANY;

    //FIXME: small?
    char buffer[256];

#ifdef _WIN64
    wcstombs(buffer, addr, sizeof(buffer));
#else
    strcpy(buffer, (const char*)addr);
#endif

    char* pp = strstr(buffer, ":");

    if (pp) {
        port = atoi(pp + 1);
        *pp = 0;
    }

    unsigned long ip = inet_addr(buffer);

    a.port = port;
    a.addr.ip = ip;

    return a;
}

CTP_HOST* CTP_Connect(BRDCHAR* adress, U32 port, U32 params)
{
    static BOOL isFirstRun = TRUE;

    IPC_sockaddr anAddr;
    fd_set Sel;

    if (isFirstRun) {
        if (IPC_initSocket() == -1)
            return NULL;

        isFirstRun = FALSE;
    }
    IPC_handle s;

    s = IPC_openSocket(IPC_tcp); // Определение сокета и протокола

    if (s == nullptr)
        return 0;

    ULONG ipaddr = _IPC_resolve(adress).addr.ip;

    if (INADDR_NONE == ipaddr || INADDR_ANY == ipaddr) {

        ipaddr = IPC_gethostbyname(adress).addr.ip;
    }

    if (INADDR_NONE == ipaddr)
        return 0;

    // Инициализация сокета и параметров сети
    anAddr.port = port;
    anAddr.addr.ip = ipaddr; //inet_addr(adress);

    u_long uMode = 1;
    timeval tval = { MAX_TIMEOUT, 0 };

    int err = IPC_connect(s, &anAddr);

#ifndef __linux__
    err = WSAGetLastError();
#endif

    IPC_FD_ZERO(&Sel);
    IPC_FD_SET(s, &Sel);
    int r = IPC_select(s, 0, &Sel, 0, &tval);
    if ((r == IPC_SOCKET_ERROR) || (r == 0)) {
        return 0;
    }

    IPC_FD_CLR(s, &Sel);

    CTP_HOST* host = new CTP_HOST;

    host->s = s;
    ctp_cmd_CNT = 0; // счетчик команд CTP

    return host;
}

S32 CTP_Disconnect(CTP_HOST* host)
{
    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    if (!host)
        return 0;

    // get device index
    IPC_handle s = host->s;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;

    ctp_cmd.type = CTP_DISCONNECT;

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    IPC_shutdown(s, IPC_SD_BOTH);
    IPC_closeSocket(s);

    host->s = (IPC_handle)-1;

    delete host;

    return 0;
}

CTP_DEV* CTP_Open(CTP_HOST* host, U32 brdid)
{
    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;

    ctp_cmd.type = CTP_BRD_OPEN;

    ctp_cmd.data[0] = brdid;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    CTP_DEV* dev = new CTP_DEV;

    dev->host = host;
    dev->devid = ctp_cmd_ack.data[DEF_RESULT];

    return dev;
}

CTP_DEV* CTP_Attach(CTP_HOST* host, CTP_DEV* dev)
{
    CTP_DEV* outdev = new CTP_DEV;

    outdev->host = host;
    outdev->devid = dev->devid;

    return outdev;
}

S32 CTP_GetInfo(CTP_HOST* host, U32 brdid, TCTP_GetInfo* info)
{
    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;

    ctp_cmd.type = CTP_BRD_GETINFO;

    ctp_cmd.data[0] = brdid; //LID
    ctp_cmd.data[1] = sizeof(TCTP_GetInfo);

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    recvDataBlock(host->s, info, sizeof(TCTP_GetInfo));

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_Close(CTP_DEV* dev)
{
    if (!dev)
        return -1;

    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_CLOSE;

    ctp_cmd.data[0] = dev->devid;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    delete dev;

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_UploadFile(CTP_DEV* dev, BRDCHAR* filename, void* buffer, U32 size)
{
    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_UPLOAD;

    ctp_cmd.data[0] = size;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    char str[DEF_CTPSTRING];

    CTP_bcstombs(str, filename, DEF_CTPSTRING);

    sendDataBlock(host->s, str, DEF_CTPSTRING);

    sendDataBlock(host->s, buffer, size);

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_LoadDSP(CTP_DEV* dev, U32 nodeId, BRDCHAR** argv, U32 argc)
{
    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_LOAD;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = nodeId;
    ctp_cmd.data[2] = argc;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    char str[DEF_CTPSTRING];

    for (U32 i = 0; i < argc; i++) {
        CTP_bcstombs(str, argv[i], DEF_CTPSTRING);
        sendDataBlock(host->s, str, DEF_CTPSTRING);
    }

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_Start(CTP_DEV* dev, U32 nodeId)
{
    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_START;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = nodeId;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_Stop(CTP_DEV* dev, U32 nodeId)
{
    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_STOP;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = nodeId;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_PULoad(CTP_DEV* dev, BRDCHAR* filename, U32 puID)
{
    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_PULOAD;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = puID;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    char str[DEF_CTPSTRING];
    CTP_bcstombs(str, filename, DEF_CTPSTRING);

    sendDataBlock(host->s, str, DEF_CTPSTRING);

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_PUState(CTP_DEV* dev, U32 puID)
{
    if (!dev)
        return -1;

    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_PUSTATE;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = puID;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_PUList(CTP_DEV* dev, TCTP_PuList* list, U32 nItems)
{
    if (!dev)
        return -1;

    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_PULIST;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = nItems;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    recvDataBlock(host->s, list, sizeof(TCTP_PuList) * nItems);

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_Read(CTP_DEV* dev, U32 nodeID, U32 type, U32 offset, void* data, U32 size, S32 timeout)
{
    return CTP_ReadEx(dev, nodeID, type, 0, offset, data, size, timeout);
}

S32 CTP_ReadEx(CTP_DEV* dev, U32 nodeID, U32 type, U32 devID, U32 offset, void* data, U32 size, S32 timeout)
{
    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_READ;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = nodeID;
    ctp_cmd.data[2] = type;
    ctp_cmd.data[3] = size;
    ctp_cmd.data[4] = offset;
    ctp_cmd.data[5] = timeout;
    ctp_cmd.data[6] = devID;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    recvDataBlock(host->s, data, size);

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_WriteEx(CTP_DEV* dev, U32 nodeID, U32 type, U32 devID, U32 offset, void* data, U32 size, S32 timeout)
{
    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_WRITE;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = nodeID;
    ctp_cmd.data[2] = type;
    ctp_cmd.data[3] = size;
    ctp_cmd.data[4] = offset;
    ctp_cmd.data[5] = timeout;
    ctp_cmd.data[6] = devID;

    // send command block to CCP

    // 	sendDataBlock(host->s,&ctp_cmd,sizeof(CTP_COMMAND));
    //
    // 	sendDataBlock(host->s,data,size);

    char* buf = (char*)malloc(sizeof(CTP_COMMAND) + size);
    memcpy(buf, &ctp_cmd, sizeof(CTP_COMMAND));
    memcpy(buf + sizeof(CTP_COMMAND), data, size);
    sendDataBlock(host->s, buf, sizeof(CTP_COMMAND) + size);
    free(buf);

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_Write(CTP_DEV* dev, U32 nodeID, U32 type, U32 offset, void* data, U32 size, S32 timeout)
{
    return CTP_WriteEx(dev, nodeID, type, 0, offset, data, size, timeout);
}

S32 CTP_PutMsg(CTP_DEV* dev, U32 nodeId, void* data, U32 size)
{
    return CTP_Write(dev, nodeId, CTP_IO_MSG, 0, data, size);
}

S32 CTP_GetMsg(CTP_DEV* dev, U32 nodeId, void* data, U32 size)
{
    return CTP_Read(dev, nodeId, CTP_IO_MSG, 0, data, size);
}

S32 CTP_Signal(CTP_DEV* dev, U32 nodeID, U32 type, int sigId, U32* param)
{
    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_SIGNAL;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = nodeID;
    ctp_cmd.data[2] = type;
    ctp_cmd.data[3] = sigId;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    if (param)
        *param = ctp_cmd_ack.data[4];

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_Ext(HANDLE handle, U32 cmd, void* data)
{
    return 0;
}

S32 CTP_StartStream(CTP_DEV* dev, U32 dir, U32 fifoId, S32 size, S32 blknum, U32 isCycle, U32 drq)
{
    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_STARTSTREAM;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = dir;
    ctp_cmd.data[2] = fifoId;
    ctp_cmd.data[3] = size;
    ctp_cmd.data[4] = isCycle;
    ctp_cmd.data[5] = drq;
    ctp_cmd.data[6] = blknum;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_StopStream(CTP_DEV* dev, U32 dir, U32 fifoId, S32 size, S32 blknum, U32 isCycle)
{
    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_STOPSTREAM;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = dir;
    ctp_cmd.data[2] = fifoId;
    ctp_cmd.data[3] = size;
    ctp_cmd.data[4] = isCycle;
    ctp_cmd.data[5] = blknum;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_ReadStream(CTP_DEV* dev, CTP_HOST* host, int src, void* data, U32 size)
{

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_READSTREAM;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = 1;
    ctp_cmd.data[2] = src;
    ctp_cmd.data[3] = size;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    //set error
    ctp_cmd_ack.data[DEF_RESULT] = -1;

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    if (ctp_cmd_ack.data[DEF_RESULT] == 0)
        recvDataBlock(host->s, data, size);

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_WriteStream(CTP_DEV* dev, CTP_HOST* host, int src, void* data, U32 size, S32 blknum)
{
    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_WRITESTREAM;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = 2;
    ctp_cmd.data[2] = 0;
    ctp_cmd.data[3] = size;
    ctp_cmd.data[4] = blknum;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    //set error
    ctp_cmd_ack.data[DEF_RESULT] = -1;

    sendDataBlock(host->s, data, size);

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_ResetFifoStream(CTP_DEV* dev, U32 dir, U32 fifoId)
{
    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    CTP_HOST* host = dev->host;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_RESETFIFOSTREAM;

    ctp_cmd.data[0] = dir;
    ctp_cmd.data[1] = fifoId;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_CBufAllocStream(CTP_DEV* dev, U32 dir, U32 fifoId, S32 size, S32 blknum, U32 isCycle)
{
    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_CBUFALLOCSTREAM;

    ctp_cmd.data[0] = dev->devid; //LID
    ctp_cmd.data[1] = dir;
    ctp_cmd.data[2] = fifoId;
    ctp_cmd.data[3] = size;
    ctp_cmd.data[4] = isCycle;
    ctp_cmd.data[5] = blknum;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

S32 CTP_CBufFreeStream(CTP_DEV* dev, U32 dir, U32 fifoId)
{
    CTP_HOST* host = dev->host;

    CTP_COMMAND ctp_cmd;
    CTP_COMMAND ctp_cmd_ack;

    memset(&ctp_cmd, 0, sizeof(CTP_COMMAND));
    memset(&ctp_cmd_ack, 0, sizeof(CTP_COMMAND));

    ctp_cmd.tag = CTP_CMD_TAG;
    ctp_cmd.cnt = ctp_cmd_CNT++;
    ctp_cmd.type = CTP_BRD_CBUFFREESTREAM;

    ctp_cmd.data[0] = dir;
    ctp_cmd.data[1] = fifoId;

    // send command block to CCP

    sendDataBlock(host->s, &ctp_cmd, sizeof(CTP_COMMAND));

    // get return status
    recvDataBlock(host->s, &ctp_cmd_ack, sizeof(CTP_COMMAND));

    return ctp_cmd_ack.data[DEF_RESULT];
}

//
// End of file
//
