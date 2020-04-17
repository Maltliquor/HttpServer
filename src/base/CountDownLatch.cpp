#include "src/base/CountDownLatch.h"

using namespace serverlib;

CountDownLatch::CountDownLatch(int count)
  : _mutex(),
    _condition(_mutex),
    _count(count)
{
}

void CountDownLatch::wait()
{
  MutexLockGuard lock(_mutex);
  while (_count > 0)
  {
    _condition.wait();
  }
}

void CountDownLatch::countDown()
{
  MutexLockGuard lock(_mutex);
  --_count;
  if (_count == 0)
  {
    _condition.condBroadcast();
  }
}

int CountDownLatch::getCount() const
{
  MutexLockGuard lock(_mutex);
  return _count;
}