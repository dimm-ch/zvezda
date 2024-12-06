
#ifndef CTPSERVER_H
#define CTPSERVER_H

#include "gipcy.h"

#define SRV_ERROR -1
#define DEF_PORT _BRDC("6021")
#define DEF_CONFIG _BRDC(".\\isagent.ini")

extern int CreateServer( void );
extern int DestroyServer( void );
extern int ServerLoop( void );

typedef struct  {
	int id;
    IPC_handle SData;
} SERVER_THREAD_PARAM;

#define MAX_TIMEOUT 1 //msec
#define MAX_CMDTIMEOUT 10 //msec

extern volatile long g_clCnt;

#endif
