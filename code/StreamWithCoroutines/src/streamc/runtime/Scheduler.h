#pragma once

#include "streamc/runtime/HashHelpers.h"

#include <mutex>
#include <unordered_map>

namespace streamc
{

class WorkerThread;
class OperatorContextImpl;

class Scheduler
{
private:
  class ThreadInfo
  {
  public:
    enum ThreadState { Running, Waiting };
    ThreadInfo() {}
    ThreadInfo(WorkerThread * thread) : thread_(thread), state_(Running) {}
    WorkerThread & getThread() { return *thread_; }
    ThreadState getState() { return state_; }
    void setState(ThreadState state) { state_ = state; }
  private:
    WorkerThread * thread_;
    ThreadState state_;
  };
  class OperatorInfo
  {
  public:
    enum OperatorState { Running, Ready, Waiting, Completed };
    OperatorInfo() {}
    OperatorInfo(OperatorContextImpl * oper) : oper_(oper), state_(Ready) {}
    OperatorContextImpl & getOperatorContext() { return *oper_; }
    OperatorState getState() { return state_; }
    void setState(OperatorState state) { state_ = state; }
  private:
    OperatorContextImpl * oper_;
    OperatorState state_;
  };
public:
  Scheduler() {}
  void addThread(WorkerThread & thread);
  void addOperatorContext(OperatorContextImpl & oper);
  void markOperatorCompleted(OperatorContextImpl & oper);
  void start();
  void stop();
protected:
  ThreadInfo & getThreadInfo(WorkerThread & thread);
  OperatorInfo & getOperatorInfo(OperatorContextImpl & context);
  void updateOperatorState(OperatorContextImpl & oper, OperatorInfo::OperatorState state);
private:
  std::unordered_map<WorkerThread *, ThreadInfo> threads_;
  std::unordered_map<OperatorContextImpl *, OperatorInfo> operContexts_;
  // TODO: add indices
  std::mutex mutex_; 
};

} /* streamc */
