/********************************************************************
 *  (C) Copyright 1992-2005 InSys Corp.  All rights reserved.
 *
 *  ======== ccpcmd.h ========
 *  Universal test command definition:
 *
 *  version 1.0
 *
 ********************************************************************/

#ifndef __NETCMD_H__
#define __NETCMD_H__

#define NET_STRING 64
#define NET_DATA 12

#ifdef __linux__
#define __packed__   __attribute__((packed))
#else
#define __packed__
#endif

#ifdef __linux__
#include <stdint.h>
#else
// TODO: add uint32_t definition
#endif

typedef struct NET_COMMAND {
    uint32_t tag;                    // tag (0xAABBCCDD)
    uint32_t cnt;                    // message counter
    uint32_t type;                   // type of command
    uint32_t status;                 // status of operation
    uint32_t data[NET_DATA];
} __packed__ NET_COMMAND, *PNET_COMMAND;

#define NET_CMD_TAG     0xAABBCCDD

//Network commands
enum
{
    NET_UPLOAD,
    NET_DOWNLOAD,
    NET_CLOSE,
    NET_WRITE_FLAG,
    NET_READ_FLAG,
    NET_FILE_SIZE,
    NET_FILE_DATA,
    NET_FILE_EXISTS,
    NET_EXEC,
    NET_EXIT,
    NET_MAP_FPGA,
    NET_UNMAP_FPGA,
    NET_REG_PEEK_DIR,
    NET_REG_POKE_DIR,
    NET_REG_PEEK_IND,
    NET_REG_POKE_IND,
    NET_BLOCK_READ,
    NET_BLOCK_WRITE,
    NET_EXEC_WAIT,
	NET_STDIO,
	NET_READ_FLAG_SHM,
	NET_WRITE_FLAG_SHM,
	NET_DATABIN_SHM,
	NET_DATASIZE_SHM
};

#endif  // __NETCMD_H__
