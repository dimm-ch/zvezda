//=********************************************************
//
// SERVSTRM.CPP
//
// BRD Driver  for ADP24PCI Board.
//
// The file defines: 
//      BRD_RealStrm Class
//      Stream Support Helper Functions
//
// Used with B24.CPP
//
// (C) Instrumental Systems 
//
// Created by Ekkore Feb. 2005
//
//=********************************************************

#include	<malloc.h>
//#include	<windows.h>
//#include	<winioctl.h>

#include	"utypes.h"
#include	"brderr.h"
#include	"brdfunx.h"
#include	"buni.h"
#include	"ctrlstrm.h"
#include	"ctpapi.h"
#include	"servstrm.h"

//#include	"WinBase.h"

class StreamThread
{
private:
	volatile TStream *_RealStream;

	CTP_HOST *pH;
	CTP_DEV *pCTPStream;

	volatile BRDstrm_Stub *pStub;
	
	S32 blkNum;
public:

	
	
	StreamThread( TStream *RealStream )
	{
		_RealStream = RealStream;
		
		pStub = &_RealStream->Stub;
		
		//pCTPDev = NULL;
	}

	void tick( )
	{
		S32 ret;
		
		

		if (pStub->state != BRDstrm_STAT_RUN )
			return;

		S32 err;
	
		if ( _RealStream->m_dir == 1)
			err = CTP_ReadStream( pCTPStream, pH, _RealStream->_src, (void *) _RealStream->m_pBlockPointersExcl[blkNum], _RealStream->m_blkSizeExcl );
		if ( _RealStream->m_dir == 2 ) 
			err = CTP_WriteStream( pCTPStream, pH, _RealStream->_src, (void *) _RealStream->m_pBlockPointersExcl[blkNum], _RealStream->m_blkSizeExcl, _RealStream->m_blkNumExcl );
		
		if( err == -1 )
			return;

		//_RealStream->totalBlocksEvt++;
        IPC_interlockedIncrement( &_RealStream->totalBlocksEvt );
	

		pStub->totalCounter++;
		
		pStub->lastBlock = blkNum;
		blkNum++;
	


	}

    S32 loop( )
	{
		
		
		U32 ii = 0;
		S32 ret = 0;
		
		blkNum = 0;

		BUNI2Driver *owner = (BUNI2Driver*)_RealStream->owner;

		pH = CTP_Connect( ((BUNI2Driver*)owner)->ipAddr, ((BUNI2Driver*)owner)->ipPort, 0 );

		//use last stream
		pCTPStream = CTP_Open( pH, owner->brdId );
	
		_RealStream->isRunning = 1;

		while( _RealStream->isRunning)
		{
			if( pStub->state != BRDstrm_STAT_RUN )
			{
				while( pStub->state != BRDstrm_STAT_RUN )
                    IPC_delay( 10 );

				blkNum = 0;

				
			}
		
			
			
			tick();

			

			if( blkNum >= _RealStream->m_blkNumExcl )
			{
				if( _RealStream->m_isCycle )
				{
					blkNum = 0;
				}
				else
				{
					//pStub->state = BRDstrm_STAT_STOP;
                    IPC_interlockedExchange( (long*)&pStub->state, BRDstrm_STAT_STOP );
					//InterlockedExchange( &_RealStream->readyFlag, 0 );
				}

                IPC_interlockedIncrement( &_RealStream->totalBufEvt );
			}
			
		}

		/*
		if( pCTPDev )
		{
			CTP_Disconnect( pH );
	
			delete pCTPDev;
			pCTPDev = NULL;
		}*/

		return 0;
	}

    static thread_value __IPC_API entry(void* lpvThreadParm)
	{		
		StreamThread st( (TStream *) lpvThreadParm );

		st.loop();

		return 0;
	}
};

StreamCmdMap TStream::bmap((void*)bentryConstructor);

#define DEFINE_CMD( id, func ) { list->set( id, horrible_cast<StreamCmdMember>(&TStream::func) ); }

void TStream::bentryConstructor(StreamCmdMap *list)
{
	//DEFINE_CMD( SERVcmd_SYS_ISAVAILABLE, cmd_capture );
	DEFINE_CMD( SERVcmd_SYS_CAPTURE, cmd_capture );

	DEFINE_CMD( BRDctrl_STREAM_CBUF_ALLOC, cmd_alloc );
	DEFINE_CMD( BRDctrl_STREAM_CBUF_FREE, cmd_free );

	DEFINE_CMD( BRDctrl_STREAM_CBUF_START, cmd_start );
	DEFINE_CMD( BRDctrl_STREAM_CBUF_STOP, cmd_stop );

	DEFINE_CMD( BRDctrl_STREAM_CBUF_WAITBLOCK, cmd_waitBlock );
	DEFINE_CMD( BRDctrl_STREAM_CBUF_WAITBUF, cmd_waitBuf );
	
	DEFINE_CMD( BRDctrl_STREAM_SETSRC, cmd_setSrc );
	DEFINE_CMD( BRDctrl_STREAM_SETDRQ, cmd_setDrq );

	DEFINE_CMD( BRDctrl_STREAM_RESETFIFO, cmd_resetfifo );
}

TStream::TStream( int id )
{
	BRDC_sprintf( cname, _BRDC("BASESTREAM%d"), id );
	name = cname;

	_id = id;

	m_hThread = NULL;
	isRunning = 1;

	totalBlocksEvt = 0;
	totalBufEvt = 0;

	ppBlk = NULL;
		
}

S32 TStream::cmd_capture( void *param )
{
	return Capture();
}

S32 TStream::cmd_waitBlock( BRDctrl_StreamCBufWaitBlock *param )
{
	U32 timer = 0;

	while( totalBlocksEvt == 0 )
	{
        IPC_delay( 10 );

		if( param->timeout == 0xFFFFFFFF )
			continue;

		if( timer > param->timeout )
		{
			return BRDerr_WAIT_TIMEOUT;
		}

		timer+=10;
	}

    IPC_interlockedExchange( &totalBlocksEvt , 0 );
	
	return 0;
}

S32 TStream::cmd_waitBuf( BRDctrl_StreamCBufWaitBuf *param )
{
	U32 timer = 0;

	while( totalBufEvt == 0 )
	{
        IPC_delay( 10 );

		if( param->timeout == 0xFFFFFFFF )
			continue;

		if( timer > param->timeout )
		{
			return BRDerr_WAIT_TIMEOUT;
		}

		timer+=10;
	}

	//totalBufEvt = 0;
    IPC_interlockedExchange( &totalBufEvt , 0 );
	
	return 0;
}

S32 TStream::cmd_setSrc( BRDctrl_StreamSetSrc *param )
{
	_src = param->src;

	return 0;
}

S32 TStream::cmd_setDrq( BRDctrl_StreamSetDrq *param )
{
	m_drq = param->drq;

	return 0;
}

S32 TStream::cmd_resetfifo(void *pParam )
{
	BUNI2Driver *owner = (BUNI2Driver*)this->owner;

	CTP_ResetFifoStream((CTP_DEV*)owner->pCTPDev, m_dir, _src );

	return 0;
}

S32 TStream::cmd_alloc( BRDctrl_StreamCBufAlloc *param )
{
	
	
	param->pStub = &Stub;

	ppBlk = (U32**)param->ppBlk;

	if( !ppBlk )
	{
		ppBlk = (U32**)malloc( sizeof(void*)*param->blkNum );
	}

	for( int i=0; i<param->blkNum; i++ )
	{
		ppBlk[i] = (U32*)malloc( param->blkSize );
	}
	
	param->ppBlk = (void**)ppBlk;

	m_dir = param->dir;

	m_pBlockPointersExcl = (U08**)ppBlk;
	m_blkSizeExcl = param->blkSize;

	//Stub.

	m_blkNumExcl = param->blkNum;
	
	startOnce();



	return 0;
}

S32 TStream::cmd_free( void *param )
{
	if (ppBlk == NULL)
		return BRDerr_STREAM_NOT_ALLOCATED_YET;

	//kill();
	
	for( int i=0; i<m_blkNumExcl; i++ )
	{
		free( ppBlk[i] );
	}
	
	//free( ppBlk );
	ppBlk = NULL;

	BUNI2Driver *owner = (BUNI2Driver*)this->owner;

	CTP_CBufFreeStream( (CTP_DEV*)owner->pCTPDev, m_dir, _src );
	
	return 0;
}

void TStream::startOnce()
{
    if( m_hThread )
        if(IPC_waitThread(m_hThread, 0) != IPC_OK)
			return;

    m_hThread = IPC_createThread( _BRDC("StreamThread"), StreamThread::entry, this );
	if (m_hThread == NULL)
		printf("Cannot create StreamThread\n");
}

void TStream::kill()
{
	isRunning = FALSE;
	
    if( IPC_waitThread( m_hThread, 20 ) != IPC_OK )
            IPC_deleteThread( m_hThread );

    m_hThread = 0;
}

S32 TStream::cmd_start( BRDctrl_StreamCBufStart *param )
{
	
	
	cmd_stop( NULL );

	startOnce();	

	totalBlocksEvt = 0;
	totalBufEvt = 0;
	

	Stub.totalCounter = 0;
	Stub.lastBlock = -1;
	Stub.offset = 0;

	m_isCycle = param->isCycle;

	BUNI2Driver *owner = (BUNI2Driver*)this->owner;
		
	CTP_CBufAllocStream((CTP_DEV*)owner->pCTPDev, m_dir, _src, m_blkSizeExcl, m_blkNumExcl, m_isCycle);

	CTP_StartStream( (CTP_DEV*)owner->pCTPDev, m_dir, _src,  m_blkSizeExcl, m_blkNumExcl, m_isCycle, m_drq );

    IPC_interlockedExchange( &readyFlag , 0 );
    IPC_interlockedExchange( (long*)&Stub.state , BRDstrm_STAT_RUN );

	return 0;
}

S32 TStream::cmd_stop( BRDctrl_StreamCBufStop *param )
{
	if( Stub.state == BRDstrm_STAT_RUN )
	{
		//kill thread if unexpected stop
		kill();
	}

	Stub.state = BRDstrm_STAT_STOP;
	
	
	//close socket
	pCTPStream = NULL;

	BUNI2Driver *owner = (BUNI2Driver*)this->owner;

	CTP_StopStream( (CTP_DEV*)owner->pCTPDev, m_dir, _src,  m_blkSizeExcl, m_blkNumExcl, m_isCycle );

	return 0;
}

S32 TStream::Ctrl( S32 cmd,void *pParam )
{
	switch( cmd )
	{
	case SERVcmd_SYS_ISAVAILABLE:
		{
			SERV_CMD_IsAvailable		*pSA = (SERV_CMD_IsAvailable*)pParam;

			pSA->isAvailable = 1;
			pSA->attr = 0;
			return 0;
		}

	}
	
	StreamCmdMember pm = horrible_cast<StreamCmdMember>(bmap.get(cmd));
	

	
	if( pm == NULL )
		return BRDerr_CMD_UNSUPPORTED;

	return ((TStream*)this->*pm)( pParam );
}



S32 TStream::Capture( )
{
	
	
	return 0;
}



//
// End of File
//


