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
  uintptr_t flowAddr = reinterpret_cast<uintptr_t>(&flow);
  flowContexts_[flowAddr] = unique_ptr<FlowContext>(new FlowContext(flow));
  flowContexts_[flowAddr]->run(numThreads);
}

//pause the main thread of flow
void FlowRunnerImpl::wait(Flow & flow)
{
  lock_guard<mutex> lock(mutex_);
  uintptr_t flowAddr = reinterpret_cast<uintptr_t>(&flow);
  FlowContext & flowContext = *flowContexts_[flowAddr];
  flowContext.wait();
}

//shutdown the flow execution
void FlowRunnerImpl::requestShutdown(Flow & flow)
{
  lock_guard<std::mutex> lock(mutex_);
  uintptr_t flowAddr = reinterpret_cast<uintptr_t>(&flow);
  FlowContext & flowContext = *flowContexts_[flowAddr];
  flowContext.requestShutdown();
}
