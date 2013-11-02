#include "streamc/runtime/FlowRunnerImpl.h"

#include "streamc/runtime/FlowContext.h"

#include <memory>

using namespace std;
using namespace streamc;

FlowRunnerImpl::FlowRunnerImpl()
{}

FlowRunnerImpl::~FlowRunnerImpl()
{}

void FlowRunnerImpl::run(Flow & flow, int numThreads)
{
  lock_guard<std::mutex> lock(mutex_);
  uintptr_t flowAddr = reinterpret_cast<uintptr_t>(&flow);
  flowContexts_[flowAddr] = unique_ptr<FlowContext>(new FlowContext(flow));
  flowContexts_[flowAddr]->run(numThreads);
}

void FlowRunnerImpl::wait(Flow & flow)
{
  lock_guard<mutex> lock(mutex_);
  uintptr_t flowAddr = reinterpret_cast<uintptr_t>(&flow);
  FlowContext & flowContext = *flowContexts_[flowAddr];
  flowContext.wait();
}

void FlowRunnerImpl::requestShutdown(Flow & flow)
{
  lock_guard<std::mutex> lock(mutex_);
  uintptr_t flowAddr = reinterpret_cast<uintptr_t>(&flow);
  FlowContext & flowContext = *flowContexts_[flowAddr];
  flowContext.requestShutdown();
}
