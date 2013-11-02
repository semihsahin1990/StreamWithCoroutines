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
  FlowRunnerImpl();
  ~FlowRunnerImpl();
  void run(Flow & flow, int numThreads);
  void requestShutdown(Flow & flow);  
  void wait(Flow & flow);  
private:
  std::unordered_map<uintptr_t, std::unique_ptr<FlowContext>> flowContexts_;
  std::mutex mutex_; 
};

} // namespace streamc
