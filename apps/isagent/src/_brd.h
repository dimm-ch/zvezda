#ifndef	__CTP_BRD_H_
#define	__CTP_BRD_H_


#include	<stdio.h>
#include	"utypes.h"
#include	"brd.h"
#include	"brderr.h"
#include	"ctrlstrm.h"
#include "ctrladmpro/ctrlreg.h"

enum _BRD_SERVICE
{
	_BRD_STREAM0 = 1,
	_BRD_STREAM1, 
	_BRD_STREAM2,
	_BRD_STREAM3

};

BRD_Handle		_BRD_open (U32 lid, U32 flag, void *ptr );
S32				_BRD_close(BRD_Handle handle ); 

BRD_Handle		_BRD_reg( BRD_Handle handle );

BRD_Handle		_BRD_captureStream(BRD_Handle handle, int src, BRDctrl_StreamCBufAlloc *pAlloc, U32 timeout );
S32				_BRD_release( BRD_Handle handle, U32 nodeId );

BRD_Handle		_BRD_findStream(BRD_Handle handle, int src);

S32				_BRD_cleanup( void );

#endif