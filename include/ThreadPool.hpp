//
// Created by Sungjun Park on 2022/11/26.
//

#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include "WebservDefines.hpp"
#include <pthread.h>
#include <vector>
#include <map>
#include <queue>
#include <sys/event.h>

class ThreadPool
{
public:
    const size_t NUM_THREADS;
    bool _stopAll;
    pthread_mutex_t _jobQueueMutex;
    std::vector<pthread_t> _workerThreads;
    std::queue<struct kevent*> _eventQueue;
    FileDescriptor _serverKQ;

    explicit ThreadPool(size_t threadNumber);
    ~ThreadPool();
    pthread_mutex_t* getMutex();
    void attachNewEvent(struct kevent* event);
    bool isStop() const;
    void createPool();
};

#endif //THREADPOOL_HPP
