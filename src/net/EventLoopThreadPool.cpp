#include <stdio.h>

#include "src/net/EventLoopThreadPool.h"
#include "src/net/EventLoop.h"
#include "src/net/EventLoopThread.h"



using namespace serverlib;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& name)
  : _base_loop(baseLoop),
    _name(name),
    _started(false),
    _num_threads(0),
    _next(0)
{
}
EventLoopThreadPool::~EventLoopThreadPool()
{
  // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
  assert(!_started);
  _base_loop->assertInLoopThread();

  _started = true;

  for (int i = 0; i < _num_threads; ++i)
  {
    char buf[_name.size() + 32];
    snprintf(buf, sizeof buf, "%s%d", _name.c_str(), i);
    EventLoopThread* t = new EventLoopThread(cb, buf);    //创建EventLoopThread
    _threads.push_back(std::unique_ptr<EventLoopThread>(t));
    _loops.push_back(t->startLoop());  //为每一个EventLoopThread创建一个EventLoop
  }
  if (_num_threads == 0 && cb)
  {
    cb(_base_loop);
  }
}

EventLoop* EventLoopThreadPool::getNextLoop(){
	_base_loop->assertInLoopThread();
	assert(_started);
	EventLoop* loop = _base_loop;

	//if (!loops_.empty()) {
	//  loop = loops_[next_];
	//  next_ = (next_ + 1) % _loops.size();
	//}

	if (!_loops.empty()){
		// round-robin
		loop = _loops[_next];
		++_next;
		if (_next >= _loops.size())	{
		  _next = 0;
		}
	}
	return loop;
}


std::vector<EventLoop*> EventLoopThreadPool::getAllLoops(){
	_base_loop->assertInLoopThread();
	assert(_started);
	if (_loops.empty()) {
		return std::vector<EventLoop*>(1, _base_loop);
	}
	else {
		return _loops;
	}
}