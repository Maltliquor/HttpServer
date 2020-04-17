#include <sstream>
#include <poll.h>

#include "src/net/Channel.h"
#include "src/net/EventLoop.h"


using namespace serverlib;

const int serverlib::Channel::kReadEvent = POLLIN | POLLPRI;
const int serverlib::Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
  : _loop(loop),
    _fd(fd),
    _events(0),
    _revents(0),
    _index(-1),
    _tied(false),
    _event_handling(false),
    _added_to_loop(false)
{
}

Channel::~Channel()
{
  assert(!_event_handling);
  assert(!_added_to_loop);
  if (_loop->isInLoopThread())
  {
    assert(!_loop->hasChannel(this));
  }
}



void Channel::update()
{
	_added_to_loop = true;
	_loop->updateChannel(this);
}

void Channel::remove()
{
	assert(isNoneEvent());
	_added_to_loop = false;
	_loop->removeChannel(this);
}
/*
handleEvent:
作用：供EventLoop类调用，因为EventLoop类只会发现你这个channel有事件
    ，至于是什么事件需要channel类自己查看revent变量（EPollPoller传递的），
    然后根据其值来调用相关的事件回调函数。（这也是handleEventWithGuard的逻辑）
逻辑：判断这个channel是否有锁，一般该事件涉及多事件调用才需要用到锁。
若有锁，则获得锁后调用handleEventWithGuard
若无锁，直接调用handleEventWithGuard。
*/
void Channel::handleEvent(){
	//std::shared_ptr<void> guard;
	//if (_tied)  //判断是否有绑定的context。
	//{
	//	//weak_ptr.lock() 会把weak_ptr提升为一个shared_ptr对象
	//	//当引用计数为0，那么shared_ptr为空
	//	guard = _holder.lock();
	//	if (guard)  //是否shared_ptr不为空
	//	{
		  handleEventWithGuard();
	//	}
	//}
	//else{
	//	handleEventWithGuard();
	//}
}
/*
handleEventWithGuard：
根据revent值来调用相关的事件回调函数。
man 2 poll 可见每个宏定义POLLXXX的含义。
*/
void Channel::handleEventWithGuard()
{
	_event_handling = true;
	if ((_revents & POLLHUP) && !(_revents & POLLIN)) {
		if (_close_callback) _close_callback();
	}

    if (_revents & POLLNVAL){ //fd未打开，见man 2 poll
        printf("Warning: fd = %d  Channel::handle_event() POLLNVAL", _fd);
    }

    if (_revents & (POLLERR | POLLNVAL)) {  //出错  
		if (_error_callback) _error_callback();
    }
    if (_revents & (POLLIN | POLLPRI | POLLRDHUP)){    //收到
	    if (_read_callback) _read_callback();
    }
    if (_revents & POLLOUT){  //写
		if (_write_callback) _write_callback();
    }
  _event_handling = false;
}


