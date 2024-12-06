//=*******************************************************
//
// IDXBRD.CPP - Class CIdxBrdCollection
//
// BRD Initialization Helper functions definition.
//
// (C) Instrumental Systems 2002-2004
//
// Created: Ekkore Aug 2004
//
//=*******************************************************

#include	<malloc.h>
#include	<stdio.h>
#include	<string.h>

#include	"utypes.h"
#include	"idxbrd.h"

//
//====== Types: CIdxBrdCollection
//
/*
	struct
	{
		char	name[32];
		int		brdIdx;
	} CIdxBrdItem;
*/
//=******* CIdxBrdCollection::CIdxBrdCollection ***********
//=********************************************************
	CIdxBrdCollection::CIdxBrdCollection( int aMaxsize )
{
	// aMaxsize = aMaxsize;
	maxsize = 64;
	paIdxBrdItem = (CIdxBrdItem*)calloc( maxsize, sizeof(CIdxBrdItem) );
}

//=******* CIdxBrdCollection::~CIdxBrdCollection **********
//=********************************************************
	CIdxBrdCollection::~CIdxBrdCollection(void)
{
	if( paIdxBrdItem!=NULL )
		free( paIdxBrdItem );
}

//=******* CIdxBrdCollection::Clear ***********************
//=********************************************************
void	CIdxBrdCollection::Clear(void)
{
	int			ii;

	if( paIdxBrdItem==NULL )
		return;

	for( ii=0; ii<maxsize; ii++ )
	{
		paIdxBrdItem[ii].name[0] = '\0';
		paIdxBrdItem[ii].brdIdx = 0;
	}
}

//=******* CIdxBrdCollection::PutName *********************
//=********************************************************
void	CIdxBrdCollection::PutName( BRDCHAR *pName )
{
	int			ii;

	if( paIdxBrdItem==NULL )
		return;

	for( ii=0; ii<maxsize; ii++ )
	{
		if( paIdxBrdItem[ii].name[ii] == '\0' )
		{
			//_tcsncpy( paIdxBrdItem[ii].name, pName, 32 );
			BRDC_strncpy( paIdxBrdItem[ii].name, pName, 32 );
			paIdxBrdItem[ii].brdIdx = 0;
		}
	}
}

//=******* CIdxBrdCollection::GetIdxBrd *******************
//=********************************************************
int		CIdxBrdCollection::GetIdxBrd( BRDCHAR *pName )
{
	int		item = GetItem( pName );

	if( paIdxBrdItem==NULL )
		return -2;

	if( item>=0 )
		return paIdxBrdItem[item].brdIdx;
	return -1;
}

//=******* CIdxBrdCollection::IncrementIdxBrd *************
//=********************************************************
void	CIdxBrdCollection::IncrementIdxBrd( BRDCHAR *pName )
{
	int		item = GetItem( pName );

	if( paIdxBrdItem==NULL )
		return;

	if( item>=0 )
		paIdxBrdItem[item].brdIdx++;
}

//=******* CIdxBrdCollection::GetItem *********************
//=********************************************************
int		CIdxBrdCollection::GetItem( BRDCHAR *pName )
{
	int			ii;

	if( paIdxBrdItem==NULL )
		return -2;

	for( ii=0; ii<maxsize; ii++ )
	{
		if( BRDC_stricmp( paIdxBrdItem[ii].name, pName) == 0 )
			return ii;
	}
	return -1;
}



//
// End of File
//

