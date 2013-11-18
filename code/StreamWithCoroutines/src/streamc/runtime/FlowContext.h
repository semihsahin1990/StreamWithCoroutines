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
  //constructor with flow
  FlowContext(Flow & flow);
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

private:
  Flow & flow_;
  
  //number of operators completed
  size_t numCompleted_;
  std::mutex mutex_; 
  std::condition_variable cv_;
  
  // maps from Operator * to operator context
  std::unordered_map<uintptr_t, std::unique_ptr<OperatorContextImpl>> operatorContexts_;
  std::atomic<bool> isShutdownRequested_;
};

} // namespace streamc
