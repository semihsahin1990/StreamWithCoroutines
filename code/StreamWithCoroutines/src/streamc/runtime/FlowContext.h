#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unordered_map>

namespace streamc
{

class Flow;
class Operator;
class OperatorContextImpl;

class FlowContext
{
public:
  FlowContext(Flow & flow);
  ~FlowContext();

  void run(int numThreads);
  void wait();
  // called by the OperatorContextImpl when an operator completes its execution
  void markOperatorCompleted(Operator * oper);

  void requestShutdown();
  bool isShutdownRequested();
private:
  Flow & flow_;
  
  size_t numCompleted_;
  std::mutex mutex_; 
  std::condition_variable cv_;
  
  // maps from Operator * to operator context
  std::unordered_map<uintptr_t, std::unique_ptr<OperatorContextImpl>> operatorContexts_;
  std::atomic<bool> isShutdownRequested_;
};

} // namespace streamc
