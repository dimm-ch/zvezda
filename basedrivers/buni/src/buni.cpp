/********************************************************************
 *  (C) Copyright 1992-2005 InSys Corp.  All rights reserved.
 *
 *  ======== b101E.cpp ========
 *  Driver for:
 *		ADP101E
 *
 *  version 1.0
 *
 ********************************************************************/
 
// ----- include files -------------------------
#include	<malloc.h>
#include	<stdio.h>
//#include	<conio.h>
#include	<string.h>
#include	<ctype.h>
//#include	<dos.h>
#include	<time.h>
//#include	<windows.h>
//#include	<winioctl.h>

//#define	DEV_API		DllExport
//#define	DEVS_API	DllExport

#define	DEV_API_EXPORTS
#define	DEVS_API_EXPORTS

#include	"utypes.h"
#include	"brdapi.h"
#include	"ctpapi.h"
#include	"dbprintf.h"
#include	"buni.h"

//
//======= Макроопределения
//

#define	SIGNATURESIZE	5
#define	SIGREGISTSIZE	32

#define	MEMBLOCK_PAGE	4
#define	MEMBLOCK_SIZE	MEMBLOCK_PAGE*4096

#define DEF_FOTRPORT 0

//
//====  Константы
//

//
//==== Типы
//

//
//==== Глобальные переменные
//
S32	windowsVersion = -1;

//
//==== Объявления функций
//

#ifndef __linux__
//=********************** DllMain ***********************
//=******************************************************
BOOL __stdcall DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	switch( fdwReason )
	{
		case DLL_PROCESS_ATTACH:
			break;

		case DLL_PROCESS_DETACH:
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;
	}

	return TRUE;
}


//=********************** DllEntryPoint *****************
//=******************************************************
BOOL __stdcall DllEntryPoint(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   // Included for compatibility with Borland 
	return DllMain (hinstDLL, fdwReason, lpvReserved);
}
#endif

//----------------- Internal Functions ---------------------------------------

//=******************** GetWinVersion ******************
// Функция возвращает код типа и версии Windows 
//=*****************************************************
// return value (decimal):
//	95	-	Windows 95
//	98	-	Windows 98
//	1000-	Windows ME
//	351	-	Windows NT 3.51
//	400	-	Windows NT 4.0
//	2000-	Windows 2000
//	0x314-	Windows XP
//	-1	-	unknow
S32	GetWinVersion(void)
{
#ifndef __linux__
ULONG   ver;

OSVERSIONINFOEX osVersion;

        osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        GetVersionEx((OSVERSIONINFO *) &osVersion);

        if(osVersion.dwMajorVersion == 4) 
		{
            switch(osVersion.dwMinorVersion) 
			{
             case 0:
                if(osVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
                        ver = 95;                // WINDOWS 95
                else if(osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT)
                        ver = 400;               // WINDOWS NT4.0
                break;
             case 10: ver = 98; break;           // WINDOWS 98
             case 90: ver = 1000; break;         // WINDOWS ME
             default: ver = -1; break;           // unknow
            }
        } 
		else if(osVersion.dwMajorVersion == 3) 
		{
            switch(osVersion.dwMinorVersion) 
			{
             case 51: ver = 351; break;          // WINDOWS NT 3.51
             default: ver = -1;  break;          // unknow
            }
        } 
		else if(osVersion.dwMajorVersion == 5) 
		{
            switch(osVersion.dwMinorVersion) 
			{
             case 0:  ver = 2000; break;          // WINDOWS 2000
             case 1:  ver = 0x314; break;         // WINDOWS XP
             default: ver = -1; break;            // unknow
            }
        }
		else
		{
			ver = 10;//WIN10
		}

        return ver;
#endif

    return -1;
}

//=********************** Dev_ErrorPrintf ****************
//=*******************************************************
S32	 Dev_ErrorPrintf( S32 errorCode, void *ptr, BRDCHAR *title, BRDCHAR *format, ... )
{
    va_list		arglist;

	if(ptr && errorCode < 0)
	{
		BRDDRV_Board		*pDev = (BRDDRV_Board*)ptr;
	
		pDev->errcnt++;
		pDev->errcode = errorCode;
	}

	va_start( arglist, format );		 
	BRDI_printf( 1<<EXTERRLVL(errorCode), title, format, arglist );
	va_end( arglist );

	return errorCode;
}

//=********************** Dev_Printf *********************
//=*******************************************************
void	Dev_Printf( S32 errLevel, BRDCHAR *title, BRDCHAR *format, ... )
{
	va_list		arglist;

	va_start( arglist, format );		 
	BRDI_printf( errLevel, title, format, arglist );
	va_end( arglist );
}



//=********************** Dev_ParseBoard *****************
//  Функция открывает сетевое устройство, 
//  выделяет и заполняет структуру параметров устройства.
//  Функция вызывается из Dev_initData()
//=*******************************************************
BRDDRV_Board*	 Dev_CreateBoard(TIniData *pIniData, S32 idxThisBrd, TDict *dict = NULL )
{
	//
	//=== Ищем в системе устройство
	//

	CTP_HOST *pCTPHost= CTP_Connect( pIniData->host, pIniData->ipport,0 );

	if( pCTPHost == 0 ) 
	{
		Dev_Printf( DRVERR(BRDerr_NO_KERNEL_SUPPORT),  CURRFILE ,
			_BRDC("INFO: Can't open FOTR device\n") );

		return NULL;
	}

	//
	//=== Allocate Device Extension
	//
	BRDDRV_Board		*pDev = NULL;

	windowsVersion = GetWinVersion();

	//
	// Allocate Device Extension
	// 

	//Check board existance
	TCTP_GetInfo sGetInfo;
	if( CTP_GetInfo( pCTPHost, pIniData->brdId, &sGetInfo ) != 0 )
    {
		CTP_Disconnect( pCTPHost );
		
		return NULL;
	}	
	
	pDev = new BUNI2Driver();//(BRDDRV_Board*)HAlloc(sizeof(BRDDRV_Board));

	pDev->errcode = 0;
	pDev->errcnt = 0;
	pDev->brdId = pIniData->brdId;
	
	pDev->pid    = sGetInfo.pid;
	pDev->BOARD = sGetInfo.boardType;

	BRDC_strcpy(  pDev->ipAddr, pIniData->host );
	pDev->ipPort = pIniData->ipport;

#ifdef _WIN64
	BRDCHAR ctpname[64];

	mbstowcs(ctpname, sGetInfo.name, 64 );
#else
	BRDCHAR *ctpname = sGetInfo.name;
#endif

	BRDC_sprintf( pDev->boardName, _BRDC("BUNI2%s"), ctpname);

	pDev->pCTPHost = pCTPHost;
	pDev->fillServList( dict );
	
	//*ppDev = pDev;      
	//sprintf( pBoardName, "BUNI_%d%d", pDev->pid, pDev->brdId );
	
	pDev->pCTPHost = NULL;
	CTP_Disconnect( pCTPHost );

	//pDev->pCTPHost = pCTPHost;

	return pDev;
}

//=********************** Dev_FreeBoard ******************
//=*******************************************************
S32	 Dev_FreeBoard( BRDDRV_Board *pDev )
{
	try
	{
		if (pDev != NULL)
		{
			//
			// TODO: Clear Resources
			//

			//HFree( pDev );
			delete pDev;
		}
	}
	catch(...)
	{
		pDev = NULL;
	}


	return DRVERR(BRDerr_OK);
}

//=******************************************************
//
// DRIVER EXPORTED FUNCTIONS
//
//=******************************************************

//=************************ DEV_initData ****************
//  Функция производит разбор параметров, полученных из файла brd.ini
//  Return:  if OK returns boardType, else >0.
//  Эта функция вызывается из BRD_initData() (brd.dll)
//*******************************************************
DEV_API	S32	 STDCALL DEV_initData(S32 idxThisBrd, void **ppDev, BRDCHAR *pBoardName, TBRD_InitData *pInitData, S32 size)
{
	TIniData			iniData;		// Data from Registry or INI File
	TDict dict( pInitData, size );

	//
	//=== Attach Board and Create DEV Driver Extension
	// 
	
	iniData.host = dict.getString( _BRDC("host") );//keyValString[0];

	if( iniData.host == NULL )
		iniData.host = dict.getString(_BRDC("ipaddr"), _BRDC("127.0.0.1") );//keyValString[0];

	iniData.ipport = dict.getInt(_BRDC("ipport"), 6021 );//atoi( keyValString[1] );
	iniData.brdId = dict.getInt(_BRDC("brdid"), idxThisBrd);//atoi( keyValString[2] );

	BRDDRV_Board *pDev = Dev_CreateBoard( &iniData, idxThisBrd, &dict );

	if( pDev == NULL )
	{
		return DRVERR(BRDerr_ERROR);
	}

	BRDC_sprintf( pBoardName, _BRDC("%s_%d%d"), pDev->boardName, pDev->pid, pDev->brdId );



	*ppDev = pDev;

	return btB101;
}

//=******************** DEV_initAuto **************
// Автоматический поиск и инициализация устройства
//=*****************************************************
DEV_API	S32		STDCALL DEV_initAuto( S32 idxThisBrd, void **ppDev, BRDCHAR *pBoardName, BRDCHAR *pLin, S32 linSize, 
									 TBRD_SubEnum *pSubEnum, U32 sizeSubEnum )
{
	return -1;
}

//=************************ DEV_entry ******************
// Точка входа всех функций CMD_
//=*****************************************************
DEV_API S32     STDCALL DEV_entry(void *pDev, S32 cmd, void *pParam)
{
	BUNI2Driver *bd = (BUNI2Driver*) pDev ;

	return bd->entry( cmd, pParam );
}

DEVS_API S32     STDCALL DEVS_entry(void *pDev, void *pContext, S32 cmd, void *pArgs )
{
	BUNI2Driver *bd = (BUNI2Driver*) pDev ;

	return bd->sentry( cmd, pArgs );
}


//
// End of file
//
