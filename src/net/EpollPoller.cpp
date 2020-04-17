#include "src/net/EpollPoller.h"
#include "src/net/Channel.h"


const int EVENTSNUM = 16;

using namespace serverlib;


EPollPoller::EPollPoller(EventLoop* loop)
  : Poller(loop),
    _epoll_fd(::epoll_create1(EPOLL_CLOEXEC)),
    _events(EVENTSNUM){
		
	if (_epoll_fd < 0)  {
		perror("EPollPoller::EPollPoller");
	}
}

EPollPoller::~EPollPoller(){
    ::close(_epoll_fd);
}

void EPollPoller::poll(int timeoutMs, std::vector<Channel*>* activeChannels){
	int num_events = ::epoll_wait(_epoll_fd,
							   &*_events.begin(),
							   static_cast<int>(_events.size()),
							   timeoutMs);
	int savedErrno = errno;
	//返回timestamp信息
	struct timeval tv;
	//gettimeofday(&tv, NULL);
	//int64_t seconds = tv.tv_sec;
	//now = seconds * kMicroSecondsPerSecond + tv.tv_usec);

	if (num_events > 0) {//有活跃事件
		assert(num_events <= _events.size());
		
		 /*
		  将记录了活跃事件成员events塞到activeChannels中，因为poll函数主要
		  由EventLoop类调用，传递参数，activeChannels是EventLoop类的成员变量。
		  */
		for (int i = 0; i < num_events; ++i){
			Channel* channel = static_cast<Channel*>(_events[i].data.ptr);
			//进行出错验证
			//ChannelMap::const_iterator it = _channels.find(fd);
			//assert(it != _channels.end());
			//assert(it->second == channel);
			
			channel->setRevents(_events[i].events);
			activeChannels->push_back(channel);
		}
		if (num_events == _events.size()){
			_events.resize(_events.size()*2);
		}
	}
	else if (num_events == 0)  {}
	else{
		// error happens, log uncommon ones
		if (savedErrno != EINTR){
			errno = savedErrno;
			perror("EPollPoller::poll()");
		}
	}	
}

void EPollPoller::updateChannel(Channel* channel)
{
	Poller::assertInLoopThread();
	const int index = channel->index();
	
	int fd = channel->fd();
	struct epoll_event event;
	bzero(&event, sizeof event);
	//这两行代码反应channel与struct epoll_event的映射。
	event.events = channel->events();
	event.data.ptr = channel;
	

	if (index == E_NEW || index == E_DELETED){ //说明该channel未注册到epollfd中
		// a new one, add with EPOLL_CTL_ADD		
		if (index == E_NEW) {//kNew表示std::map<int,channel*> _channels也没有，添加进去。
			assert(_channels.find(fd) == _channels.end()); 
			_channels[fd] = channel;
		}
		else{ // index == E_DELETED
			assert(_channels.find(fd) != _channels.end());
			assert(_channels[fd] == channel);
		}		
		if (::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0){//epoll添加事件结点
			perror("epoll_add error");
		}      
		channel->set_index(E_ADDED);
	}
	else { // index == E_ADDED
		// update existing one with EPOLL_CTL_MOD/DEL
		(void)fd;
		assert(_channels.find(fd) != _channels.end());
		assert(_channels[fd] == channel);
		assert(index == E_ADDED);
		
		fd = channel->fd();		

		if (channel->isNoneEvent()){//不监听事件了
		    if (::epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, &event) < 0){//epoll删除事件结点
				perror("epoll_del error");
			}      
		    channel->set_index(E_DELETED);
		}
		else {//已注册的channel，也许是更改了一些监听事件类型
			if (::epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0){//epoll修改事件结点
				perror("epoll_mod error");
			}  
		}
	}
}

void EPollPoller::removeChannel(Channel* channel)
{
	Poller::assertInLoopThread();
	int fd = channel->fd();
	assert(_channels.find(fd) != _channels.end());
	assert(_channels[fd] == channel);
	assert(channel->isNoneEvent());
	
	int index = channel->index();
	assert(index == E_ADDED || index == E_DELETED);
	
	size_t n = _channels.erase(fd);
	(void)n;
	assert(n == 1);

	if (index == E_ADDED){
		struct epoll_event event;
		bzero(&event, sizeof event);
		event.events = channel->events();
		event.data.ptr = channel;
		if (::epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, &event) < 0){//epoll删除事件结点
			perror("epoll_del error");
		}  
	}
	channel->set_index(E_NEW);
}

