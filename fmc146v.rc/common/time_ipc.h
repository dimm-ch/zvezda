
#ifndef TIMEIPC_H
#define TIMEIPC_H

#include <thread>
#include <chrono>

using ipc_time_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

inline ipc_time_t ipc_get_time()
{
    return std::chrono::high_resolution_clock::now();
}

inline double ipc_get_difftime(ipc_time_t start, ipc_time_t end)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

inline void ipc_delay(int ms)
{
#ifdef __linux__
    std::this_thread::sleep_for(std::chrono_literals::operator""ms(ms));
#else
    Sleep(ms);
#endif
}

#endif //TIMEIPC_H
