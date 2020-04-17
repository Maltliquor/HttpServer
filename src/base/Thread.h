#pragma once

#include <functional>
#include <memory>
#include <pthread.h>
#include <string>  //c++ string类的头文件
#include <assert.h>

#include "src/base/Atomic.h"
#include "src/base/Defination.h"
#include "src/base/CountDownLatch.h"

namespace serverlib
{

class Thread
{
 public:
	typedef std::function<void ()> ThreadFunc;

	explicit Thread(ThreadFunc, const std::string& name = std::string());
	~Thread();

	void start();
	int join(); // return pthread_join()

	bool started() const { return _started; }
	// pthread_t pthreadId() const { return _pthreadId; }
	pid_t tid() const { return _tid; }
	const std::string& name() const { return _name; }

	static int numCreated() { return numCreated_.get(); }
	
	
 private:
	void setDefaultName();

	bool       _started;
	bool       _joined;
	pthread_t  _pthreadId;
	pid_t      _tid;
	ThreadFunc _func;
	std::string     _name;
	CountDownLatch _latch;

	static AtomicInt32 numCreated_;
};

}  // namespace serverlib