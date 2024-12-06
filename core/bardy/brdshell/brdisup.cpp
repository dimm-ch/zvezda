//=********************************************************
//
// BRDISUP.CPP
//
// BRDI_support() Function definition:
//
// (C) Instrumental System, 2002-2003
//
// Created: Ekkore 10.02.2003
//
// Modifications:
//
//=********************************************************


#include	<malloc.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<signal.h>

#ifdef _WIN32
#define	BRDI_API	DllExport
#endif

#include	"utypes.h"
#include	"brdi.h"
#include	"captor.h"
#include	"brderr.h"



//=********************** BRDI_support ********************
//=********************************************************
BRDI_API S32	STDCALL BRDI_support( S32 cmd, void *pArgs )
{
	switch( cmd )
	{
		case BRDIsup_CAPTURE:
			return Brdi_Capture( (BRDI_SUP_Capture*)pArgs );
		case BRDIsup_RELEASE:
			return Brdi_Release( (BRDI_SUP_Release*)pArgs );
	}

	return BRDerr_CMD_UNSUPPORTED;
}


//=********************** Brdi_Capture ********************
//=********************************************************
S32		Brdi_Capture( BRDI_SUP_Capture *pArgs )
{
	TBRD_Board		*pBoard;
	BRD_Handle		hServ;
	S32				err;
	U32				captMode = pArgs->captMode;

	//
	// Check Open Mode
	//
	if( pArgs->handle & BRD_HANDLE_SPY )
		return BRDerr_BAD_PARAMETER;

	//
	// Get Board 
	//
	err = Brd_GetPtrBoard( pArgs->handle, &pBoard, 1 );
	if( err<0 )
		return BRDerr_BAD_PARAMETER;
	if( pArgs->idxMain >= pBoard->nServListSize )
		return BRDerr_BAD_PARAMETER;

	//
	// Capture Service
	//
	err = Brd_Serv_Capture( pArgs->handle, &hServ, pArgs->nodeId, &captMode, pBoard->pServList[pArgs->idxMain].name, BRDtim_AT_ONCE );
	if( 0>err )
		return err; 

	//
	// Send DEVcmd_CAPTURE to Service
	//
	DEV_CMD_Capture	paramCapt;

	paramCapt.handle = hServ;
	paramCapt.nodeId = pArgs->nodeId;
	err = Brd_CallDriver( pArgs->handle, DEVcmd_CAPTURE, &paramCapt, 1 );

	//
	// Send DEVcmd_CTRL + SERVcmd_SYS_CAPTURE to Service
	//
	DEV_CMD_Ctrl	paramCtrl;

	paramCtrl.handle = hServ;
	paramCtrl.nodeId = pArgs->nodeId;
	paramCtrl.cmd = SERVcmd_SYS_CAPTURE;
	paramCtrl.arg = NULL;
	err = Brd_CallDriver( pArgs->handle, DEVcmd_CTRL, &paramCtrl, 1 );
	if( 0>err )
		return err; 

	return BRDerr_OK;
}

//=********************** Brdi_Release ********************
//=********************************************************
S32		Brdi_Release( BRDI_SUP_Release *pArgs )
{
	TBRD_Board		*pBoard;
	S32				err;

	//
	// Get Board 
	//
	err = Brd_GetPtrBoard( pArgs->handle, &pBoard, 1 );
	if( err<0 )
		return err;
/*
	//
	// Send DEVcmd_CTRL + SERVcmd_SYS_RELEASE to Service
	//
	DEV_CMD_Ctrl	paramCtrl;

	paramCtrl.handle = pArgs->handle;
	paramCtrl.nodeId = pArgs->nodeId;
	paramCtrl.cmd = SERVcmd_SYS_RELEASE;
	paramCtrl.arg = NULL;
	Brd_CallDriver( pArgs->handle, DEVcmd_CTRL, &paramCtrl, 1 );
    
	//
	// Send DEVcmd_RELEASE to Service
	//
	DEV_CMD_Release	paramRel;

	paramRel.handle = pArgs->handle;
	paramRel.nodeId = pArgs->nodeId;
	Brd_CallDriver( pArgs->handle, DEVcmd_RELEASE, &paramRel, 1 );
*/
	//
	// Use Captor
	//
	CCaptor *pCaptor = (CCaptor*)pBoard->pServList[pArgs->idxMain].ptrCaptor;

	if( pCaptor==NULL )
		return BRDERR(BRDerr_BAD_HANDLE);
	pCaptor->Release();

	return BRDerr_OK;
}


//
// End of File
//

