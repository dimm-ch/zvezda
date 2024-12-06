//=*******************************************************
//
// CAPTOR.CPP - Class CCaptor
//
// BRD Initialization Helper functions definition.
//
// (C) Instrumental Systems 2002-2003
//
// Created: Ekkore Feb 2003
//
//=*******************************************************

#include	<malloc.h>
#include	<stdio.h>
#include	<string.h>
#include	<assert.h>
#include	"gipcy.h"
#include	"utypes.h"
#include	"captor.h"


CCaptorCollection	g_captorCollection;

//=********************* CCaptor::CCaptor ***************************
//=******************************************************************
CCaptor::CCaptor( const BRDCHAR *devName, const BRDCHAR *resName )
{
	BRDCHAR	szName[128];

	m_nCounter = 1;						// Release() hasn't been called yet
	m_nState = 0;						// Resource UNUSED

	//
	// Create Mutex, Event, SHAREDINFO
	//
	BRDC_sprintf( szName, _BRDC("%s%sCaptorMU"), devName, resName );
	m_hMutex = IPC_createMutex( szName, FALSE );
	assert( m_hMutex!=NULL );

	BRDC_sprintf( szName, _BRDC("%s%sCaptorEV"), devName, resName );
	m_hEvent = IPC_createEvent(szName, FALSE, FALSE);
	assert( m_hEvent!=NULL );

	BRDC_sprintf( szName, _BRDC("%s%sCaptorFM"), devName, resName );
	m_hFM = IPC_createSharedMemory( szName, sizeof(*m_pSI) );
	assert( m_hFM!=NULL );

	m_pSI = (SHAREDINFO*)IPC_mapSharedMemory( m_hFM );
	assert( m_pSI!=NULL );
}

//=********************* CCaptor::~CCaptor **************************
//=******************************************************************
CCaptor::~CCaptor( void )
{
	//
	// Clear Shared Data if need
	//
	if( m_nState )
	{
		IPC_captureMutex( m_hMutex, INFINITE );
		m_pSI->m_nState -= m_nState;
		m_nState = 0;
		if( 0==m_pSI->m_nState )
		{
			IPC_setEvent( m_hEvent );
		}
		IPC_releaseMutex( m_hMutex );
	}

	//
	// Remove Mutex, Event, SHAREDINFO
	//
	if( m_hMutex )
		IPC_deleteMutex( m_hMutex );
	if( m_hEvent )
		IPC_deleteEvent( m_hEvent );
	if( m_pSI )
		IPC_unmapSharedMemory( m_pSI );
	if( m_hFM )
		IPC_deleteSharedMemory( m_hFM );
}


//=********************* CCaptor::WaitForShared *********************
//=******************************************************************
int		CCaptor::WaitForShared( DWORD dwMilliseconds )
{
	int			wasTimeout = 0;

	for(;;)
	{
		//
		//=== Check Resource state
		//
		IPC_captureMutex( m_hMutex, INFINITE );
		if( m_pSI->m_nState >= 0 )
		{
			//
			//=== Resource is SHARED or UNUSED
			//
			if( m_pSI->m_nState == 0 )
				IPC_resetEvent( m_hEvent );

			m_pSI->m_nState++;
			m_nState++;
			IPC_releaseMutex( m_hMutex );

			return m_nCounter;
		}

		//
		//=== Resource is EXCLUSIVE
		//
		IPC_releaseMutex( m_hMutex );
		if( wasTimeout )
			break;
        if( IPC_WAIT_TIMEOUT==(U32)IPC_waitEvent( m_hEvent, dwMilliseconds ) )
			wasTimeout = 1;

	}

	return -1;
}

//=********************* CCaptor::WaitForExclusive ******************
//=******************************************************************
int		CCaptor::WaitForExclusive( DWORD dwMilliseconds )
{
	for(;;)
	{
		//
		//=== Check Resource state
		//
		IPC_captureMutex( m_hMutex, INFINITE );
		if( m_pSI->m_nState == 0 )
		{
			//
			//=== Resource is UNUSED
			//
			IPC_resetEvent( m_hEvent );

			m_pSI->m_nState = -1;
			m_nState = -1;
			IPC_releaseMutex( m_hMutex );

			return m_nCounter;
		}

		//
		//=== Resource is SHARED or EXCLUSIVE
		//
		IPC_releaseMutex( m_hMutex );
        if( (int)IPC_WAIT_TIMEOUT==IPC_waitEvent( m_hEvent, dwMilliseconds ) )
			break;
		

		//
		//=== Resource has just been made UNUSED
		//
		continue;
	}

	return -1;
}

//=********************* CCaptor::Release ***************************
//=******************************************************************
int		CCaptor::Release( void )
{
	IPC_captureMutex( m_hMutex, INFINITE );

	//
	//=== Resource is EXCLUSIVE
	//
	if( m_pSI->m_nState <= 0 )
	{
		m_pSI->m_nState = 0;
		m_nState = 0;
		m_nCounter = (m_nCounter+1) & 0xFFFF;
		IPC_setEvent( m_hEvent );		
	}

	//
	//=== Resource is SHARED
	//
	else if( m_pSI->m_nState >= 0 )
	{
		m_pSI->m_nState--;
		m_nState--;
		if( m_pSI->m_nState==0)
			IPC_setEvent( m_hEvent );		
		if( m_nState==0)
			m_nCounter = (m_nCounter+1) & 0xFFFF;
	}
	
	IPC_releaseMutex( m_hMutex );
	return 0;
}

//=********************* CCaptor::IsCapturedHandle ******************
//=******************************************************************
BOOL	CCaptor::IsCapturedHandle( void )
{
	if( m_nState==0 )
		return FALSE;
	return TRUE;
}

//=********************* CCaptor::IsValid ***************************
//=******************************************************************
BOOL	CCaptor::IsValid( void )
{
	if( m_hMutex && m_hEvent && m_hFM && m_pSI )
		return TRUE;
	return FALSE;
}


//
// End of File
//

