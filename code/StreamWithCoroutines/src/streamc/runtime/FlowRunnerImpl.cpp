#include "streamc/runtime/FlowRunnerImpl.h"

#include "streamc/runtime/FlowContext.h"

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
void FlowRunnerImpl::run(Flow & flow, int numThreads)
{
  lock_guard<std::mutex> lock(mutex_);
  flowContexts_[&flow] = unique_ptr<FlowContext>(new FlowContext(flow));
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
