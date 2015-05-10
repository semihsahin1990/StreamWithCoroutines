#include "streamc/runtime/OutputPortImpl.h"

#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/Scheduler.h"

#include <thread>
#include <mutex>

using namespace std;
using namespace streamc;

OutputPortImpl::OutputPortImpl(OperatorContextImpl & oper, FlowContext & flowContext, Scheduler & scheduler)
  : oper_(&oper), flowContext_(&flowContext), scheduler_(&scheduler)
{}

// add (subscriber operator(operator context), portNo) pair to this port
void OutputPortImpl::addSubscriber(OperatorContextImpl & oper, size_t inPort)
{
  subscribers_.push_back(std::make_pair(&oper, inPort));
}

void OutputPortImpl::removeSubscriber(size_t index)
{
  subscribers_.erase(subscribers_.begin()+index);
}

void OutputPortImpl::removeSubscriber(OperatorContextImpl & oper) {
  size_t numberOfSubscribers = subscribers_.size();
  for (size_t i=0; i<numberOfSubscribers; i++) {
    if(subscribers_[i].first == &oper) {
      removeSubscriber(i);
      break;
    }
  }
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
  bool needToWait = true;
  while (needToWait) {
    unordered_map<InputPortImpl *, size_t> waitSpec;
    {
      lock_guard<mutex> lock(mutex_);
      // If there is a global shutdown request, we don't care about queue
      // sizes, we push and return, so that our own operator can exit its
      // main loop as well
      if (!flowContext_->isShutdownRequested()) {
        for(auto const & opPortPair : subscribers_) {
          OperatorContextImpl * op = opPortPair.first;
          size_t inPort = opPortPair.second;
          InputPortImpl & iport = op->getInputPortImpl(inPort);      
          if (iport.getTupleCount()>=FlowContext::getMaxQueueSize()) 
            waitSpec[&iport] = FlowContext::getMaxQueueSize();
        }
      }      
      if (waitSpec.size()==0) { // no need to wait
        needToWait = false;

        // update production counters
        OperatorInfo & oinfo = scheduler_->getOperatorInfo(oper_);
        oinfo.updateOPortCounter(*this);

        for(auto const & opPortPair : subscribers_) {
          OperatorContextImpl * op = opPortPair.first;
          size_t inPort = opPortPair.second;
          op->getInputPortImpl(inPort).pushTuple(tuple);
        }
      }
    }
    if (needToWait) {
      // we need to ask the scheduler to move us into blocked state
      scheduler_->markOperatorAsWriteBlocked(*oper_, waitSpec); 
      // Scheduler returns back to us either because the downstream ports 
      // are now not full (safe to submit), or because one of the dowstream 
      // operators are complete due to global shutdown request and their intput 
      // port is full. In the latter case, we should submit the tuple (potentially 
      // exceeding the limit and go back to operator code, so that our operator 
      // exists as well.
    } else {
      // we need to check with the scheduler to see if we need to preempt
      scheduler_->checkOperatorForBlocking(*oper_);
      scheduler_->checkOperatorForPreemption(*oper_);
    }
  }
}

double OutputPortImpl::getWriteBlockedRatio() {
  lock_guard<mutex> lock(mutex_);

  double maxWriteBlockedRatio=0;
  for(auto const & opPortPair : subscribers_) {
    OperatorContextImpl * op = opPortPair.first;
    size_t inPort = opPortPair.second;
    InputPortImpl & iport = op->getInputPortImpl(inPort);
    
    double writeBlockedRatio = iport.getWriteBlockedRatio();
    if(writeBlockedRatio > maxWriteBlockedRatio)
      maxWriteBlockedRatio = writeBlockedRatio;
      
  }
  return maxWriteBlockedRatio;
}

void OutputPortImpl::resetBeginTime() {
  lock_guard<mutex> lock(mutex_);

  for(auto const & opPortPair : subscribers_) {
    OperatorContextImpl * op = opPortPair.first;
    size_t inPort = opPortPair.second;
    InputPortImpl & iport = op->getInputPortImpl(inPort);
    iport.resetBeginTime();
  }
}