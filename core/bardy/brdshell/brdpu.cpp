//=********************************************************
//
// BRDPU.CPP
//
// BRD PU Support Helper function definitions:
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
#include	"brdpu.h"
#include	"brderr.h"


//=***************** Brd_Pu_CreateBoardList ***************
//*********************************************************
S32		Brd_Pu_CreateBoardList( int idx )
{
	TBRD_Board			*pBoard = &boardArr[idx];
	DEV_CMD_PuList		rPL;
	BRD_PuList			*paPLI;
	U32					item;
	int					ii;

	//
	// Init PU Support Table
	//
	pBoard->paPuList = NULL;
	pBoard->nPuListSize = 0;

	//
	// Get Number of PUs
	//
	rPL.pList = NULL;
	rPL.item  = 0;
	rPL.pItemReal = &item;
	Brd_CallDriver( idx+1, DEVcmd_PULIST, &rPL, 0 );

	//
	// If There are no PUs on The Board
	//
	if( item==0 )
		return 0;

	//
	// Get PU Information
	//
	paPLI = (BRD_PuList*)calloc( sizeof(BRD_PuList), item );
	if( paPLI==NULL )
		return -1;
	rPL.pList = paPLI;
	rPL.item  = item;
	rPL.pItemReal = &item;
	Brd_CallDriver( idx+1, DEVcmd_PULIST, &rPL, 0 );

	//
	// Create PU Support Table
	//
	pBoard->paPuList = (SBRD_PuList*)calloc( sizeof(SBRD_PuList), item );
	if( pBoard->paPuList==NULL )
	{
		free( paPLI );
		return -1;
	}
	pBoard->nPuListSize = item;

	//
	// Fill PU Support Table
	//
	for( ii=0; ii<pBoard->nPuListSize; ii++ )
	{
		pBoard->paPuList[ii].puId = paPLI[ii].puId;
		pBoard->paPuList[ii].puCode = paPLI[ii].puCode;
		pBoard->paPuList[ii].puAttr = paPLI[ii].puAttr;
		pBoard->paPuList[ii].puEnable = 0;
	}
	free( paPLI );

	return 0;
}

//=****************** Brd_Pu_DestroyBoardList *************
//*********************************************************
S32  Brd_Pu_DestroyBoardList( TBRD_Board *pBoard )
{
	if( pBoard->paPuList==NULL )
	{
		pBoard->nPuListSize = 0;
		return 0;
	}

	//
	// Free PU Support Table
	//
	free( pBoard->paPuList );
	pBoard->paPuList = NULL;
	pBoard->nPuListSize = 0;

	return 0;
}

//=****************** Brd_Pu_SetWriteEnable ***************
//*********************************************************
S32  Brd_Pu_SetWriteEnable( TBRD_Board *pBoard, U32 puId )
{
	int			ii;

	for( ii=0; ii<pBoard->nPuListSize; ii++ )
	{
		if( pBoard->paPuList[ii].puId == puId )
		{
			pBoard->paPuList[ii].puEnable = 1;
			return 0;
		}
	}

	return -1;
}

//=****************** Brd_Pu_IsWriteEnable ****************
//*********************************************************
S32  Brd_Pu_IsWriteEnable( TBRD_Board *pBoard, U32 puId )
{
	int			ii;

	for( ii=0; ii<pBoard->nPuListSize; ii++ )
	{
		if( pBoard->paPuList[ii].puId == puId )
		{
			if( pBoard->paPuList[ii].puAttr & BRDpua_Danger )
				if( 0 == pBoard->paPuList[ii].puEnable )
					return 0;
			pBoard->paPuList[ii].puEnable = 0;
			return 1;
		}
	}

	return 0;
}

//
// End of File
//


