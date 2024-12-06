//=*******************************************************
//
// BRDINIT.CPP
//
// BRD Initialization Helper function definitions.
//
// (C) Instr.Sys. by Ekkore Dec, 1998-2001
//
// Modifications:
//   14.06.2002 Ekkore: Func DEV_initData() are got new parameter 'idxThisBrd'.
//
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

#include <vector>
using namespace std;
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
extern TBRD_Board			boardArr[BRD_COUNT];	// boardArr has BRD_COUNT entries
CIdxBrdCollection			g_rIdxBrdCollection;

//
//====== Functions Declaration
//

void	Brd_initIniFile_ParseLine( BRDCHAR *pSrcLin, BRDCHAR **ppKeyLin, BRDCHAR **ppValLin, S32 *pIsEqual );
S32 Brd_ParseAutoInitWithRegistry( TBRD_SubEnum **ppBaseEnum, U32 *pSizeBaseEnum, TBRD_SubEnum **ppSubEnum, U32 *pSizeSubEnum );
S32 Brd_ParseAutoInitWithIniFile( U32 mode, const void *pSrc, TBRD_SubEnum **ppBaseEnum, U32 *pSizeBaseEnum, TBRD_SubEnum **ppSubEnum, U32 *pSizeSubEnum );
S32 Brd_ParseAutoInit( U32 mode, const void *pSrc, const BRDCHAR *logFile );
S32	Brd_InitData( int idxBrd, int boardLid, TBRD_InitData *pInitData, S32 initDataSize );
#ifdef _WIN32
int	Brd_ParseRegistry_FormInitData( HKEY hBrdKey, TBRD_InitData *pInitData, S32 *pInitDataSize );
S32 Brd_ParseRegistry( void );
#endif
S32 Brd_ParseIniFile( U32 mode, const BRDCHAR *regFile );
FILE	*Brd_OpenIniFile( U32 mode, const void *pSrc );

S32	Brd_CallDriver( BRD_Handle handle, S32 cmd, void *pParam, S32 isCheckOpen );
S32	Brd_ErrorPrintf( S32 errorCode, BRDCHAR *title, BRDCHAR *format, ... );
void	Brd_Printf( S32 errLevel, BRDCHAR *title, BRDCHAR *format, ... );

S32		_Brd_gets( BRDCHAR* &pp, BRDCHAR *line )
{
	if( *pp == 0 )
		return -1;
	
	BRDCHAR *end = BRDC_strstr( pp, _BRDC("\n") );

	if( end )
	{
		BRDC_strncpy( line, pp, end-pp );

		line[ end-pp ] = 0;
	}
	else
	{
		BRDC_strcpy( line, pp );

		
	}

	int len = BRDC_strlen( line );
	pp += len + 1;

	return 0;
}

IPC_handle _BRD_openLibraryEx(const IPC_str *baseName, unsigned param)
{
    IPC_handle hLib = 0;
#ifdef	__IPC_WIN__
	hLib = IPC_openLibraryEx(baseName, param);
#endif
#ifdef	__IPC_LINUX__

    char *libAbsNames[3] = {0,0,0};
    for(unsigned i=0; i<sizeof(libAbsNames)/sizeof(libAbsNames[0]); i++) {
        libAbsNames[i] = (char*)malloc(PATH_MAX);
    }

    // prepare first case if library placed in current directory
    if(libAbsNames[0]) {
        const char *currdir = getcwd(libAbsNames[0], PATH_MAX);
        if(currdir) {
            unsigned dirlen = strlen(libAbsNames[0]);
            snprintf(libAbsNames[0]+dirlen, PATH_MAX-dirlen, "%s%s%s", "/lib", baseName, ".so");
        }
    }

    // prepare second case if library placed in pointed BARDYLIB environment variable
    if(libAbsNames[1]) {
        const char *envpath = getenv("BARDYLIB");
        if(envpath) {
            snprintf(libAbsNames[1], PATH_MAX, "%s%s%s%s", envpath, "/lib", baseName, ".so");
        } else {
            free(libAbsNames[1]);
            libAbsNames[1] = 0;
        }
    }

    // prepare third absname in predefined directory "/usr/local/lib/bardy/lib"
    if(libAbsNames[2]) {
        snprintf(libAbsNames[2], PATH_MAX, "%s%s%s", "/usr/local/lib/bardy/lib", baseName, ".so");
    }

    for(unsigned i=0; i<sizeof(libAbsNames)/sizeof(libAbsNames[0]); i++) {
        //fprintf(stderr, "TRY LIBRARY: ---- %s\n", libAbsNames[i]);
        if(libAbsNames[i]) {
            hLib = IPC_openLibrary(libAbsNames[i], param);
            if(!hLib) {
                continue;
            } else {
                break;
            }
        }
    }

    for(unsigned i=0; i<sizeof(libAbsNames)/sizeof(libAbsNames[0]); i++) {
        if(libAbsNames[i]) {
            free(libAbsNames[i]);
        }
    }
#endif
    return hLib;
}

//=*************** Brd_initIniFile_GetKeyWord ***********
//*******************************************************
void	Brd_initIniFile_ParseLine( BRDCHAR *pSrcLin, BRDCHAR **ppKeyLin, BRDCHAR **ppValLin, S32 *pIsEqual )
{
	BRDCHAR	*pLin = pSrcLin;
	BRDCHAR	*pLin2;


	*ppValLin = *ppKeyLin = pSrcLin+BRDC_strlen(pSrcLin);
	*pIsEqual = 0;

	//
	// Remove Comment
	//
	//pLin2 = strchr( pLin, ';' );
	pLin2 = BRDC_strchr( pLin, ';' );
	if( pLin2 )
		*pLin2 = '\0';

	//
	// Remove Starter Spaces
	//
	while( isspace(*pLin) )
		pLin++;
	if( *pLin=='\0' )
		return;

	//
	// Form Key Word Token
	//
	*ppKeyLin = pLin;
	if( *pLin=='[' )		// if Section Header
	{
		while( *pLin!=']' && *pLin!='\0' )
			pLin++;
		if( *pLin==']' )
			*(pLin+1) = '\0';
		return;
	}
	
	// if Section Body
	while( !isspace(*pLin) && *pLin!='=' && *pLin!='\0' )
		pLin++;
	if( *pLin=='=' )
		*pIsEqual = 1;
	if( *pLin=='\0' )
		return;
	*pLin='\0';
	pLin++;

	//
	// Search Value Token
	//
	while( (isspace(*pLin) || *pLin=='=') && *pLin!='\0' )
	{	if( *pLin=='=' )
			*pIsEqual = 1;			
		pLin++;
	}
	if( *pLin=='\0' )
		return;
	
	//
	// Form Value Token
	//
	*ppValLin = pLin;
	while( !isspace(*pLin) && *pLin!='\0' )
		pLin++;
	*pLin='\0';		
}

//=*************** Brd_ParseAutoInitWithRegistry ********
//=******************************************************
S32 Brd_ParseAutoInitWithRegistry( TBRD_SubEnum **ppBaseEnum, U32 *pSizeBaseEnum, 
										  TBRD_SubEnum **ppSubEnum,  U32 *pSizeSubEnum  )
{
#ifdef _WIN32	
	DWORD		getRegistryLong(HKEY hKey, BRDCHAR *ValName, DWORD DefValue);

    HKEY		hMainKey, hBaseEnumKey, hSubEnumKey;	// Registry Keys
	DWORD		idxKey, idxKeyMax;						// Index to Enum Registry Values
	BRDCHAR		linName[132];							// Name of Registry Key
	DWORD		len;									// Length of Name of Registry Key
	//int			isSubEnum = 1;							// 0-if [SUB ENUM] is abcent


	//
	//==== Clear Return Values
	//
	*ppBaseEnum = *ppSubEnum = NULL;
	*pSizeBaseEnum = *pSizeSubEnum = 0;

	//
	//==== Find BRD Shell section
	//
	if( RegOpenKey(HKEY_LOCAL_MACHINE, REGBRD_LIN, &hMainKey) != ERROR_SUCCESS)
	{
		return Brd_ErrorPrintf( BRDERR(BRDerr_BAD_REGISTRY), CURRFILE, _BRDC("<Brd_ParseAutoInitWithRegistry> Can't open Registry section") );
	}

	//
	//==== Find BASE ENUM section
	//
	if( RegOpenKey( hMainKey, _BRDC("BASE ENUM"), &hBaseEnumKey ) != ERROR_SUCCESS)
	{
		RegCloseKey( hMainKey );
		return Brd_ErrorPrintf( BRDERR(BRDerr_BAD_ENUMERATION), CURRFILE, _BRDC("<Brd_ParseAutoInitWithRegistry> None [BASE ENUM] section in the Registry") );
	}

	//
	//==== Calc BaseEnum Size
	//
	for( idxKey=0;; idxKey++ )
	{
		len = 132;
		if( ERROR_SUCCESS != RegEnumValue( hBaseEnumKey, idxKey, linName, &len, 0, NULL, NULL, NULL ) )
			break;
	}
	*pSizeBaseEnum = idxKeyMax = idxKey;
	*ppBaseEnum = (idxKeyMax==0) ? NULL : (TBRD_SubEnum*)calloc( sizeof(TBRD_SubEnum), idxKeyMax );


	//
	//==== Fill BaseEnum 
	//
	for( idxKey=0; idxKey<idxKeyMax; idxKey++ )
	{
		len = 132;
		if( ERROR_SUCCESS != RegEnumValue( hBaseEnumKey, idxKey, linName, &len, 0, NULL, NULL, NULL ) )
			break;
		BRDC_strcpy( (*ppBaseEnum)[idxKey].name, linName );
	}

	//
	//==== Close BASE ENUM section
	//
	RegCloseKey( hBaseEnumKey );

	//
	//==== Find SUB ENUM section
	//
	if( RegOpenKey( hMainKey, _BRDC("SUB ENUM"), &hSubEnumKey ) != ERROR_SUCCESS)
	{
		RegCloseKey( hMainKey );
		return Brd_ErrorPrintf( BRDERR(BRDerr_OK), CURRFILE, _BRDC("<Brd_ParseAutoInitWithRegistry> None [SUB ENUM] section in the Registry") );
	}

	//
	//==== Calc SubEnum Size
	//
	for( idxKey=0;; idxKey++ )
	{
		len = 132;
		if( ERROR_SUCCESS != RegEnumValue( hSubEnumKey, idxKey, linName, &len, 0, NULL, NULL, NULL ) )
			break;
	}
	*pSizeSubEnum = idxKeyMax = idxKey;
	*ppSubEnum = (idxKeyMax==0) ? NULL : (TBRD_SubEnum*)calloc( sizeof(TBRD_SubEnum), idxKeyMax );

	//
	//==== Fill SubEnum 
	//
	for( idxKey=0; idxKey<idxKeyMax; idxKey++ )
	{
		len = 132;
		if( ERROR_SUCCESS != RegEnumValue( hSubEnumKey, idxKey, linName, &len, 0, NULL, NULL, NULL ) )
			break;
		BRDC_strcpy( (*ppSubEnum)[idxKey].name, linName );
		(*ppSubEnum)[idxKey].val = getRegistryLong( hSubEnumKey, linName, 0 );

	}

	//
	//==== Close SUB ENUM section
	//
	RegCloseKey( hSubEnumKey );

	//
	//==== Close Main Registry Key
	//

	RegCloseKey( hMainKey );

	return BRDERR(BRDerr_OK);
#else // _WIN32
	//return Brd_ErrorPrintf( BRDERR(BRDerr_FUNC_UNIMPLEMENTED), CURRFILE, _BRDC("<Brd_ParseAutoInitWithRegistry> not not implemented") );
	fprintf(stderr, "%s, %d: - %s not implemented\n", __FILE__, __LINE__, __FUNCTION__);
	return BRDERR(BRDerr_FUNC_UNIMPLEMENTED);
#endif
}



//=**************** Brd_ParseAutoInitWithIniFile ********
//=******************************************************
S32 Brd_ParseAutoInitWithString( U32 mode, const void *pSrc, 
										 TBRD_SubEnum **ppBaseEnum, U32 *pSizeBaseEnum,
										 TBRD_SubEnum **ppSubEnum,  U32 *pSizeSubEnum )
{
	
	BRDCHAR		line[201], *pCh2, *nptr;
	U32			number;				// items number 

	//
	//==== Clear Return Values
	//
	*ppBaseEnum = *ppSubEnum = NULL;
	*pSizeBaseEnum = *pSizeSubEnum = 0;

	//
	//==== Open INI File
	//
	
	if( pSrc==NULL )
		return Brd_ErrorPrintf( BRDERR(BRDerr_BAD_INI_FILE), CURRFILE, 
				_BRDC("<Brd_ParseAutoInitWithString> Can't parse NULL string") );

	//
	//==== Search [BASE ENUM] Section
	//
	
	BRDCHAR *pp = (BRDCHAR*)pSrc;

	for(;;)
	{
		// Get Next Line
		if( _Brd_gets( pp, line ) == -1 )
			return BRDERR(BRDerr_ERROR);
		
		if( !BRDC_strnicmp( line, _BRDC("[BASE ENUM]"), 11 ) )//_strnicmp
			break;
	}
	

	//
	//==== Calc BaseEnum Size
	//
	number = 0;

	BRDCHAR *pos = pp;

	for(;;)
	{
		// Get Next Line
		if( _Brd_gets( pp, line ) == -1 )
			break;

		while( isspace(line[0]) )				// Remove Starting Space
			BRDC_strcpy( line, line+1 );
		if( line[0]=='\0' )						// If empty Line
			continue;
		if( line[0]=='[' )						// If End of Section
			break;
		number++;
	}
	*pSizeBaseEnum = number;
	*ppBaseEnum = (number==0) ? NULL : (TBRD_SubEnum*)calloc( sizeof(TBRD_SubEnum), number );

	//
	//==== Fill BaseEnum 
	//
	pp = pos;

	number = 0;
	for( ; number<*pSizeBaseEnum; )
	{
		// Get Next Line
		if( _Brd_gets( pp, line ) == -1 )
			break;

		while( isspace(line[0]) )				// Remove Starting Space
			BRDC_strcpy( line, line+1 );
		if( line[0]=='\0' )						// If empty Line
			continue;
		if( line[0]=='[' )						// If End of Section
			break;
		while( isspace(line[BRDC_strlen(line)-1]) )	// Remove Ending Space
			line[BRDC_strlen(line)-1] = '\0';
		BRDC_strncpy( (*ppBaseEnum)[number].name, line, 132 );//strncpy
		number++;
	}


	//
	//==== Search [SUB ENUM] Section
	//
	pp = pos;

	for(;;)
	{
		// Get Next Line
		if( _Brd_gets( pp, line ) == -1 )
			return BRDERR(BRDerr_OK);

		

		if( !BRDC_strnicmp( line, _BRDC("[SUB ENUM]"), 10 ) )//_strnicmp
			break;
	}

	pos = pp;

	//
	//==== Calc SubEnum Size
	//
	number = 0;
	for(;;)
	{
		// Get Next Line
		if( _Brd_gets( pp, line ) == -1 )
		{
			break;
		}

		while( isspace(line[0]) )				// Remove Starting Space
			BRDC_strcpy( line, line+1 );
		if( line[0]=='\0' )						// If empty Line
			continue;
		if( line[0]=='[' )						// If End of Section
			break;
		number++;
	}
	*pSizeSubEnum = number;
	*ppSubEnum = (number==0) ? NULL : (TBRD_SubEnum*)calloc( sizeof(TBRD_SubEnum), number );

	//
	//==== Fill SubEnum 
	//
	pp =pos;

	number = 0;
	for( ; number<*pSizeSubEnum; )
	{
				// Get Next Line
		if( _Brd_gets( pp, line ) == -1 )
			break;

		while( isspace(line[0]) )				// Remove Starting Space
			BRDC_strcpy( line, line+1 );
		if( line[0]=='\0' )						// If empty Line
			continue;
		if( line[0]=='[' )						// If End of Section
			break;
		pCh2 = BRDC_strchr( line, '=' );//strchr				// Search '='
		if( pCh2==NULL)							// None Value
			continue;
		(*ppSubEnum)[number].val = BRDC_strtol( pCh2+1, &nptr, 0 );//strtoul
		*pCh2 = '\0';							// Remove '='
		while( isspace(line[BRDC_strlen(line)-1]) )	// Remove Ending Space
			line[BRDC_strlen(line)-1] = '\0';
		BRDC_strncpy( (*ppSubEnum)[number].name, line, 132 );
		number++;
	}

	

	return BRDERR(BRDerr_OK);
}

//=**************** Brd_ParseAutoInitWithIniFile ********
//=******************************************************
S32 Brd_ParseAutoInitWithIniFile( U32 mode, const void *pSrc, 
										 TBRD_SubEnum **ppBaseEnum, U32 *pSizeBaseEnum,
										 TBRD_SubEnum **ppSubEnum,  U32 *pSizeSubEnum )
{
	FILE		*fin;			// INI File
	long		curSeek=0;		// current pointer of REG file
	BRDCHAR		line[201], *pCh2, *nptr;
	U32			number;				// items number 

	//
	//==== Clear Return Values
	//
	*ppBaseEnum = *ppSubEnum = NULL;
	*pSizeBaseEnum = *pSizeSubEnum = 0;

	//
	//==== Open INI File
	//
	fin = Brd_OpenIniFile( mode, pSrc );
	if( fin==NULL )
		return Brd_ErrorPrintf( BRDERR(BRDerr_BAD_INI_FILE), CURRFILE, 
				_BRDC("<Brd_ParseAutoInitWithIniFile> Can't open INI file") );

	//
	//==== Search [BASE ENUM] Section
	//
	rewind( fin );
	for(;;)
	{
		if(feof(fin))					// If End of REG File
			break;
		BRDC_fgets( line, 200, fin );		// Get Next Line
		curSeek = ftell( fin );
		if( !BRDC_strnicmp( line, _BRDC("[BASE ENUM]"), 11 ) )//_strnicmp
			break;
	}
	if( feof(fin) )
		return Brd_ErrorPrintf( BRDERR(BRDerr_BAD_ENUMERATION), CURRFILE, _BRDC("<Brd_ParseAutoInitWithIniFile> None [BASE ENUM] section in the Line") );

	//
	//==== Calc BaseEnum Size
	//
	number = 0;
	for(;;)
	{
		if(feof(fin))					// If End of REG File
			break;
		BRDC_fgets( line, 200, fin );		// Get Next Line
		while( isspace(line[0]) )				// Remove Starting Space
			BRDC_strcpy( line, line+1 );
		if( line[0]=='\0' )						// If empty Line
			continue;
		if( line[0]=='[' )						// If End of Section
			break;
		number++;
	}
	*pSizeBaseEnum = number;
	*ppBaseEnum = (number==0) ? NULL : (TBRD_SubEnum*)calloc( sizeof(TBRD_SubEnum), number );

	//
	//==== Fill BaseEnum 
	//
	fseek( fin, curSeek, SEEK_SET );
	number = 0;
	for( ; number<*pSizeBaseEnum; )
	{
		if(feof(fin))					// If End of REG File
			break;
		BRDC_fgets( line, 200, fin );		// Get Next Line
		while( isspace(line[0]) )				// Remove Starting Space
			BRDC_strcpy( line, line+1 );
		if( line[0]=='\0' )						// If empty Line
			continue;
		if( line[0]=='[' )						// If End of Section
			break;
		while( isspace(line[BRDC_strlen(line)-1]) )	// Remove Ending Space
			line[BRDC_strlen(line)-1] = '\0';
		BRDC_strncpy( (*ppBaseEnum)[number].name, line, 132 );//strncpy
		number++;
	}


	//
	//==== Search [SUB ENUM] Section
	//
	rewind( fin );
	for(;;)
	{
		if(feof(fin))					// If End of REG File
			break;
		BRDC_fgets( line, 200, fin );		// Get Next Line
		curSeek = ftell( fin );
		if( !BRDC_strnicmp( line, _BRDC("[SUB ENUM]"), 10 ) )//_strnicmp
			break;
	}
	if( feof(fin) )
	{
		fclose( fin );
		return Brd_ErrorPrintf( BRDERR(BRDerr_OK), CURRFILE, _BRDC("<Brd_ParseAutoInitWithIniFile> None [SUB ENUM] section in the Line") );
	}

	//
	//==== Calc SubEnum Size
	//
	number = 0;
	for(;;)
	{
		if(feof(fin))					// If End of REG File
			break;
		BRDC_fgets( line, 200, fin );		// Get Next Line
		while( isspace(line[0]) )				// Remove Starting Space
			BRDC_strcpy( line, line+1 );
		if( line[0]=='\0' )						// If empty Line
			continue;
		if( line[0]=='[' )						// If End of Section
			break;
		number++;
	}
	*pSizeSubEnum = number;
	*ppSubEnum = (number==0) ? NULL : (TBRD_SubEnum*)calloc( sizeof(TBRD_SubEnum), number );

	//
	//==== Fill SubEnum 
	//
	fseek( fin, curSeek, SEEK_SET );
	number = 0;
	for( ; number<*pSizeSubEnum; )
	{
		if(feof(fin))					// If End of REG File
			break;
		BRDC_fgets( line, 200, fin );		// Get Next Line
		while( isspace(line[0]) )				// Remove Starting Space
			BRDC_strcpy( line, line+1 );
		if( line[0]=='\0' )						// If empty Line
			continue;
		if( line[0]=='[' )						// If End of Section
			break;
		pCh2 = BRDC_strchr( line, '=' );//strchr				// Search '='
		if( pCh2==NULL)							// None Value
			continue;
		(*ppSubEnum)[number].val = BRDC_strtol( pCh2+1, &nptr, 0 );//strtoul
		*pCh2 = '\0';							// Remove '='
		while( isspace(line[BRDC_strlen(line)-1]) )	// Remove Ending Space
			line[BRDC_strlen(line)-1] = '\0';
		BRDC_strncpy( (*ppSubEnum)[number].name, line, 132 );
		number++;
	}

	fclose( fin );

	return BRDERR(BRDerr_OK);
}

//=********************** Brd_ParseAutoInit *************
//  Parses [BASE ENUM] section of INI-File or Registry.
//=******************************************************
S32  Brd_ParseAutoInit( U32 mode, const void *pSrc, const BRDCHAR *logFile )
{
	TBRD_SubEnum	*pBaseEnum;
	U32				sizeBaseEnum;
	TBRD_SubEnum	*pSubEnum;
	U32				sizeSubEnum;
	S32				ret;
	U32				number;

	IPC_handle		hLib;				// DLL header
	int				idxBrd=0;			// index of boardArr[] and as well LID-1
	S32				idxThisBrd;			// index of boards of one type
	DEV_API_initAuto	*pDEV_initAuto;
	DEV_API_entry		*pEntry;

	FILE		*flog=NULL;		// LOG File
	S32			boardType;
	void		*pDev;
	TBRD_Board	*pBoard;

	BRDCHAR		line[201];
	S32			linLen;			// sizeof(line)
	BRDCHAR		boardName[128];



	//
	//==== Open LOG File
	//
	if( logFile != NULL )
	{
		flog = BRDC_fopen( logFile, _BRDC("wt") );	
		if( flog==NULL )
			return Brd_ErrorPrintf( BRDERR(BRDerr_BAD_LOG_FILE), CURRFILE, _BRDC("<BRD_autoinit> Can't create LOG file") );
	}

	//
	//==== Parse [BASE ENUM] and [SUB ENUM] Sections
	//
	if((mode&0xF)==BRDinit_REGISTRY)
		ret = Brd_ParseAutoInitWithRegistry( &pBaseEnum, &sizeBaseEnum, &pSubEnum, &sizeSubEnum );	
	else if((mode&0xF)==BRDinit_STRING)
		ret = Brd_ParseAutoInitWithString( mode, pSrc, &pBaseEnum, &sizeBaseEnum, &pSubEnum, &sizeSubEnum );
	else 
		ret = Brd_ParseAutoInitWithIniFile( mode, pSrc, &pBaseEnum, &sizeBaseEnum, &pSubEnum, &sizeSubEnum );

	if( ret<0 )
		return ret;

	//
	// Enumerate All Declared Drivers
	//
	for( number=0; number<sizeBaseEnum; number++ )
	{
		//
		// Open Library
		//
		hLib = _BRD_openLibraryEx( pBaseEnum[number].name, 0 );		// Load helper instatce of DLL
		if( !hLib )
			continue;
		//linLibName[strlen(linLibName)-4] = 0;				// Remove .DLL Extension


		//
		// Get Entry Point
		//
#if defined(_WIN64) || defined(__linux__)
		pDEV_initAuto = (DEV_API_initAuto*)IPC_getEntry( hLib, "DEV_initAuto" );
		pEntry        = (DEV_API_entry*)IPC_getEntry( hLib, "DEV_entry" );
#else
		pDEV_initAuto = (DEV_API_initAuto*)IPC_getEntry( hLib, "_DEV_initAuto@28" );
		pEntry        = (DEV_API_entry*)IPC_getEntry( hLib, "_DEV_entry@12" );
#endif
		if( pDEV_initAuto == 0 || pEntry == 0 )
		{
			IPC_closeLibrary( hLib );
			continue;
		}

		//
		// Enumerate All Boards of this type
		//
		idxThisBrd = 0;
		for(;;)
		{
			if(idxBrd>=BRD_COUNT )			// If end of boardArr[]
				break;
			
			linLen = sizeof(line)/sizeof(line[0]);
			boardType =	(pDEV_initAuto)( idxThisBrd, &pDev, boardName, line, linLen, pSubEnum, sizeSubEnum );
			if( boardType<=0 )
			{
				break;
			}
			

			//
			// Init Device Object for Board
			//
			pBoard = boardArr+idxBrd;


			pBoard->boardType = boardType;
			pBoard->boardLid   = idxBrd+1;
			BRDC_strncpy( pBoard->boardName, boardName, 32 );//strncpy
			pBoard->pEntry = pEntry;
			pBoard->pDev = pDev;
			pBoard->hLib = _BRD_openLibraryEx( pBaseEnum[number].name, 0 );
			idxBrd++;
			idxThisBrd++;

			//
			// Form LOG File ([LID:##] Section)
			//
			if( flog!=NULL )
			{
				BRDC_fprintf( flog, _BRDC("[LID:%d]\ntype=%s\n%s\n\n"), idxBrd, pBaseEnum[number].name, line );
			}
        }

		IPC_closeLibrary( hLib );		// Free helper instatce of DLL
	}


	//
	// Form LOG File: [BASE ENUM] and [SUB ENUM] Sections
	//
	if( flog!=NULL )
	{
		BRDC_fputs( _BRDC("\n[BASE ENUM]\n"), flog );
		for( number=0; number<sizeBaseEnum; number++ )
		{
			BRDC_fprintf( flog, _BRDC("%s\n"), pBaseEnum[number].name );
		}
		BRDC_fputs( _BRDC("\n[SUB ENUM]\n"), flog );
		for( number=0; number<sizeSubEnum; number++ )
		{
			BRDC_fprintf( flog, _BRDC("%s=0x%X\n"), pSubEnum[number].name, pSubEnum[number].val );
		}
		fclose(flog);
	}


	return BRDERR(BRDerr_OK);
}


//=********************** Brd_InitData ******************
//=******************************************************
S32		Brd_InitData( int idxBoard, int boardLid, TBRD_InitData *pInitData, S32 initDataSize )
{
	S32						ii;
	DEV_API_initData		*pDEV_initData;
	DEV_API_entry			*pEntry;
	IPC_handle				hLib;				// DLL header
	S32						boardType;
	void					*pDev;
	TBRD_Board				*pBoard;
	BRDCHAR					boardName[128];
	int						idxBrd;

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

		if( BRDC_stricmp( pInitData[ii].key, _BRDC("type") ) )	// Found keyword "type"
			continue;

		//
		// Open Library
		//
        hLib = _BRD_openLibraryEx( pInitData[ii].val, 0 );
		if( !hLib )
			continue;
		
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
		pDEV_initData  = (DEV_API_initData*)IPC_getEntry( hLib, "DEV_initData" );
		pEntry         = (DEV_API_entry*)IPC_getEntry( hLib, "DEV_entry" );
#else
		pDEV_initData  = (DEV_API_initData*)IPC_getEntry( hLib, "_DEV_initData@20" );
		pEntry         = (DEV_API_entry*)IPC_getEntry( hLib, "_DEV_entry@12" );
#endif
		if( pDEV_initData == 0 || pEntry == 0 )
		{
			IPC_closeLibrary( hLib );
			continue;
		}

		//
		// Parse the rest of the record
		//
		boardType =	(pDEV_initData)( idxBrd, &pDev, boardName, pInitData, initDataSize );
		if( boardType<=0 )
		{
			IPC_closeLibrary( hLib );
			continue;
		}


		//
		// Init Device Object for Board
		//
		pBoard = boardArr+idxBoard;

		pBoard->boardType = boardType;
		pBoard->boardLid  = boardLid;
		BRDC_strncpy( pBoard->boardName, boardName, 32 );//strncpy
		pBoard->pEntry = pEntry;
		pBoard->pDev = pDev;
		pBoard->hLib = hLib;

		return BRDerr_OK;
    }
	
	return BRDerr_ERROR;
}

//=**************** Brd_ParseRegistry_FormInitData ******
//=******************************************************
#ifdef _WIN32
int	Brd_ParseRegistry_FormInitData( HKEY hBrdKey, TBRD_InitData *pInitData, S32 *pInitDataSize )
{
	HKEY		hSubKey;
	S32			initDataSize = 0;
	S32			initDataSize2;
	BRDCHAR		keyName[32];
	BRDCHAR		valName[MAX_PATH];
	//void		*ptr = (void*)valName;
	DWORD		keyNameSize;
	DWORD		valNameSize;
	DWORD		keyType;
	DWORD		idx = 0;

	//
	// Parse KeyWords
	//
	for( idx=0;;idx++ )
	{
		keyNameSize = 32;
		valNameSize = MAX_PATH;
		if( ERROR_SUCCESS!=RegEnumValue( hBrdKey, idx, keyName, &keyNameSize, NULL, &keyType, (LPBYTE)valName, &valNameSize ) )
			break;

		if( keyType!=REG_DWORD && keyType!=REG_SZ )
			continue;

		if( pInitData )	
		{
			if( initDataSize >= *pInitDataSize )
				break;
			BRDC_strncpy( pInitData[initDataSize].key, keyName, sizeof(pInitData->key)/sizeof(pInitData->key[0]) );//strncpy
			if( keyType==REG_SZ )
				BRDC_strncpy( pInitData[initDataSize].val, valName, sizeof(pInitData->val)/sizeof(pInitData->val[0]) );//strncpy
			if( keyType==REG_DWORD )
				BRDC_sprintf( pInitData[initDataSize].val, _BRDC("0x%X"), *(S32*)(void*)valName );
		}
		initDataSize++;
	}
	
	//
	// Parse Directories
	//
	for( idx=0;;idx++ )
	{
		keyNameSize = 32;
		if( ERROR_SUCCESS!=RegEnumKeyEx( hBrdKey, idx, keyName, &keyNameSize, NULL, NULL, NULL, NULL ) )
			break;
		if( RegOpenKey( hBrdKey, keyName, &hSubKey ) != ERROR_SUCCESS)
			continue;

		if( pInitData )	
		{
			if( initDataSize >= *pInitDataSize )
				break;
			BRDC_strncpy( pInitData[initDataSize].key, _BRDC("#begin"), sizeof(pInitData->key)/sizeof(pInitData->key[0]) );//strncpy
			BRDC_strncpy( pInitData[initDataSize].val, keyName,  sizeof(pInitData->val)/sizeof(pInitData->val[0]) );//strncpy

			initDataSize++;

			if( initDataSize >= *pInitDataSize )
				break;
			initDataSize2 = *pInitDataSize - initDataSize;
			Brd_ParseRegistry_FormInitData( hSubKey, pInitData+initDataSize, &initDataSize2 );
			initDataSize += initDataSize2;

			if( initDataSize >= *pInitDataSize )
				break;
			BRDC_strncpy( pInitData[initDataSize].key, _BRDC("#end"), sizeof(pInitData->key)/sizeof(pInitData->key[0]) );//strncpy
			BRDC_strncpy( pInitData[initDataSize].val, _BRDC(""),     sizeof(pInitData->val)/sizeof(pInitData->val[0]) );//strncpy
			initDataSize++;
		}
		else
		{
			initDataSize2 = 0;
			Brd_ParseRegistry_FormInitData( hSubKey, NULL, &initDataSize2 );
			initDataSize += initDataSize2 + 2;
		}
	}

	*pInitDataSize = initDataSize;
	return 0;
}

//=********************** Brd_ParseRegistry *************
//  Parses Registry.
//  It can have this type of directories:
//
// [HKEY_LOCAL_MACHINE\SOFTWARE\Instrumental Systems\BRD Shell\LID:nn]
// [HKEY_LOCAL_MACHINE\SOFTWARE\Instrumental Systems\BRD Shell\BASE ENUM]
// [HKEY_LOCAL_MACHINE\SOFTWARE\Instrumental Systems\BRD Shell\SUB ENUM]
//
//=******************************************************
S32  Brd_ParseRegistry( void )
{
	int			getRegistryString(HKEY  hKey, LPSTR ValName, LPSTR Buf, int   Len, LPSTR DefValue);
	int			boardLid;
    HKEY		hMainKey, hBrdKey;	// Registry Keys
	int			idxBrd=0;			// index of boardArr[]
	DWORD		idxKey;				// Index to Enum Registry Keys
	

	BRDCHAR		linBrd[80];

	TBRD_InitData	*pInitData;		// Array of "Key+Value" Pares
	S32				initDataSize;	// Item's Number of pInitData Array
	

	g_rIdxBrdCollection.Clear();

	//
	// Open Main Registry Section
	//
	if( RegOpenKey(HKEY_LOCAL_MACHINE, REGBRD_LIN, &hMainKey) != ERROR_SUCCESS)
	{
		return Brd_ErrorPrintf( BRDERR(BRDerr_BAD_REGISTRY), CURRFILE, _BRDC("<Brd_ParseRegistry> Can't open Registry section") );
	}

	//
	//==== Read Registry and Form TBRD_InitData Array
	//


	hBrdKey = 0;
	idxKey = 0;
	for(;;)
	{
		//
		// Get Registry [LID:##] Section
		//
		DWORD	len;

		len = 80;
		if( ERROR_SUCCESS != RegEnumKeyEx( hMainKey, idxKey++, linBrd, &len, 0, NULL, NULL, NULL ) )
			break;
		if( BRDC_strnicmp( linBrd, _BRDC("LID:"), 4 ) )//_strnicmp
			continue;

		//
		// Get LID Number 
		//
		boardLid = BRDC_atoi( linBrd+4 );
		//
		// Open Key
		//
		if( RegOpenKey( hMainKey, linBrd, &hBrdKey ) != ERROR_SUCCESS)
		{	
			continue;
		}

		//
		// Calculate the size of TBRD_InitData Array
		//
		initDataSize = 0;
		Brd_ParseRegistry_FormInitData( hBrdKey, NULL, &initDataSize );
		if( initDataSize == 0 )
			continue;

		//
		// Allocate TBRD_InitData Array
		//
		pInitData = (TBRD_InitData*)calloc( initDataSize, sizeof(TBRD_InitData) );
		if( initDataSize == 0 )
		{
			Brd_Printf( BRDdm_WARN, CURRFILE, _BRDC("<Brd_ParseRegistry> No enough memory to initialize board LID:%02d "), boardLid );
			continue;
		}


		//
		// Fill TBRD_InitData Array
		//
		Brd_ParseRegistry_FormInitData( hBrdKey, pInitData, &initDataSize );

		//
		// Use TBRD_InitData Array
		//
		if( 0<=Brd_InitData( idxBrd, boardLid, pInitData, initDataSize ) )
			idxBrd++;

		//
		// Free TBRD_InitData Array
		//
		free( pInitData );

		RegCloseKey( hBrdKey );
	}

	RegCloseKey( hMainKey );

	return BRDERR(BRDerr_OK);
}
#endif



//=********************** Brd_ParseIniFile **************
//=******************************************************
S32		Brd_ParseBuffer( U32 mode, BRDCHAR *regData )
{
	int			boardLid;
	int			idxBrd=0;			// index of boardArr[]
	


	BRDCHAR		line[256];
	BRDCHAR		*pKeyLin, *pValLin;
	int			isEqual;

//	TBRD_InitData	*pInitData;		// Array of "Key+Value" Pares
//	S32				initDataSize;	// Item's Number of pInitData Array
//	S32				initDataSize2;	// Item's Number of pInitData Array
	
	
	g_rIdxBrdCollection.Clear();


	//
	//==== Open INI File
	//
	
	if( regData==NULL )
		return Brd_ErrorPrintf( BRDERR(BRDerr_BAD_INI_FILE), CURRFILE, 
				_BRDC("<Brd_ParseBuffer> Can't parse ini string.\"\"") );

	//
	//==== Read INI File and Form TBRD_InitData Array
	//

	BRDCHAR *pp = regData;

	BRDCHAR *end = pp + BRDC_strlen( pp );

	for(;;)
	{
		if( *pp == 0 )
			break;

		if( pp >= end )
			break;

		if(idxBrd>=BRD_COUNT )			// If end of boardArr[]
			break;

		

		
		_Brd_gets( pp, line );

		Brd_initIniFile_ParseLine( line, &pKeyLin, &pValLin, &isEqual );
		if(*pKeyLin=='[')					// May be it's a board definition
		{
			//
			// Check line that it's a board definition
			//

			if( 0 != BRDC_strnicmp( pKeyLin, _BRDC("[LID:"), 5 ) )//_strnicmp
				continue;

			if( 1 != BRDC_sscanf( pKeyLin+5, _BRDC("%d"), &boardLid ) )
            	continue;

			std::vector<TBRD_InitData> vid;

			//
			// Fill TBRD_InitData Array
			//

			BRDCHAR *prepp;

			for( ;; )
			{
				prepp = pp;

				if( _Brd_gets( pp, line ) == -1 )
					break;

				Brd_initIniFile_ParseLine( line, &pKeyLin, &pValLin, &isEqual );
				if( *pKeyLin == '[' )		// Found new record
					break;
				if( *pKeyLin == '\0' )
					continue;

				TBRD_InitData tmp;

				BRDC_strncpy( tmp.key, pKeyLin, sizeof(tmp.key) / sizeof(tmp.key[0]) ); //strncpy
				BRDC_strncpy( tmp.val, pValLin, sizeof(tmp.val) / sizeof(tmp.val[0]) ); //strncpy

				vid.push_back( tmp );
				
			}
			
			pp = prepp;

			int initDataSize = vid.size();
			TBRD_InitData *pInitData = (TBRD_InitData*)calloc( vid.size() , sizeof(TBRD_InitData) );

			for(std::vector<int>::size_type i = 0; i != vid.size(); i++) 
			{
				pInitData[i] = vid[i];
			}

			//
			// Use TBRD_InitData Array
			//
			if( 0<=Brd_InitData( idxBrd, boardLid, pInitData, initDataSize ) )
				idxBrd++;

			//
			// Free TBRD_InitData Array
			//
			free( pInitData );
			
		}
	}


	return BRDERR(BRDerr_OK);
}


//=********************** Brd_ParseIniFile **************
//=******************************************************
S32		Brd_ParseIniFile( U32 mode, const BRDCHAR *regFile)
{
	int			boardLid;
	int			idxBrd=0;			// index of boardArr[]
	
//	FILE		*fin;			// INI File
	long		curSeek;		// current pointer of REG file
	BRDCHAR		line[512];
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
				_BRDC("<Brd_ParseIniFile> Can't open INI file \"%s\""), regFile );

	//
	//==== Read INI File and Form TBRD_InitData Array
	//

	for(;;)
	{
		if(feof(fin))					// If End of REG File
			break;
		if(idxBrd>=BRD_COUNT )			// If end of boardArr[]
			break;
		BRDC_fgets( line, 512, fin );
		curSeek = ftell( fin );
		Brd_initIniFile_ParseLine( line, &pKeyLin, &pValLin, &isEqual );
		if(*pKeyLin=='[')					// May be it's a board definition
		{
			//
			// Check line that it's a board definition
			//

			if( 0 != BRDC_strnicmp( pKeyLin, _BRDC("[LID:"), 5 ) )//_strnicmp
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
				long curSeekTmp;
				do {
					BRDC_fgets( line, 255, fin );
					curSeekTmp = ftell(fin);
				} while(curSeekTmp<=curSeek);
				Brd_initIniFile_ParseLine( line, &pKeyLin, &pValLin, &isEqual );
				if( *pKeyLin == '[' )		// Found new record
					break;
				if( *pKeyLin != '\0' )
					initDataSize++;
			}

			if( initDataSize == 0 )
			{
				continue;
			}

			//
			// Allocate TBRD_InitData Array
			//
			pInitData = (TBRD_InitData*)calloc( initDataSize, sizeof(TBRD_InitData) );
			if( pInitData == NULL )
			{
				Brd_Printf( BRDdm_WARN, CURRFILE, _BRDC("<Brd_ParseIniFile> No enough memory to initialize board LID:%02d "), boardLid );
				continue;
			}


			//
			// Fill TBRD_InitData Array
			//
			
			fseek( fin, curSeek, SEEK_SET );
			for( initDataSize2=0; initDataSize2<initDataSize; )
			{
				if(feof(fin))				// Found End of INI file  
					break;

				long curSeekTmp;
				do {
					BRDC_fgets( line, 255, fin );
					curSeekTmp = ftell(fin);
				} while(curSeekTmp<=curSeek);

				Brd_initIniFile_ParseLine( line, &pKeyLin, &pValLin, &isEqual );
				if( *pKeyLin == '[' )		// Found new record
					break;
				if( *pKeyLin == '\0' )
					continue;
				BRDC_strncpy( pInitData[initDataSize2].key, pKeyLin, sizeof(pInitData[0].key) / sizeof(pInitData[0].key[0]) ); //strncpy
				BRDC_strncpy( pInitData[initDataSize2].val, pValLin, sizeof(pInitData[0].val) / sizeof(pInitData[0].val[0]) ); //strncpy
				initDataSize2++;
			}

			//
			// Use TBRD_InitData Array
			//
			if( 0<=Brd_InitData( idxBrd, boardLid, pInitData, initDataSize2 ) )
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


//=**************** Brd_OpenIniFile *************************
//=******************************************************
FILE	*Brd_OpenIniFile( U32 mode, const void *pSrc )
{
	FILE		*fin = NULL;	// INI File
	const BRDCHAR	*fileName = (BRDCHAR*)pSrc;
#ifdef _WIN32
	BRDCHAR		line[201], *pCh;
#endif

	//
	//==== Open INI File
	//
	if( (mode&0xF)==BRDinit_FILE_KNOWN )
	{
		fin = BRDC_fopen( REG_FILENAME, _BRDC("rt") );
		if( fin==NULL )
		{
#ifdef _WIN32
			if( SearchPath( NULL, REG_FILENAME, NULL, 200, line, &pCh ))
				fin = BRDC_fopen( line, _BRDC("rt") );
#endif
		}
		if( fin==NULL )
		{
			BRDCHAR	*env = (BRDCHAR*)BRDC_getenv(REG_ENVNAME);

			if( env!=NULL )
				fin = BRDC_fopen( env, _BRDC("rt") );
		}
	}
	else if( (mode&0xF)==BRDinit_FILE_ENV )
	{
		BRDCHAR	*env = (BRDCHAR*)BRDC_getenv(REG_ENVNAME);

		if( env!=NULL )
			fin = BRDC_fopen( env, _BRDC("rt") );
	}
	else if( (mode&0xF)==BRDinit_FILE )
	{
		fin = BRDC_fopen( fileName, _BRDC("rt") );
	}


	return fin;
}

//
// End of File
//

