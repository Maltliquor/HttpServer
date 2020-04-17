#include "src/net/EventLoopThread.h"
#include "src/net/EventLoop.h"

using namespace serverlib;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const std::string& name)
  : _loop(NULL),
    _exiting(false),
    _thread(std::bind(&EventLoopThread::threadFunc, this), name),
    _mutex(),
    _cond(_mutex),
    _callback(cb)
{
}

EventLoopThread::~EventLoopThread()
{
  _exiting = true;
  if (_loop != NULL) // not 100% race-free, eg. threadFunc could be running _callback.
  {
    // still a tiny chance to call destructed object, if threadFunc exits just now.
    // but when EventLoopThread destructs, usually programming is exiting anyway.
    _loop->quit();
    _thread.join();
  }
}

EventLoop* EventLoopThread::startLoop()
{
	assert(!_thread.started());
	_thread.start();//启动线程，此时有两个线程在运行，
    //一个是调用EventLoopThread::startLoop()的线程，一个是执行EventLoopThread::threadFunc()的线程（IO线程）

	EventLoop* loop = NULL;
	{
		MutexLockGuard lock(_mutex);
		while (_loop == NULL){
		  _cond.wait();
		}
		loop = _loop;
	}

  return loop;
}

void EventLoopThread::threadFunc()
{
	EventLoop loop;

	if (_callback) {
		_callback(&loop);
	}

	{
		MutexLockGuard lock(_mutex);
		_loop = &loop;
		_cond.condSignal();
	}

	loop.loop();
	//assert(_exiting);
	MutexLockGuard lock(_mutex);
	_loop = NULL;
}