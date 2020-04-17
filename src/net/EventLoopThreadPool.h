#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <string>

namespace serverlib
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThreadPool(EventLoop* baseLoop, const std::string& name);
  ~EventLoopThreadPool();
  void setThreadNum(int num_threads) { _num_threads = num_threads; }
  void start(const ThreadInitCallback& cb = ThreadInitCallback());

  // valid after calling start()
  /// round-robin
  EventLoop* getNextLoop();

  std::vector<EventLoop*> getAllLoops();

  bool started() const
  { return _started; }

  const std::string& name() const
  { return _name; }

 private:

  EventLoop* _base_loop;
  std::string _name;
  bool _started;
  int _num_threads;
  int _next;
  std::vector<std::unique_ptr<EventLoopThread>> _threads;
  std::vector<EventLoop*> _loops;
};

}  // namespace serverlib