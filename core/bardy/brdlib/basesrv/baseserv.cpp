//=********************************************************
// BSERV.CPP - BARDY Service Support Definitions
//
// Class Definitions:
//    SERV_Collection
//    SERV_ServiceCollection
//=********************************************************

#include	<string.h>

#include	"utypes.h"
#include	"baseserv.h"
#include	"brderr.h"

//=********************************************************
//=********************************************************
//=********************************************************
//
// SERV_Collection
//

//=********** SERV_Collection::SERV_Collection ************
//=********************************************************
	SERV_Collection::SERV_Collection( S32 itemSize, S32 maxSize, S32 delta )
{
	m_itemSize = itemSize;
	m_size     = 0;
	m_maxSize  = maxSize;
	m_delta    = delta;

	if( m_maxSize < 2 )
		m_maxSize = 2;
	if( m_delta < 2 )
		m_delta = 2;
	
    m_ptr      = IPC_heapAlloc( m_itemSize*m_maxSize );

}

//=********** SERV_Collection::~SERV_Collection ***********
//=********************************************************
	SERV_Collection::~SERV_Collection(void)
{
	if( m_ptr )
        IPC_heapFree( m_ptr );
}

//=********** SERV_Collection::Include ********************
//=********************************************************
S32		SERV_Collection::Include( void *pItem )
{
	if( m_size>=m_maxSize )
	{
        void	*ptr = IPC_heapAlloc( m_itemSize*(m_maxSize+m_delta) );

		if( ptr==NULL )
			return -1;
		memcpy( ptr, m_ptr, m_itemSize*m_maxSize );
        IPC_heapFree( m_ptr );
		m_ptr = ptr;
		m_maxSize = m_delta;
	}

	void *pNextItem = ((S08*)m_ptr) + (m_size*m_itemSize);

	memcpy( pNextItem, pItem, m_itemSize );
	m_size++;

	return 0;
}

//=********************************************************
//=********************************************************
//=********************************************************
//
// SERV_ServiceCollection
//


//=*** SERV_ServiceCollection::SERV_ServiceCollection ***
//=********************************************************
SERV_ServiceCollection::SERV_ServiceCollection(S32 maxSize, S32 delta ) 
	: SERV_Collection(sizeof(SERV_ServiceListItem), maxSize, delta )
{ 
}

//=*** SERV_ServiceCollection::~SERV_ServiceCollection **
//=********************************************************
SERV_ServiceCollection::~SERV_ServiceCollection(void)
{
	SERV_ServiceListItem	*pSLI;
	S32						itemNo;

	for( itemNo=0; itemNo<GetSize(); itemNo++ )
	{
		pSLI = GetItem(itemNo);
		if( pSLI->pCmd )
            IPC_heapFree( pSLI->pCmd );
	}
}

//=********* SERV_ServiceCollection::IncludeService *****
//=********************************************************
S32		SERV_ServiceCollection::IncludeService( const BRDCHAR *pName, U32 attr, S32 idxMain, void *pServData, SERV_CmdListItem *pCmd, S32 cmdSize )
{
	SERV_ServiceListItem	sRLI;

	//
	// Fill Item
	//
	BRDC_strncpy( sRLI.name, pName, 16 );
	sRLI.attr = attr;
	sRLI.idxMain = idxMain;
	sRLI.pServData = pServData;
    sRLI.pCmd = (SERV_CmdListItem*)IPC_heapAlloc( sizeof(SERV_CmdListItem)*(cmdSize+1) );
	if( sRLI.pCmd==NULL )
		return -1;
	memcpy( sRLI.pCmd, pCmd, sizeof(SERV_CmdListItem)*cmdSize );

	//
	// Include Item
	//
	if( 0>Include( &sRLI ) )
	{
        IPC_heapFree( sRLI.pCmd );
		return -1;
	}
	return 0;
}

//=********* SERV_ServiceCollection::GetServiceList *****
//=********************************************************
S32		SERV_ServiceCollection::GetServiceList( SIDE_CMD_GetServList *pGSL )
{
	SERV_ServiceListItem	*pSLI;
	int						itemNo;

	pGSL->itemReal = GetSize();
	if( (S32)pGSL->item < GetSize() )
		return BRDerr_BUFFER_TOO_SMALL;

	//
	// Fill Array
	//
	for( itemNo=0; itemNo<GetSize(); itemNo++ )
	{
		pSLI = GetItem(itemNo);
		BRDC_strncpy( pGSL->pSLI[itemNo].name, pSLI->name, 16 );
		pGSL->pSLI[itemNo].attr    = pSLI->attr;
		pGSL->pSLI[itemNo].idxMain = pSLI->idxMain;
	}

	return BRDerr_OK;
}

//=********* SERV_ServiceCollection::SetServiceList *****
//=********************************************************
S32		SERV_ServiceCollection::SetServiceList( SIDE_CMD_SetServList *pSSL )
{
	SERV_ServiceListItem	*pSLI;
	int						itemNo;

	for( itemNo=0; itemNo<GetSize() && itemNo<(S32)pSSL->item; itemNo++ )
	{
		pSLI = GetItem(itemNo);
		BRDC_strncpy( pSLI->name, pSSL->pSLI[itemNo].name, 16 );
		pSLI->attr    = pSSL->pSLI[itemNo].attr;
		pSLI->idxMain = pSSL->pSLI[itemNo].idxMain;
	}

	return 0;
}

//=********* SERV_ServiceCollection::Ctrl ****************
//=********************************************************
S32		SERV_ServiceCollection::Ctrl( BRD_Handle handle, void *pSide, void *pContext, U32 cmd, void *args )
{
	SERV_ServiceListItem	*pSLI;
	int						itemNo;
	int						ii;

	S32				idxServ = (handle >> 16) & 0x3F;
	U32				mode   = (handle >> 23) & 0x3;


	//
	// Search Service
	//
	for( itemNo=0; itemNo<GetSize(); itemNo++ )
	{
		pSLI = GetItem(itemNo);
		if( pSLI->idxMain == idxServ )
		{
			break;
		}
	}
	if( itemNo >= GetSize() )
		return BRDerr_BAD_HANDLE;

	//
	// Search Command
	//
	for( ii=0; pSLI->pCmd[ii].cmd!=0; ii++ )
	{
		if( pSLI->pCmd[ii].cmd == cmd )
			break;
	}
	if( pSLI->pCmd[ii].cmd==0 )
		return BRDerr_CMD_UNSUPPORTED;

	//
	// Check Capture Mode
	//
	if( mode==BRDcapt_SPY && pSLI->pCmd[ii].isSpy==0 )
		return BRDerr_BAD_MODE;

	return pSLI->pCmd[ii].pEntry( handle, pSide, pSLI->pServData, pContext, cmd, args);
}


//
// End of File
//


 
