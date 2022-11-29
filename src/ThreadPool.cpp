//
// Created by Sungjun Park on 2022/11/26.
//
#include "ThreadPool.hpp"
#include "ServerManager.hpp"

static void* jobHandler(void *_threadPool)
{
  ThreadPool& tp = *reinterpret_cast<ThreadPool*>(_threadPool);
  FileDescriptor kq = kqueue();
  struct kevent event;
  struct timespec ts = {0, 5 * 1000 * 1000};

  while (true)
  {
    // check Connection event
    struct kevent* newEvent = NULL;

    // get connection event from job queue
    pthread_mutex_lock(tp.getMutex());
    if (!tp._eventQueue.empty())
    {
      newEvent = tp._eventQueue.front();
      tp._eventQueue.pop();
      struct Context* context = reinterpret_cast<struct Context*>(newEvent->udata);
      context->threadKQ = kq;
    }
    else if (tp._eventQueue.empty() && tp.isStop())
    {
      pthread_mutex_unlock(tp.getMutex());
      return (NULL);
    }
    pthread_mutex_unlock(tp.getMutex());

    if (newEvent == NULL)
    {
      // 서버 시작. 새 이벤트(Req)가 발생할 때 까지 무한루프. (감지하는 kevent)
      int newEventCount = kevent(kq, NULL, 0, &event, 1, &ts);

      if (newEventCount == -1)
      { // nothing happen
        printLog("EV ERR (-1)\n", PRINT_RED);
        continue;
      }
      else if (newEventCount == 0)
      { // time limit expired -> never happen
        continue;
      }
      else if (event.filter == EVFILT_READ || event.filter == EVFILT_WRITE)
      {
        newEvent = &event;
      }
    }
    try
    {
      handleEvent(newEvent);
    }
    catch (std::exception& e)
    {
      printLog(e.what(), PRINT_RED);
    }
  }
}

ThreadPool::ThreadPool(size_t threadNumber):
  NUM_THREADS(threadNumber),
  _stopAll(false)
{
  pthread_mutex_init(&_jobQueueMutex, NULL);
  _workerThreads.reserve(NUM_THREADS);

}

ThreadPool::~ThreadPool()
{
  _stopAll = true;
  for (size_t i = 0; i < NUM_THREADS; ++i)
  {
    pthread_join(_workerThreads[i], NULL);
  }
}

pthread_mutex_t* ThreadPool::getMutex()
{
  return (&_jobQueueMutex);
}

bool ThreadPool::isStop() const
{
  return (_stopAll);
}

void ThreadPool::attachNewEvent(struct kevent* event)
{
  pthread_mutex_lock(getMutex());
  _eventQueue.push(event);
  pthread_mutex_unlock(getMutex());
}

void ThreadPool::createPool()
{
  for (size_t i = 0; i < NUM_THREADS; ++i)
  {
    pthread_t newThread;

    pthread_create(&newThread, NULL, jobHandler, this);
    _workerThreads.push_back(newThread);
  }
}
