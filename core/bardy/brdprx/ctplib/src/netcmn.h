#pragma once

#ifdef WIN32
//#include <winsock2.h>
//#include <ws2tcpip.h>
#endif

#ifdef LINUX
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#endif

#include "gipcy.h"
#include "utypes.h"

S32 sendDataBlock(IPC_handle s, void* buf, S32 sizeOfBytes);
S32 recvDataBlock(IPC_handle s, void* buf, S32 sizeOfBytes);
