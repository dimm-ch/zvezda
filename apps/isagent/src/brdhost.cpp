#include "netcmn.h"

#include "ctpapi.h"
#include "winnet.h"
#include "io.h"


#include "host.h"
#include "_brd.h"

#include "ctrladmpro/ctrlreg.h"
#include "ctrlstrm.h"

#include "gipcy.h"

#define MAX_BOARDS 4

U08 g_aCycle[2] = {77, 77};

extern FILE* islog;

class CTPhost_BRD : public CTPhost
{
protected:

	BRD_Handle hBrd;
	BRD_Handle hReg;

	BRD_Handle hStream;//one stream per socket
	void **ppBlk;

    int m_nRecvBufCount;
	int m_nBlkNum;
    volatile int m_isStart;
    BRDstrm_Stub *m_pStub;

public:
	CTPhost_BRD()
	{
		hBrd = 0;
		hReg = 0;
		hStream = 0;

        m_nRecvBufCount = 0;
		m_nBlkNum =0;
        m_isStart = 0;
		m_pStub = NULL;

		ppBlk = 0;
	}




	virtual ~CTPhost_BRD()
	{
		//close all opened

		if(  hStream )
			_BRD_release( hStream, 0 );

		if(  hReg )
			BRD_release( hReg, 0 );

		_BRD_close( hBrd );
		hBrd = 0;


		hReg = 0;
		hStream = 0;
	}





	virtual  S32 CTPhost_Open(U32 lid)
	{
		if (hBrd)
			return 0;

		U32 bmode = BRDopen_SHARED;
		hBrd = _BRD_open(lid, bmode, &bmode);

		if (hBrd < 0)
		{
			fprintf(islog, "_BRD_open 0x%x\n", hBrd );
			return hBrd;
		}

		U32 mode = BRDcapt_SHARED;
		hReg = _BRD_reg( hBrd );

		if (hReg < 0)
		{
			fprintf(islog, "_BRD_reg 0x%x\n", hReg);

			hReg = 0;
		}

		return hBrd;
	}

	virtual  S32 CTPhost_Stream( BRD_Handle h, U32 dir, U32 src, U32 size, U32 blknum, U32 isCycle )
	{
		if( hStream && m_pStub != NULL )
		{
			return 0;
		}

// 		if(blknum == 1)
// 			blknum++;

		if( blknum != 0)
			ppBlk = new PVOID[blknum];

		BRDctrl_StreamCBufAlloc buf_dscr;

		buf_dscr.dir = dir;

#ifndef __linux__
		buf_dscr.isCont = 1;
#else
        buf_dscr.isCont = 1;
#endif

        buf_dscr.blkNum = blknum;
        buf_dscr.blkSize = size;
		buf_dscr.ppBlk = (void**)ppBlk;
        buf_dscr.pStub = NULL;

		m_nBlkNum = blknum;





		hStream = _BRD_captureStream( hBrd, src, &buf_dscr, 1000 );

        ppBlk = buf_dscr.ppBlk;

		if( hStream == 0 )
			return -1;

        m_pStub = buf_dscr.pStub;

		//FIXME
        //U32 drq = BRDstrm_DRQ_HALF;

		//BRD_ctrl( hStream, 0, BRDctrl_STREAM_SETDRQ, &drq);







		return 0;
	}

	virtual S32 CTPhost_SetCycle( U32 dir, U32 isCycle )
	{
		g_aCycle[dir - 1] = isCycle;

		return 0;
	}

	virtual S32 CTPhost_IsCycle( U32 dir )
	{
		return g_aCycle[dir - 1];
	}

	virtual S32 CTPhost_StartStream( U32 dir, U32 src, U32 size, U32 blknum, U32 isCycle, U32 drq )
	{
		hStream = _BRD_findStream( hBrd, src );


		//BRDctrl_StreamCBufAdjust adj_pars;
		//adj_pars.isAdjust = 0; //  cycle


		//BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_ADJUST, &adj_pars); // ����� ���

//		BRD_ctrl( hStream, 0, BRDctrl_STREAM_SETSRC, &src);

		BRDctrl_StreamCBufStart start_pars;
        start_pars.isCycle = isCycle; //  cycle

		CTPhost_SetCycle(dir, isCycle);

        m_nRecvBufCount = 0;

		//BRD_ctrl( hStream, 0, BRDctrl_STREAM_RESETFIFO, NULL );

        BRD_ctrl( hStream, 0, BRDctrl_STREAM_SETSRC, &src);

        BRD_ctrl( hStream, 0, BRDctrl_STREAM_SETDRQ, &drq );

		if( dir == 1 )
		{
			S32 err = BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_START, &start_pars); // ����� ���
		}


		return 0;
	}

    virtual S32 CTPhost_StopStream(  U32 dir, U32 src, U32 size, U32 blknum, U32 isCycle )
    {
		hStream = _BRD_findStream(hBrd, src);

		if( hStream )
			return BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_STOP, 0);

		return BRDerr_CMD_UNSUPPORTED;
    }

	virtual S32 CTPhost_CBufAllocStream( U32 dir, U32 src, U32 size, U32 blknum, U32 isCycle )
	{
		auto err = CTPhost_Stream( 0, dir, src, size , blknum, isCycle);
		if (err == -1) return -1;

        BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_STOP, 0);

        m_pStub->totalCounter = 0;

		//BRDctrl_StreamCBufAdjust adj_pars;
		//adj_pars.isAdjust = 0; //  cycle


		//BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_ADJUST, &adj_pars); // ����� ���

		return 0;
	}

    virtual S32 CTPhost_CBufFreeStream( U32 dir, U32 src )
    {
		hStream = _BRD_findStream(hBrd, src);

		if(hStream == 0)
			return 0;

        //S32 err = BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_FREE, 0);

		//fprintf(logfile, "BRDctrl_STREAM_CBUF_FREE, 0x%X\n", err);

		hStream = 0;

        return 0;
    }

	virtual S32 CTPhost_StreamBuf( U32 dir, char **data, U32 size )
	{
        U32 nTotal = m_pStub->totalCounter;
        U32 nDif = m_nRecvBufCount - nTotal;
		U32	isCycle = CTPhost_IsCycle(dir);

		if( (dir == 2) && (!isCycle) )
		{ // ����� ������, ����������� �����
			*data = (char*)ppBlk[m_nRecvBufCount];
			m_nRecvBufCount++;
			return 0;
		}else if( dir == 2  )
		{
            if(m_nRecvBufCount < 2)
            {
                *data = (char*)ppBlk[m_nRecvBufCount];
                m_nRecvBufCount++;

               //fprintf( logfile, "StreamBuf %d\n" , m_nRecvBufCount);
               //fprintf( logfile, "StreamBuf total = %d\n" , nTotal);
                return 0;
            }

            if(nDif > 1)
            {
                ULONG msTimeout = 20; // wait end input (20 s)
                ULONG nCnt = 0;
                S32 status;

                while(nCnt < 1000)
                {
                    status = BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_WAITBLOCK, &msTimeout);

#ifndef __linux__
                    if(BRD_errcmp(status, (S32)BRDerr_WAIT_TIMEOUT))
                    {
                        nCnt++;
                        continue;
                    }
                    else if(status == -1)
                    {
                        fprintf(islog, "return -1\n");
                        return -1;
                    }
#else
                    if(status == -1)
                    {
                        nCnt++;
                        continue;
                    }
#endif

                    break;
                }

                if(nCnt == 1000)
                {
                    fprintf(islog, "return -1\n");
                    return -1;
                }

 /*               if(BRD_errcmp(status, (S32)BRDerr_WAIT_TIMEOUT) || (status == -1))
                {
                    fprintf(logfile, "return -1\n");
                    return -1;
                }*/

                nTotal = m_pStub->totalCounter;
            }

            m_nRecvBufCount++;
            *data = (char*)ppBlk[(nTotal - 1) & 1];

            //fprintf( logfile, "StreamBuf %d\n" , m_nRecvBufCount);
            //fprintf( logfile, "StreamBuf total = %d\n" , nTotal);

            return 0;
		}

		return 0;
	}

	virtual S32 CTPhost_WorkStream( U32 dir, char **data, U32 size, U32 blknum )
	{
		if( !hStream )
			return -1;

		{
			BRDctrl_StreamCBufStart start_pars;
            start_pars.isCycle = CTPhost_IsCycle(dir); //  cycle

            S32 err = 0;

			if((dir == 2) && (!start_pars.isCycle))
			{
				if(m_nRecvBufCount < blknum)
					return 0;

				err = BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_START, &start_pars );
				int nTimeout = 20 * 1000;
				err = BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_WAITBUF, &nTimeout );

		        if(BRD_errcmp(err, (S32)BRDerr_WAIT_TIMEOUT))
				{
					fprintf(islog, "timeout!!!\n");
	    				return -1;
				}

				m_nRecvBufCount = 0;
			}
			else if((dir == 2) && (!m_isStart))
			 {
				 err = BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_START, &start_pars );
				 m_isStart++;
			 }

			if( err < 0 )
				return -1;

            if(dir == 2)
                return 0;
		}

		//do all the job
		while( true )
		{

			/*
			if( tickTimeout <= 0 )
			{
				BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_STOP, NULL );
				return -1;
			}
			*/

			ULONG msTimeout = 200; // wait end input (20 s)


			BRDctrl_StreamCBufState st;

			st.timeout = msTimeout;

			//S32 status = BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_STATE, &st);

			S32 status = BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_WAITBLOCK, &msTimeout);

            if(BRD_errcmp(status, (S32)BRDerr_WAIT_TIMEOUT))
			{
//                 IPC_delay( 10 );
// 				continue;
				return -1;
			}

			if(BRD_errcmp(status, BRDerr_OK))
			{
				break;
			}

			BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_STOP, NULL );
			return -1;

		}

		*data = (char*)ppBlk[0];


//		BRD_ctrl( hStream, 0, BRDctrl_STREAM_CBUF_WAITBLOCK, &msTimeout);

		return 0;
	}



	virtual S32 CTPhost_ResetFifoStream(U32 dir, U32 src)
	{
		hStream = _BRD_findStream(hBrd, src);

		m_nRecvBufCount = 0;

		//if we have stream -> reset it
		if( hStream )
			return BRD_ctrl( hStream, 0, BRDctrl_STREAM_RESETFIFO, NULL );

		return BRDerr_CMD_UNSUPPORTED;
	}


	virtual S32 tetrReadInd( U32 pid, U32 node,S32 tetr, S32 reg, void *dst, S32 size )
	{
		BRD_Reg arg = { tetr, reg, 0, 0, 0 };

		S32 ret = BRD_ctrl( hReg, 0, BRDctrl_REG_READIND, &arg );

		if (ret < 0)
			fprintf( islog, "REG_READIND 0x%x, 0x%x\n", hReg, ret );

		*((U32*)dst) = arg.val;



		return ret;
	}

	virtual S32 tetrWriteInd( U32 pid, U32 node,S32 tetr, S32 reg, void *dst, S32 size )
	{
		if (size <= 4)
		{
			BRD_Reg arg = { tetr, reg, *(U32*)dst, 0, 0 };

			return BRD_ctrl(hReg, 0, BRDctrl_REG_WRITEIND, &arg);
		}
		else
		{
			BRD_Reg arg = { tetr, reg, 0, dst, size };

			return BRD_ctrl(hReg, 0, BRDctrl_REG_WRITESIND, &arg);
		}


	}

	virtual S32 tetrReadDir( U32 pid, U32 node,S32 tetr, S32 reg, void *dst, S32 size )
	{
		if( size <= 4 )
		{
			BRD_Reg arg = { tetr, reg, 0, 0, 0 };

			BRD_ctrl( hReg, 0, BRDctrl_REG_READDIR, &arg );

			*((U32*)dst) = arg.val;

			return 0;
		}

		BRD_Reg arg = { tetr, reg, 0, dst, size };

		return BRD_ctrl( hReg, 0, BRDctrl_REG_READSDIR, &arg );
	}

	virtual S32 tetrWriteDir( U32 pid, U32 node,S32 tetr, S32 reg, void *dst, S32 size )
	{
		if( size <= 4 )
		{
			BRD_Reg arg = { tetr, reg, *(U32*)dst, 0, 0 };

			return BRD_ctrl( hReg, 0, BRDctrl_REG_WRITEDIR, &arg );
		}

		BRD_Reg arg = { tetr, reg, 0, dst, size };

		return BRD_ctrl( hReg, 0, BRDctrl_REG_WRITESDIR, &arg );
	}

	virtual S32 puRead( U32 puID, S32 offset, void *dst, S32 size )
	{
		return BRD_puRead( hBrd, puID, offset, dst, size );
	}

	virtual S32 puWrite( U32 puID, S32 offset, void *dst, S32 size )
	{
		return BRD_puWrite( hBrd, puID, offset, dst, size );
	}

	virtual S32 CTPhost_Close( U32 pid )
	{
        if( hBrd == (S32)pid )
		{
			hBrd = 0;

		}

		return _BRD_close( pid );
	}

	virtual S32 CTPhost_GetInfo( U32 lid, TCTP_GetInfo *info )
	{
		BRD_Info brdInfo;
		brdInfo.size = sizeof(BRD_Info);

		S32 nRet = (BRD_getInfo( lid, &brdInfo )!=DRVERR(BRDerr_OK));

		info->boardType = brdInfo.boardType;
		info->pid = brdInfo.pid;
		info->verMajor = brdInfo.verMajor;
		info->verMinor = brdInfo.verMinor;

#ifdef _WIN64
		wcstombs(info->name,brdInfo.name, sizeof(brdInfo.name));
#else
		BRDC_strcpy(info->name, brdInfo.name);
#endif


		return nRet;
	}

	virtual S32 CTPhost_UploadFile( char* filename, char *buffer, U32 size)
	{
		if( size )
		{
			FILE *f = fopen( filename, "wb");

			fwrite( buffer, 1, size, f );

			fclose( f );
		}

		return 0;
	}

	virtual S32 CTPhost_Load( U32 pid, U32 node, BRDCHAR *argv[32],U32 argc )
	{
		BRDCHAR *filename = argv[0];

		S32 nRet = (BRD_load( pid, node, filename,  argc ,argv)!=DRVERR(BRDerr_OK));

		return nRet;
	}

	virtual S32 CTPhost_PULoad( U32 pid, U32 node, BRDCHAR *filename )
	{
		U32 state;
		S32 nRet = (BRD_puLoad( pid, node, filename, &state )!=DRVERR(BRDerr_OK));

		return nRet;
	}

	virtual S32 CTPhost_GetMsg( U32 pid, U32 node,  void *data, U32 *size )
	{

		S32 nRet = (BRD_getMsg( pid, node, data, size, 1000 ));

		return nRet;
	}

	virtual S32 CTPhost_PutMsg( U32 pid, U32 node,  void *data, U32 size )
	{
		U32 nSize = size;
		S32 nRet = (BRD_putMsg( pid, node, data, &nSize, 1000 ));

		return nRet;
	}

	virtual S32 CTPhost_PUList( U32 pid,  TCTP_PuList *list, U32 nItems )
	{
		U32 nReal = 0;

		BRD_PuList tmplist[32];
        int nntmp = BRD_min(32, nItems);

		BRD_puList( pid, tmplist, nntmp, &nReal );

		for (int i = 0; i<nItems; i++)
		{
			list[i].puAttr = tmplist[i].puAttr;
			list[i].puCode = tmplist[i].puCode;
			list[i].puId = tmplist[i].puId;

#ifdef _WIN64
			wcstombs(list[i].puDescription, tmplist[i].puDescription, 128);
#else
			memcpy(list[i].puDescription, tmplist[i].puDescription, 128);
#endif

		}

		return nReal;
	}

	virtual S32 CTPhost_ReadFIFO( U32 pid, U32 node,  void *data, U32 size, S32 timeout )
	{
		S32 nRet = BRD_readFIFO( pid, node, 8,data, size, timeout );

		return nRet;
	}

	virtual S32 CTPhost_WriteFIFO( U32 pid, U32 node,  void *data, U32 size, S32 timeout )
	{
		S32 nRet = BRD_writeFIFO( pid, node, 8,data, size, timeout );


		return nRet;
	}

	virtual S32 CTPhost_WriteDPRAM( U32 pid, U32 node, U32 offset, void *data, U32 size )
	{
		S32 nRet = BRD_writeDPRAM( pid, node, offset, data, size );

		return nRet;
	}

	S32 CTPhost_ReadDPRAM( U32 pid, U32 node, U32 offset, void *data, U32 size )
	{
		S32 nRet = BRD_readDPRAM( pid, node, offset, data, size );

		return nRet;
	}

	virtual S32 CTPhost_WriteRAM( U32 pid, U32 node, U32 offset, void *data, U32 size )
	{
		S32 nRet = BRD_writeRAM( pid, node, offset, data, size,1 );

		return nRet;
	}

	virtual S32 CTPhost_ReadRAM( U32 pid, U32 node, U32 offset, void *data, U32 size )
	{
		S32 nRet = BRD_readRAM( pid, node, offset, data, size,1 );

		return nRet;
	}

	virtual S32 CTPhost_StartStop( U32 pid, U32 node, U32 flag )
	{
		S32 nRet = 0;

		if( flag )
			nRet = BRD_start( pid, node );
		else
			nRet = BRD_stop( pid, node );

		if( nRet < 0 )
			printf( "eeee" );

		return nRet;
	}

	 virtual  S32 CTPhost_signalSend(U32 pid, U32 nodeId, U32 sigId )
	 {
		 return BRD_signalSend( pid, nodeId, sigId );
	 }

	 virtual S32 CTPhost_signalGrab(U32 pid, U32 nodeId, U32 sigId, U32 *pSigCounter, U32 timeout )
	 {
		 return BRD_signalGrab( pid, nodeId, sigId, pSigCounter, timeout );
	 }

	 virtual S32 CTPhost_signalWait(U32 pid, U32 nodeId, U32 sigId, U32 *pSigCounter, U32 timeout )
	 {
		 return BRD_signalWait( pid, nodeId, sigId, pSigCounter, timeout );
	 }

	 virtual S32 CTPhost_signalIack(U32 pid, U32 nodeId, U32 sigId )
	 {
		 return BRD_signalIack( pid, nodeId, sigId );
	 }
};

CTPhost* CTPhost::getHost()
{
	return new CTPhost_BRD();
}
