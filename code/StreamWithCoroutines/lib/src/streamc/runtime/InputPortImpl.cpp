#include "streamc/runtime/InputPortImpl.h"

#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/Scheduler.h"
#include "streamc/Operator.h"

#include <thread>
#include <mutex>

using namespace std;
using namespace streamc;

//constructor
InputPortImpl::InputPortImpl(OperatorContextImpl & oper, Scheduler & scheduler)
  : oper_(&oper), scheduler_(&scheduler), isComplete_(false) 
{}

//add publisher operator(operator context) to this port
void InputPortImpl::addPublisher(OperatorContextImpl & oper)
{
  publishers_.push_back(&oper);
}

// push tuple to the queue
void InputPortImpl::pushTuple(Tuple const & tuple)
{
  {
    lock_guard<mutex> lock(mutex_);
    portQueue_.push_back(tuple);
  }

  // scheduler has to check if this causes the operator to go into ready state 
  scheduler_->markInputPortAsWritten(*this);
}

//return isCompleteNoLock()
bool InputPortImpl::isComplete() 
{
  lock_guard<mutex> lock(mutex_);
  return isCompleteNoLock();
}

/*
  if isComplete_ is true return true
  if queue is not empty return false
  if all publishers are complete, then set isComplete as true and return true, otherwise return false
*/
bool InputPortImpl::isCompleteNoLock()
{
  if (isComplete_)
    return true; 
  if (!portQueue_.empty())
    return false;
  bool allComplete = true;
  for (OperatorContextImpl * opc : publishers_) {
    if (!opc->isComplete()) {
      allComplete = false;
      break;
    }
  }
  if (allComplete) {
    isComplete_ = true;
    return true;
  }
  return false;
}

// return whether queue has tuple or not
bool InputPortImpl::hasTuple() 
{
  lock_guard<mutex> lock(mutex_);
  return !portQueue_.empty();
}

//return the size of the queue
size_t InputPortImpl::getTupleCount() 
{
  lock_guard<mutex> lock(mutex_);
  return portQueue_.size();
}

// return true iff port is completed
bool InputPortImpl::waitTuple() 
{
  bool needToWait = true;
  while (needToWait) {
    {
      lock_guard<mutex> lock(mutex_);
      if (!portQueue_.empty())
        needToWait = false;
      else if (isCompleteNoLock())
        return true;
    }
    if (needToWait) {
      // we need to ask the scheduler to move us into blocked state
      scheduler_->markOperatorAsReadBlocked(*oper_, { {this, 1} });  
      // Reaching here does not mean that we do not need to wait anymore,
      // as it might be the case that the scheduler has woken us because
      // the port has closed! That is why we have the while loop above.
    } else {
      // we need to check with the scheduler to see if we need to preempt
      scheduler_->checkOperatorForPreemption(*oper_);
    }
  }
  return false;
}

// return the next tuple
Tuple & InputPortImpl::getFrontTuple() 
{
  lock_guard<mutex> lock(mutex_);
  if (portQueue_.size()==0)
    throw runtime_error("getFrontTuple() called on empty queue, oper="+oper_->getOperator().getName());
  return portQueue_.front();
}

// return the index-th tuple
Tuple & InputPortImpl::getTupleAt(size_t index) 
{
  lock_guard<mutex> lock(mutex_);
  size_t size = portQueue_.size();
  if (size<=index)
    throw runtime_error("getTupleAt("+to_string(index)+") called on queue of size "+to_string(size)+", oper="+oper_->getOperator().getName());
  return portQueue_[index];
}

// remove the next tuple
void InputPortImpl::popTuple() 
{
  {
    lock_guard<mutex> lock(mutex_);
    portQueue_.pop_front();
  }
  // scheduler has to check if this causes other operators to go into ready state 
  scheduler_->markInputPortAsRead(*this);
}

// remove the next tuple
void InputPortImpl::drain() 
{
  {
    lock_guard<mutex> lock(mutex_);
    portQueue_.clear();
  }
  // scheduler has to check if this causes other operators to go into ready state 
  scheduler_->markInputPortAsRead(*this);
}
