#pragma once

#include "streamc/FlowRunner.h"

#include <unordered_map>
#include <memory>
#include <mutex>

namespace streamc
{

class Flow;
class FlowContext;

class FlowRunnerImpl : public FlowRunner
{
public:
  //constructor
  FlowRunnerImpl();

  //destructor
  ~FlowRunnerImpl();

  //run flow with numThreads threads
  void run(Flow & flow, int numThreads);

  //shutdown the flow execution
  void requestShutdown(Flow & flow);  

  //pause the main thread of a flow
  void wait(Flow & flow);

private:
  //maps flows to flowContexts
  std::unordered_map<uintptr_t, std::unique_ptr<FlowContext>> flowContexts_;
  std::mutex mutex_; 
};

} // namespace streamc
