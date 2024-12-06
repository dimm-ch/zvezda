

#include "utypes.h"

#include "ctpapi.h"
#include "_brd.h"

class CTPhost
{
public:	



	virtual ~CTPhost()
	{

	}


	virtual S32 CTPhost_Open( U32 lid )
	{
		return 0;
	}

	virtual S32 CTPhost_Stream( BRD_Handle h, U32 dir, U32 src, U32 size , U32 blknum, U32 isCycle)
	{
		return 0;
	}

	virtual S32 CTPhost_SetCycle( U32 dir, U32 isCycle )
	{
		return 0;
	}

	virtual S32 CTPhost_IsCycle( U32 dir )
	{
		return 0;
	}

	virtual S32 CTPhost_StartStream( U32 dir, U32 src, U32 size, U32 blknum, U32 isCycle, U32 drq )
	{
		return 0;
	}

    virtual S32 CTPhost_StopStream(  U32 dir, U32 src, U32 size, U32 blknum, U32 isCycle )
    {
        return 0;
    }

	virtual S32 CTPhost_CBufAllocStream( U32 dir, U32 src, U32 size, U32 blknum, U32 isCycle )
	{
		return 0;
	}

    virtual S32 CTPhost_CBufFreeStream(U32 dir, U32 src )
    {
        return 0;
    }
	
	virtual S32 CTPhost_WorkStream( U32 dir, char **data, U32 size, U32 blknum )
	{
		return 0;
	}

	virtual S32 CTPhost_ResetFifoStream(U32 dir, U32 src)
	{
		return 0;
	}

	virtual S32 CTPhost_StreamBuf(U32 dir, U32 src, char **data, U32 size )
	{

		return 0;
	}


	virtual S32 tetrReadInd( U32 pid, U32 node,S32 tetr, S32 reg, void *dst, S32 size )
	{
		return 0;
	}

	virtual S32 tetrWriteInd( U32 pid, U32 node,S32 tetr, S32 reg, void *dst, S32 size )
	{
		return 0;
	}
	
	virtual S32 tetrReadDir( U32 pid, U32 node,S32 tetr, S32 reg, void *dst, S32 size )
	{
		return 0;
	}

	virtual S32 tetrWriteDir( U32 pid, U32 node,S32 tetr, S32 reg, void *dst, S32 size )
	{
		return 0;
	}
	
	virtual S32 puRead( U32 puID, S32 offset, void *dst, S32 size )
	{
		return 0;
	}
	
	virtual S32 puWrite( U32 puID, S32 offset, void *dst, S32 size )
	{
		return 0;
	}

	virtual S32 CTPhost_Close( U32 pid )
	{
		return 0;
	}

	virtual S32 CTPhost_GetInfo( U32 lid, TCTP_GetInfo *info )
	{
		return 0;
	}

	virtual S32 CTPhost_UploadFile( BRDCHAR* filename, char *buffer, U32 size)
	{
		if( size )
		{
			FILE *f = BRDC_fopen( filename, _BRDC("wb") );

			fwrite( buffer, 1, size, f );

			fclose( f );
		}

		return 0;
	}

	virtual S32 CTPhost_Load( U32 pid, U32 node,char *argv[32],U32 argc )
	{
		return 0;
	}

	virtual S32 CTPhost_PULoad( U32 pid, U32 node, BRDCHAR *filename )
	{
		return 0;
	}

	virtual S32 CTPhost_GetMsg( U32 pid, U32 node,  void *data, U32 *size )
	{
		return 0;
	}

	virtual S32 CTPhost_PutMsg( U32 pid, U32 node,  void *data, U32 size )
	{
		U32 nSize = size;
		S32 nRet = (BRD_putMsg( pid, node, data, &nSize, 1000 ));

		return nRet;
	}

	virtual S32 CTPhost_PUList( U32 pid, TCTP_PuList *list, U32 nItems )
	{
		return 0;
	}

	virtual S32 CTPhost_ReadFIFO( U32 pid, U32 node,  void *data, U32 size, S32 timeout )
	{
		S32 nRet = BRD_readFIFO( pid, node, 8,data, size, timeout ); 

		return nRet;
	}

	virtual S32 CTPhost_WriteFIFO( U32 pid, U32 node,  void *data, U32 size, S32 timeout )
	{
		return 0;
	}

	virtual S32 CTPhost_WriteDPRAM( U32 pid, U32 node, U32 offset, void *data, U32 size )
	{
		return 0;
	}

	virtual S32 CTPhost_ReadDPRAM( U32 pid, U32 node, U32 offset, void *data, U32 size )
	{
		return 0;
	}

	virtual S32 CTPhost_WriteRAM( U32 pid, U32 node, U32 offset, void *data, U32 size )
	{
		return 0;
	}

	virtual S32 CTPhost_ReadRAM( U32 pid, U32 node, U32 offset, void *data, U32 size )
	{
		return 0;
	}

	S32 CTPhost_StartStop( U32 pid, U32 node, U32 flag )
	{
		return 0;
	}

	virtual  S32 CTPhost_signalSend(U32 pid, U32 nodeId, U32 sigId )
	{
		return 0;
	}

	virtual  S32 CTPhost_signalGrab(U32 pid, U32 nodeId, U32 sigId, U32 *pSigCounter, U32 timeout )
	{
		return 0;
	}

	virtual  S32 CTPhost_signalWait(U32 pid, U32 nodeId, U32 sigId, U32 *pSigCounter, U32 timeout )
	{
		return 0;
	}

	virtual  S32 CTPhost_signalIack(U32 pid, U32 nodeId, U32 sigId )
	{
		return 0;
	}

	static CTPhost* getHost();


};
