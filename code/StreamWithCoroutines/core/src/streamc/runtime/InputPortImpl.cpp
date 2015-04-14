#include "streamc/runtime/InputPortImpl.h"

#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/Scheduler.h"
#include "streamc/Operator.h"

#include <thread>
#include <mutex>

using namespace std;
using namespace std::chrono;
using namespace streamc;

//constructor
InputPortImpl::InputPortImpl(OperatorContextImpl & oper, Scheduler & scheduler)
  : oper_(&oper), scheduler_(&scheduler), isClosed_(false)
{
  inWriteBlockedState_ = false;
  createdAt_ = chrono::high_resolution_clock::now();
  totalWriteBlockedDuration_ = std::chrono::microseconds(0);
}

//add publisher operator(operator context) to this port
void InputPortImpl::addPublisher(OperatorContextImpl & oper, size_t outPort)
{
  publishers_.push_back(std::make_pair(&oper, outPort));
}

size_t InputPortImpl::getNumberOfPublishers()
{
  return publishers_.size();
}

pair<OperatorContextImpl *, size_t> InputPortImpl::getPublisher(size_t index)
{
  return publishers_[index];
}

void InputPortImpl::removePublisher(size_t index)
{
  publishers_.erase(publishers_.begin()+index);
}

void InputPortImpl::removePublisher(OperatorContextImpl & oper) {
  size_t numberOfPublishers = publishers_.size();
  for (size_t i=0; i<numberOfPublishers; i++) {
    if (publishers_[i].first == &oper) {
      removePublisher(i);
      break;
    }
  }
}

// push tuple to the queue
void InputPortImpl::pushTuple(Tuple const & tuple)
{
  {
    lock_guard<mutex> lock(mutex_);
    portQueue_.push_back(make_pair(tuple, chrono::high_resolution_clock::now()));
  }

  // scheduler has to check if this causes the operator to go into ready state 
  scheduler_->markInputPortAsWritten(*this);
}

bool InputPortImpl::isClosed() 
{
  lock_guard<mutex> lock(mutex_);
  return isClosedNoLock();
}

bool InputPortImpl::isClosedNoLock()
{
  if (isClosed_)
    return true;
  for (auto const & opcOportPair : publishers_) {
    if (!opcOportPair.first->isComplete()) 
      return false;
  }
  isClosed_ = true;
  return true;
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

// return true if wait cannot be completed
bool InputPortImpl::waitTuple() 
{
  return waitTuple(1);
}

// return true if wait cannot be completed
bool InputPortImpl::waitTuple(size_t n) 
{
  bool needToWait = true;
  while (needToWait) {
    {
      lock_guard<mutex> lock(mutex_);
      if (portQueue_.size()>=n)
        needToWait = false;
      else if (isClosedNoLock())
        return true;
    }
    if (needToWait) {
      // we need to ask the scheduler to move us into blocked state
      scheduler_->markOperatorAsReadBlocked(*oper_, { {this, n} }, true);
      // Reaching here does not mean that we do not need to wait anymore,
      // as it might be the case that the scheduler has woken us because
      // the port has closed! That is why we have the while loop above.
    } else {
      // we need to check with the scheduler to see if we need to preempt
      scheduler_->checkOperatorForBlocking(*oper_);
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

  return portQueue_.front().first; 
}

std::chrono::high_resolution_clock::time_point & InputPortImpl::getFrontTimestamp() 
{
  lock_guard<mutex> lock(mutex_);
  if (portQueue_.size()==0)
    throw runtime_error("getFrontTimestamp() called on empty queue, oper="+oper_->getOperator().getName());

  return portQueue_.front().second; 
}

// return the index-th tuple
Tuple & InputPortImpl::getTupleAt(size_t index) 
{
  lock_guard<mutex> lock(mutex_);
  size_t size = portQueue_.size();
  if (size<=index)
    throw runtime_error("getTupleAt("+to_string(index)+") called on queue of size "+to_string(size)+", oper="+oper_->getOperator().getName());
  return portQueue_[index].first;
}

// remove the next tuple
void InputPortImpl::popTuple() 
{
  {
    lock_guard<mutex> lock(mutex_);
    portQueue_.pop_front();
  }
  
  // update consumption counters
  OperatorInfo & oinfo = scheduler_->getOperatorInfo(oper_);
  oinfo.updateIPortCounter(*this);
  
  // scheduler has to check if this causes other operators to go into ready state 
  scheduler_->markInputPortAsRead(*this);
}

// remove all the tuples
void InputPortImpl::drain() 
{
  {
    lock_guard<mutex> lock(mutex_);
    portQueue_.clear();
  }
  // scheduler has to check if this causes other operators to go into ready state 
  scheduler_->markInputPortAsRead(*this);
}

void InputPortImpl::markAsWriteBlocked() {
  lock_guard<mutex> lock(mutex_);

  if(inWriteBlockedState_ == true)
    return;

  inWriteBlockedState_ = true;
  writeBlockedBeginTime_ = chrono::high_resolution_clock::now();
}

void InputPortImpl::unmarkAsWriteBlocked() {
  lock_guard<mutex> lock(mutex_);

  if(inWriteBlockedState_ == false)
    return;

  inWriteBlockedState_ = false;
  high_resolution_clock::time_point currentTime = high_resolution_clock::now();

  high_resolution_clock::duration writeBlockedDuration = currentTime - writeBlockedBeginTime_;
  totalWriteBlockedDuration_ =  totalWriteBlockedDuration_ + duration_cast<microseconds>(writeBlockedDuration);
}

bool InputPortImpl::isWriteBlocked(double threshold) {
  lock_guard<mutex> lock(mutex_);

  high_resolution_clock::time_point currentTime = high_resolution_clock::now();
  high_resolution_clock::duration timeDiff = currentTime - createdAt_;
  microseconds timeDiffInMicrosecs = duration_cast<std::chrono::microseconds>(timeDiff);

  double writeBlockedRatio = ((double)totalWriteBlockedDuration_.count()) / timeDiffInMicrosecs.count();
  return writeBlockedRatio >= threshold;
}