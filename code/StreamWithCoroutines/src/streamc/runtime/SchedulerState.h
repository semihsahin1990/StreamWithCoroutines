#pragma once

#include "streamc/runtime/HashHelpers.h"
#include "streamc/runtime/SchedulerState.h"

#include <condition_variable>
#include <unordered_set>
#include <unordered_map>

namespace streamc
{

class ThreadInfo
{
public:
  enum ThreadState { Running,  // running an operator
                     Ready,    // can run an operator (given scheduler assigns one)
                     Waiting,  // waiting for an operator to be available
                     Completed // no more work to do
  };
  ThreadInfo() {}
  ThreadInfo(WorkerThread * thread) : thread_(thread), state_(Ready), oper_(NULL) {}
  WorkerThread & getThread() { return *thread_; }
  ThreadState getState() { return state_; }
  void setState(ThreadState state) { state_ = state; }
  OperatorContextImpl & getOperator() { return *oper_; }
  void setOperator(OperatorContextImpl & oper) { oper_ = &oper; }
  std::condition_variable & getCV() { return cv_; }
private:
  WorkerThread * thread_; 
  ThreadState state_;
  OperatorContextImpl * oper_; // last operator executed by this thread
  std::condition_variable cv_;
};
  
class OperatorInfo
{
public:
  class WaitCondition
  {
  public:
    typedef std::unordered_map<InputPortImpl *, size_t> PortWaitList;
    WaitCondition() {}
    WaitCondition(OperatorContextImpl & oper);
    void setWait(InputPortImpl & iport, size_t count);
    size_t getWait(InputPortImpl & iport);
    bool isReady();
    void reset(); 
  private:
    PortWaitList portWaits_;
  };
public:
  enum OperatorState { Running,  // running on a thread
                       Ready,    // can run on a thread (given scheduler assigns one)
                       Waiting,  // waiting for data to be available
                       Completed // no work to do
  };
  OperatorInfo() {}
  OperatorInfo(OperatorContextImpl & oper) : oper_(&oper), state_(Ready), 
                                             thread_(NULL) , cond_(oper) {}
  OperatorContextImpl & getOperatorContext() { return *oper_; }
  OperatorState getState() { return state_; }
  void setState(OperatorState state) { state_ = state; }
  WorkerThread & getThread() { return *thread_; }
  void setThread(WorkerThread & thread) { thread_ = &thread; }
  WaitCondition & getWaitCondition();
private:
  OperatorContextImpl * oper_;
  OperatorState state_;
  WorkerThread * thread_; // last thread that executed this operator
  WaitCondition cond_;
};

} /* namespace streamc */
