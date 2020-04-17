#include "src/base/Thread.h"
#include "src/base/CurrentThread.h"
#include "src/base/Exception.h"

#include <type_traits>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>


namespace serverlib{

namespace detail
{

pid_t gettid()
{
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

void afterFork()
{
  serverlib::CurrentThread::t_cachedTid = 0;
  serverlib::CurrentThread::t_threadName = "main";
  CurrentThread::tid();
  // no need to call pthread_atfork(NULL, NULL, &afterFork);
}

class ThreadNameInitializer
{
 public:
  ThreadNameInitializer()
  {
    serverlib::CurrentThread::t_threadName = "main";
    CurrentThread::tid();
    pthread_atfork(NULL, NULL, &afterFork);
  }
};

ThreadNameInitializer init;

struct ThreadData
{
  typedef serverlib::Thread::ThreadFunc ThreadFunc;
  ThreadFunc _func;
  string _name;
  pid_t* _tid;
  CountDownLatch* _latch;

  ThreadData(ThreadFunc func,
             const string& name,
             pid_t* tid,
             CountDownLatch* latch)
    : _func(std::move(func)),
      _name(name),
      _tid(tid),
      _latch(latch)
  { }

  void runInThread()
  {
    *_tid = serverlib::CurrentThread::tid();
    _tid = NULL;
    _latch->countDown();
    _latch = NULL;

    serverlib::CurrentThread::t_threadName = _name.empty() ? "muduoThread" : _name.c_str();
    ::prctl(PR_SET_NAME, serverlib::CurrentThread::t_threadName);
	
	printf("runInThread() _func is %d\n",(_func==NULL));
    try
    {
      _func();
      serverlib::CurrentThread::t_threadName = "finished";
    }
    catch (const Exception& ex)
    {
      serverlib::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", _name.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
      abort();
    }
    catch (const std::exception& ex)
    {
      serverlib::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", _name.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      abort();
    }
    catch (...)
    {
      serverlib::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "unknown exception caught in Thread %s\n", _name.c_str());
      throw; // rethrow
    }
  }
};

void* startThread(void* obj)
{
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->runInThread();
  delete data;
  return NULL;
}

}  // namespace detail

void CurrentThread::cacheTid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = detail::gettid();
    t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}

bool CurrentThread::isMainThread()
{
  return tid() == ::getpid();
}

void CurrentThread::sleepUsec(int64_t usec)
{
  struct timespec ts = { 0, 0 };
  ts.tv_sec = static_cast<time_t>(usec /(1000 * 1000));
  ts.tv_nsec = static_cast<long>(usec % (1000 * 1000) * 1000);
  ::nanosleep(&ts, NULL);
}

AtomicInt32 Thread::numCreated_;

Thread::Thread(ThreadFunc func, const string& n)
  : _started(false),
    _joined(false),
    _pthreadId(0),
    _tid(0),
    _func(std::move(func)),
    _name(n),
    _latch(1)
{
  setDefaultName();
}

Thread::~Thread()
{
  if (_started && !_joined)
  {
    pthread_detach(_pthreadId);
  }
}

void Thread::setDefaultName()
{
  int num = numCreated_.incrementAndGet();
  if (_name.empty())
  {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    _name = buf;
  }
}

void Thread::start()
{
  assert(!_started);
  _started = true;
  // FIXME: move(_func)
  detail::ThreadData* data = new detail::ThreadData(_func, _name, &_tid, &_latch);
  if (pthread_create(&_pthreadId, NULL, &detail::startThread, data))
  {
    _started = false;
    delete data; // or no delete?
    perror("Failed in pthread_create");
  }
  else
  {
    _latch.wait();
    assert(_tid > 0);
  }
}

int Thread::join()
{
  assert(_started);
  assert(!_joined);
  _joined = true;
  return pthread_join(_pthreadId, NULL);
}


}// namespace serverlib
