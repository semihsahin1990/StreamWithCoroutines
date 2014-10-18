#include "streamc/runtime/FlowRunnerImpl.h"
#include "streamc/runtime/SchedulerPlugin.h"
#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/RuntimeLogger.h"

#include <memory>

using namespace std;
using namespace streamc;

//constructor
FlowRunnerImpl::FlowRunnerImpl()
{}

//destructor
FlowRunnerImpl::~FlowRunnerImpl()
{}

//run the flow with numThreads threads
void FlowRunnerImpl::run(Flow & flow, int numThreads, SchedulerPlugin * plugin/*=nullptr*/)
{
  lock_guard<std::mutex> lock(mutex_);
  flowContexts_[&flow] = unique_ptr<FlowContext>(new FlowContext(flow, plugin));
  flowContexts_[&flow]->run(numThreads);
}

//pause the main thread of flow
void FlowRunnerImpl::wait(Flow & flow)
{
  lock_guard<mutex> lock(mutex_);
  FlowContext & flowContext = *flowContexts_[&flow];
  flowContext.wait();
}

//shutdown the flow execution
void FlowRunnerImpl::requestShutdown(Flow & flow)
{
  lock_guard<std::mutex> lock(mutex_);
  FlowContext & flowContext = *flowContexts_[&flow];
  flowContext.requestShutdown();
}

void FlowRunnerImpl::setInfrastructureLogLevel(LogLevel level) 
{
  lock_guard<std::mutex> lock(mutex_);
  RuntimeLogger::getLogger().setInfrastructureLogLevel(level);
}

void FlowRunnerImpl::setApplicationLogLevel(LogLevel level) 
{
  lock_guard<std::mutex> lock(mutex_);
  RuntimeLogger::getLogger().setApplicationLogLevel(level);
}
