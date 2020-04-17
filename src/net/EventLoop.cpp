#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <iostream>
#include <algorithm>
#include <signal.h>
#include <unistd.h>

#include "src/net/EventLoop.h"
#include "src/net/Channel.h"
#include "src/net/SocketOp.h"
#include "src/net/EpollPoller.h"

using namespace std;
using namespace serverlib;

const int kPollTimeMs = 10000;
__thread EventLoop* t_loopInThisThread = 0;

int createEventfd() {
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop()
    : _looping(false),
      _poller(new EPollPoller(this)),//FIXME:Poller::newDefaultPoller(this)
      _wakeup_fd(createEventfd()),
      _quit(false),
      _event_handling(false),
      _calling_pending_funcs(false),
      _threadID(CurrentThread::tid()),
      _wakeup_channel(new Channel(this, _wakeup_fd)) {
  if (t_loopInThisThread) {
  } else {
    t_loopInThisThread = this;
  }
  _wakeup_channel->setReadCallback(std::bind(&EventLoop::handleRead, this));
  _wakeup_channel->bindReadingEvent();
}

EventLoop::~EventLoop() {
  _wakeup_channel->unbindAll();
  _wakeup_channel->remove();
  ::close(_wakeup_fd);
  t_loopInThisThread = NULL;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread(){
  return t_loopInThisThread;
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = sockets::write(_wakeup_fd, (char*)(&one), sizeof one);
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = sockets::read(_wakeup_fd, &one, sizeof one);
}

void EventLoop::loop() {
  assert(!_looping);
  assertInLoopThread();
  _looping = true;
  _quit = false;  // FIXME: what if someone calls quit() before loop() ?

  while (!_quit) {
    _active_channels.clear();
	
	/// 使用epoll_wait查看是否有可操作的channel
    _poller->poll(kPollTimeMs, &_active_channels); //_active_channels保存了epoll_wait()得到的可执行的epoll_event
	
    ++_iter;
    // TODO sort channel by priority
    _event_handling = true;
	
	/// 执行loop中的可操作事件
    for (Channel* channel : _active_channels){
		
      _cur_active_channel = channel;
      _cur_active_channel->handleEvent();  //此处执行的事TCPServer注册的handleRead()函数
	  
    }
    _cur_active_channel = NULL;
    _event_handling = false;
	
	/// 执行vector中存放的等待中的函数_pending_funcs
    doPendingFunctors();
  }

  _looping = false;
}

void EventLoop::doPendingFunctors() {
	std::vector<FuncCallBack> functors;
	_calling_pending_funcs = true;

  {
    MutexLockGuard lock(_mutex);
    functors.swap(_pending_funcs);
  }

	/// 执行_pending_funcs中存放的等待中的函数
	for (size_t i = 0; i < functors.size(); ++i) 
		functors[i]();
	
	_calling_pending_funcs = false;
}

void EventLoop::quit(){
	_quit = true;
	// There is a chance that loop() just executes while(!_quit) and exits,
	// then EventLoop destructs, then we are accessing an invalid object.
	// Can be fixed using _mutex in both places.
	if (!isInLoopThread()) {
		wakeup();
	}
}

void EventLoop::runInLoop(FuncCallBack cb){
  if (isInLoopThread()){
    cb();
  }
  else{
    queueInLoop(std::move(cb));
  }
}

void EventLoop::queueInLoop(FuncCallBack cb){
  {
	  MutexLockGuard lock(_mutex);
	  _pending_funcs.push_back(std::move(cb));
  }

  if (!isInLoopThread() || _calling_pending_funcs){
      wakeup();
  }
}

void EventLoop::updateChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  _poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (_event_handling)
  {
    assert(_cur_active_channel == channel ||
        std::find(_active_channels.begin(), _active_channels.end(), channel) == _active_channels.end());
  }
  _poller->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  return _poller->hasChannel(channel);
}