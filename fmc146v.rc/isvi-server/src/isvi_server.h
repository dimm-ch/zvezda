#ifndef __ISVI_SERVER_H__
#define __ISVI_SERVER_H__

#include "gipcy.h"

#include <pthread.h>
#include <vector>
#include <list>
#include <algorithm>

struct srv_thread_param {
    IPC_handle id;
    int exitFlag;
    int acceptedSocket;
    void *classObject;
};

class isvi_server
{
public:
    isvi_server(unsigned& port);
    virtual ~isvi_server();
    bool start();
    void stop();

private:
    unsigned& m_port;
    int m_socket;
    bool m_exit;
    pthread_t m_server_thread;
    std::vector<struct srv_thread_param*> m_thread_param;

    void insert_entry(struct srv_thread_param *entry);
    void remove_entry(struct srv_thread_param *entry);
    void clear_all_entry();
    bool create_socket();
    bool is_exit() { return m_exit; }
    int wait_exit(IPC_handle thread, int timeout);
    int  get_socket() { return m_socket; }

    static thread_value server_main_thread(void* data);
    static thread_value server_processing_thread(void* data);
    static thread_value server_wait_thread(void* data);
};

#endif // __ISVI_SERVER_H__
