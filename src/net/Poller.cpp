#include "src/net/Poller.h"
#include "src/net/Channel.h"
#include "src/net/EventLoop.h"

using namespace serverlib;

Poller::Poller(EventLoop* loop)
  : _owner_loop(loop)
{
}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel* channel) const
{
  assertInLoopThread();
  std::map<int, Channel*>::const_iterator it = _channels.find(channel->fd());
  return it != _channels.end() && it->second == channel;
}

void Poller::assertInLoopThread() const
{
	_owner_loop->assertInLoopThread();
}

