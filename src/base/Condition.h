#pragma once

#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <cstdint>

#include "src/base/Mutex.h"


class Condition {
 public:
  explicit Condition(MutexLock &_mutex) : mutex(_mutex) {
    pthread_cond_init(&cond, NULL);
  }
  ~Condition() { pthread_cond_destroy(&cond); }
  void wait() { pthread_cond_wait(&cond, mutex.get()); }
  void condSignal() { pthread_cond_signal(&cond); }
  void condBroadcast() { pthread_cond_broadcast(&cond); }
  bool waitForSeconds(int seconds) {
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    abstime.tv_sec += static_cast<time_t>(seconds);
    return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get(), &abstime);
  }

 private:
  MutexLock &mutex;
  pthread_cond_t cond;
};