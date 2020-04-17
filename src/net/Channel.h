#pragma once

#include <functional>
#include <memory>
#include <assert.h>

namespace serverlib
{

class EventLoop;

///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd
class Channel{ 
  
 public:
	typedef std::function<void()> EventCallback;

	Channel(EventLoop* loop, int fd);
	~Channel();

	void handleEvent();
	void setReadCallback(EventCallback cb)
	{ _read_callback = std::move(cb); }
	void setWriteCallback(EventCallback cb)
	{ _write_callback = std::move(cb); }
	void setCloseCallback(EventCallback cb)
	{ _close_callback = std::move(cb); }
	void setErrorCallback(EventCallback cb)
	{ _error_callback = std::move(cb); }
	void setConnHandler(EventCallback cb) 
	{ _conn_handler = cb; }

	int fd() const { return _fd; }
	int events() const { return _events; }
	void setRevents(int revt) { _revents = revt; } // used by pollers
	
	/// Tie this channel to the owner object managed by shared_ptr,
	/// prevent the owner object being destroyed in handleEvent.
	//捆绑Channel的拥有者例如TcpConnection
	//防止Channel还在使用时，拥有者将Channel析构了。
	void tieHolder(const std::shared_ptr<void>& obj){
		_holder = obj;
		_tied = true;
	}
	void handleEvents();
	void handleEventWithGuard();
	
	int revents() const { return _revents; }
	bool isNoneEvent() const { return _events == 0; }

	void bindReadingEvent() { _events |= kReadEvent; update(); }
	void unbindReadingEvent() { _events &= ~kReadEvent; update(); }
	void bindWritingEvent() { _events |= kWriteEvent; update(); }
	void unbindWritingEvent() { _events &= ~kWriteEvent; update(); }
	void unbindAll() { _events = 0; update(); }
	bool isEnableWriting() const { return _events & kWriteEvent; }
	bool isEnableReading() const { return _events & kReadEvent; }
	
	// for Poller
	int index() { return _index; }
	void set_index(int idx) { _index = idx; }

	EventLoop* ownerLoop() { return _loop; }
	
	/// 从epoller中移除事件
	void remove();  
  
  private:
    /// 从epoller中增加、删除或修改事件
	void update();   

	//static const int kNoneEvent=0;
	static const int kReadEvent;
	static const int kWriteEvent;
	

	EventLoop* _loop;
	const int  _fd;    //per fd per channel
	int 	   _priority;
	int        _events; //监听的事件类型，可能是kReadEvent和kWriteEvent的组合
	int        _revents; // it's the received event types of epoll or poll返回的事件类型，单一类型
	int        _index; // used by Poller.  (E_NEW/E_ADDED/E_DEL)表示是否在EPollPoller中注册


	std::weak_ptr<void> _holder;  //拥有该channel的对象，如http_request
	bool _tied;
	bool _event_handling;
	bool _added_to_loop;
	
	EventCallback _read_callback;
	EventCallback _write_callback;
	EventCallback _close_callback;
	EventCallback _error_callback;
	EventCallback _conn_handler;
};

}
