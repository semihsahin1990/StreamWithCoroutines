#pragma once

#include "streamc/runtime/HashHelpers.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <unordered_map>

namespace streamc
{

class Flow;
class Operator;
class OperatorContextImpl;
class WorkerThread;
class Scheduler;
class SchedulerPlugin;

class FlowContext
{
public:
  //constructor with flow
  FlowContext(Flow & flow, SchedulerPlugin * plugin);
 
  //destructor
  ~FlowContext();
  
  //run thread with numThreads threads
  void run(int numThreads);

  //pause the main thread in the flow
  void wait();

  // called by the OperatorContextImpl when an operator completes its execution
  void markOperatorCompleted(Operator * oper);
  
  //shutdown is requested from the outside
  void requestShutdown();

  //returns whether shutdown is requested or not
  bool isShutdownRequested();

  static size_t getMaxQueueSize() { return maxQueueSize_; }

private:
  Flow & flow_;

  // scheduler and threads
  std::unique_ptr<Scheduler> scheduler_;
  std::vector<std::unique_ptr<WorkerThread>> threads_;
  static size_t maxQueueSize_;
  
  //number of operators completed
  size_t numCompleted_;
  std::mutex mutex_; 
  std::condition_variable cv_;
  
  // maps from operator to operator context
  std::unordered_map<Operator *, std::unique_ptr<OperatorContextImpl>> operatorContexts_;
  std::atomic<bool> isShutdownRequested_;
};

} // namespace streamc
