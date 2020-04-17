#pragma once

#include<string>

#include "src/base/Condition.h"
#include "src/base/Mutex.h"
#include "src/base/Thread.h"


namespace serverlib
{

class EventLoop;

class EventLoopThread{
 public:
	typedef std::function<void(EventLoop*)> ThreadInitCallback;

	EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
					const std::string& name = std::string());
	~EventLoopThread();
	EventLoop* startLoop();

 private:
	void threadFunc();

	EventLoop* _loop;
	bool _exiting;
	Thread _thread;	
	MutexLock _mutex;
	Condition _cond;
	ThreadInitCallback _callback;  // 回调函数在EventLoop::loop事件循环之前被调用
};

}  // namespace serverlib
