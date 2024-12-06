// main.cpp : Defines the entry point for the console application.
//

#include "winnet.h"

#include "_brd.h"
#include "utypes.h"
#include "gipcy.h"

int g_IsServerRunnning = 0;

HINSTANCE g_hInstance = 0;

FILE* islog = NULL;

/*#ifdef WIN32
void ResetCurrentDirectory( void )
{
 TCHAR exepath[64];
 TCHAR *c;

 GetModuleFileName( g_hInstance,exepath,64 );

 c=_tcsrchr( exepath, '\\' );

 if( *c )
  *c=0;

 SetCurrentDirectory( exepath );

}
#endif*/

/*#ifdef WIN32__
int WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_hInstance = hInstance;
#else*/
int BRDC_main(int argc, BRDCHAR *argv[])
{
//#ifdef __linux__
    U08 nStartCnt = 0;
//#endif // __linux__

    IPC_initKeyboard();

//#endif // WIN32

	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'l')
	{
		if(argv[1][2]=='a')
			islog = BRDC_fopen(_BRDC("islog.txt"), _BRDC("a"));
		else
			islog = BRDC_fopen(_BRDC("islog.txt"), _BRDC("w"));

		setbuf(islog, NULL);
	}
	else
	{
		islog = stdout;
	}



    BRDC_fprintf(islog, _BRDC("BRD_init() - "));

	BRD_displayMode(BRDdm_UNVISIBLE);

    S32 n = 0;
    S32 res = BRD_init( NULL,&n);
    if(res < 0) {
        IPC_cleanupKeyboard();
		BRDC_fprintf(islog, _BRDC(" error\n"));
        return -1;
    }
	BRDC_fprintf(islog, _BRDC(" complete\n"), n );
/*#ifdef WIN32
	HANDLE hThread = GetCurrentThread();
	int nRet = SetThreadPriority( hThread, THREAD_PRIORITY_BELOW_NORMAL	);

	if( CreateIcon() == SRV_ERROR )
		return 1; // FAILED Create icon in tray
#endif*/ // WIN32
    //fprintf(logfile, "CreateServer()\n");
    if( CreateServer() == SRV_ERROR ) {
        BRD_cleanup();
        IPC_cleanupKeyboard();
        return -1; // FAILED Create server tcp-ip session
    }
    //fprintf(logfile, "CreateServer() complete\n");
	g_IsServerRunnning = 1;

	BRDC_fprintf(islog, _BRDC("Start server\n"));

    while( g_IsServerRunnning )
	{
        IPC_delay( 10 );

//#ifdef WIN32
//		IconMessageLoop( );
//#endif // WIN32

//#ifdef __linux__
        if(IPC_kbhit())
            if(IPC_getch() == 0x1B)
                break;

        if((g_clCnt > 0) != nStartCnt)
        {
            nStartCnt = g_clCnt > 0;

            if(nStartCnt)
                printf("g_clCnt = %ld\n", g_clCnt);
            else
                printf("g_clCnt = 0\n");
        }
//#endif // __linux__
        //fprintf(logfile, "ServerLoop()\n");
		ServerLoop( );
        //fprintf(logfile, "ServerLoop() complete\n");
	}
    fprintf(islog, "DestroyServer()\n");
	DestroyServer();
    fprintf(islog, "DestroyServer() complete\n");
//#ifdef WIN32
//	DestroyIcon();
//#endif // WIN32

    _BRD_cleanup();
    BRD_cleanup();

    IPC_cleanupKeyboard();

    printf("Stop server\n");

	return 0;
}
