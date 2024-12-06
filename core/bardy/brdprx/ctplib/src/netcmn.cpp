//
// Low-level net functions
//

#include "brderr.h"
#include "ctpapi.h"
#include "utypes.h"

#include "gipcy.h"

#define MAX_TIMEOUT 25

static int recv_all(
    IPC_handle s,
    char* buf,
    int len,
    int flags)
{
    int cnt = 0;
    volatile int size = len;

    fd_set ReadSet;

    //int j=0;

    while (size > 0) {
        timeval tval = { MAX_TIMEOUT, 0 };

        IPC_FD_ZERO(&ReadSet);
        IPC_FD_SET(s, &ReadSet);

        int r = IPC_select(s, &ReadSet, 0, 0, &tval);

        if (r == 0)
            continue;

        if (r == IPC_SOCKET_ERROR) {
            int err = IPC_sysError();

            if (err != IPC_EWOULDBLOCK) {
                //restart();
                return IPC_SOCKET_ERROR;
            }

            continue;
        }
        cnt = IPC_recv(s, buf, size, flags);

        if ((cnt == IPC_SOCKET_ERROR)) {
            IPC_sysError();

            return IPC_SOCKET_ERROR;
        }

        buf += cnt;
        size -= cnt;
    }

    return (len - size);
}

static int send_all(
    IPC_handle s,
    char* buf,
    int len,
    int flags)
{
    int cnt = 0;
    volatile int size = len;

    fd_set WriteSet;
    timeval tval = { MAX_TIMEOUT, 0 };

    //int j=0;

    while (size > 0) {
        IPC_FD_ZERO(&WriteSet);
        IPC_FD_SET(s, &WriteSet);

        int r = IPC_select(s, 0, &WriteSet, 0, &tval);

        if (r == 0)
            continue;

        if (r == IPC_SOCKET_ERROR) {
            int err = IPC_sysError();

            if (err != IPC_EWOULDBLOCK) {
                //restart();
                return IPC_SOCKET_ERROR;
            }

            continue;
        }
        cnt = IPC_send(s, buf, size, flags);

        if ((cnt == IPC_SOCKET_ERROR)) {
            IPC_sysError();

            return IPC_SOCKET_ERROR;
        }

        buf += cnt;
        size -= cnt;
    }

    return (len - size);
}

S32 sendDataBlock(IPC_handle s, void* buf, S32 sizeOfBytes)
{
    int r = send_all(s, (char*)buf, sizeOfBytes, 0); //fixme

    if (r == IPC_SOCKET_ERROR) {
        //printf( "%s, %d: %s Error in send() %d\n", __FILE__, __LINE__, __FUNCTION__, r );
        return -1;
    }

    return 0;
}

S32 recvDataBlock(IPC_handle s, void* buf, S32 sizeOfBytes)
{
    if (!sizeOfBytes)
        return -1;

    int r = recv_all(s, (char*)buf, sizeOfBytes, 0);

    if (r == IPC_SOCKET_ERROR) {
        //printf( "%s, %d: %s Erorr in recv_all() %d\n", __FILE__, __LINE__, __FUNCTION__, r );
        return -1;
    }

    return 0;
}
