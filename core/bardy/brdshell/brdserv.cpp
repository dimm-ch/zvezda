//=********************************************************
//
// BRDSERV.CPP
//
// BRD Service Support Helper function definitions:
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

#include	"utypes.h"
#include	"brdi.h"
#include	"captor.h"
#include	"brderr.h"


//=***************** Brd_Serv_CreateBoardList *************
//*********************************************************
S32		Brd_Serv_CreateBoardList( int idx )
{
	TBRD_Board			*pBoard = &boardArr[idx];
	DEV_CMD_GetServList	sGRL = {0,0,0};
	DEV_CMD_ServListItem	*pSLI;
	CCaptor				*pCaptor;
	int					ii;

	//
	// Init Service Support Table
	//
	pBoard->pServList = NULL;
	pBoard->nServListSize = 0;

	//
	// Get Number of Services (into "sGRL.itemReal")
	//
	Brd_CallDriver( idx+1, DEVcmd_GETSERVLIST, &sGRL, 0 );

	//
	// If There are no Services on The Board
	//
	if( sGRL.itemReal==0 )
		return 0;

	//
	// Get Service Information
	//
	pSLI = (DEV_CMD_ServListItem*)calloc( sizeof(DEV_CMD_ServListItem), sGRL.itemReal );
	if( pSLI==NULL )
		return -1;
	sGRL.pSLI = pSLI;
	sGRL.item = sGRL.itemReal;
	Brd_CallDriver( idx+1, DEVcmd_GETSERVLIST, &sGRL, 0 );

	//
	// Create Service Support Table
	//
	pBoard->pServList = (SBRD_ServList*)calloc( sizeof(SBRD_ServList), sGRL.itemReal );
	if( pBoard->pServList==NULL )
	{
		free( pSLI );
		return -1;
	}
	pBoard->nServListSize = sGRL.itemReal;

	//
	// Fill Service Support Table
	//
	for( ii=0; ii<pBoard->nServListSize; ii++ )
	{
		BRDC_strcpy( pBoard->pServList[ii].name, pSLI[ii].name );
		pBoard->pServList[ii].attr = pSLI[ii].attr;
		pBoard->pServList[ii].idxSuperServ = 0;

		pCaptor = new CCaptor( pBoard->boardName, pBoard->pServList[ii].name );
		if( pCaptor!=NULL )
			if( !pCaptor->IsValid() )
			{
				delete pCaptor;
				pCaptor = NULL;
			}
		if( pCaptor!=NULL )
		{
			g_captorCollection.reg( pCaptor );
		}
		pBoard->pServList[ii].ptrCaptor = (void*)pCaptor;
	}
	free( pSLI );

	return 0;
}

//=****************** Brd_Serv_DestroyBoardList ***********
//*********************************************************
S32  Brd_Serv_DestroyBoardList( TBRD_Board *pBoard )
{
	int			ii;

	if( pBoard->pServList==NULL )
	{
		pBoard->nServListSize = 0;
		return 0;
	}

	//
	// Clear Service Support Table
	//
	for( ii=0; ii<pBoard->nServListSize; ii++ )
	{
		if( pBoard->pServList[ii].ptrCaptor!=NULL )
			g_captorCollection.unreg( (CFineDestructor*)pBoard->pServList[ii].ptrCaptor );
		pBoard->pServList[ii].ptrCaptor = NULL;
	}

	//
	// Free Service Support Table
	//
	free( pBoard->pServList );
	pBoard->pServList = NULL;
	pBoard->nServListSize = 0;

	return 0;
}


//=********************** Brd_Serv_Capture ****************
//*********************************************************
S32  Brd_Serv_Capture( BRD_Handle handle, BRD_Handle *pNewHandle, U32 nodeId, U32 *pMode, const BRDCHAR *servName, U32 timeout )
{
	TBRD_Board		*pBoard;
	BRD_Handle		baseHandle = BRD_mkBaseHandle( handle );
	BRD_Handle		newHandle;
	U32				mode = *pMode;
	S32				err;
	S32				idx = -1;
	int				ii;

	if( mode!=BRDcapt_EXCLUSIVE &&
		mode!=BRDcapt_SHARED &&
		mode!=BRDcapt_SPY )
		return BRDERR(BRDerr_BAD_PARAMETER);

	//
	// Get Board
	//
	err = Brd_GetPtrBoard( baseHandle, &pBoard, 1 );
	if( err<0 )
		return err;

	//
	// Search Service Name
	//
	for( ii=0; (ii<pBoard->nServListSize) && (ii<0x7F); ii++ )
	{
		if( 0== BRDC_stricmp( pBoard->pServList[ii].name, servName ) ) //_stricmp
		{
			idx = ii;
			break;
		}
	}

	//
	// If No Such Service Name
	//
	if( idx<0)
		return BRDERR(BRDerr_BAD_PARAMETER);

	//
	// Check Service Avialability
	//
	DEV_CMD_Ctrl			param;
	SERV_CMD_IsAvailable	sSA;
	sSA.attr = 0;

	param.handle = baseHandle | (idx<<16);
	param.nodeId = nodeId;
	param.cmd = SERVcmd_SYS_ISAVAILABLE;
	param.arg = &sSA;
	Brd_CallDriver( baseHandle, DEVcmd_CTRL, &param, 1 );

	if( sSA.isAvailable==0 )
		return BRDERR(BRDerr_BAD_PARAMETER);

	//
	// Use Captor
	//
	CCaptor *pCaptor = (CCaptor*)pBoard->pServList[ii].ptrCaptor;
	int		captCounter;

	if( pCaptor==NULL )
		mode = BRDcapt_SPY;
	if( (mode==BRDcapt_SHARED) && (pBoard->pServList[ii].attr & BRDserv_ATTR_EXCLUSIVE_ONLY) )
		mode = BRDcapt_EXCLUSIVE;

	captCounter = -1;
	if( mode==BRDcapt_SHARED )
		captCounter = pCaptor->WaitForShared( timeout );
	if( mode==BRDcapt_EXCLUSIVE )
		captCounter = pCaptor->WaitForExclusive( timeout );
	if( captCounter < 0 )
	{
		captCounter = pCaptor->GetCounter();
		mode = BRDcapt_SPY;
	}

	//
	// Form Handle
	//
	newHandle  = baseHandle;
	newHandle |= (idx&0x7F) << 16;
	newHandle |= (mode&0x3) << 23;
	newHandle |= (captCounter&0x3F) << 25;

	//
	// Return Results
	//
	*pMode = mode;
	*pNewHandle = newHandle;

    return BRDERR(BRDerr_OK);
}

//=********************** Brd_Serv_Release ****************
//*********************************************************
S32  Brd_Serv_Release( BRD_Handle servHandle, U32 nodeId )
{
	// nodeId = nodeId;
	TBRD_Board		*pBoard;
	U32				handMode = (servHandle>>23) & 0x3;
	U32				captMode;
	U32				handCounter = (servHandle>>25) & 0x3F;
	U32				captCounter;
	S32				err;
	S32				idx = (servHandle>>16) & 0x7F;

	if( handMode == BRDcapt_SPY )
		return BRDERR(BRDerr_OK);

	//
	// Get Board
	//
	err = Brd_GetPtrBoard( servHandle, &pBoard, 1 );
	if( err<0 )
		return err;

	//
	// Use Captor
	//
	CCaptor *pCaptor = (CCaptor*)pBoard->pServList[idx].ptrCaptor;

	if( pCaptor==NULL )
		return BRDERR(BRDerr_BAD_HANDLE);
	captMode    = pCaptor->GetCaptureMode();
	captCounter = pCaptor->GetCounter();
	if( handMode!=captMode || handCounter!=(captCounter&0x3F) )
		return BRDERR(BRDerr_BAD_HANDLE);

	pCaptor->Release();

	return BRDERR(BRDerr_OK);
}

//=********************** Brd_Serv_List *******************
//*********************************************************
S32  Brd_Serv_List( BRD_Handle handle, U32 nodeId, BRD_ServList *pList, U32 item, U32 *pItemReal )
{
	TBRD_Board				*pBoard;
	S32						err;
	U32						itemReal;
	int						ii;

	//
	// Get Board
	//
	err = Brd_GetPtrBoard( handle, &pBoard, 1 );
	if( err<0 )
		return err;

	//
	// Fill List BRDI_feof
	//
	itemReal = 0;
	for( ii=0; ii<pBoard->nServListSize && ii<0x7F; ii++ )
	{
		DEV_CMD_Ctrl			param;
		SERV_CMD_IsAvailable	sSA;
		sSA.attr = 0;

		//
		// Check Service Visibility
		//
		if( pBoard->pServList[ii].attr & BRDserv_ATTR_UNVISIBLE )
			continue;

		//
		// Check Service Avialability
		//
		param.handle = handle | (ii<<16);
		param.nodeId = nodeId;
		param.cmd = SERVcmd_SYS_ISAVAILABLE;
		param.arg = &sSA;
		Brd_CallDriver( handle, DEVcmd_CTRL, &param, 1 );

		if( sSA.isAvailable==0 )
			continue;
		//
		// Put Service to List
		//
		if( (itemReal<item) && (pList!=NULL) )
		{
			BRDC_strcpy( pList[itemReal].name, pBoard->pServList[ii].name );
			pList[itemReal].attr = (sSA.attr) ? sSA.attr : pBoard->pServList[ii].attr;
		}
		itemReal++;
	}

	*pItemReal = itemReal;

	return BRDERR(BRDerr_OK);
}

//=********************** Brd_Serv_Check ******************
//*********************************************************
S32  Brd_Serv_Check( BRD_Handle handle, U32 nodeId )
{
	// nodeId = nodeId;

	TBRD_Board		*pBoard;
	U32				handMode;
	U32				captMode;
	U32				handCounter;
	U32				captCounter;
	S32				err;
	S32				idx;

	handMode = (handle>>23) & 0x3;
	if( handMode == BRDcapt_SPY )
		return BRDERR(BRDerr_OK);

	//
	// Get Board
	//
	err = Brd_GetPtrBoard( handle, &pBoard, 1 );
	if( err<0 )
		return err;

	//
	// Check Service List
	//
	if( NULL==pBoard->pServList )
		return BRDERR(BRDerr_BAD_HANDLE);

	//
	// Use Captor
	//
	idx = (handle>>16) & 0x7F;

	CCaptor *pCaptor = (CCaptor*)pBoard->pServList[idx].ptrCaptor;

	if( pCaptor==NULL )
		return BRDERR(BRDerr_BAD_HANDLE);
	captMode    = pCaptor->GetCaptureMode();
	captCounter = pCaptor->GetCounter();
	handCounter = (handle>>25) & 0x3F;
	if( handMode!=captMode || handCounter!=(captCounter&0x3F) )
		return BRDERR(BRDerr_BAD_HANDLE);

	return BRDERR(BRDerr_OK);
}
//
// End of File
//
