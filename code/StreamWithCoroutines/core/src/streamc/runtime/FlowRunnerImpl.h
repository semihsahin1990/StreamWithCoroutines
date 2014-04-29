#pragma once

#include "streamc/runtime/HashHelpers.h"
#include "streamc/FlowRunner.h"

#include <unordered_map>
#include <memory>
#include <mutex>

namespace streamc
{

class Flow;
class FlowContext;
class SchedulerPlugin;
class Operator;

class FlowRunnerImpl : public FlowRunner
{
public:
  //constructor
  FlowRunnerImpl();

  //destructor
  ~FlowRunnerImpl();

  //run flow with numThreads threads
  void run(Flow & flow, int numThreads, SchedulerPlugin & plugin) override;

  //shutdown the flow execution
  void requestShutdown(Flow & flow) override;  

  //pause the main thread of a flow
  void wait(Flow & flow) override;

  void addFission(Flow & flow, Operator * oper, size_t replicaCount) override;
/*
  // set the log level of the runtime infrastructure
  void setInfrastructureLogLevel(LogLevel level);

  // set the log level of the application
  void setApplicationLogLevel(LogLevel level);
*/
private:
  //maps flows to flowContexts
  std::unordered_map<Flow *, std::unique_ptr<FlowContext>> flowContexts_;
  std::mutex mutex_; 
};

} // namespace streamc
