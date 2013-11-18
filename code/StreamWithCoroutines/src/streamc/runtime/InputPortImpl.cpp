#include "streamc/runtime/InputPortImpl.h"

#include "streamc/runtime/OperatorContextImpl.h"

#include <thread>
#include <mutex>

using namespace std;
using namespace streamc;

//constructor
InputPortImpl::InputPortImpl()
  : isComplete_(false) 
{}

//add publisher operator(operator context) to this port
void InputPortImpl::addPublisher(OperatorContextImpl * oper)
{
  publishers_.push_back(oper);
}

//push tuple to the queue
void InputPortImpl::pushTuple(Tuple const & tuple)
{
  lock_guard<mutex> lock(mutex_);
  portQueue_.push_back(tuple);
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
  for (OperatorContextImpl * opc : publishers_)
  {
    if (!opc->isComplete())
    {
      allComplete = false;
      break;
    }
  }
  if (allComplete) 
  {
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
return true if port is completed
return false if portQueue has tuple
*/
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

