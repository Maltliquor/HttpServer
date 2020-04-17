#pragma once

#include "src/base/Condition.h"
#include "src/base/Mutex.h"

namespace serverlib
{

class CountDownLatch
{
 public:

	explicit CountDownLatch(int count);

	void wait();

	void countDown();

	int getCount() const;

 private:
	mutable MutexLock _mutex;
	Condition _condition;
	int _count;
};

}  // namespace serverlib
