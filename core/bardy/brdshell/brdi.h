/***************************************************
*
* BRDI.H
*
* BRD Shell Internal Declaration.
*
* (C) Instr.Sys. by Ekkore Dec, 1997-2001
*
****************************************************/


#ifndef	__BRDI_H_
#define	__BRDI_H_


#include	<stdio.h>
#include	"utypes.h"
#include	"brdapi.h"
#include	"gipcy.h"

//
//======= Types
//
#define	BRD_COUNT		64
#define	REG_FILENAME	_BRDC("brd.ini")
#define	REG_ENVNAME		_BRDC("BRDINI")
#define	REGBRD_LIN		_BRDC("SOFTWARE\\Instrumental Systems\\BRD Shell")
#define	BRD_HANDLE_SPY	0x8000


//
//==== Board Types
//
//  Defines only btNONE Board Type.
//  Other types are defined into DSP Driver Header Files.
//

#define		btNONE 				-1


//
//=== BRD_Board type and Others
//
#ifdef	__cplusplus
extern "C" {
#endif

//
// Service List of Board
//
typedef struct tagSBRD_ServList
{
	BRDCHAR		name[SERVNAME_SIZE];// Service name with Number
	U32			attr;			// Attributes of Service (Look BRDserv_ATTR_XXX constants)
	S32			idxSuperServ;	// Index of SuperService which forced this Service to be captured
	void		*ptrCaptor;		// NULL or Pointer to Captor To Capture/Release the Service
} SBRD_ServList;

//
// PU List of Board
//
typedef struct
{
	U32		puId;				// Programmable Unit ID
	U32		puCode;				// Programmable Unit Code
	U32		puAttr;				// Programmable Unit Attribute
	U32		puEnable;			// Enable to Write or Load Dagerous PU
} SBRD_PuList, *PSBRD_PuList;

//
//=== Device Object for Board
//
typedef struct SBRD_BoardSharedInfo
{
	long	openCnt;			// Shareable Open Counter
} SBRD_BoardSharedInfo;

typedef struct
{
	S32		boardType;			// Board type, default btNONE 
	U32		boardLid;			// Board LID from REG-file 
	BRDCHAR	boardName[32];		// Board Name (ASCII string)

	long			openCnt;		// Local Open Counter
	volatile long	cleanupCnt;		// Cleanup Processind Flag
	volatile long	workCnt;		// Work Function Processing Flag

	DEV_API_entry	*pEntry;		// Device Driver Entry Point
	void			*pDev;        	// Device Extension Created by Driver
	IPC_handle		hLib;			// Driver DLL handle
	IPC_handle		hOpenMutex;		// Mutex for Multythread Open Operation
	IPC_handle		hLockMutex;		// Mutex for Multythread Lock Operation
	IPC_handle		hCaptMutex;		// Mutex for Multythread Capture/Release Service Operation
	IPC_handle		hFileMap;		// File Mapping to Keep Open Protection Shareable Information
	SBRD_BoardSharedInfo *pBSI;		// Pointer      to Keep Open Protection Shareable Information
	SBRD_ServList	*pServList;		// Array of Service Controls
	S32				nServListSize;	// Number of Items in the "pServList[]"
	SBRD_PuList		*paPuList;		// Array of PUs
	S32				nPuListSize;	// Number of Items in the "paPuList[]"
} TBRD_Board;

void	DB_errdisplay( S32 errLevel );


#ifdef	__cplusplus
};  //extern "C"
#endif

//
//==== Global Variables
//
extern TBRD_Board		boardArr[BRD_COUNT];	// boardArr has BRD_COUNT entries

//
//==== Funx Declarations
//

//
// BRD.CPP
//
void Brd_ProcessDetach( void );
S32	Brd_GetPtrBoard( BRD_Handle handle, TBRD_Board **ppBoard, S32 isCheckOpen );
S32	Brd_CleanupDevice( TBRD_Board *pBoard );
S32	Brd_CallDriver( BRD_Handle handle, S32 cmd, void *pParam, S32 isCheckOpen );
S32	Brd_ErrorPrintf( S32 errorCode, const BRDCHAR *title, const BRDCHAR *format, ... );
void	Brd_Printf( S32 errLevel, const BRDCHAR *title, const BRDCHAR *format, ... );
 
//
// BRDINIT.CPP
//
S32  Brd_ParseAutoInit( U32 mode, const void *pSrc, const BRDCHAR *logFile );
#ifdef _WIN32
S32  Brd_ParseRegistry( void );
#endif
S32  Brd_ParseIniFile( U32 mode, const BRDCHAR *regFile );
S32	Brd_ParseBuffer( U32 mode, BRDCHAR *regData );

//
// BRDSERV.CPP
//
S32  Brd_Serv_CreateBoardList( int idx );
S32  Brd_Serv_DestroyBoardList( TBRD_Board *pBoard );
S32  Brd_Serv_Capture( BRD_Handle handle, BRD_Handle *pServHandle, U32 nodeId, U32 *pMode, const BRDCHAR *servName, U32 timeout );
S32  Brd_Serv_Release( BRD_Handle servHandle, U32 nodeId );
S32  Brd_Serv_List( BRD_Handle handle, U32 nodeId, BRD_ServList *pList, U32 item, U32 *pItemReal );
S32  Brd_Serv_Check( BRD_Handle handle, U32 nodeId );

//
// BRDPU.CPP
//
S32  Brd_Pu_CreateBoardList( int idx );
S32  Brd_Pu_DestroyBoardList( TBRD_Board *pBoard );
S32  Brd_Pu_SetWriteEnable( TBRD_Board *pBoard, U32 puId );
S32  Brd_Pu_IsWriteEnable( TBRD_Board *pBoard, U32 puId );

//
// BRDISUP.CPP
//

// BRDI_API S32	STDCALL BRDI_support( S32 cmd, void *pArgs )
S32		Brdi_Capture( BRDI_SUP_Capture *pArgs );
S32		Brdi_Release( BRDI_SUP_Release *pArgs );



#endif	// __BRDI_H_ 

//
// End of File
//


