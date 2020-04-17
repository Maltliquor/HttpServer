#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <atomic>

#include "src/net/Poller.h"
#include "src/base/CurrentThread.h"
#include "src/base/Thread.h"


#include <iostream>
using namespace std;


namespace serverlib{

class Channel;
	
class EventLoop {
 public:
  typedef std::function<void()> FuncCallBack;
  EventLoop();
  ~EventLoop();
 
  /// Loops forever.
  /// Must be called in the same thread as creation of the object.
  void loop();
  
  /// Quits loop.
  /// This is not 100% thread safe, if you call through a raw pointer,
  /// better to call through shared_ptr<EventLoop> for 100% safety.
  void quit();
  
  void runInLoop(FuncCallBack cb);
  void queueInLoop(FuncCallBack cb);
  bool isInLoopThread() const { return _threadID == CurrentThread::tid(); }
  void assertInLoopThread() { assert(isInLoopThread()); }
  
  int iteration() const { return _iter; }
  bool isEventHandling() const { return _event_handling; }
  EventLoop* getEventLoopOfCurrentThread();
  ///
  /// Cancels the timer.
  /// Safe to call from other threads.
  ///
  //void cancel();
  
  // internal usage
  void wakeup();
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);
  bool hasChannel(Channel* channel);  
  
  //void shutdown(shared_ptr<Channel> channel) { shutDownWR(channel->getFd()); }


 private:
	// 声明顺序 wakeupFd_ > pwakeupChannel_
	bool _looping;                   //是否处于循环之中
	std::unique_ptr<Poller> _poller;
	int _wakeup_fd;
	std::atomic<bool> _quit;               //退出while(1)循环  
	bool _event_handling;            //是否正在处理事件
	bool _calling_pending_funcs;
	const pid_t _threadID;            //持有当前loop的线程ID
	shared_ptr<Channel> _wakeup_channel;
	
	int _iter;
	mutable MutexLock _mutex; 	
	std::vector<Channel*> _active_channels;
	std::vector<FuncCallBack> _pending_funcs;

	Channel* _cur_active_channel;

	void handleRead();
	void doPendingFunctors();
	void handleConn();
};

}