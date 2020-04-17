#pragma once

#include <map>
#include <vector>

namespace serverlib
{

class Channel;
class EventLoop;
///
/// Base class for IO Multiplexing
///
/// This class doesn't own the Channel objects.
class Poller
{
 public:

	Poller(EventLoop* loop);
	virtual ~Poller();

	/// Polls the I/O events.
	/// Must be called in the loop thread.
	virtual void poll(int timeoutMs, std::vector<Channel*>* activeChannels) = 0;

	/// Changes the interested I/O events.
	/// Must be called in the loop thread.
	virtual void updateChannel(Channel* channel) = 0;

	/// Remove the channel, when it destructs.
	/// Must be called in the loop thread.
	virtual void removeChannel(Channel* channel) = 0;
	
	/// Assert this channel is existed.
	virtual bool hasChannel(Channel* channel) const;

	static Poller* newDefaultPoller(EventLoop* loop);

	void assertInLoopThread() const;

 protected:
	std::map<int, Channel*> _channels;

 private:
	EventLoop* _owner_loop;
};

} //namespace serverlib