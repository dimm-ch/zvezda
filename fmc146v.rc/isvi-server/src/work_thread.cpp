#include "isvi_server.h"
#include "netcmn.h"
#include "netcmd.h"

#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <sys/wait.h>

#include "printf.h"

using namespace std;

int GetDataSize(char *pSuffix);
void GetBlksInfo(char *pSuffix, int *pNum, int *pSize);

//-----------------------------------------------------------------------------

void* isvi_server::server_processing_thread(void *data)
{
    int buffersize = 32768*4;
    static char *dataBuf = new char[buffersize];
    int res;
    int isDownload = 0;
    char sCmd[1024];
    int nCnt;
    pid_t pid;
    void *fileData = 0;
	int nFileSize = 0;
    list<string> lsProcs;

    vector<IPC_handle> vhDataBinMap;
	IPC_handle hDataFlgMap = 0;
	IPC_handle hSuffixMap = 0;

	U32  *pDataFlg = 0;
    vector<char*>vpDataBin;
	char *pSuffix = 0;

    struct srv_thread_param *tp = (struct srv_thread_param *)data;
    if(!tp) {
        Printf( "%s() Invalid data pointer\n", __FUNCTION__);
        return (void*)0x12345678;
    }

    isvi_server *object = (isvi_server*)tp->classObject;
    if(!object) {
        Printf( "%s() Invalid class pointer\n", __FUNCTION__);
        return (void*)0x12345678;
    }

    Printf( "%s() - started\n", __FUNCTION__);

    int SData = tp->acceptedSocket;
    bool exitFlag = false;

    while( !tp->exitFlag ) {

        fd_set ReadSet;
        timeval tval={0,5000};

        FD_ZERO( &ReadSet );
        FD_SET(tp->acceptedSocket,&ReadSet);
        int nErr = select(tp->acceptedSocket+1,&ReadSet,0,0,&tval);
        if((nErr==-1)||(nErr==0)) {
            continue;
        }

        NET_COMMAND cmd;

        memset(&cmd, 0, sizeof(NET_COMMAND));

        //Printf( "%s(): start recv()\n", __FUNCTION__);

        int sizeb = recv( SData, (char *)&cmd, sizeof(NET_COMMAND), 0);
        if( (sizeb == SOCKET_ERROR) || (sizeb == 0) ) {
            Printf( "%s(): error in recv()\n", __FUNCTION__);
            break;
        }

        //Printf( "%s(): sizeb = %d, cmd.tag = 0x%x\n", __FUNCTION__, sizeb, (int)cmd.tag );

        if( cmd.tag != NET_CMD_TAG ) {
            Printf( "%s(): Invalid NET_CMD_TAG\n", __FUNCTION__);
            break;
        }

        Printf( "%s(): start processing command %d ...\n", __FUNCTION__, cmd.type);

        memset(dataBuf, 0, buffersize);
        printf("printing case %d \n",cmd.type);
        switch(cmd.type)
        {
        case NET_EXEC:
        case NET_EXEC_WAIT:
        {
            // получим имя файла
            res = recvDataBlock( SData, dataBuf, cmd.data[0] );

            if(res != 0)
            {
                cmd.status = -1;
                sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));
                break;
            }

            dataBuf[cmd.data[0]] = '\0';

            pid = fork( ); /* если fork завершился успешно, pid > 0 в родительском процессе */
            if (pid > 0) {
             /* здесь располагается родительский код */

                 if(cmd.type == NET_EXEC)
                 {
                     char tname[256];
                     snprintf(tname,sizeof(tname),"server_wait_thread_%d\n", nCnt++);
                     tp->id = IPC_createThread(tname, server_wait_thread, (void *)&pid);
                     lsProcs.push_back(dataBuf);
                 }
                 else
                     server_wait_thread((void *)&pid);

                 cmd.status = 0;
                 sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));
            } else if(pid == 0){
             /* здесь располагается дочерний код */
                 //execlp(dataBuf, 0);
                 execl(dataBuf, dataBuf, (char *)0);
                 exit(-1);
             }

            break;
        }
        case NET_EXIT:
        {
            // получим имя файла
            res = recvDataBlock( SData, dataBuf, cmd.data[0] );

            if(res != 0)
            {
                cmd.status = -1;
                sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));
                break;
            }

            dataBuf[cmd.data[0]] = '\0';

            strcpy(sCmd, "killall -s SIGINT ");
            strcat(sCmd, dataBuf);

            system(sCmd);

            cmd.status = 0;

            sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));

            break;
        }
        case NET_UPLOAD: {
            int nFileSize;
            void *fileData;

            // получим имя файла
            res = recvDataBlock( SData, dataBuf, cmd.data[0] );

            dataBuf[cmd.data[0]] = '\0';

            if(res != 0)
            {
                cmd.status = -1;
                sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));
                break;
            }

            // получим размер файла
            res = recvDataBlock( SData, &nFileSize, sizeof(int) );

            if(res != 0)
            {
                cmd.status = -1;
                sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));
                break;
            }

            fileData = (char*)malloc( nFileSize );

            res = recvDataBlock( SData, fileData, nFileSize );

            if(res != 0)
            {
                cmd.status = -1;
                sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));
                free(fileData);
                break;
            }

            cmd.status = save_data_file( dataBuf, fileData, nFileSize);

            sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));

            free(fileData);

            break;
        }

        case NET_DOWNLOAD: {
            isDownload = 1;

            // получим имя файла
            res = recvDataBlock( SData, dataBuf, NET_STRING );

            if(res != 0)
                break;

            dataBuf[NET_STRING] = '\0';

            Printf( "NET_DOWNLOAD: filename %s\n", dataBuf);

            int attempt = 0;
            do {

                int flg =  ReadFlagSinc(dataBuf);
                if((u32)flg == 0xffffffff) {
                    WriteFlagSinc(dataBuf,1,0);
                    break;
                }

                ++attempt;

                IPC_delay(1);

            } while(attempt <= 5000);

            void *fileData = NULL;
            size_t fsize = 0;

            if(attempt >= 5000) {

                Printf( "NET_DOWNLOAD: Timeout for waiting FLAG = -1\n");
                cmd.status = -1;
                cmd.data[0] = 0;

                res = sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND) );

                if(res != 0)
                    break;

            } else {

                Printf( "NET_DOWNLOAD: Get file size and data...\n");

                fsize = getFileSize(dataBuf,&fileData);
                if(fsize == 0) {
                    cmd.status = -1;
                }
                cmd.data[0] = fsize;

                res = sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND) );

                if(res != 0)
                    break;

                Printf( "NET_DOWNLOAD: send fileSize %d\n", fsize);

                res = sendDataBlock( SData, fileData, fsize );

                if(res != 0)
                    break;

                Printf( "NET_DOWNLOAD: send fileData %d\n", fsize);

                free( fileData );         

                res = recvDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND) );

                if(res != 0)
                    break;

                res = sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND) );

                if(res != 0)
                    break;
            }

        } break;

        case NET_READ_FLAG: {
            int flg[2] = {0, 0};

            // получим имя файла
            res = recvDataBlock( SData, dataBuf, cmd.data[0] );

            if(res != 0)
                cmd.status = -1;
            else
            {
                cmd.status = 0;
                flg[0] = ReadFlagSinc(dataBuf);
            }

            sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));

            if(cmd.status == 0)
                sendDataBlock( SData, (char *)flg, sizeof(int) * 2);

            break;
        }

        case NET_WRITE_FLAG: {
            int flg[2] = {0, 0};

            // получим имя файла
            res = recvDataBlock( SData, dataBuf, cmd.data[0] );

            if(res != 0)
            {
                cmd.status = -1;
                sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));
                break;
            }

            cmd.status = 0;
            sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));

            // получим флаги
            res = recvDataBlock( SData, flg, sizeof(int) * 2);

            if(res != 0)
            {
                cmd.status = -1;
                sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));
                break;
            }

            WriteFlagSinc(dataBuf, flg[0], flg[1]);

            cmd.status = 0;
            sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));

            break;
        }

        case NET_FILE_SIZE: {
            // получим имя файла
            res = recvDataBlock( SData, dataBuf, cmd.data[0] );

            if(res != 0)
                cmd.status = -1;
            else
            {
                cmd.status = 0;
                nFileSize = getFileSize(dataBuf, &fileData);
            }

            sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));

            if(cmd.status == 0)
                sendDataBlock( SData, (char *)&nFileSize, sizeof(int));

            break;
        }

        case NET_FILE_DATA: {
            // получим имя файла
            res = recvDataBlock( SData, dataBuf, cmd.data[0] );

            if(res != 0)
            {
                cmd.status = -1;
                sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));
                break;
            }

            cmd.status = 0;
            sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));
            sendDataBlock( SData, fileData, nFileSize );

            if(fileData)
            {
                free(fileData);
                fileData = 0;
            }

            break;
        }

		case NET_FILE_EXISTS: {
			// получим имя файла
			res = recvDataBlock( SData, dataBuf, cmd.data[0] );

			if(res != 0)
				cmd.status = -1;
			else
				cmd.status = FileExists(dataBuf);

			sendDataBlock( SData, (char *)&cmd, sizeof(NET_COMMAND));

			break;
		}

        case NET_CLOSE: {

            cmd.status = 0;
            exitFlag = true;
            Printf( "NET CLOSE\n" );

        } break;

		case NET_READ_FLAG_SHM: {
			int flg[2] = {0, 0};

			// получим имя файла
			res = recvDataBlock(SData, dataBuf, cmd.data[0]);

			if(res != 0)
			{
				cmd.status = -1;
				sendDataBlock(SData, (char *)&cmd, sizeof(NET_COMMAND));
				break;
			}

			if(hDataFlgMap == 0)
			{
				char sFlg[32] = "";

				strcat(sFlg, dataBuf);
				strcat(sFlg, "_flg");
				hDataFlgMap = IPC_openSharedMemory(sFlg);
			}

			if(hDataFlgMap && (pDataFlg == 0))
			{
				do
				{
					IPC_delay(100);
					pDataFlg = (U32*)IPC_mapSharedMemory(hDataFlgMap);
				} while(pDataFlg == 0);				
			}
			
			cmd.status = 0;
						
			sendDataBlock(SData, (char *)&cmd, sizeof(NET_COMMAND));

			if(cmd.status == 0)
			{
				flg[0] = pDataFlg[0];
				flg[1] = pDataFlg[1];
				sendDataBlock(SData, (char *)flg, sizeof(int) * 2);
			}

			break;
		}

		case NET_WRITE_FLAG_SHM: {
			int flg[2] = {0, 0};

			// получим имя файла
			res = recvDataBlock(SData, dataBuf, cmd.data[0]);

			if(res != 0)
			{
				cmd.status = -1;
				sendDataBlock(SData, (char *)&cmd, sizeof(NET_COMMAND));
				break;
			}

			if(hDataFlgMap == 0)
			{
				char sFlg[32] = "";

				strcat(sFlg, dataBuf);
				strcat(sFlg, "_flg");

				hDataFlgMap = IPC_openSharedMemory(sFlg);
			}

			if(hDataFlgMap && (pDataFlg == 0))
			{
				do
				{
					IPC_delay(100);
					pDataFlg = (U32*)IPC_mapSharedMemory(hDataFlgMap);
				} while(pDataFlg == 0);
			}

			cmd.status = 0;
			sendDataBlock(SData, (char *)&cmd, sizeof(NET_COMMAND));

			// получим флаги
			res = recvDataBlock(SData, flg, sizeof(int) * 2);

			if(res != 0)
			{
				cmd.status = -1;
				sendDataBlock(SData, (char *)&cmd, sizeof(NET_COMMAND));
				break;
			}

			pDataFlg[0] = flg[0];
			pDataFlg[1] = flg[1];
			
			cmd.status = 0;
			sendDataBlock(SData, (char *)&cmd, sizeof(NET_COMMAND));

			break;
		}

		case NET_DATABIN_SHM: {
			// получим имя файла
			res = recvDataBlock(SData, dataBuf, cmd.data[0]);

            int nBlkNum, nBlkSize, nAllSize;
            int i;

            nAllSize = nFileSize - strlen(pSuffix);

            GetBlksInfo(pSuffix, &nBlkNum, &nBlkSize);

            if(nBlkNum == 1)
                nBlkSize = nAllSize;

            if(vhDataBinMap.empty())
                for(i = 0; i < nBlkNum; i++)
                {
                    IPC_handle hDataBinMap = 0;
                    char *pDataBin = 0;

                    char sData[32] = "";

                    sprintf(sData, "%s_blk%d", dataBuf, i);
                    hDataBinMap = IPC_openSharedMemory(sData);

                    do
                    {
                        IPC_delay(100);
                        pDataBin = (char*)IPC_mapSharedMemory(hDataBinMap);
                    } while(pDataBin == 0);

                    vhDataBinMap.push_back(hDataBinMap);
                    vpDataBin.push_back(pDataBin);
                }

			cmd.status = 0;
			sendDataBlock(SData, (char *)&cmd, sizeof(NET_COMMAND));

            // Пошлем данные
            for(i = 0; i < nBlkNum; i++)
            {
                sendDataBlock(SData, vpDataBin[i], nBlkSize);

                nAllSize -= nBlkSize;

                if(nAllSize < nBlkSize)
                    nBlkSize = nAllSize;
            }

            // Пошлем суффикс
			sendDataBlock(SData, pSuffix, strlen(pSuffix));

			break;
		}

		case NET_DATASIZE_SHM:
		{
			// получим имя файла
			res = recvDataBlock(SData, dataBuf, cmd.data[0]);

			if (hSuffixMap == 0)
			{
				char sData[32] = "";

				strcat(sData, dataBuf);
				strcat(sData, "_postfix");

				hSuffixMap = IPC_openSharedMemory(sData);
			}

			if (hSuffixMap && (pSuffix == 0))
			{
				do
				{
					IPC_delay(100);
					pSuffix = (char*)IPC_mapSharedMemory(hSuffixMap);
				} while (pSuffix == 0);
			}

			cmd.status = 0;
			sendDataBlock(SData, (char *)&cmd, sizeof(NET_COMMAND));

			nFileSize = GetDataSize(pSuffix);

			// Пошлем размер
			sendDataBlock(SData, (char *)&nFileSize, sizeof(int));

			break;
		}

        default:
            Printf( "DEFAULT\n" );
        }


        if(isDownload)
        {
            isDownload = 0;
            WriteFlagSinc(dataBuf, 0,0);
        }

        if(res != 0) {
            Printf( "%s(): res = %d\n", __FUNCTION__, res);
            break;
        }

        if(exitFlag) {
            Printf( "%s(): exitFlag = %d\n", __FUNCTION__, exitFlag);
            break;
        }
    }

    for(size_t i = 0; i < vhDataBinMap.size(); i++)
    {
        IPC_unmapSharedMemory(vpDataBin[i]);
        IPC_deleteSharedMemory(vhDataBinMap[i]);
	}

	if(hSuffixMap)
	{
		IPC_unmapSharedMemory(pSuffix);
		IPC_deleteSharedMemory(hSuffixMap);
	}

	if(hDataFlgMap)
    {
		IPC_unmapSharedMemory(pDataFlg);
		IPC_deleteSharedMemory(pDataFlg);
	}

    shutdown(SData,SHUT_RDWR);
    close(SData);

    if(!lsProcs.empty())
    {
        string sCmd = "killall -s SIGINT " + lsProcs.back();
        lsProcs.pop_back();
        system(sCmd.c_str());
    }

    Printf( "%s() - finished\n", __FUNCTION__);

    return 0;
}

//-----------------------------------------------------------------------------

thread_value isvi_server::server_wait_thread(void* data)
{
    pid_t pid = *(pid_t *)data;
    int nRet;

    waitpid(pid, &nRet, 0);

    return 0;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
int GetDataSize(char *pSuffix)
{
	int nSize = 1;
	char *pVal;

	pVal = strstr(pSuffix, "NUMBER_OF_CHANNELS_");

	if(pVal == 0)
		return -1;

	pVal += 21;

	nSize *= atoi(pVal);

	pVal = strstr(pSuffix, "NUMBER_OF_SAMPLES_");

	if(pVal == 0)
		return -1;

	pVal += 20;

	nSize *= atoi(pVal);

	pVal = strstr(pSuffix, "BYTES_PER_SAMPLES_");

	if(pVal == 0)
		return -1;

	pVal += 20;

	nSize *= atoi(pVal);
	nSize += strlen(pSuffix);
	
	return nSize;
}

void GetBlksInfo(char *pSuffix, int *pNum, int *pSize)
{
    char *pVal;

    pVal = strstr(pSuffix, "BLKNUM ");

    if(pVal == 0)
    {
        *pNum = 0;
        return;
    }

    *pNum = atoi(pVal + 7);

    pVal = strstr(pSuffix, "BLKSIZE ");

    if(pVal == 0)
    {
        *pSize = 0;
        return;
    }

    *pSize = atoi(pVal + 8);
}
