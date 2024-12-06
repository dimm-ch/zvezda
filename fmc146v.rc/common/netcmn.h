
#ifndef __NETCMN_H__
#define __NETCMN_H__

#ifdef WIN32
#include <winsock2.h>

#define MAX_TIMEOUT 1000
#endif

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#ifndef ZeroMemory
#    define ZeroMemory(x,y) memset(x, 0, y)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR    -1
#endif

#ifndef MAX_TIMEOUT
#define MAX_TIMEOUT 	25
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  -1
#endif

//#define max(a, b) a > b ? a : b
#endif

#include "utypes.h"
#include "gipcy.h"

int sendDataBlock(SOCKET s,void *buf, size_t sizeOfBytes);
int recvDataBlock(SOCKET s,void *buf, size_t sizeOfBytes);
size_t getFileSize(const char *fname, void **fileBuffer);
int FileExists(const char *fname);
size_t save_data_file(const char* filename, void *buffer, size_t size);
void WriteFlagSinc(char *fileName, int flg, int isNewParam);
int  ReadFlagSinc(char *filename);
void Delay(int ms);

#endif  //__NETCMN_H__
