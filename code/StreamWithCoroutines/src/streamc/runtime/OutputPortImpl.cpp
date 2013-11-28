#include "streamc/runtime/OutputPortImpl.h"

#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/Scheduler.h"

#include <thread>
#include <mutex>

using namespace std;
using namespace streamc;

OutputPortImpl::OutputPortImpl(OperatorContextImpl & oper, Scheduler & scheduler)
  : oper_(&oper), scheduler_(&scheduler)
{}

// add (subscriber operator(operator context), portNo) pair to this port
void OutputPortImpl::addSubscriber(OperatorContextImpl & oper, size_t inPort)
{
  subscribers_.push_back(std::make_pair(&oper, inPort));
}

size_t OutputPortImpl::getNumberOfSubscribers()
{
  return subscribers_.size();
}

pair<OperatorContextImpl *, size_t> OutputPortImpl::getSubscriber(size_t index)
{
  return subscribers_[index];
}

// push tuple to the input port of each subscriber
void OutputPortImpl::pushTuple(Tuple const & tuple)
{
  {
    lock_guard<mutex> lock(mutex_);
    for(auto const & opPortPair : subscribers_) {
      OperatorContextImpl * op = opPortPair.first;
      size_t inPort = opPortPair.second;
      op->getInputPortImpl(inPort).pushTuple(tuple);
    }
  }
  // we need to check with the scheduler to see if we need to preempt
  scheduler_->checkOperatorForPreemption(*oper_);
}

