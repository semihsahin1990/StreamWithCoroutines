#pragma once

#include "streamc/runtime/HashHelpers.h"
#include "streamc/runtime/SchedulerState.h"

#include <chrono>
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
  struct ThresholdAndCount
  {
    size_t threshold;
    size_t currentCount;
    ThresholdAndCount() {}
    ThresholdAndCount(size_t thresholdIn, size_t currentCountIn)
      : threshold(thresholdIn), currentCount(currentCountIn) {}
  };
  class ReadWaitCondition
  {
  public:
    typedef std::unordered_map<InputPortImpl *, ThresholdAndCount> PortWaitList;
    ReadWaitCondition() {}
    ReadWaitCondition(OperatorContextImpl & oper);
    void setWaitThreshold(InputPortImpl & iport, size_t thresh);
    size_t getWaitThreshold(InputPortImpl & iport);
    bool computeReadiness();
    bool isReady(InputPortImpl & iport);
    void reset(); 
    PortWaitList const & getWaitList() { return portWaits_; }
  private:
    PortWaitList portWaits_;
  };
  class WriteWaitCondition
  {
  public:
    typedef std::unordered_map<InputPortImpl *, ThresholdAndCount> PortWaitList;
    WriteWaitCondition() {}
    WriteWaitCondition(OperatorContextImpl & oper);
    void setWaitThreshold(InputPortImpl & iport, size_t thresh);
    size_t getWaitThreshold(InputPortImpl & iport);
    bool computeReadiness();
    bool isReady(InputPortImpl & iport);
    PortWaitList const & getWaitList() { return portWaits_; }
  private:
    PortWaitList portWaits_;
  };
public:
  enum OperatorState { Running,      // running on a thread
                       Ready,        // can run on a thread (given scheduler assigns one)
                       ReadBlocked,  // waiting for data to be available at its input ports
                       WriteBlocked, // waiting for data to be available at its output ports
                       Completed     // no work to do
  };
  OperatorInfo() {}
  OperatorInfo(OperatorContextImpl & oper) : oper_(&oper), state_(Ready), thread_(NULL) , 
                                             readCond_(oper), writeCond_(oper) {}
  OperatorContextImpl & getOperatorContext() { return *oper_; }
  OperatorState getState() { return state_; }
  void setState(OperatorState state) { state_ = state; }
  WorkerThread & getThread() { return *thread_; }
  void setThread(WorkerThread & thread) { thread_ = &thread; }
  ReadWaitCondition & getReadWaitCondition() { return readCond_; }
  WriteWaitCondition & getWriteWaitCondition() { return writeCond_; }
  void setBeginTime(std::chrono::high_resolution_clock::time_point beginTime) { beginTime_ = beginTime; }
  std::chrono::high_resolution_clock::time_point getBeginTime() { return beginTime_; }
  void setEndTime(std::chrono::high_resolution_clock::time_point endTime) { endTime_ = endTime; }
  std::chrono::high_resolution_clock::time_point getEndTime() { return endTime_; }
private:
  OperatorContextImpl * oper_;
  OperatorState state_;
  WorkerThread * thread_; // last thread that executed this operator
  std::chrono::high_resolution_clock::time_point beginTime_;
  std::chrono::high_resolution_clock::time_point endTime_;
  ReadWaitCondition readCond_;
  WriteWaitCondition writeCond_;
};

} /* namespace streamc */
