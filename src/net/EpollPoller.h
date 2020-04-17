#pragma once

#include <sys/epoll.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <unistd.h>
#include <string.h>

#include "src/net/Poller.h"

struct epoll_event;

namespace serverlib{
	

class EPollPoller : public Poller{
	
 public:
	enum epoll_flag{
		E_NEW = -1,
		E_ADDED = 1,
		E_DELETED = 2,
	 };
	EPollPoller(EventLoop* loop);
	~EPollPoller() override;

	void poll(int timeoutMs, std::vector<Channel*>* activeChannels) override;
	void updateChannel(Channel* channel) override;
	void removeChannel(Channel* channel) override;
	int getEpollfd() { return _epoll_fd; }
	
 private:
	int _epoll_fd;      //epoll红黑树的根节点
	std::vector<struct epoll_event> _events;
};

}