/***************************************************
*
* BRD.CPP
*
* BRD Shell functions definition.
*
* (C) Instr.Sys. by Ekkore Dec, 1998-2001
*
****************************************************/

#include	<malloc.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>

#ifdef _WIN32
	#define	BRD_API		DllExport
	#define	BRDI_API	DllExport
#endif

#include	"utypes.h"
#include	"brdi.h"
#include	"brderr.h"
#include	"extn.h"
#include	"idxbrd.h"
//
//====== Types
//

//
//====== Global Variables
//

#define	CURRFILE	_BRDC("BRD")
#define	VER_MAJOR	0x00010000L
#define	VER_MINOR	0x00010000L

TBRD_Board				boardArr[BRD_COUNT] {};	// boardArr has BRD_COUNT entries
static volatile int		g_initCounter=0;			// Counter of calling to BRD_init()/BRD_autoinit()
//static CRITICAL_SECTION	g_initCritSection;			// Serialize _init() and _cleanup() operation
//static CRITICAL_SECTION	g_openCritSection;			// Protect "openCnt" member of struct SBRD_Board
static IPC_tls_key		g_dwTlsIndex = 0;				// TLS area to keep Error Informaton
static IPC_handle		g_hInitMutex = 0;				// Mutex for Multythread Init Operation
static volatile int		g_isProcessDetach = 0;		// 1 - if DLL_PROCESS_DETACH occured

TBRD_Board				g_reinitBoardArr[BRD_COUNT] {};		// Need to call _reinit().
static BRDCHAR			g_reinitIniFileName[MAX_PATH*2] = _BRDC("");	// Fill with _initEx(). Need to call _reinit().
static U32				g_reinitMode = 0;						// Fill with _initEx(). Need to call _reinit().
//
//======= Macros
//


//
//====== Functions Declaration
//

//
//====== Functions Definition
//

//=********************** DllMain ***********************
//=******************************************************
#ifdef _WIN32
BOOL __stdcall DllMain( HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved )
//BOOL APIENTRY DllMain(HANDLE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) // WinCE
{
	// hinstDll = hinstDll;
	// lpvReserved =lpvReserved;
	switch( fdwReason )
	{
		case DLL_PROCESS_ATTACH:
			//InitializeCriticalSection( &g_initCritSection );
//			InitializeCriticalSection( &g_openCritSection );
			g_dwTlsIndex = IPC_createTlsKey();
			g_hInitMutex = IPC_createMutex( _BRDC("Brd_Shell_Init_Mutex"), FALSE );
			break;
		case DLL_PROCESS_DETACH:
			Brd_ProcessDetach();
			if( g_hInitMutex ) IPC_deleteMutex( g_hInitMutex );
			IPC_deleteTlsKey( g_dwTlsIndex );
			//DeleteCriticalSection( &g_initCritSection );
//			DeleteCriticalSection( &g_openCritSection );
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

//=**************** Brd_ProcessDetach *******************
//=******************************************************
void Brd_ProcessDetach( void )
{
	int		idx;

	g_isProcessDetach = 1;

	if( g_initCounter <= 0 )
		return;

	//
	//=== Clear Board Shared Info
	//
//	for( idx=0; idx<BRD_COUNT; idx++ )
//	{
//		LONG openCnt = InterlockedExchange( &boardArr[idx].openCnt, 0 );
//		if( 0<openCnt )		InterlockedExchangeAdd( &boardArr[idx].pBSI->openCnt, -openCnt );
//		if( 0>openCnt )		InterlockedExchange( &boardArr[idx].pBSI->openCnt, 0 );
//	}

	//
	// Clean Up Drivers
	//
	for( idx=0; idx<BRD_COUNT; idx++ )
	{	if( boardArr[idx].pDev != NULL )
		{
			Brd_CleanupDevice( boardArr+idx );
		}
	}
}
#endif

S32 BRD_initEntry( TBRD_Board &boardArr, S32 *ptrNum)
{
	BRDCHAR	szName[64];

	(*ptrNum)++;

	//
	// Create Mutex, File Mapping, Shareable Info: hLockMutex, hFileMap, pBSI
	//
	BRDC_strcpy(szName, boardArr.boardName);
	BRDC_strcat(szName, _BRDC("_LockMutex"));
	boardArr.hLockMutex = IPC_createMutex(szName, FALSE);
	if (boardArr.hLockMutex == 0)
		Brd_ErrorPrintf(BRDERR(BRDerr_INSUFFICIENT_RESOURCES), CURRFILE, _BRDC("<BRD_init> Can't create Lock Mutex \"%s\""), szName);

	BRDC_strcpy(szName, boardArr.boardName);
	BRDC_strcat(szName, _BRDC("_OpenMutex"));
	boardArr.hOpenMutex = IPC_createMutex(szName, FALSE);
	if (boardArr.hOpenMutex == 0)
		Brd_ErrorPrintf(BRDERR(BRDerr_INSUFFICIENT_RESOURCES), CURRFILE, _BRDC("<BRD_init> Can't create Open Mutex \"%s\""), szName);

	BRDC_strcpy(szName, boardArr.boardName);
	BRDC_strcat(szName, _BRDC("_CaptMutex"));
	boardArr.hCaptMutex = IPC_createMutex(szName, FALSE);
	if (boardArr.hCaptMutex == 0)
		Brd_ErrorPrintf(BRDERR(BRDerr_INSUFFICIENT_RESOURCES), CURRFILE, _BRDC("<BRD_init> Can't create Open Mutex \"%s\""), szName);

	BRDC_strcpy(szName, boardArr.boardName);
	BRDC_strcat(szName, _BRDC("_FileMap"));
	boardArr.hFileMap = IPC_createSharedMemory(szName, sizeof(*boardArr.pBSI));
	if (boardArr.hFileMap == 0)
		Brd_ErrorPrintf(BRDERR(BRDerr_INSUFFICIENT_RESOURCES), CURRFILE, _BRDC("<BRD_init> Can't create File Mapping \"%s\""), szName);

	boardArr.pBSI = (struct SBRD_BoardSharedInfo *)IPC_mapSharedMemory(boardArr.hFileMap);
	if (boardArr.pBSI == NULL)
		Brd_ErrorPrintf(BRDERR(BRDerr_INSUFFICIENT_RESOURCES), CURRFILE, _BRDC("<BRD_init> Can't create Shared Info \"%s\""), szName);

	return 0;
}

//=********************** BRD_init **********************
//=******************************************************
BRD_API S32 STDCALL	BRD_init( const BRDCHAR *iniFile, S32 *pNum )
{
	return BRD_initEx( (iniFile) ? BRDinit_FILE : BRDinit_FILE_KNOWN, iniFile, NULL, pNum );
}

//=********************** BRD_initEx **********************
//=******************************************************
BRD_API S32 STDCALL	BRD_initEx( U32 mode, const void *pSrc, const BRDCHAR *logFile, S32 *pNum )
{
	int			idx;
	S32			ret;
	S32			num;
	S32			*ptrNum = (pNum) ? pNum : &num;

	//EnterCriticalSection( &g_initCritSection );
#ifdef __linux__
#ifdef _INSYS_IPC_
        int status = IPC_init();
        if(status != IPC_OK) {
            return Brd_ErrorPrintf( BRDERR(BRDerr_NONE_DEVICE), CURRFILE, _BRDC("<BRD_initEx> Can't init IPC library. Possibly IPC driver not loaded!"));
        }
#endif
	if(!g_dwTlsIndex)
		g_dwTlsIndex = IPC_createTlsKey();
	if( !g_hInitMutex )
		g_hInitMutex = IPC_createMutex( _BRDC("Brd_Shell_Init_Mutex"), FALSE );
#endif // __linux__
	if( g_hInitMutex ) IPC_captureMutex( g_hInitMutex, INFINITE );
	*ptrNum = 0;

	//int cnt =0;
	//
	// Second call of BRD_init()
	//
	if( g_initCounter>0 )
	{
		g_initCounter++;
		for( idx=0; idx<BRD_COUNT; idx++ )
		{	if( boardArr[idx].pDev != NULL )
			{
				(*ptrNum)++;
			}
		}

		if( g_hInitMutex )
		{
			IPC_releaseMutex( g_hInitMutex );
#ifdef __linux__
			IPC_deleteMutex( g_hInitMutex );
            g_hInitMutex = 0;
#endif // __linux__
		}
		//LeaveCriticalSection( &g_initCritSection );
		return BRDerr_INIT_REPEATEDLY;
	}

	//
	// First call of BRD_init()
	//
	for( idx=0; idx<BRD_COUNT; idx++ )
	{
		boardArr[idx].boardType = btNONE;
		boardArr[idx].boardLid = (U32)-1;
		boardArr[idx].openCnt = 0;
		boardArr[idx].cleanupCnt = 0;
		boardArr[idx].workCnt = 0;
		boardArr[idx].boardName[0] = '\0';
		boardArr[idx].pDev = NULL;
		boardArr[idx].hLib = NULL;
		boardArr[idx].hLockMutex = 0;
		boardArr[idx].hFileMap = 0;
		boardArr[idx].pBSI = NULL;
	}

	//
	//  Source of Records for BRD_init()
	//
	// mode=0:		Predefined INI-File
	// mode=1:		Global INI-File (Defined with EnvVariable BRDINI)
	// mode=2:		Specified INI-File
	// mode=3:		Registry
	// mode=4:		Formatted Line of Text
	//

	if( mode & BRDinit_AUTOINIT )
		ret = Brd_ParseAutoInit( mode, pSrc, logFile );
#ifdef _WIN32
	else if((mode&0xF)==BRDinit_REGISTRY)
		ret = Brd_ParseRegistry();
#endif
	else if((mode&0xF)==BRDinit_STRING)
	{
		ret = Brd_ParseBuffer( mode, (BRDCHAR *)pSrc );
	}
	else if ((mode & 0xF) == BRDinit_LAZY)
	{
		ret = 0;
	}
	else
	{
		ret = Brd_ParseIniFile( mode, (const BRDCHAR *)pSrc );
	}

	if( 0>ret )
	{
		if( g_hInitMutex )
		{
			IPC_releaseMutex( g_hInitMutex );
#ifdef __linux__
			IPC_deleteMutex( g_hInitMutex );
            g_hInitMutex = 0;
#endif // __linux__
		}
		//LeaveCriticalSection( &g_initCritSection );
		return ret;
	}

	//
	// Calculate All Initialized Device
	//
	for( idx=0; idx<BRD_COUNT; idx++ )
	{	if( boardArr[idx].pDev != NULL )
		{
			BRD_initEntry(boardArr[idx], ptrNum);
		}
	}

	if( *ptrNum==0 && (mode & 0xF) != BRDinit_LAZY)
	{
		if( g_hInitMutex )
		{
			IPC_releaseMutex( g_hInitMutex );
#ifdef __linux__
			IPC_deleteMutex( g_hInitMutex );
            g_hInitMutex = 0;
#endif // __linux__
		}
		//LeaveCriticalSection( &g_initCritSection );
		return Brd_ErrorPrintf( BRDERR(BRDerr_NONE_DEVICE), CURRFILE, _BRDC("<BRD_init> None initialized board") );
	}

	//
	// Increment Init Counter
	//
	g_initCounter++;

	//
	// Keep Reinit Params
	//
	if( pSrc )
		BRDC_strcpy( g_reinitIniFileName, (BRDCHAR*)pSrc );
	else
		BRDC_strcpy( g_reinitIniFileName, _BRDC("") );
	g_reinitMode = mode;

	//
	// Send DEVcmd_INIT to all Drivers
	//
	for( idx=0; idx<(*ptrNum); idx++ )
	{
		Brd_CallDriver( idx+1, DEVcmd_INIT, NULL, 0 );
	}


	if( g_hInitMutex )
	{
		IPC_releaseMutex( g_hInitMutex );
#ifdef __linux__
			IPC_deleteMutex( g_hInitMutex );
            g_hInitMutex = 0;
#endif // __linux__
	}
	//LeaveCriticalSection( &g_initCritSection );

	//
	// Prepare Resources
	//
	for( idx=0; idx<BRD_COUNT; idx++ )
	{	if( boardArr[idx].pDev != NULL )
		{
			Brd_Serv_CreateBoardList( idx );
			Brd_Pu_CreateBoardList( idx );
		}
	}

	return BRDERR(BRDerr_OK);
}

//=************************ BRD_reinit *****************
//=*****************************************************
BRD_API S32		STDCALL BRD_reinit( S32 *pNum )
{
	int			idx, idx2;
	S32			ret = BRDerr_OK;
	S32			num;

	S32	Brd_Reinit_ParseIniFile( U32 mode, const BRDCHAR *regFile, S32 nummax );

	//EnterCriticalSection( &g_initCritSection );
#ifdef __linux__
	if( !g_hInitMutex )
		g_hInitMutex = IPC_createMutex( _BRDC("Brd_Shell_Init_Mutex"), FALSE );
#endif // __linux__
	if( g_hInitMutex ) IPC_captureMutex( g_hInitMutex, INFINITE );

	//
	// Check Init Mode
	//
	if( (g_reinitMode & BRDinit_AUTOINIT) ||
		((g_reinitMode&0xF)==BRDinit_REGISTRY) )
	{
		if( g_hInitMutex )
		{
			IPC_releaseMutex( g_hInitMutex );
#ifdef __linux__
			IPC_deleteMutex( g_hInitMutex );
            g_hInitMutex = 0;
#endif // __linux__
		}
		//LeaveCriticalSection( &g_initCritSection );
		return BRDERR(BRDerr_BAD_MODE);
	}

	//
	// Calc the unused part of boardArr[]
	//
	num = 0;
	for( idx=0; idx<BRD_COUNT; idx++ )
		if( boardArr[idx].pDev != NULL )
			num++;

	//
	// Reinit
	//
	for( idx2=0; idx2<BRD_COUNT; idx2++ )
	{
		g_reinitBoardArr[idx2].boardType = btNONE;
		g_reinitBoardArr[idx2].boardLid = (U32)-1;
		g_reinitBoardArr[idx2].openCnt = 0;
		g_reinitBoardArr[idx2].cleanupCnt = 0;
		g_reinitBoardArr[idx2].workCnt = 0;
		g_reinitBoardArr[idx2].boardName[0] = '\0';
		g_reinitBoardArr[idx2].pDev = NULL;
		g_reinitBoardArr[idx2].hLib = NULL;
		g_reinitBoardArr[idx2].hLockMutex = 0;
		g_reinitBoardArr[idx2].hFileMap = 0;
		g_reinitBoardArr[idx2].pBSI = NULL;
	}
	ret = Brd_Reinit_ParseIniFile( g_reinitMode, g_reinitIniFileName, BRD_COUNT-num );

	if( 0>ret )
	{
		if( g_hInitMutex )
		{
			IPC_releaseMutex( g_hInitMutex );
#ifdef __linux__
			IPC_deleteMutex( g_hInitMutex );
            g_hInitMutex = 0;
#endif // __linux__
		}
		//LeaveCriticalSection( &g_initCritSection );
		return ret;
	}

///////////////////////////////////////////////////////
	//
	// Calculate All Initialized Device
	//
	num = 0;
	for( idx2=0; idx2<BRD_COUNT; idx2++ )
	{
		if( g_reinitBoardArr[idx2].pDev != NULL )
		{
			BRDCHAR	szName[64];

			//
			// Search free item of boardArr[]
			//
			for( idx=0; idx<BRD_COUNT; idx++ )
			{
				if( boardArr[idx].pDev == NULL )
					break;
			}
			if( idx >= BRD_COUNT )
				break;

			//
			// Move Item from g_reinitBoardArr[] to boardArr[]
			//
			num++;
			boardArr[idx].boardType = g_reinitBoardArr[idx2].boardType;
			boardArr[idx].boardLid  = g_reinitBoardArr[idx2].boardLid;
			BRDC_strncpy( boardArr[idx].boardName, g_reinitBoardArr[idx2].boardName, 32 );
			boardArr[idx].pEntry    = g_reinitBoardArr[idx2].pEntry;
			boardArr[idx].pDev      = g_reinitBoardArr[idx2].pDev;
			boardArr[idx].hLib      = g_reinitBoardArr[idx2].hLib;

			//
			// Create Mutex, File Mapping, Shareable Info: hLockMutex, hFileMap, pBSI
			//
			BRDC_strcpy( szName, boardArr[idx].boardName );
			BRDC_strcat( szName, _BRDC("_LockMutex"));
			boardArr[idx].hLockMutex = IPC_createMutex(szName, FALSE);
			if( boardArr[idx].hLockMutex == 0 )
				Brd_ErrorPrintf( BRDERR(BRDerr_INSUFFICIENT_RESOURCES), CURRFILE, _BRDC("<BRD_init> Can't create Lock Mutex \"%s\""), szName );

			BRDC_strcpy( szName, boardArr[idx].boardName );
			BRDC_strcat( szName, _BRDC("_OpenMutex"));
			boardArr[idx].hOpenMutex = IPC_createMutex(szName, FALSE);
			if( boardArr[idx].hOpenMutex == 0 )
				Brd_ErrorPrintf( BRDERR(BRDerr_INSUFFICIENT_RESOURCES), CURRFILE, _BRDC("<BRD_init> Can't create Open Mutex \"%s\""), szName );

			BRDC_strcpy( szName, boardArr[idx].boardName );
			BRDC_strcat( szName, _BRDC("_CaptMutex"));
			boardArr[idx].hCaptMutex = IPC_createMutex(szName, FALSE);
			if( boardArr[idx].hCaptMutex == 0 )
				Brd_ErrorPrintf( BRDERR(BRDerr_INSUFFICIENT_RESOURCES), CURRFILE, _BRDC("<BRD_init> Can't create Open Mutex \"%s\""), szName );

			BRDC_strcpy( szName, boardArr[idx].boardName );
			BRDC_strcat( szName, _BRDC("_FileMap"));
			boardArr[idx].hFileMap = IPC_createSharedMemory( szName, sizeof(*boardArr[idx].pBSI) );
			if( boardArr[idx].hFileMap == 0 )
				Brd_ErrorPrintf( BRDERR(BRDerr_INSUFFICIENT_RESOURCES), CURRFILE, _BRDC("<BRD_init> Can't create File Mapping \"%s\""), szName );

			boardArr[idx].pBSI = (struct SBRD_BoardSharedInfo *)IPC_mapSharedMemory( boardArr[idx].hFileMap );
			if( boardArr[idx].pBSI == NULL )
				Brd_ErrorPrintf( BRDERR(BRDerr_INSUFFICIENT_RESOURCES), CURRFILE, _BRDC("<BRD_init> Can't create Shared Info \"%s\""), szName );

			//
			// Send DEVcmd_INIT to Driver
			//
			Brd_CallDriver( idx+1, DEVcmd_INIT, NULL, 0 );

			//
			// Prepare Resources
			//
			Brd_Serv_CreateBoardList( idx );
			Brd_Pu_CreateBoardList( idx );
		}
	}

	if( pNum )
		*pNum = num;

	if( g_hInitMutex )
	{
		IPC_releaseMutex( g_hInitMutex );
#ifdef __linux__
		IPC_deleteMutex( g_hInitMutex );
        g_hInitMutex = 0;
#endif // __linux__
	}
	//LeaveCriticalSection( &g_initCritSection );
	return BRDERR(BRDerr_OK);
}


//=************************ BRD_cleanup ****************
//=*****************************************************
BRD_API	S32 STDCALL		BRD_cleanup( void )
{
	int			idx;

	//EnterCriticalSection( &g_initCritSection );

	//
	// Decrement InitCounter
	//
	g_initCounter--;
	if( g_initCounter < 0 )
	{
		g_initCounter = 0;
		//LeaveCriticalSection( &g_initCritSection );
		return BRDERR(BRDerr_OK);
	}


	if( g_initCounter>0 )	// Don't Clean Up because of this Application just uses BRD.DLL
	{
		//LeaveCriticalSection( &g_initCritSection );
		return BRDERR(BRDerr_SHELL_IN_USE);
	}

	//
	// Clean Up Drivers
	//
	for( idx=0; idx<BRD_COUNT; idx++ )
	{	if( boardArr[idx].pDev != NULL )
		{
			Brd_CleanupDevice( boardArr+idx );
		}
	}

#ifdef __linux__
	if(g_dwTlsIndex)
		IPC_deleteTlsKey( g_dwTlsIndex );
#ifdef _INSYS_IPC_
	IPC_cleanup();
#endif
#endif // __linux__
	//LeaveCriticalSection( &g_initCritSection );
	return BRDERR(BRDerr_OK);
}

//=********************** BRD_shell *********************
//=******************************************************
BRD_API S32		STDCALL BRD_shell( U32 cmd, void *arg )
{
	switch( cmd )
	{
		case BRDshl_LID_LIST:
		{
			BRD_LidList		*pLidList = (BRD_LidList*)arg;
			U32				ii, item = 0;

			for( ii=0; ii<BRD_COUNT; ii++ )
			{
				if( boardArr[ii].pDev != NULL )
				{
					if( item<pLidList->item )
						pLidList->pLID[item] = boardArr[ii].boardLid;
					item++;
				}
			}
			pLidList->itemReal = item;
			break;
		}
		default:
			return BRDERR(BRDerr_CMD_UNSUPPORTED);
	}
	return BRDERR(BRDerr_OK);
}

//=********************** BRD_lidCtrl *******************
//=******************************************************
BRD_API S32		STDCALL BRD_lidCtrl( U32 lid, U32 nodeId, U32 cmd, void *arg )
{
	struct { U32 nodeId; U32 cmd; void *arg; } param;
	int			idx;
	BRDCHAR		lin[50];

	if( g_initCounter==0 )
	{
		return Brd_ErrorPrintf( BRDERR(BRDerr_SHELL_UNINITIALIZED), CURRFILE, _BRDC("<BRD_lidCtrl> BRD is not initialized") );
	}

	//
	// Search Board with 'id'
	//

	for( idx=0; idx<BRD_COUNT; idx++ )
		if( (boardArr[idx].boardLid == lid) && (boardArr[idx].pDev != NULL ) )
			break;
	if( idx>=BRD_COUNT )
	{	BRDC_sprintf( lin, _BRDC("<BRD_lidCtrl> Wrong Board LID:%02d"), lid);
		return Brd_ErrorPrintf( BRDERR(BRDerr_BAD_LID), CURRFILE, lin );
	}

	//
	// Prepare Param
	//
	param.nodeId = nodeId;
	param.cmd = cmd;
	param.arg = arg;
	return Brd_CallDriver( idx+1, DEVcmd_CTRL, &param, 0 );
}

//=********************** BRD_lidList *******************
//=******************************************************
BRD_API S32		STDCALL BRD_lidList( U32 *pList, U32 item, U32 *pItemReal )
{
	U32				ii, idxItem = 0;

	for( ii=0; ii<BRD_COUNT; ii++ )
	{
		if( boardArr[ii].pDev != NULL )
		{
			if( idxItem < item )
				pList[idxItem] = boardArr[ii].boardLid;
			idxItem++;
		}
	}
	*pItemReal = idxItem;

	return BRDERR(BRDerr_OK);
}

//=********************** BRD_open **********************
//=******************************************************


BRD_API BRD_Handle	STDCALL BRD_openReal(U32 lid, U32 flag, void *ptr);

BRD_API	BRD_Handle STDCALL 	BRD_open(U32 lid, U32 flag, void *ptr)
{
	return BRD_openReal( lid, flag, ptr );
}

S32		Brd_InitData(int idxBoard, int boardLid, TBRD_InitData *pInitData, S32 initDataSize);

extern CIdxBrdCollection			g_rIdxBrdCollection;

S32		_Brd_xgets(BRDCHAR* &pp, BRDCHAR *line)
{
	if (*pp == 0)
		return -1;

	BRDCHAR *end = BRDC_strstr(pp, _BRDC("\n"));

	if (end == NULL)
		end = pp + BRDC_strlen(pp);

	BRDCHAR *dst = line;

	while (pp<end)
	{
		if (*pp == '/' || *pp == '\\' )
			break;

		if (*pp == '(' || *pp == ')' )
				break;

		if (*pp == ',')
			break;

		*dst = *pp;

		dst++;
		pp++;
	}


	*dst = 0;

	return 0;
}

void	Brd_initIniFile_ParseLine(BRDCHAR *pSrcLin, BRDCHAR **ppKeyLin, BRDCHAR **ppValLin, S32 *pIsEqual);

BRD_API BRD_Handle	STDCALL BRD_openEx(const BRDCHAR *init, U32 flag, void *ptr)
{
	int lid = -1;
	int idx;

	if (g_hInitMutex) IPC_captureMutex(g_hInitMutex, INFINITE);

	for (idx = 0; idx < BRD_COUNT; idx++)
		if (boardArr[idx].pDev == NULL)
		{
			lid = idx + 1;

			break;
		}



	if (lid == -1)
	{
		if (g_hInitMutex) IPC_releaseMutex(g_hInitMutex);

		return Brd_ErrorPrintf(BRDERR(BRDerr_BAD_LID), CURRFILE, _BRDC("<BRD_open> Insufusiend lids"));
	}

	S32 num;

	int size = 16;
	TBRD_InitData *pInitData = (TBRD_InitData*)calloc(size, sizeof(TBRD_InitData));


	BRDCHAR		line[201];

	BRDCHAR * pSrc = (BRDCHAR*)init;
	_Brd_xgets(pSrc, line );

	int nnkeyval = 0;

	BRDC_strcpy(pInitData[nnkeyval].key, _BRDC("type"));
	BRDC_strcpy(pInitData[nnkeyval].val, line);
	nnkeyval++;

	TBRD_InitData *pPid = &pInitData[nnkeyval];

	BRDC_strcpy(pInitData[nnkeyval].key, _BRDC("pid"));
	BRDC_strcpy(pInitData[nnkeyval].val, _BRDC("0") );
	nnkeyval++;

	int			isEqual;
	BRDCHAR		*pKeyLin, *pValLin;

	if (*pSrc == '(')
	{

		do
		{
			pSrc++;
			_Brd_xgets(pSrc, line);

			Brd_initIniFile_ParseLine(line, &pKeyLin, &pValLin, &isEqual);

			if (*pKeyLin == '[')		// Found new record
				break;
			if (*pKeyLin == '\0')
				continue;

			if (BRDC_strcmp(pKeyLin, pPid->key) == 0)		// Found new record
			{
				BRDC_strncpy(pPid->val, pValLin, sizeof(pInitData[0].val) / sizeof(pInitData[0].val[0])); //strncpy
				continue;
			}

			BRDC_strncpy(pInitData[nnkeyval].key, pKeyLin, sizeof(pInitData[0].key) / sizeof(pInitData[0].key[0])); //strncpy
			BRDC_strncpy(pInitData[nnkeyval].val, pValLin, sizeof(pInitData[0].val) / sizeof(pInitData[0].val[0])); //strncpy

			nnkeyval++;

		} while (*pSrc == ',');


		if (*pSrc == ')')
			pSrc++;
	}

    const BRDCHAR * subtypes[] = { _BRDC("type"), _BRDC("btype0") , _BRDC("btype1")  };
	int cntsubtypes = sizeof(subtypes) / sizeof(subtypes[0]);
	int nnsubtypes = 0;

	if (*pSrc == '/' || *pSrc == '\\')
	{
		BRDC_strcpy(pInitData[nnkeyval].key, _BRDC("#begin"));
		BRDC_strcpy(pInitData[nnkeyval].val, _BRDC("SUBUNIT"));
		nnkeyval++;
		do
		{
			pSrc++;
			_Brd_xgets(pSrc, line);

			BRDC_strcpy(pInitData[nnkeyval].key, subtypes[nnsubtypes++] );
			BRDC_strcpy(pInitData[nnkeyval].val, line);
			nnkeyval++;

		} while (*pSrc == ',' && nnsubtypes<cntsubtypes);

		BRDC_strcpy(pInitData[nnkeyval].key, _BRDC("#end"));
		nnkeyval++;
	}

	S32 err = Brd_InitData(idx, lid, pInitData, nnkeyval);

	if (err >= 0)
	{

		BRD_initEntry(boardArr[idx], &num);

		Brd_CallDriver(idx + 1, DEVcmd_INIT, NULL, 0);

		free(pInitData);

		Brd_Serv_CreateBoardList(idx);
		Brd_Pu_CreateBoardList(idx);
	}

	if (g_hInitMutex) IPC_releaseMutex(g_hInitMutex);

	return BRD_openReal(lid, flag, ptr);
}

BRD_API BRD_Handle	STDCALL BRD_openReal(U32 lid, U32 flag, void *ptr)
{
	struct { U32 flag; void *ptr; S32 isFirstOpen; } param;
	int			ret;
	int			idx;
	BRD_Handle	handle;
	BRDCHAR		lin[50];

	if( g_initCounter==0 )
	{
		return Brd_ErrorPrintf( BRDERR(BRDerr_SHELL_UNINITIALIZED), CURRFILE, _BRDC("<BRD_open> BRD is not initialized") );
	}

	if( ((flag&3) != BRDopen_EXCLUSIVE) &&
		((flag&3) != BRDopen_SHARED) &&
		((flag&3) != BRDopen_SPY) )
	{
		return Brd_ErrorPrintf( BRDERR(BRDerr_BAD_MODE), CURRFILE, _BRDC("<BRD_open> Wrong Open Mode") );
	}

	//
	// Search Board with LID
	//

	for( idx=0; idx<BRD_COUNT; idx++ )
		if( (boardArr[idx].boardLid == lid) && (boardArr[idx].pDev != NULL ) )
			break;
	if( idx>=BRD_COUNT )
	{	BRDC_sprintf( lin, _BRDC("<BRD_open> Wrong Board LID:%02d"), lid);
		return Brd_ErrorPrintf( BRDERR(BRDerr_BAD_LID), CURRFILE, lin );
	}

	//
	// Protect Open/Close for Multytreaded Applications
	//

	if( boardArr[idx].hOpenMutex )
		IPC_captureMutex( boardArr[idx].hOpenMutex, INFINITE );

	//
	// Analise Open Mode and Open Cnt
	//
	long	oldOpenCnt = 0;

	if( (flag&3) == BRDopen_SHARED )  // If you Try to Open SHARED Device
	{
		oldOpenCnt = IPC_interlockedIncrement( &boardArr[idx].pBSI->openCnt );
		oldOpenCnt--;				// Get Initial state of "openCnt"
		if( 0>oldOpenCnt )								// Already Opened as EXCLUSIVE
		{
			IPC_interlockedDecrement( &boardArr[idx].pBSI->openCnt );

			if (ptr == NULL)
			{
				ret = BRDerr_BAD_MODE;
				goto errorOpen;
			}

			//if( boardArr[idx].hOpenMutex ) ReleaseMutex( boardArr[idx].hOpenMutex );
			//return Brd_ErrorPrintf( BRDERR(BRDerr_ALREADY_OPENED), CURRFILE,
			//			"<BRD_open> Board with LID:%02d is already opened as exclusive", lid );
			flag &= ~3;
			flag |= BRDopen_SPY;
			Brd_ErrorPrintf( BRDERR(BRDerr_INFO), CURRFILE,
						_BRDC("<BRD_open> Board with LID:%02d is already opened as exclusive"), lid );
		}
		else
		{
			IPC_interlockedIncrement( &boardArr[idx].openCnt );
		}
	}
	else if( (flag&3) == BRDopen_EXCLUSIVE )	// If you Try to Open EXCLUSIVE Device
	{
//#if _MSC_VER < 1300
//		if( 0!=InterlockedCompareExchange( (PVOID*)&boardArr[idx].pBSI->openCnt, (PVOID*)-60000, (PVOID*)0 ) )
//#else
		if( 0!=IPC_interlockedCompareExchange( &boardArr[idx].pBSI->openCnt,   (long)-60000,   (long)0 ) )
//#endif
		{
			if (ptr == NULL)
			{
				ret = BRDerr_BAD_MODE;
				goto errorOpen;
			}

			//if( boardArr[idx].hOpenMutex ) ReleaseMutex( boardArr[idx].hOpenMutex );
			//return Brd_ErrorPrintf( BRDERR(BRDerr_ALREADY_OPENED), CURRFILE,
			//			"<BRD_open> Board with LID:%02d is already opened as shared", lid );
			flag &= ~3;
			flag |= BRDopen_SPY;
			Brd_ErrorPrintf( BRDERR(BRDerr_INFO), CURRFILE,
						_BRDC("<BRD_open> Board with LID:%02d is already opened as shared or exclusive"), lid );
		}
		else
		{
			IPC_interlockedExchange( &boardArr[idx].openCnt, -60000 );
		}
	}

	if( (flag&3) == BRDopen_SPY )	// If you Try to Open SPY Device
	{
		IPC_interlockedExchange( &oldOpenCnt, boardArr[idx].pBSI->openCnt );
	}

	//
	// Form Handle
	//
	handle = idx+1;
	if( (flag&3) == BRDopen_SPY )
		handle |= BRD_HANDLE_SPY;


	//
	// Call Driver Command DEVcmd_DEVHARDWAREINIT
	// if
	//    1) it is the 1st Device Open
	//    2) Open Mode is SPY and Device wasn't Opened before
	//
	ret=0;
	if( oldOpenCnt==0 )
	{
		DEV_CMD_DevHardwareInit	sDHI = {(U32)NODEALL};
		ret = Brd_CallDriver( handle, DEVcmd_DEVHARDWAREINIT, &sDHI, 0 );
	}

	//
	// Call Driver Command DEVcmd_OPEN
	//
	if( ret>=0 )
	if( (flag&3) != BRDopen_SPY )
	{
		param.flag = flag;
		param.ptr  = ptr;
		param.isFirstOpen = ( oldOpenCnt>0 ) ? 0 : 1;
		ret = Brd_CallDriver( handle, DEVcmd_OPEN, &param, 0 );
	}

	if( ret<0 )
	{

		// Restore Open Cnt
		if( (flag&3) == BRDopen_SHARED )			// If you Try to Open SHARED Device
		{	IPC_interlockedDecrement( &boardArr[idx].openCnt );
			IPC_interlockedDecrement( &boardArr[idx].pBSI->openCnt );
		}else if( (flag&3) == BRDopen_EXCLUSIVE )	// If you Try to Open EXCLUSIVE Device
		{	IPC_interlockedExchange( &boardArr[idx].openCnt, 0 );
			IPC_interlockedExchange( &boardArr[idx].pBSI->openCnt, 0 );
		}

	errorOpen:

		if( boardArr[idx].hOpenMutex ) IPC_releaseMutex( boardArr[idx].hOpenMutex );
		return ret;
	}

	//
	// Release Protection for Open/Close
	//

	if( boardArr[idx].hOpenMutex ) IPC_releaseMutex( boardArr[idx].hOpenMutex );

	//
	// Increment Open Counter
	//

//	EnterCriticalSection( &g_openCritSection );
//	boardArr[idx].openCnt++;
//	LeaveCriticalSection( &g_openCritSection );

	//
	// Return
	//
	if( ptr )
		*(U32*)ptr = flag&3;

	return handle;
}


//=********************** BRD_close **********************
//=*******************************************************
BRD_API	S32 STDCALL	BRD_close( BRD_Handle handle )
{
	TBRD_Board *pBoard=NULL;
	S32			ret;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_OK;

	if( 0 > Brd_GetPtrBoard(  handle, &pBoard, 1 ) )
		pBoard=NULL;

	//
	// Protect Open/Close for Multytreaded Applications
	//

	if( pBoard )
	if( pBoard->hOpenMutex )
		IPC_captureMutex( pBoard->hOpenMutex, INFINITE );

	ret = Brd_CallDriver( handle, DEVcmd_CLOSE, NULL, 1 );

	//
	// Release Protection for Open/Close
	//

	if( pBoard )
	if( pBoard->hOpenMutex )
		IPC_releaseMutex( pBoard->hOpenMutex );

	return ret;
}


//=********************** BRD_getInfo **********************
//=******************************************************

S32		STDCALL BRD_getInfo(BRD_Handle handle, BRD_Info *pInfo)
{
	return BRD_getInfoEx(handle,NULL,pInfo);
}

BRD_API S32		STDCALL BRD_getInfoEx(BRD_Handle handle, U32 *lid, BRD_Info *pInfo)
{
	TBRD_Board	*pBoard;
	S32			err;

	err = Brd_GetPtrBoard(handle, &pBoard, 1);
	if (err<0)
		return err;

	if (lid)
		*lid = pBoard->boardLid;


	return BRD_getInfo(pBoard->boardLid, pInfo);
}

BRD_API S32		STDCALL BRD_getInfo( U32 lid, BRD_Info *pInfo )
{
	struct { BRD_Info *pInfo; } param;
	int			idx;
	BRDCHAR		lin[50];

	if( g_initCounter==0 )
	{
		return Brd_ErrorPrintf( BRDERR(BRDerr_SHELL_UNINITIALIZED), CURRFILE, _BRDC("<BRD_getinfo> BRD is not initialized") );
	}

	//
	// Search Board with 'id'
	//

	for( idx=0; idx<BRD_COUNT; idx++ )
		if( (boardArr[idx].boardLid == lid) && (boardArr[idx].pDev != NULL ) )
			break;
	if( idx>=BRD_COUNT )
	{	BRDC_sprintf( lin, _BRDC("<BRD_getinfo> Wrong Board LID:%02d"), lid);
		return Brd_ErrorPrintf( BRDERR(BRDerr_BAD_LID), CURRFILE, lin );
	}

	//
	// Clear *pInfo
	//
	U32		keeSize = pInfo->size;
	memset( pInfo, 0, keeSize );
	pInfo->size = keeSize;

	//
	// Get Info From Driver
	//
	param.pInfo = pInfo;
	return Brd_CallDriver( idx+1, DEVcmd_GETINFO, &param, 0 );
}

//=********************** BRD_load **********************
//=******************************************************
BRD_API	S32 STDCALL	BRD_load( BRD_Handle handle, U32 nodeId, const BRDCHAR *fileName, int argc, BRDCHAR *argv[] )
{
	DEV_CMD_Load param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	param.fileName = fileName;
	param.argc = argc;
	param.argv = argv;
	return Brd_CallDriver( handle, DEVcmd_LOAD, &param, 1 );
}


//=********************** BRD_puLoad ********************
//=******************************************************
BRD_API S32		STDCALL BRD_puLoad( BRD_Handle handle, U32 puId, const BRDCHAR *fileName, U32 *state )
{
	DEV_CMD_PuLoad	param;
	U32				aState;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	//
	// Check Write Enable
	//
	TBRD_Board	*pBoard;
	S32			err;

	err = Brd_GetPtrBoard( handle, &pBoard, 1 );
	if( err<0 )
		return err;

	if( 0== Brd_Pu_IsWriteEnable( pBoard, puId ) )
		return BRDerr_BAD_ID;

	//
	// Do Operation
	//
	param.puId = puId;
	param.fileName = fileName;
	param.state = (state) ? state : &aState;
	return Brd_CallDriver( handle, DEVcmd_PULOAD, &param, 1 );
}

//=********************** BRD_puState *******************
//=******************************************************
BRD_API S32		STDCALL BRD_puState( BRD_Handle handle, U32 puId, U32 *state )
{
	DEV_CMD_PuState param;

	param.puId  = puId;
	param.state = state;
	return Brd_CallDriver( handle, DEVcmd_PUSTATE, &param, 1 );
}

//=********************** BRD_puList ********************
//=******************************************************
BRD_API S32		STDCALL BRD_puList( BRD_Handle handle, BRD_PuList *pList, U32 item, U32 *pItemReal )
{
	DEV_CMD_PuList param;

	param.pList = pList;
	param.item  = item;
	param.pItemReal = pItemReal;
	return Brd_CallDriver( handle, DEVcmd_PULIST, &param, 1 );
}

//=************************ BRD_puRead ********************
//  Read array from Programable Unit to Host. Size unit is bytes.
//=********************************************************
BRD_API S32 STDCALL	BRD_puRead( BRD_Handle handle, U32 puId,
								U32 offset, void *hostAdr, U32 size )
{
	DEV_CMD_PuRead param;

	param.puId = puId;
	param.offset = offset;
	param.pBuf = hostAdr;
	param.size = size;
	return Brd_CallDriver( handle, DEVcmd_PUREAD, &param, 1 );
}


//=************************ BRD_puWrite *******************
//  Write array from Host to Programable Unit. Size unit is bytes.
//=********************************************************
BRD_API S32 STDCALL	BRD_puWrite( BRD_Handle handle, U32 puId,
								 U32 offset, void *hostAdr, U32 size )
{
	DEV_CMD_PuWrite param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	//
	// Check Write Enable
	//
	TBRD_Board	*pBoard;
	S32			err;

	err = Brd_GetPtrBoard( handle, &pBoard, 1 );
	if( err<0 )
		return err;

	if( 0== Brd_Pu_IsWriteEnable( pBoard, puId ) )
		return BRDerr_BAD_ID;

	//
	// Do Operation
	//
	param.puId = puId;
	param.offset = offset;
	param.pBuf = hostAdr;
	param.size = size;
	return Brd_CallDriver( handle, DEVcmd_PUWRITE, &param, 1 );
}


//=************************ BRD_puEnable ******************
//  Enable Write/Load Operation for the Programable Unit.
//=********************************************************
BRD_API S32 STDCALL	BRD_puEnable( BRD_Handle handle, U32 puId )
{
	TBRD_Board	*pBoard;
	S32			err;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	//
	// Get Device Object
	//
	err = Brd_GetPtrBoard( handle, &pBoard, 1 );
	if( err<0 )
		return err;

	//
	// Set Write Enable for the PU
	//
	err = Brd_Pu_SetWriteEnable( pBoard, puId );
	if( err<0 )
		return BRDerr_BAD_ID;
	return BRDerr_OK;
}


//=********************** BRD_reset *********************
//=******************************************************
BRD_API S32 STDCALL	BRD_reset( BRD_Handle handle, U32 nodeId )
{
	struct { U32 nodeId; } param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	return Brd_CallDriver( handle, DEVcmd_RESET, &param, 1 );
}


//=********************** BRD_start *********************
//=******************************************************
BRD_API S32 STDCALL	BRD_start( BRD_Handle handle, U32 nodeId )
{
	struct { U32 nodeId; } param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	return Brd_CallDriver( handle, DEVcmd_START, &param, 1 );
}


//=********************** BRD_stop **********************
//=******************************************************
BRD_API S32 STDCALL	BRD_stop( BRD_Handle handle, U32 nodeId )
{
	struct { U32 nodeId; } param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	return Brd_CallDriver( handle, DEVcmd_STOP, &param, 1 );
}


//=************************ BRD_symbol *****************
//=*****************************************************
BRD_API S32 STDCALL	BRD_symbol( BRD_Handle handle, const BRDCHAR *fileName,
								const BRDCHAR *symbName, U32 *val )
{
	struct { const BRDCHAR *fileName; const BRDCHAR *symbName; U32 *val; } param;

	param.fileName = fileName;
	param.symbName = symbName;
	param.val = val;
	return Brd_CallDriver( handle, DEVcmd_SYMBOL, &param, 1 );
}


//=************************ BRD_version ****************
//=*****************************************************
BRD_API S32 STDCALL	BRD_version( BRD_Handle handle, BRD_Version *pVersion )
{
	struct { BRD_Version *pVersion; } param;

	if( pVersion->size < sizeof(BRD_Version) )
		return BRDERR(BRDerr_BUFFER_TOO_SMALL);

	pVersion->brdMajor = VER_MAJOR;
	pVersion->brdMinor = VER_MINOR;

	param.pVersion = pVersion;
	if( 0>Brd_CallDriver( handle, DEVcmd_VERSION, &param, 1 ))
	{
		pVersion->drvMajor = VER_NONE;
		pVersion->drvMinor = VER_NONE;
	}

    return BRDERR(BRDerr_OK);
}


//=********************** BRD_capture *******************
//=******************************************************
BRD_API BRD_Handle STDCALL BRD_capture(BRD_Handle handle, U32 nodeId, U32 *pMode, const BRDCHAR *resName, U32 timeout )
{
	BRD_Handle		resHandle;
	DEV_CMD_Capture	paramCapt;
	DEV_CMD_Ctrl	paramCtrl;
	S32				err;

	if( handle & BRD_HANDLE_SPY )
		*pMode = BRDcapt_SPY;

	err = Brd_Serv_Capture( handle, &resHandle, nodeId, pMode, resName, timeout );
	if (err < 0) return err;

	//
	// Send DEVcmd_CAPTURE to Service
	//
	paramCapt.handle = resHandle;
	paramCapt.nodeId = nodeId;
	Brd_CallDriver( handle, DEVcmd_CAPTURE, &paramCapt, 1 );

	//
	// Send DEVcmd_CTRL + SERVcmd_SYS_CAPTURE to Service
	//
	paramCtrl.handle = resHandle;
	paramCtrl.nodeId = nodeId;
	paramCtrl.cmd = SERVcmd_SYS_CAPTURE;
	paramCtrl.arg = NULL;
	err = Brd_CallDriver( handle, DEVcmd_CTRL, &paramCtrl, 1 );
	if (err < 0) {
		return err;
	}

	return resHandle;
}

//=********************** BRD_release *******************
//=******************************************************
BRD_API S32		STDCALL BRD_release(BRD_Handle handle, U32 nodeId )
{
	DEV_CMD_Release	paramRel;
	DEV_CMD_Ctrl	paramCtrl;

	//
	// Send DEVcmd_CTRL + SERVcmd_SYS_RELEASE to Service
	//
	paramCtrl.handle = handle;
	paramCtrl.nodeId = nodeId;
	paramCtrl.cmd = SERVcmd_SYS_RELEASE;
	paramCtrl.arg = NULL;
	Brd_CallDriver( handle, DEVcmd_CTRL, &paramCtrl, 1 );

	//
	// Send DEVcmd_RELEASE to Service
	//
	paramRel.handle = handle;
	paramRel.nodeId = nodeId;
	Brd_CallDriver( handle, DEVcmd_RELEASE, &paramRel, 1 );

	if( 0>Brd_Serv_Release( handle, nodeId ) )
		return BRDERR(BRDerr_ERROR);

    return BRDERR(BRDerr_OK);
}

//=********************** BRD_serviceList ***************
//=******************************************************
BRD_API S32		STDCALL BRD_serviceList(BRD_Handle handle, U32 nodeId, BRD_ServList *pList, U32 item, U32 *pItemReal )
{
	U32		itemReal;

	return Brd_Serv_List( handle, nodeId, pList, item, (pItemReal)?pItemReal:&itemReal );
}

//=********************** BRD_ctrl **********************
//=******************************************************
BRD_API S32 STDCALL	BRD_ctrl( BRD_Handle handle, U32 nodeId, U32 cmd, void *arg )
{
	DEV_CMD_Ctrl param;
	//struct { S32 handle; U32 nodeId; U32 cmd; void *arg; } param;
	S32			err;

	err = Brd_Serv_Check( handle, nodeId );
	if( 0>err )
		return err;

	param.handle = handle;
	param.nodeId = nodeId;
	param.cmd = cmd;
	param.arg = arg;
	return Brd_CallDriver( handle, DEVcmd_CTRL, &param, 1 );
}


//=********************** BRD_extension *****************
//=******************************************************
BRD_API S32 STDCALL	BRD_extension( BRD_Handle handle, U32 nodeId, U32 cmd, void *arg )
{
	DEV_CMD_Extension param;
	//struct { U32 nodeId; U32 cmd; void *arg; } param;

	if( cmd == BRDextn_TURN_DEASY )
	{
	//#ifndef NOT_DEASY_SUPPORT
	//	extern S32				TurnDeasy( BRD_Handle handle, U32 nodeId, BRDextn_TurnDeasy *arg );
	//	BRDextn_TurnDeasy		*prTurnDeasy = (BRDextn_TurnDeasy*)arg;
	//
	//	return TurnDeasy( handle, nodeId, prTurnDeasy );
	//#endif
	}

	param.nodeId = nodeId;
	param.cmd = cmd;
	param.arg = arg;
	return Brd_CallDriver( handle, DEVcmd_EXTENSION, &param, 1 );
}

//=********************** BRD_peek **********************
//=******************************************************
BRD_API U32 STDCALL	BRD_peek( BRD_Handle handle, U32 nodeId, U32 brdAdr )
{
	struct { U32 nodeId; U32 brdAdr; U32 val; } param;

	param.nodeId = nodeId;
	param.brdAdr = brdAdr;
	if( 0>Brd_CallDriver( handle, DEVcmd_PEEK, &param, 1 ))
		param.val=0xFFFFFFFF;

	return param.val;
}


//=********************** BRD_poke ***********************
//=*******************************************************
BRD_API S32 STDCALL	BRD_poke( BRD_Handle handle, U32 nodeId, U32 brdAdr, U32 val )
{
	struct { U32 nodeId; U32 brdAdr; U32 val; } param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	param.brdAdr = brdAdr;
	param.val = val;
	return Brd_CallDriver( handle, DEVcmd_POKE, &param, 1 );
}



//=********************** BRD_readRAM **********************
// Read array from Board to Host. Size unit is byte.
//*******************************************************
BRD_API S32 STDCALL	BRD_readRAM( BRD_Handle handle, U32 nodeId,
								 U32 brdAdr, void *hostAdr, U32 itemNum, U32 itemSize )
{
	struct { U32 nodeId; U32 brdAdr; void *hostAdr; U32 itemNum; U32 itemSize; } param;

	param.nodeId = nodeId;
	param.brdAdr = brdAdr;
	param.hostAdr = hostAdr;
	param.itemNum = itemNum;
	param.itemSize = itemSize;
	return Brd_CallDriver( handle, DEVcmd_READRAM, &param, 1 );
}


//=********************** BRD_writeRAM *********************
// Write array from Host to Board. Size bytes.
//=******************************************************
BRD_API S32 STDCALL	BRD_writeRAM( BRD_Handle handle, U32 nodeId,
								  U32 brdAdr, void *hostAdr, U32 itemNum, U32 itemSize )
{
	struct { U32 nodeId; U32 brdAdr; void *hostAdr; U32 itemNum; U32 itemSize; } param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	param.brdAdr = brdAdr;
	param.hostAdr = hostAdr;
	param.itemNum = itemNum;
	param.itemSize = itemSize;
	return Brd_CallDriver( handle, DEVcmd_WRITERAM, &param, 1 );
}


//=************************ BRD_readFIFO ***************
// Read array from Board FIFO to Host. Size unit is bytes.
//=*****************************************************
BRD_API S32 STDCALL	BRD_readFIFO( BRD_Handle handle, U32 nodeId,
								  U32 brdAdr, void *hostAdr, U32 size, U32 timeout )
{
	struct { U32 nodeId; U32 brdAdr; void *hostAdr; U32 size; U32 timeout; } param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	param.brdAdr = brdAdr;
	param.hostAdr = hostAdr;
	param.size = size;
	param.timeout = timeout;
	return Brd_CallDriver( handle, DEVcmd_READFIFO, &param, 1 );
}


//=************************ BRD_writeFIFO **************
// Write array from Host to Board FIFO. Size unit is bytes.
//=*****************************************************
BRD_API S32 STDCALL	BRD_writeFIFO( BRD_Handle handle, U32 nodeId,
								   U32 brdAdr, void *hostAdr, U32 size, U32 timeout )
{
	struct { U32 nodeId; U32 brdAdr; void *hostAdr; U32 size; U32 timeout; } param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	param.brdAdr = brdAdr;
	param.hostAdr = hostAdr;
	param.size = size;
	param.timeout = timeout;
	return Brd_CallDriver( handle, DEVcmd_WRITEFIFO, &param, 1 );
}


//=************************ BRD_readDPRAM *****************
//  Read array from Board DPRAM to Host. Size unit is bytes.
//=********************************************************
BRD_API S32 STDCALL	BRD_readDPRAM( BRD_Handle handle, U32 nodeId,
								   U32 offset, void *hostAdr, U32 size )
{
	DEV_CMD_ReadDPRAM		 param;

	param.nodeId = nodeId;
	param.offset = offset;
	param.hostAdr = hostAdr;
	param.size = size;
	return Brd_CallDriver( handle, DEVcmd_READDPRAM, &param, 1 );
}


//=************************ BRD_writeDPRAM ****************
//  Write array from Host to Board DPRAM. Size unit is bytes.
//=********************************************************
BRD_API S32 STDCALL	BRD_writeDPRAM( BRD_Handle handle, U32 nodeId,
									U32 offset, void *hostAdr, U32 size )
{
	DEV_CMD_WriteDPRAM		 param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	param.offset = offset;
	param.hostAdr = hostAdr;
	param.size = size;
	return Brd_CallDriver( handle, DEVcmd_WRITEDPRAM, &param, 1 );
}


//=********************** BRD_read *********************
//  Read Msg Buffer from Board to Host. Size bytes.
//=*******************************************************
BRD_API S32 STDCALL		BRD_read( BRD_Handle handle, U32 nodeId,
									void *hostAdr, U32 size, U32 timeout )
{
	DEV_CMD_ReadWrite param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	param.hostAdr = hostAdr;
	param.size = size;
	param.timeout = timeout;
	return Brd_CallDriver( handle, DEVcmd_READ, &param, 1 );
}


//=********************** BRD_write *********************
//  Write Msg Buffer from Host to Board. Size bytes.
//=*******************************************************
BRD_API S32 STDCALL	BRD_write( BRD_Handle handle, U32 nodeId,
								void *hostAdr, U32 size, U32 timeout )
{
	DEV_CMD_ReadWrite param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	param.hostAdr = hostAdr;
	param.size = size;
	param.timeout = timeout;
	return Brd_CallDriver( handle, DEVcmd_WRITE, &param, 1 );
}


//=********************** BRD_getMsg *********************
//=*******************************************************
BRD_API S32 STDCALL BRD_getMsg(BRD_Handle handle, U32 nodeId, void *hostAdr, U32 *pBytes, U32 timeout )
{
	DEV_CMD_GetMsg	param;
	S32				err;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId  = nodeId;
	param.hostAdr = hostAdr;
	param.size    = *pBytes;
	param.timeout = timeout;

	err = Brd_CallDriver( handle, DEVcmd_GETMSG, &param, 1 );

	*pBytes = param.size;

	return err;
}

//=********************** BRD_putMsg *********************
//=*******************************************************
BRD_API S32 STDCALL BRD_putMsg(BRD_Handle handle, U32 nodeId, void *hostAdr, U32 *pBytes, U32 timeout )
{
	DEV_CMD_PutMsg	param;
	S32				err;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId  = nodeId;
	param.hostAdr = hostAdr;
	param.size    = *pBytes;
	param.timeout = timeout;

	err = Brd_CallDriver( handle, DEVcmd_PUTMSG, &param, 1 );

	*pBytes = param.size;

	return err;
}

//=********************** BRD_signalSend *********************
//=***********************************************************
BRD_API S32		STDCALL BRD_signalSend(BRD_Handle handle, U32 nodeId, U32 sigId )
{
	DEV_CMD_SignalSend param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	param.sigId  = sigId;
	return Brd_CallDriver( handle, DEVcmd_SIGNALSEND, &param, 1 );
}


//=********************** BRD_signalGrab *********************
//=***********************************************************
BRD_API S32		STDCALL BRD_signalGrab(BRD_Handle handle, U32 nodeId, U32 sigId, U32 *pSigCounter, U32 timeout )
{
	DEV_CMD_SignalWait	param;
	S32					ret;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	param.sigId  = sigId;
	param.sigCounter = 0;
	param.timeout = timeout;

	ret = Brd_CallDriver( handle, DEVcmd_SIGNALGRAB, &param, 1 );
	if( pSigCounter )
		*pSigCounter = param.sigCounter;

	return ret;
}


//=********************** BRD_signalWait *********************
//=***********************************************************
BRD_API S32		STDCALL BRD_signalWait(BRD_Handle handle, U32 nodeId, U32 sigId, U32 *pSigCounter, U32 timeout )
{
	DEV_CMD_SignalGrab	param;
	S32					ret;

	param.nodeId = nodeId;
	param.sigId  = sigId;
	param.sigCounter = 0;
	param.timeout = timeout;

	ret = Brd_CallDriver( handle, DEVcmd_SIGNALWAIT, &param, 1 );
	if( pSigCounter )
		*pSigCounter = param.sigCounter;

	return ret;
}


//=********************** BRD_signalIack *********************
//=***********************************************************
BRD_API S32		STDCALL BRD_signalIack(BRD_Handle handle, U32 nodeId, U32 sigId )
{
	DEV_CMD_SignalIack param;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	param.sigId  = sigId;
	return Brd_CallDriver( handle, DEVcmd_SIGNALIACK, &param, 1 );
}


//=********************** BRD_signalFresh *********************
//=***********************************************************
BRD_API S32		STDCALL BRD_signalFresh(BRD_Handle handle, U32 nodeId, U32 sigId, U32 *pSigCounter )
{
	DEV_CMD_SignalFresh	param;
	S32					ret;

	if( handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_MODE;

	param.nodeId = nodeId;
	param.sigId  = sigId;
	param.sigCounter = 0;

	ret = Brd_CallDriver( handle, DEVcmd_SIGNALFRESH, &param, 1 );
	if( pSigCounter )
		*pSigCounter = param.sigCounter;

	return ret;
}


//=********************** BRD_signalInfo *********************
//=***********************************************************
BRD_API S32		STDCALL BRD_signalInfo(BRD_Handle handle, U32 nodeId, U32 sigId, BRD_SigInfo *pInfo )
{
	DEV_CMD_SignalInfo param;

	param.nodeId = nodeId;
	param.sigId  = sigId;
	param.pInfo  = pInfo;
	return Brd_CallDriver( handle, DEVcmd_SIGNALINFO, &param, 1 );
}


//=********************** BRD_signalList *********************
//=***********************************************************
BRD_API S32		STDCALL BRD_signalList(BRD_Handle handle, U32 nodeId, BRD_SigList *pList, U32 item, U32 *pItemReal )
{
	DEV_CMD_SignalList param;

	param.nodeId = nodeId;
	param.pList  = pList;
	param.item  = item;
	param.pItemReal  = pItemReal;
	return Brd_CallDriver( handle, DEVcmd_SIGNALLIST, &param, 1 );
}


//=********************** BRD_lock *****************
//=*******************************************************
BRD_API S32		STDCALL BRD_lock(BRD_Handle handle, U32 nodeId, const BRDCHAR * resName, U32 timeout )
{
	// resName = resName;
	// nodeId = nodeId;

	TBRD_Board	*pBoard;
	S32			err;
	DWORD		ret;

	//
	// Get Device Object
	//
	err = Brd_GetPtrBoard( handle, &pBoard, 1 );
	if( err<0 )
		return err;

	//
	// Lock Mutex
	//
	if( pBoard->hLockMutex == NULL )
		return BRDERR(BRDerr_INSUFFICIENT_RESOURCES);
	ret = IPC_captureMutex( pBoard->hLockMutex, timeout );
	if( ret == IPC_WAIT_TIMEOUT )
		return BRDERR(BRDerr_WAIT_TIMEOUT);
	if( ret == IPC_WAIT_ABANDONED )
		return BRDERR(BRDerr_WAIT_ABANDONED);
	return BRDERR(BRDerr_OK);
}


//=********************** BRD_unlock *****************
//=*******************************************************
BRD_API S32		STDCALL BRD_unlock(BRD_Handle handle, U32 nodeId, const BRDCHAR * resName )
{
	// resName = resName;
	// nodeId = nodeId;

	TBRD_Board	*pBoard;
	S32			err;

	//
	// Get Device Object
	//
	err = Brd_GetPtrBoard( handle, &pBoard, 1 );
	if( err<0 )
		return err;

	//
	// Lock Mutex
	//
	if( pBoard->hLockMutex == NULL )
		return BRDERR(BRDerr_INSUFFICIENT_RESOURCES);
	IPC_releaseMutex(pBoard->hLockMutex);
	return BRDERR(BRDerr_OK);
}


//=********************** BRD_displayMode ****************
//=*******************************************************
BRD_API	void STDCALL		BRD_displayMode( S32 errLevel )
{
	DB_errdisplay( (errLevel<0) ? BRDdm_VISIBLE : errLevel );
}


//=********************** BRD_error *****************
//=*******************************************************
BRD_API S32	  	STDCALL BRD_error( BRD_Error **ppError )
{
	*ppError = (BRD_Error*)IPC_tlsGetValue( g_dwTlsIndex );
	return BRDERR(BRDerr_OK);
}

//=********************** BRDI_setError ******************
//=*******************************************************
BRDI_API S32  	STDCALL BRDI_setError( BRD_Error *pError )
{
	BRD_Error	*ptr;

	//
	// Get Area of Error Information
	//
	ptr = (BRD_Error*)IPC_tlsGetValue( g_dwTlsIndex );
	if( ptr == NULL )
	{
		IPC_tlsSetValue(g_dwTlsIndex, malloc( sizeof(BRD_Error) ));
		ptr = (BRD_Error*)IPC_tlsGetValue( g_dwTlsIndex );
	}
	if( ptr == NULL )
	{
		return BRDERR(BRDerr_INSUFFICIENT_RESOURCES);
	}

	//
	// Fill Area of Error Information
	//
	if( pError == NULL )
	{
		memset( ptr, 0, sizeof(BRD_Error) );
	}
	else
	{
		memcpy( ptr, pError, sizeof(BRD_Error) );
	}

	return BRDERR(BRDerr_OK);
}

//=********************** Brd_GetPtrBoard *****************
//=*******************************************************
S32		Brd_GetPtrBoard( BRD_Handle handle, TBRD_Board **ppBoard, S32 isCheckOpen )
{
	S32	idxArr = (S32)(handle&0x7FFF);
	TBRD_Board	*pBoard;

	//
	// Check BRD Shell
	//
	if( g_initCounter<=0 )
	{
		return BRDERR(BRDerr_SHELL_UNINITIALIZED);
	}

	//
	// Get Device Pointer
	//
	idxArr--;
	pBoard = boardArr+idxArr;

	//
	// Check handle
	//
	if(idxArr<0 || idxArr>=BRD_COUNT )
		return BRDERR(BRDerr_BAD_HANDLE);			// Error: idx out of boardArr
	if( pBoard->pDev == NULL )
		return BRDERR(BRDerr_BAD_HANDLE);			// Error: idx of unexistant device

	if( (handle & BRD_HANDLE_SPY) == 0 )
	if( isCheckOpen )
		if( 0 == pBoard->openCnt )
			return BRDERR(BRDerr_CLOSED_HANDLE);	// Error: boardArr[idx] not opened

	*ppBoard = pBoard;
	return BRDERR(BRDerr_OK);
}

//=********************** Brd_CleanupDevice *****************
//=*******************************************************
S32		Brd_CleanupDevice( TBRD_Board *pBoard )
{
	//EnterCriticalSection( &g_initCritSection );

	//
	// If Device is already Cleanuped
	//
	if( pBoard->pDev == 0 )
	{
		//LeaveCriticalSection( &g_initCritSection );
		return BRDERR(BRDerr_OK);
	}

	//
	// Protect Work Function from Cleanup
	//
	IPC_interlockedIncrement(&pBoard->cleanupCnt);
	while( pBoard->workCnt != 0 )
	{
		if( g_isProcessDetach )
			break;
		IPC_delay(1);
	}

	//
	// Restore Open Cnt
	//
	{
		LONG openCnt = IPC_interlockedExchange( &pBoard->openCnt, 0 );
		if( 0<openCnt )		IPC_interlockedExchangeAdd( &pBoard->pBSI->openCnt, -openCnt );
		if( 0>openCnt )		IPC_interlockedExchange( &pBoard->pBSI->openCnt, 0 );
	}

	//
	// Clean Up
	//
	pBoard->pEntry( pBoard->pDev, DEVcmd_CLEANUP, NULL );
	Brd_Serv_DestroyBoardList( pBoard );
	Brd_Pu_DestroyBoardList( pBoard );
	if( pBoard->hLib != NULL )			IPC_closeLibrary(pBoard->hLib);
	if( pBoard->hLockMutex != NULL )	IPC_deleteMutex(pBoard->hLockMutex);
	if( pBoard->hOpenMutex != NULL )	IPC_deleteMutex(pBoard->hOpenMutex);
	if( pBoard->hCaptMutex != NULL )	IPC_deleteMutex(pBoard->hCaptMutex);
	if( pBoard->pBSI != NULL )			IPC_unmapSharedMemory(pBoard->hFileMap);
	if( pBoard->hFileMap != NULL )		IPC_deleteSharedMemory(pBoard->hFileMap);
	pBoard->boardType = btNONE;
	pBoard->boardLid = (U32)-1;
	pBoard->boardName[0] = '\0';
	pBoard->pDev = NULL;
	pBoard->hLib = NULL;
	pBoard->hLockMutex = NULL;

//	EnterCriticalSection( &g_openCritSection );
//	pBoard->openCnt = 0;
//	LeaveCriticalSection( &g_openCritSection );

	IPC_interlockedDecrement(&pBoard->cleanupCnt);

	//LeaveCriticalSection( &g_initCritSection );
	return BRDERR(BRDerr_OK);
}

//=********************** Brd_CallDriver *****************
//=*******************************************************
S32		Brd_CallDriver( BRD_Handle handle, S32 cmd, void *pParam, S32 isCheckOpen )
{
	TBRD_Board	*pBoard;
	S32			err;

	//
	// Get Device Object
	//
	err = Brd_GetPtrBoard( handle, &pBoard, isCheckOpen );
	if( err<0 )
		return err;

	//
	// Protect Work Function from Cleanup
	//
	IPC_interlockedIncrement(&pBoard->workCnt);
	if( pBoard->cleanupCnt > 0 )
	{
		IPC_interlockedDecrement(&pBoard->workCnt);
		return BRDERR(BRDerr_BAD_HANDLE);			// Error: device is just cleanuped
	}
	if( pBoard->pDev == NULL )
	{
		IPC_interlockedDecrement(&pBoard->workCnt);
		return BRDERR(BRDerr_BAD_HANDLE);			// Error: idx of unexistant device
	}

	//
	// Call Driver
	//
	err = pBoard->pEntry( pBoard->pDev, cmd, pParam );

	//
	// Free Protection
	//
	IPC_interlockedDecrement(&pBoard->workCnt);

	//
	// Decrement openCnt if Close Operation
	//
	if( cmd == DEVcmd_CLOSE )
	{
		if( 0<pBoard->openCnt )						// If Device was Opened as SHARED
		{	IPC_interlockedDecrement( &pBoard->openCnt );
			IPC_interlockedDecrement( &pBoard->pBSI->openCnt );
		}else if( 0>pBoard->openCnt )				// If Device was Opened as EXCLUSIVE
		{	IPC_interlockedExchange( &pBoard->openCnt, 0 );
			IPC_interlockedExchange( &pBoard->pBSI->openCnt, 0 );
		}

//		EnterCriticalSection( &g_openCritSection );
//		pBoard->openCnt--;
//		if( pBoard->openCnt < 0 )
//			pBoard->openCnt = 0;
//		LeaveCriticalSection( &g_openCritSection );
	}

	//
	// Cleanup Device Object if Device was Removed
	//
	if( err == (S32)BRDerr_DEVICE_LOST )
	{
		Brd_CleanupDevice( pBoard );
		return BRDerr_BAD_HANDLE;
	}

	return err;
}

//=********************** Brd_ErrorPrintf ****************
//=*******************************************************
S32	 Brd_ErrorPrintf( S32 errorCode, const BRDCHAR *title, const BRDCHAR *format, ... )
{
	va_list		arglist;

	va_start( arglist, format );
	BRDI_printf( 1<<EXTERRLVL(errorCode), title, format, arglist );
	va_end( arglist );

	return errorCode;
}

//=********************** Brd_Printf *********************
//=*******************************************************
void	Brd_Printf( S32 errLevel, const BRDCHAR *title, const BRDCHAR *format, ... )
{
	va_list		arglist;

	va_start( arglist, format );
	BRDI_printf( errLevel, title, format, arglist );
	va_end( arglist );
}

extern "C" {
BRDI_API int    STDCALL BRDI_feof( FILE *stream);
BRDI_API BRDCHAR*  STDCALL BRDI_fgets( BRDCHAR *string, int n, FILE *stream );
BRDI_API long   STDCALL BRDI_ftell( FILE *stream );
BRDI_API int    STDCALL BRDI_fseek( FILE *stream, long offset, int origin );
};

//=********************** BRDI_feof **********************
//=*******************************************************
BRDI_API int		STDCALL BRDI_feof(FILE *stream)
{
	return feof( stream );
}

//=********************** BRDI_fgets *********************
//=*******************************************************
BRDI_API BRDCHAR*	STDCALL BRDI_fgets( BRDCHAR *string, int n, FILE *stream )
{
	return BRDC_fgets( string, n, stream );
}

//=********************** BRDI_ftell *********************
//=*******************************************************
BRDI_API long	STDCALL BRDI_ftell( FILE *stream )
{
	return ftell( stream );
}

//=********************** BRDI_fseek *********************
//=*******************************************************
BRDI_API int	STDCALL BRDI_fseek( FILE *stream, long offset, int origin )
{
	return fseek( stream, offset, origin );
}

//=********************** BRDI_service *******************
//=*******************************************************
BRDI_API S32	STDCALL BRDI_service( S32 cmd, void *pParam )
{
	// pParam = pParam;
	// cmd = cmd;

	return 0;
}



//
//  End of File
//
