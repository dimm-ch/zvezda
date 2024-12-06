//=*******************************************************
//
// BRDREINIT.CPP
//
// BRD Initialization Helper function definitions.
//
// (C) Instr.Sys. by Ekkore 2009
//=*******************************************************


#include	<malloc.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<signal.h>

#ifdef _WIN32
#define	BRD_API		DllExport
#define	BRDI_API	DllExport
#endif

#include	"utypes.h"
#include	"brdi.h"
#include	"brderr.h"
#include	"idxbrd.h"
//#include	"dbprintf.h"

//
//====== Types 
//


//
//======= Macros
//

// INI File Comment Symbol
#define	COMMENT_SYMB		';'



//
//====== Global Variables 
//

#define	CURRFILE	_BRDC("BRD")
extern TBRD_Board			boardArr[BRD_COUNT];			// boardArr has BRD_COUNT entries
extern TBRD_Board			g_reinitBoardArr[BRD_COUNT];	// boardArr has BRD_COUNT entries
extern CIdxBrdCollection	g_rIdxBrdCollection;

//
//====== Functions Declaration
//
S32	Brd_Reinit_InitData( int idxBrd, int boardLid, TBRD_InitData *pInitData, S32 initDataSize );
S32	Brd_Reinit_ParseIniFile( U32 mode, const char *regFile, S32 nummax);

//
// BRDINIT.CPP
//
void	Brd_initIniFile_ParseLine( BRDCHAR *pSrcLin, BRDCHAR **ppKeyLin, BRDCHAR **ppValLin, S32 *pIsEqual );
FILE	*Brd_OpenIniFile( U32 mode, const void *pSrc );


S32	Brd_CallDriver( BRD_Handle handle, S32 cmd, void *pParam, S32 isCheckOpen );
S32	Brd_ErrorPrintf( S32 errorCode, BRDCHAR *title, BRDCHAR *format, ... );
void	Brd_Printf( S32 errLevel, BRDCHAR *title, BRDCHAR *format, ... );

IPC_handle _BRD_openLibraryEx(const IPC_str *baseName, unsigned param);

//=********************** Brd_Reinit_InitData ***********
//=******************************************************
S32		Brd_Reinit_InitData( int idxBoard, int boardLid, TBRD_InitData *pInitData, S32 initDataSize )
{
	S32						ii, jj;
	DEV_API_initData		*pDEV_initData;
	DEV_API_entry			*pEntry;
	IPC_handle				hLib;				// DLL header
	S32						boardType;
	void					*pDev;
	TBRD_Board				*pBoard;
	BRDCHAR					boardName[128];
	int						idxBrd;

	//
	// Search Driver name
	//
	for( ii=0; ii<initDataSize; ii++ )
	{
		//
		// Go through #begin/#end block 
		//
		
		if( !BRDC_stricmp( pInitData[ii].key, _BRDC("#begin") ) )	// Found keyword "#begin"
		{
			int beginCnt = 1;
			for(;;)
			{
				if( ++ii >= initDataSize )
					break;
				if( !BRDC_stricmp( pInitData[ii].key, _BRDC("#begin") ) )		// Begin of nested SubSection
					beginCnt++;
				if( !BRDC_stricmp( pInitData[ii].key, _BRDC("#end") ) )			// End of this or nested SubSection
					if( --beginCnt == 0 )
						break;
			}
			continue;
		}

		//
		// Search Driver name
		//

		if( !BRDC_stricmp( pInitData[ii].key, _BRDC("type") ) )	// Found keyword "type"
			break;
	}
	if( BRDC_stricmp( pInitData[ii].key, _BRDC("type") ) )	// Not Found keyword "type"
		return BRDerr_ERROR;

	//
	// Open Library
	//
	hLib = _BRD_openLibraryEx( pInitData[ii].val, 0 );
	if( !hLib )
	{
		return BRDerr_ERROR;
	}
		
	//
	// Get idxBrd
	//
	idxBrd = g_rIdxBrdCollection.GetIdxBrd( pInitData[ii].val );
	if( idxBrd == -1 )
	{
		idxBrd = 0;
		g_rIdxBrdCollection.PutName( pInitData[ii].val );
	}
	g_rIdxBrdCollection.IncrementIdxBrd( pInitData[ii].val );


	//
	// Get Entry Point
	//
#if defined(_WIN64) || defined(__linux__)
	pDEV_initData  = (DEV_API_initData*)IPC_getEntry( hLib, "_DEV_initData" );
	pEntry         = (DEV_API_entry*)IPC_getEntry( hLib, "_DEV_entry" );
#else
	pDEV_initData  = (DEV_API_initData*)IPC_getEntry( hLib, "_DEV_initData@20" );
	pEntry         = (DEV_API_entry*)IPC_getEntry( hLib, "_DEV_entry@12" );
#endif
	if( pDEV_initData == 0 || pEntry == 0 )
	{
		IPC_closeLibrary( hLib );
		return BRDerr_ERROR;
	}

	//
	// Search uninitialized Board
	//
	for(;;)
	{
		boardType =	(pDEV_initData)( idxBrd, &pDev, boardName, pInitData, initDataSize );
		if( boardType<=0 )
		{
			IPC_closeLibrary( hLib );
			break;
		}

		//
		// Check if this Board has been already initialized
		// Look through boardArr[]
		//
		for( jj=0; jj<BRD_COUNT; jj++ )
			if( !BRDC_stricmp( boardArr[jj].boardName, boardName ))
				break;
		if( jj<BRD_COUNT )
		{
			//
			// The Board already has been initialized
			//
			pEntry( pDev, DEVcmd_CLEANUP, NULL );

			idxBrd = g_rIdxBrdCollection.GetIdxBrd( pInitData[ii].val );
			g_rIdxBrdCollection.IncrementIdxBrd( pInitData[ii].val );
			continue;
		}


		//
		// Init Device Object for Board
		//
		pBoard = g_reinitBoardArr+idxBoard;

		pBoard->boardType = boardType;
		pBoard->boardLid  = boardLid;
		BRDC_strncpy( pBoard->boardName, boardName, 32 );
		pBoard->pEntry = pEntry;
		pBoard->pDev = pDev;
		pBoard->hLib = hLib;

		return BRDerr_OK;
    }
	
	return BRDerr_ERROR;
}

//=********************** Brd_ParseIniFile **************
//=******************************************************
S32		Brd_Reinit_ParseIniFile( U32 mode, const BRDCHAR *regFile, S32 nummax )
{
	int			boardLid;
	int			idxBrd=0;			// index of boardArr[]
	
//	FILE		*fin;			// INI File
	long		curSeek;		// current pointer of REG file
	BRDCHAR		line[256];
	BRDCHAR		*pKeyLin, *pValLin;
	int			isEqual;

	TBRD_InitData	*pInitData;		// Array of "Key+Value" Pares
	S32				initDataSize;	// Item's Number of pInitData Array
	S32				initDataSize2;	// Item's Number of pInitData Array
	
	
	g_rIdxBrdCollection.Clear();

	//
	//==== Open INI File
	//
	FILE* fin = Brd_OpenIniFile( mode, regFile );
	if( fin==NULL )
		return Brd_ErrorPrintf( BRDERR(BRDerr_BAD_INI_FILE), CURRFILE, 
				_BRDC("<Brd_ParseIniFile> Can't open INI file \"\""), regFile );

	//
	//==== Read INI File and Form TBRD_InitData Array
	//

	for(;;)
	{
		if(feof(fin))					// If End of REG File
			break;
		if( idxBrd >= nummax )			// If too many boards
			break;
		BRDC_fgets( line, 255, fin );
		curSeek = ftell( fin );
		Brd_initIniFile_ParseLine( line, &pKeyLin, &pValLin, &isEqual );
		if(*pKeyLin=='[')					// May be it's a board definition
		{
			//
			// Check line that it's a board definition
			//

			if( 0!=BRDC_strnicmp( pKeyLin, _BRDC("[LID:"), 5 ) )
				continue;

			if( 1 != BRDC_sscanf( pKeyLin+5, _BRDC("%d"), &boardLid ) )
            	continue;

			//
			// Calculate the size of TBRD_InitData Array
			//
			initDataSize = 0;
			for(;;)
			{
				if(feof(fin)) {				// Found End of INI file  
					break;
				}
				BRDC_fgets( line, 255, fin );
				Brd_initIniFile_ParseLine( line, &pKeyLin, &pValLin, &isEqual );
				if( *pKeyLin == '[' )		// Found new record
					break;
				if( *pKeyLin != '\0' )
					initDataSize++;
			}
			fseek( fin, curSeek, SEEK_SET );
			if( initDataSize == 0 )
			{
				continue;
			}

			//
			// Allocate TBRD_InitData Array
			//
			pInitData = (TBRD_InitData*)calloc( initDataSize, sizeof(TBRD_InitData) );
		//	if( initDataSize == 0 )
			if( pInitData == NULL )
			{
				Brd_Printf( BRDdm_WARN, CURRFILE, _BRDC("<Brd_ParseIniFile> No enough memory to initialize board LID:%02d "), boardLid );
				continue;
			}


			//
			// Fill TBRD_InitData Array
			//
			for( initDataSize2=0; initDataSize2<initDataSize; )
			{
				if(feof(fin))				// Found End of INI file  
					break;
				BRDC_fgets( line, 255, fin );
				Brd_initIniFile_ParseLine( line, &pKeyLin, &pValLin, &isEqual );
				if( *pKeyLin == '[' )		// Found new record
					break;
				if( *pKeyLin == '\0' )
					continue;
				BRDC_strncpy( pInitData[initDataSize2].key, pKeyLin, sizeof(pInitData[0].key) / sizeof(pInitData[0].key[0]));
				BRDC_strncpy( pInitData[initDataSize2].val, pValLin, sizeof(pInitData[0].val) / sizeof(pInitData[0].val[0]) );
				initDataSize2++;
			}
			fseek( fin, curSeek, SEEK_SET );

			//
			// Use TBRD_InitData Array
			//
			if( 0<=Brd_Reinit_InitData( idxBrd, boardLid, pInitData, initDataSize2 ) )
				idxBrd++;

			//
			// Free TBRD_InitData Array
			//
			free( pInitData );
		}
	}

	fclose(fin);
	return BRDERR(BRDerr_OK);
}



//
// End of File
//

