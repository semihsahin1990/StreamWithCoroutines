#include "streamc/runtime/InputPortImpl.h"

#include "streamc/runtime/OperatorContextImpl.h"

#include <thread>
#include <mutex>

using namespace std;
using namespace streamc;

InputPortImpl::InputPortImpl()
  : isComplete_(false) 
{}

void InputPortImpl::addPublisher(OperatorContextImpl * oper)
{
  publishers_.push_back(oper);
}

void InputPortImpl::pushTuple(Tuple const & tuple)
{
  lock_guard<mutex> lock(mutex_);
  portQueue_.push_back(tuple);
}

bool InputPortImpl::isComplete() 
{
  lock_guard<mutex> lock(mutex_);
  return isCompleteNoLock();
}

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

bool InputPortImpl::hasTuple() 
{
  lock_guard<mutex> lock(mutex_);
  return !portQueue_.empty();
}

size_t InputPortImpl::getTupleCount() 
{
  lock_guard<mutex> lock(mutex_);
  return portQueue_.size();
}

// return true if port is completed
bool InputPortImpl::waitTuple() 
{
  {
    lock_guard<mutex> lock(mutex_);
    if (!portQueue_.empty())
      return false; 
    if (isComplete())
      return true;
  }
  // TODO: we need to hook into the scheduler to get
  // descheduled, since we need to wait
  return false;
}

Tuple & InputPortImpl::getFrontTuple() 
{
  lock_guard<mutex> lock(mutex_);
  return portQueue_.front();
}

Tuple & InputPortImpl::getTupleAt(size_t index) 
{
  lock_guard<mutex> lock(mutex_);
  return portQueue_.front();
}

void InputPortImpl::popTuple() 
{
  lock_guard<mutex> lock(mutex_);
  portQueue_.pop_front();
}



