#pragma once

#include <atomic>

namespace streamc
{

class SpinLock 
{
private:
  enum LockState { Locked, Unlocked };
  std::atomic<LockState> state_;

public:
  SpinLock()
    : state_(Unlocked) 
  {}

  void lock()
  {
    while (state_.exchange(Locked, std::memory_order_acquire) == Locked);
  }

  void unlock()
  {
    state_.store(Unlocked, std::memory_order_release);
  }

};

} // namespace streamc
