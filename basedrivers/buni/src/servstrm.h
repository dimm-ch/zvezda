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

class TStream;

typedef S32 (TStream::*StreamCmdMember)( void *pParam );
typedef CmdMap<StreamCmdMember> StreamCmdMap;

class TStream : public TSrv
{
private:
	BRDCHAR cname[32];

	static StreamCmdMap bmap;
	static void bentryConstructor(StreamCmdMap *list);

public:

	HANDLE m_hThread;

	CTP_DEV *pCTPStream;

    volatile long totalBlocksEvt;
    volatile long totalBufEvt;

    volatile long readyFlag;


	U32 m_blkSizeExcl;

	U32 m_isCycle;

	BRDstrm_Stub Stub;
	
	BRDstrm_StubPointer		m_stubPointerExcl;
	U08**	m_pBlockPointersExcl;	// Array of Block Pointers
	
	U32						m_blkNumExcl;			// Number of Blocks
	U32						m_dir;

    U08 					isRunning;

	U32 _src;
	U32						m_drq;
	
	U32 **ppBlk;

	U32 _id;

	TStream( int id = 0 );

	S32 Capture();

	S32 cmd_capture( void *param );
	S32 cmd_alloc( BRDctrl_StreamCBufAlloc *param );
	S32 cmd_free( void *param );

	S32 cmd_start( BRDctrl_StreamCBufStart *param );
	S32 cmd_stop( BRDctrl_StreamCBufStop *param );

	S32 cmd_waitBuf( BRDctrl_StreamCBufWaitBuf *param );
	S32 cmd_waitBlock( BRDctrl_StreamCBufWaitBlock *param );
	
	S32 cmd_setSrc( BRDctrl_StreamSetSrc *param );
	S32 cmd_setDrq( BRDctrl_StreamSetDrq *param );

	S32 cmd_resetfifo( void *pParam);

	void startOnce();
	void kill();

	void tick();

	virtual S32 Ctrl( S32 cmd,void *pParam );
};


//
// End of File
//


