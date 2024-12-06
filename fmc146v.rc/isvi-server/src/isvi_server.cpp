#include "isvi_server.h"
#include "netcmn.h"
#include "netcmd.h"

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

#include "printf.h"

//-----------------------------------------------------------------------------

isvi_server::isvi_server(unsigned& port) : m_port(port)
{
    Printf( "%s()\n", __FUNCTION__);
    m_exit = false;
}

//-----------------------------------------------------------------------------

isvi_server::~isvi_server()
{
    Printf( "%s()\n", __FUNCTION__);
    stop();
    clear_all_entry();
}

//-----------------------------------------------------------------------------

bool isvi_server::start()
{
    if(create_socket()) {
        int hThread = pthread_create(&m_server_thread, NULL, server_main_thread, (void *)this);
        if( 0 > hThread) {
            Printf( "%s(): Error start server_main_thread()\n", __FUNCTION__);
            return false;
        }
    }

    Printf( "%s(): server_main_thread() was started\n", __FUNCTION__);

    return true;
}

//-----------------------------------------------------------------------------

void isvi_server::stop()
{
    m_exit = true;
    Printf( "%s()\n", __FUNCTION__);
}

//-----------------------------------------------------------------------------

void isvi_server::insert_entry(struct srv_thread_param *entry)
{
    m_thread_param.push_back(entry);
}

//-----------------------------------------------------------------------------

void isvi_server::remove_entry(struct srv_thread_param *entry)
{
    for(unsigned i=0; i<m_thread_param.size(); i++) {
        struct srv_thread_param *var = m_thread_param.at(i);
        if(var == entry) {
            m_thread_param.erase(m_thread_param.begin()+i);
            delete var;
        }
    }
}

//-----------------------------------------------------------------------------

void isvi_server::clear_all_entry()
{
    for(unsigned i=0; i<m_thread_param.size(); i++) {
        struct srv_thread_param *tp = m_thread_param.at(i);
        tp->exitFlag = 1;
    }

    IPC_delay(10);

    for(unsigned i=0; i<m_thread_param.size(); i++) {
        struct srv_thread_param *tp = m_thread_param.at(i);
        wait_exit(tp->id, 100);
        delete tp;
    }
    m_thread_param.clear();
    m_thread_param.resize(0);
}

//-----------------------------------------------------------------------------

bool isvi_server::create_socket()
{
    struct sockaddr_in anAddr;

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_socket == -1) {
        Printf( "%s(): %s\n", __FUNCTION__, strerror(errno));
        return false;
    }

    anAddr.sin_family = AF_INET;
    anAddr.sin_port = htons(m_port);
    anAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int flag = 1;
    int res = ioctl(m_socket, FIONBIO, &flag);
    if( res == -1 ) {
        Printf( "%s(): %s\n", __FUNCTION__, strerror(errno));
        return false;
    }

    int yes = 1;
    res = setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
    if(res == -1) {
        Printf( "%s(): %s\n", __FUNCTION__, strerror(errno));
        return false;
    }

    res = setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if(res == -1) {
        Printf( "%s(): %s\n", __FUNCTION__, strerror(errno));
        return false;
    }

    res = bind(m_socket, (struct sockaddr *)&anAddr, sizeof(anAddr));
    if( res < 0 ) {
        Printf( "%s(): %s\n", __FUNCTION__, strerror(errno));
        return false;
    }

    res = listen(m_socket, 10);
    if(res == -1) {
        Printf( "%s(): %s\n", __FUNCTION__, strerror(errno));
        return false;
    }

    Printf( "%s(): start listeninig socket...\n", __FUNCTION__);

    return true;
}

//-----------------------------------------------------------------------------

thread_value isvi_server::server_main_thread(void *data)
{
    int err = -1;
    int acceptedSocket = -1;
    fd_set ReadSet;
    timeval tval;
    sockaddr_in _anAddr;

    isvi_server *object = (isvi_server *)data;
    if(!object) {
        Printf( "%s(): Invalid server class pointer\n", __FUNCTION__);
        return (thread_value)0x12345678;
    }

    Printf( "%s()\n", __FUNCTION__);

again:
    FD_ZERO( &ReadSet );
    FD_SET( object->get_socket(), &ReadSet );
    tval.tv_sec = 0;
    tval.tv_usec = 5000;
    err = select(object->get_socket()+1, &ReadSet, 0, 0, &tval);
    if((err==-1)||(err==0)) {
        if(object->is_exit()) {
            Printf( "%s(): %s\n", __FUNCTION__, strerror(errno));
            return (thread_value)0;
        }
        goto again;
    }

    socklen_t addrsize = sizeof(_anAddr);
    acceptedSocket = accept(object->get_socket(), (sockaddr*)&_anAddr, &addrsize);
    if(acceptedSocket == -1) {
        Printf( "%s(): %s\n", __FUNCTION__, strerror(errno));
        return (thread_value)0x12345678;
    }

    Printf( "%s(): accept socket - %d\n", __FUNCTION__, acceptedSocket );

    int yes = 1;
    int res = setsockopt(acceptedSocket, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
    if(res == -1) {
        return (thread_value)0x12345678;
    }

    res = setsockopt(acceptedSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if(res == -1) {
        return (thread_value)0x12345678;
    }

    struct srv_thread_param *tp = new (struct srv_thread_param);
    if(!tp) {
        Printf( "%s(): %s\n", __FUNCTION__, strerror(errno));
        return (thread_value)0x12345678;
    }

    char tname[256];
    snprintf(tname,sizeof(tname),"server_processing_thread_%d\n", object->m_thread_param.size());

    tp->acceptedSocket = acceptedSocket;
    tp->classObject = data;
    tp->exitFlag = false;

    tp->id = IPC_createThread(tname, server_processing_thread, (void *)tp);
    if( !tp->id ) {
        Printf( "Can't create ServerThread\n" );
        delete tp;
        return (thread_value)0x12345678;
    }

    object->m_thread_param.push_back(tp);

    Printf( "Create processing thread: %d\n", object->m_thread_param.size() );

    if(object->is_exit())
        return (thread_value)0;

    goto again;
}

//-----------------------------------------------------------------------------

int isvi_server::wait_exit(IPC_handle thread, int timeout)
{
    return IPC_waitThread(thread, timeout);
}

//-----------------------------------------------------------------------------
