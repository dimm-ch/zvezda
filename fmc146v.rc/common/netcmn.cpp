
#include "netcmn.h"
#include "printf.h"

#include <stdio.h>

//------------------------------------------------------------------------------

static int recv_all (
        SOCKET s,
        char * buf,
        int len,
        int flags)
{
    int cnt=0;
    volatile int size = len;

    fd_set ReadSet;
    timeval tval={MAX_TIMEOUT,0};

    while( size > 0 )
    {
        tval.tv_sec = MAX_TIMEOUT;
        tval.tv_usec = 0;

        FD_ZERO(&ReadSet);
        FD_SET( s,&ReadSet);

        int r = select(s+1,&ReadSet,0,0,&tval);
        if((r==SOCKET_ERROR)||(r==0)){
            Printf( "%s, %d: %s SOCKET_ERROR %d\n",
                     __FILE__, __LINE__, __FUNCTION__, r );
            return -1;
        }

        cnt = recv( s, buf, size, flags );
        if(cnt==SOCKET_ERROR)
            return SOCKET_ERROR;

        if(cnt==0) {
            Printf( "%s, %d: %s %d bytes was readed from socket\n",
                     __FILE__, __LINE__, __FUNCTION__, cnt );
            return 0;
        }

        buf += cnt;
        size -= cnt;
    }

    return (len-size);
}

//------------------------------------------------------------------------------

int send_all (
        SOCKET s,
        char *buf,
        int len,
        int flags)
{
    int cnt = 0;
    int size = len;

    fd_set WriteSet;
    timeval tval={MAX_TIMEOUT,0};

    while( size > 0 )
    {
        tval.tv_sec=MAX_TIMEOUT;
        tval.tv_usec=0;

        FD_ZERO(&WriteSet);
        FD_SET( s,&WriteSet);

        int r = select(s+1, 0, &WriteSet, 0, &tval);
        if((r==SOCKET_ERROR)||(r==0)){
            Printf( "%s, %d: %s SOCKET_ERROR %d\n", __FILE__, __LINE__, __FUNCTION__, r );
            return -1;
        }
        cnt = send( s, buf, size, flags );
        if(cnt == SOCKET_ERROR){
            return -1;
        }

        buf += cnt;
        size -= cnt;
    }

    return (len-size);
}

//------------------------------------------------------------------------------

int sendDataBlock(SOCKET s, void *buf, size_t sizeOfBytes)
{
    //Printf( "%s(): - started\n", __FUNCTION__);

    if( !sizeOfBytes ) {
        Printf( "%s(): - zero size\n", __FUNCTION__);
        return -1;
    }

//    fd_set WriteSet;
//    timeval tval={MAX_TIMEOUT,0};

//    FD_ZERO(&WriteSet);
//    FD_SET(s,&WriteSet);

//    int r=select(s+1,0,&WriteSet,0,&tval);
//    if((r==SOCKET_ERROR)||(r==0)){
//        Printf( "%s, %d: %s SOCKET_ERROR %d\n", __FILE__, __LINE__, __FUNCTION__, r );
//        return -1;
//    }

    int r = send_all( s, (char *)buf, sizeOfBytes, 0);
    if((r==SOCKET_ERROR)||(r==0)){
        Printf( "%s, %d: %s Error in send() %d\n", __FILE__, __LINE__, __FUNCTION__, r );
        return -1;
    }

    Printf( "%s, %d: %s() - send %d bytes\n", __FILE__, __LINE__, __FUNCTION__, r );

    return 0;
}

//------------------------------------------------------------------------------

int recvDataBlock(SOCKET s,void *buf, size_t sizeOfBytes)
{
    Printf( "%s() - started\n", __FUNCTION__ );

    if( !sizeOfBytes ) {
        Printf( "%s() - zero size\n", __FUNCTION__ );
        return -1;
    }

    int r = recv_all( s, (char*)buf, sizeOfBytes, 0);
    if((r==SOCKET_ERROR)||(r==0)){
        Printf( "%s, %d: %s Erorr in recv_all() %d\n", __FILE__, __LINE__, __FUNCTION__, r );
        return -1;
    }

    //Printf( "%s() - received %d bytes\n", __FUNCTION__, r );

    return 0;
}

//------------------------------------------------------------------------------

size_t getFileSize(const char *fname, void **fileBuffer)
{
    IPC_handle hFile;

    long long fileSize;

    hFile = IPC_openFile(fname, IPC_OPEN_FILE | IPC_FILE_RDONLY);
    if(hFile == 0) {
        Printf( "%s(): File %s was not found\n", __FUNCTION__, fname );
        return 0;
    }

    IPC_getFileSize(hFile, &fileSize);

    //Printf( "%s(): Size of file %s is - %lld bytes\n", __FUNCTION__, fname, fileSize);

    if(fileBuffer) {

        *fileBuffer = malloc( (size_t)fileSize );
        if(*fileBuffer == NULL) {
            Printf( "%s(): Cant allocate file %s buffer\n", __FUNCTION__, fname );
            return 0;
        }

        size_t bytesRead = IPC_readFile(hFile, *fileBuffer, fileSize);
        if( (long long)bytesRead != fileSize ) {
            Printf( "%s(): Cant read file %s into buffer\n", __FUNCTION__, fname );
            free(*fileBuffer);
            *fileBuffer = 0;
            return 0;
        }
    }

    IPC_closeFile(hFile);

    return fileSize;
}

//-----------------------------------------------------------------------------

int FileExists(const char *fname)
{
	IPC_handle hFile;

	hFile = IPC_openFile(fname, IPC_OPEN_FILE);
	
	if(hFile == 0)
		return 0;
	
	IPC_closeFile(hFile);

	return 1;
}

//-----------------------------------------------------------------------------

size_t save_data_file(const char* filename, void *buffer, size_t size)
{
    //Printf( "%s()\n", __FUNCTION__);

    size_t res = 0;

    if(size) {

        FILE *f = fopen( filename, "wb");
        res = fwrite( buffer, 1, size, f );
        fclose( f );
    }

    return res;
}

//-----------------------------------------------------------------------------

void WriteFlagSinc(char *fileName, int flg, int isNewParam)
{
    IPC_handle handle = NULL;
    int val[2] = {0, 0};

    while( !handle ) {
        handle = IPC_openFile(fileName, IPC_OPEN_FILE | IPC_FILE_RDWR);
    }
    val[0] = flg;
    val[1] = isNewParam;
    int res = IPC_writeFile(handle, val, sizeof(val));
    if(res < 0) {
        Printf( "Error write flag sync\r\n" );
    }
    IPC_closeFile(handle);
}

//-----------------------------------------------------------------------------

int  ReadFlagSinc(char *fileName)
{
    int flg = 0;
    IPC_handle handle = IPC_openFile(fileName, IPC_OPEN_FILE | IPC_FILE_RDONLY);

    if(!handle) {
        return 0;
    }

    int res = IPC_readFile(handle, &flg, sizeof(flg));
    if(res < 0) {
        Printf( "Error read flag sync\r\n" );
    }
    IPC_closeFile(handle);

    //Printf( "%s(): FLG = 0x%x\n", __func__, flg );

    return flg;
}

//-----------------------------------------------------------------------------
/*
void Delay(int ms)
{
    struct timeval tv = {0, 0};
    tv.tv_usec = 1000*ms;

    select(0,NULL,NULL,NULL,&tv);
}
*/
