#include "streamc/runtime/InputPortImpl.h"

#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/Scheduler.h"

#include <thread>
#include <mutex>

using namespace std;
using namespace streamc;

//constructor
InputPortImpl::InputPortImpl(Scheduler & scheduler)
  : scheduler_(&scheduler), isComplete_(false) 
{}

//add publisher operator(operator context) to this port
void InputPortImpl::addPublisher(OperatorContextImpl & oper)
{
  publishers_.push_back(&oper);
}

//push tuple to the queue
void InputPortImpl::pushTuple(Tuple const & tuple)
{
  lock_guard<mutex> lock(mutex_);
  portQueue_.push_back(tuple);
  scheduler_->markChangeInInputPortSize(*this);
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

/*
return true iff port is completed
*/
bool InputPortImpl::waitTuple() 
{
  bool needToWait = true;
  {
    lock_guard<mutex> lock(mutex_);
    if (!portQueue_.empty())
      needToWait = false;
    else if (isComplete())
      return true;
  }
  if (needToWait) {
    // TODO: we need to hook into the scheduler to get
    // descheduled, since we need to wait
  } else {
    // TODO: check with scheduler to see if we need to
    // preempt
  }
  return false;
}

//return the next tuple
Tuple & InputPortImpl::getFrontTuple() 
{
  lock_guard<mutex> lock(mutex_);
  return portQueue_.front();
}

//TODO: kod hatali sanki
//return the index-th tuple
Tuple & InputPortImpl::getTupleAt(size_t index) 
{
  lock_guard<mutex> lock(mutex_);
  return portQueue_.front();
}

//remove the next tuple
void InputPortImpl::popTuple() 
{
  lock_guard<mutex> lock(mutex_);
  portQueue_.pop_front();
}

