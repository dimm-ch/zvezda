#include <atomic>
#include <iostream>
#include <thread>
#include <unistd.h>

//=************************* main *************************
//=********************************************************

std::thread* thread;
std::atomic<bool> stop;
std::atomic<bool> isStoped;

void Work(int x)
{
    stop.store(false);
    isStoped.store(false);
    printf("befor work\n");
    while (!stop.load()) {
        sleep(1);
    }
    printf("after work\n");
    isStoped.store(true);
    printf("isStoped = false\n");
}

int main()
{
    int x = 1;
    isStoped.store(true);
    thread = new std::thread(Work, x);
    while (isStoped.load())
        sleep(1);
    printf("thread start\n");
    for (size_t i = 0; i < 100000; i++)
        ;
    printf("before stop work\n");
    stop.store(true);
    printf("stop work\n");
    while (!isStoped.load())
        sleep(1);
    printf("out from thread\n");
    return 0;
}
