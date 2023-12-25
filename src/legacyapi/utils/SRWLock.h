#pragma once
#include <synchapi.h>

class SRWLock {
  bool inited = false;
  SRWLOCK srwlock{};

public:
  SRWLock();
  void lock();
  bool try_lock();
  void unlock();
  void lock_shared();
  bool try_lock_shared();
  void unlock_shared();
};

class SRWLockHolder {
  SRWLock &locker;

public:
  inline SRWLockHolder(SRWLock &lock) : locker(lock) { locker.lock(); }
  inline ~SRWLockHolder() { locker.unlock(); }
};

class SRWLockSharedHolder {
  SRWLock &locker;

public:
  inline SRWLockSharedHolder(SRWLock &lock) : locker(lock) {
    locker.lock_shared();
  }
  inline ~SRWLockSharedHolder() { locker.unlock_shared(); }
};