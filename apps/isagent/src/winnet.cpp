#include "netcmn.h"

#include "ctpapi.h"
#include "winnet.h"

#include "brd.h"

extern BOOL g_IsServerRunnning;
volatile long g_clCnt;


static IPC_handle SComm;

extern FILE* islog;

//=====================================================================
//		 ������������� ���������� wsock32.lib
//=====================================================================
void Wsock32LibInit(void)
{
    if (IPC_initSocket() == IPC_SOCKET_ERROR)
        printf (" WSAStartup() failed: %ld\n", (long int)IPC_sysError());
}

int CreateServer( void )
{

    IPC_sockaddr anAddr;// C�������a SOCKADDR_IN ��� ���������� ������
	int nPort;
    BRDCHAR sPort[8];

    fprintf(islog, "Wsock32LibInit()\n");
	Wsock32LibInit();
    fprintf(islog, "Wsock32LibInit() complete\n");

    if(IPC_getPrivateProfileString(_BRDC("setup"), _BRDC("port"), (DEF_PORT), sPort, 8, (DEF_CONFIG)) == (int)IPC_BAD_INI_FILE)
        nPort = 6021;
    else
        nPort = BRDC_atoi(sPort);

    fprintf(islog, "IPC_openSocket(IPC_tcp)\n");
    SComm=IPC_openSocket(IPC_tcp);
    fprintf(islog, "IPC_openSocket(IPC_tcp) complete\n");

/*	if(SComm == INVALID_SOCKET)												// ����������� ������ � ���������
	{
		return SRV_ERROR;
    }*/
    anAddr.port = nPort;
	anAddr.addr.ip = htonl(INADDR_ANY);

	//anAddr = IPC_resolve( (BRDCHAR*)"127.0.0.1:6021" );

    fprintf(islog, "IPC_bind(SComm, &anAddr)\n");
    if( IPC_bind(SComm, &anAddr) == IPC_SOCKET_ERROR )
	{
#ifndef __linux__
		int err = WSAGetLastError();
#endif

        fprintf(islog, "IPC_bind(SComm, &anAddr) complete\n");
		return SRV_ERROR;
	}
    fprintf(islog, "IPC_bind(SComm, &anAddr) complete\n");

    fprintf(islog, "IPC_listen(SComm, 10 )\n");
    IPC_listen(SComm, 10 );
    fprintf(islog, "IPC_listen(SComm, 10 ) complete\n");

	return 0;
}

IPC_handle     aThreads[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};




int aFlags[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};



#define MAX_WAITTHREAD 500

thread_value __IPC_API ServerThread(void   *lpvThreadParm);

int ServerLoop( void )
{
    IPC_sockaddr _anAddr;
	// Clean up
	for( int i=0;i<32;i++)
	{
		if( aThreads[i] == 0 )
			continue;
        //fprintf(logfile, "IPC_waitThread(aThreads[i], 0 )\n");
        if( IPC_waitThread(aThreads[i], 10) == IPC_OK )
		{
            IPC_deleteThread( aThreads[i] );
			aThreads[i] = 0;
		}
        //fprintf(logfile, "IPC_waitThread(aThreads[i], 0 ) complete\n");
	}

	fd_set ReadSet;
    timeval tval={1,MAX_CMDTIMEOUT};//less timeout

    IPC_FD_ZERO( &ReadSet );
    IPC_FD_SET( SComm,&ReadSet );

    //fprintf(logfile, "IPC_select(SComm,&ReadSet,0,0,&tval)\n");
	int nErr=IPC_select(SComm,&ReadSet,0,0,&tval);
    if((nErr==IPC_SOCKET_ERROR)||(nErr==0))
	{
        //fprintf(logfile, "IPC_select(SComm,&ReadSet,0,0,&tval) timeout\n");
		return 0;
	}
    fprintf(islog, "IPC_select(SComm,&ReadSet,0,0,&tval) complete\n");

    //fprintf(logfile, "IPC_accept(SComm, &_anAddr, 0 )\n");
    IPC_handle SData = IPC_accept(SComm, &_anAddr, 0 );
    fprintf(islog, "IPC_accept(SComm, &_anAddr, 0 ) complete\n");
	int yes = 1;
	if( SData )
	{
        IPC_setsockopt(SData, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(yes));

		int slot = -1;
		for( int i=0;i<32;i++)
		{
			if( aThreads[i] == 0 )
			{
				slot = i;
				break;
			}
		}

        if(slot != -1) {

            IPC_handle  hThread = 0;
            SERVER_THREAD_PARAM *param = (SERVER_THREAD_PARAM *)malloc(sizeof(SERVER_THREAD_PARAM));

            IPC_interlockedIncrement( &g_clCnt );

            param->SData = SData;
            param->id = slot;
            BRDC_fprintf(islog, _BRDC("%s(%p) - started. Slot = %d\n"), __FUNCTION__, param, slot);
            hThread = IPC_createThread(_BRDC("ServerThread"), ServerThread, param);
            if(hThread) {
                aThreads[slot] = hThread;
            }
        }
    }

	return 0;
}

int DestroyServer( void )
{
    IPC_shutdown(SComm,IPC_SD_BOTH);
    IPC_closeSocket(SComm);

	for( int i=0;i<32;i++)
	{
		if( aThreads[i] == 0 )
			continue;

  /*      if( IPC_waitThread( aThreads[i], MAX_WAITTHREAD ) == WAIT_TIMEOUT )
            TerminateThread( aThreads[i], 0 );*/

        IPC_deleteThread( aThreads[i] );
	}

	return 0;
}
