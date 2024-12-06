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

extern FILE* islog;

class CTPBoard
{
public:

	BRD_Handle hBrd;
	BRD_Handle hReg;
};

class CTPStream
{
protected:
	BRD_Handle hReg;

public:
	U32 streamCtrl( U32 cmd, TIO in, TIO out );
};





U32 CTPStream::streamCtrl( U32 cmd, TIO in, TIO out )
{
	return 0;
}

thread_value __IPC_API ServerThread(void*   lpvThreadParm)
{
	int buffersize = 32768*4;
	static char *dataBuf = new char[buffersize];

	SERVER_THREAD_PARAM *pParam = (SERVER_THREAD_PARAM*)lpvThreadParm;

    IPC_handle SData = pParam->SData;

    if(!SData) {
        fprintf(islog, "%s(): Invalid socket handle\n", __FUNCTION__ );
        return 0;
    }


    int ServerThreadRunning = 1;

	CTPhost *srv = CTPhost::getHost();

	while( ServerThreadRunning )
	{
		fd_set ReadSet;
        timeval tval={2,MAX_CMDTIMEOUT};//less timeout

        IPC_FD_ZERO( &ReadSet );
        IPC_FD_SET( SData, &ReadSet );

		/*
		int maxfd = max(SData, SData)+1;

		select(maxfd,&ReadSet,0,0,&tval);

		if( !FD_ISSET( SData,&ReadSet ) )
		{
			//printf("%s, %d: %s: Error %d\n", __FILE__, __LINE__, __FUNCTION__, nErr );
			continue;
		}
		*/

		int nErr=IPC_select(SData, &ReadSet,0,0,&tval);
        if( nErr == 0 ) {
           continue;
        } else {
       //     fprintf(logfile, "%s(): IPC_select - nErr = %d\n", __FUNCTION__, nErr );
        }

		CTP_COMMAND cmd;

        int sizeb = IPC_recv( SData, (char *)&cmd, sizeof(CTP_COMMAND), 0);//TODO: recive to loop buffer
/*
        if(sizeb==IPC_SOCKET_ERROR)
        {
            S32 err =  IPC_sysError();
            printf("%s, err = %d, IPC_EWOULDBLOCK = %d\n", __FUNCTION__, err, IPC_EWOULDBLOCK );
            if( err != IPC_EWOULDBLOCK)
            {
                printf("%s, %d: %s: err != IPC_EWOULDBLOCK\n", __FILE__, __LINE__, __FUNCTION__ );
                break;
            } else {
                continue;
            }
        }
*/
        if( (sizeb == IPC_SOCKET_ERROR) || (sizeb == 0) )
		{
            printf("%s, %d: %s: Error in recv() %d\n", __FILE__, __LINE__, __FUNCTION__, sizeb );
            //IPC_shutdown(SData,IPC_SD_BOTH);
            //IPC_closeSocket(SData);
            //ServerThreadRunning = 0;
			break;
		}

        //fprintf(logfile, "%s, %d: %s: CMD_TAG %X, CMD.type = %d CMD.cnt = %d\n", __FILE__, __LINE__, __FUNCTION__, cmd.tag, cmd.type, cmd.cnt );

        if( cmd.tag != CTP_CMD_TAG ) {
            break;
        }

		int pid,lid,node,argc;
		int offset,datasize,target,timeout;
		char *buffer = 0;
		int nRet = 0;

		if( cmd.type == CTP_BRD_READSTREAM )
		{
			U32 dataSize = cmd.data[3];

			//fprintf(islog, "CTP_BRD_READSTREAM %x\n", SData);

			srv->CTPhost_Stream( cmd.data[0], cmd.data[1], cmd.data[2], 0, 0, 0 );



			char *pBuf = NULL;

			S32 err =srv->CTPhost_WorkStream( cmd.data[1], &pBuf, dataSize, 0 );

			cmd.data[DEF_RESULT] = err;

			//send ok
			sendDataBlock( SData, (char *)&cmd, sizeof(CTP_COMMAND) );

			if (err == 0)
				err = sendDataBlock( SData, pBuf, dataSize );


			continue;
		}
		else if( cmd.type == CTP_BRD_WRITESTREAM )
		{
			U32 dataSize = cmd.data[3];

			//fprintf(islog, "CTP_BRD_WRITESTREAM %x\n", SData);

            srv->CTPhost_Stream( cmd.data[0], cmd.data[1], 0, cmd.data[3], cmd.data[4], 0 );

			char *pBuf = NULL;

			S32 err;

			err =srv->CTPhost_StreamBuf( cmd.data[1], cmd.data[2] , &pBuf, dataSize );

			recvDataBlock( SData, pBuf, dataSize );

			err =srv->CTPhost_WorkStream( cmd.data[1], &pBuf, dataSize, cmd.data[4] );

			cmd.data[DEF_RESULT] = err;

			//send ok
			sendDataBlock( SData, (char *)&cmd, sizeof(CTP_COMMAND) );

			continue;
		}

		switch( cmd.type )
		{
		case  CTP_DISCONNECT:
			ServerThreadRunning = 0;

			break;

		case CTP_BRD_STARTSTREAM:

			//fprintf(islog, "CTP_BRD_STARTSTREAM %x\n", SData );

			nRet = srv->CTPhost_StartStream( cmd.data[1], cmd.data[2], cmd.data[3], cmd.data[6], cmd.data[4], cmd.data[5] );

			break;

        case CTP_BRD_STOPSTREAM:

			//fprintf(islog, "CTP_BRD_STOPSTREAM %x\n", SData);
			nRet = srv->CTPhost_StopStream( cmd.data[1], cmd.data[2], cmd.data[3], cmd.data[5], cmd.data[4] );

            break;

		case CTP_BRD_CBUFALLOCSTREAM:

			//fprintf(islog, "CTP_BRD_CBUFALLOCSTREAM %x\n", SData );

			nRet = srv->CTPhost_CBufAllocStream( cmd.data[1], cmd.data[2], cmd.data[3], cmd.data[5], cmd.data[4] );

			break;

        case CTP_BRD_CBUFFREESTREAM:

			//fprintf(islog, "CTP_BRD_CBUFFREESTREAM %x\n", SData );

			nRet = srv->CTPhost_CBufFreeStream( cmd.data[0], cmd.data[1]);

			break;

		case CTP_BRD_RESETFIFOSTREAM:

			//fprintf(islog, "CTP_BRD_RESETFIFOSTREAM %x %d %d\n", SData, cmd.data[0], cmd.data[1]);

			nRet = srv->CTPhost_ResetFifoStream(cmd.data[0], cmd.data[1]);

			break;

		case CTP_BRD_OPEN:
			lid =  cmd.data[0];

			nRet = srv->CTPhost_Open( lid );

			break;
		case CTP_BRD_GETINFO:
			lid =  cmd.data[0];
			datasize =  cmd.data[1];

			nRet = srv->CTPhost_GetInfo( lid,(TCTP_GetInfo*)dataBuf );

			sendDataBlock( SData, (char *)dataBuf, datasize );

			break;
		case CTP_BRD_CLOSE:
			pid =  cmd.data[0];

			//MessageBox( 0, __TEXT("eeee"), 0,0 );

			srv->CTPhost_Close( pid );


			break;
		case CTP_UPLOAD:
			datasize =  cmd.data[0];

			buffer = (char*)malloc( datasize );

			recvDataBlock( SData, dataBuf, DEF_CTPSTRING );

			BRDCHAR str[DEF_CTPSTRING];
			CTP_mbstobcs(str, dataBuf, DEF_CTPSTRING);

			recvDataBlock( SData, buffer, datasize );

			nRet = srv->CTPhost_UploadFile(str,buffer, datasize);

			free( buffer );
			break;
		case CTP_BRD_LOAD:
			pid =  cmd.data[0];
			node =  cmd.data[1];
			argc =  cmd.data[2];

			char *argv[32];

			recvDataBlock( SData, dataBuf, argc*DEF_CTPSTRING );

			for( int i=0;i<argc;i++)
				argv[i]=(char*)&dataBuf[i*DEF_CTPSTRING];

			nRet = srv->CTPhost_Load( pid, node, argv, argc );

			break;
		case CTP_BRD_START:
			pid =  cmd.data[0];
			node =  cmd.data[1];

			nRet = srv->CTPhost_StartStop( pid, node, 1 );

			break;
		case CTP_BRD_STOP:
			pid =  cmd.data[0];
			node =  cmd.data[1];

			nRet = srv->CTPhost_StartStop( pid, node, 0 );

			break;
		case CTP_BRD_PULOAD:
		{
			pid = cmd.data[0];
			node = cmd.data[1];

			recvDataBlock(SData, (char *)dataBuf, DEF_CTPSTRING);

			BRDCHAR str[DEF_CTPSTRING];
			CTP_mbstobcs(str, dataBuf, DEF_CTPSTRING);

			nRet = srv->CTPhost_PULoad(pid, node, str);

			break;
		}
		case CTP_BRD_PULIST:
			pid =  cmd.data[0];
			datasize =  cmd.data[1]; //FIXME not datasize actualy but cnt

			nRet = srv->CTPhost_PUList( pid, (TCTP_PuList*)dataBuf,datasize );

			sendDataBlock( SData, (char *)dataBuf, sizeof(TCTP_PuList)*datasize );

			break;
		case CTP_BRD_PUSTATE:
			pid =  cmd.data[0];
			target =  cmd.data[1]; //FIXME not datasize actualy but cnt

			U32 state;
			BRD_puState( pid, target, &state );

			nRet = state;

			break;

		case CTP_BRD_WRITE:
			{
				pid =  cmd.data[0];
				node =  cmd.data[1];
				target =  cmd.data[2];
				datasize =  cmd.data[3];
				offset = cmd.data[4];
				timeout = cmd.data[5];

				U32 devID = cmd.data[6];

				if( buffersize < datasize )
				{
					delete dataBuf;

					buffersize = datasize;
					dataBuf = new char[datasize];
				}

				recvDataBlock( SData, (char *)dataBuf, datasize );

				switch(target)
				{
				case CTP_IO_MSG:
					nRet = srv->CTPhost_PutMsg( pid, node, (char*)dataBuf,datasize );
					break;
				case CTP_IO_FIFO:
					nRet = srv->CTPhost_WriteFIFO( pid, node, (char*)dataBuf, datasize, timeout );
					break;
				case CTP_IO_RAM:
					nRet = srv->CTPhost_WriteRAM( pid, node, offset, (char*)dataBuf, datasize );
					break;
				case CTP_IO_DPRAM:
					nRet = srv->CTPhost_WriteDPRAM( pid, node, offset, (char*)dataBuf, datasize );
					break;

				case CTP_ADM_DIR:
					{
						nRet = srv->tetrWriteDir( pid, node, offset&0xFF, offset>>8, (char*)dataBuf, datasize );
						break;
					}
				case CTP_ADM_IND:
					{
						nRet = srv->tetrWriteInd( pid, node, offset&0xFF, offset>>8, (char*)dataBuf, datasize );
						break;
					}

				case CTP_IO_PU:
					{
						nRet = srv->puWrite( devID, offset, (char*)dataBuf, datasize );
						break;
					}
				}

				break;
			}
		case CTP_BRD_READ:
			{
				pid =  cmd.data[0];
				node =  cmd.data[1];
				target =  cmd.data[2];
				datasize =  cmd.data[3];
				offset = cmd.data[4];
				timeout = cmd.data[5];

				U32 devID = cmd.data[6];

				if( buffersize < datasize )
				{
					delete dataBuf;

					buffersize = datasize;
					dataBuf = new char[datasize];
				}

				switch(target)
				{
				case CTP_IO_MSG:
					nRet = srv->CTPhost_GetMsg( pid, node, (char*)dataBuf,(U32*)&datasize );
					break;
				case CTP_IO_FIFO:
					nRet = srv->CTPhost_ReadFIFO( pid, node, (char*)dataBuf,datasize, timeout );
					break;
				case CTP_IO_RAM:
					nRet = srv->CTPhost_ReadRAM( pid, node, offset, (char*)dataBuf, datasize );
					break;
				case CTP_IO_DPRAM:
					nRet = srv->CTPhost_ReadDPRAM( pid, node, offset, (char*)dataBuf, datasize );
					break;
				case CTP_ADM_DIR:
					{
						nRet = srv->tetrReadDir( pid, node, offset&0xFF, offset>>8, (char*)dataBuf, datasize );
						break;
					}
				case CTP_ADM_IND:
					{
						nRet = srv->tetrReadInd( pid, node, offset&0xFF, offset>>8, (char*)dataBuf, datasize );
						break;
					}

				case CTP_IO_PU:
					{
						nRet = srv->puRead( devID, offset, (char*)dataBuf, datasize );
						break;
					}
				}

				cmd.data[3] = datasize;

				sendDataBlock( SData, (char *)dataBuf, datasize);

				break;
			}
		case CTP_BRD_SIGNAL:
			pid =  cmd.data[0];
			node =  cmd.data[1];
			target =  cmd.data[2];

			switch(target)
			{
			case CTP_SIGNAL_SEND:
				nRet = srv->CTPhost_signalSend( pid, node, cmd.data[3]  );
				break;
			case CTP_SIGNAL_GRAB:
				nRet = srv->CTPhost_signalGrab( pid, node, cmd.data[3], &cmd.data[4], 100 );
				break;
			case CTP_SIGNAL_IACK:
				nRet = srv->CTPhost_signalIack( pid, node, cmd.data[3] );
				break;
			case CTP_SIGNAL_WAIT:
				nRet = srv->CTPhost_signalWait( pid, node, cmd.data[3], &cmd.data[4], 100 );
				break;
			}

			break;
		default:
			//MessageBox( 0, "eeee", 0,0 );
			fprintf(islog, "UNKNOWN MESSAGE\n" );
		}

		if(ServerThreadRunning == 0)
			break;

		cmd.data[DEF_RESULT] = nRet;
       // fprintf(logfile, "%s, %d: %s: sendDataBlock(ACK)...", __FILE__, __LINE__, __FUNCTION__ );
		sendDataBlock( SData, (char *)&cmd, sizeof(CTP_COMMAND) );
      //  fprintf(logfile, "complete\n" );
	}

    ServerThreadRunning = 0;

    IPC_shutdown(SData,IPC_SD_BOTH);
    IPC_closeSocket(SData);
    IPC_interlockedDecrement( &g_clCnt );

	free(pParam);

	delete srv;
	srv = NULL;

    fprintf(islog, "%s(%p) - finished\n", __FUNCTION__, lpvThreadParm);

	return 0;
}
