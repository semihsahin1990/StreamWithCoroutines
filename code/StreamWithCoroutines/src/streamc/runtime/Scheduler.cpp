#include "streamc/runtime/Scheduler.h"

#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/WorkerThread.h"
#include "streamc/runtime/SchedulerPlugin.h"
#include "streamc/runtime/RandomScheduling.h"
#include "streamc/runtime/OutputPortImpl.h"

using namespace std;
using namespace streamc;

Scheduler::Scheduler() 
  : stopped_(false), plugin_(new RandomScheduling()) 
{}

Scheduler::~Scheduler() 
{}

void Scheduler::addThread(WorkerThread & thread)
{
  threadInfos_.push_back(unique_ptr<ThreadInfo>(new ThreadInfo(&thread)));
  threads_[&thread] = threadInfos_.back().get();
}

void Scheduler::addOperatorContext(OperatorContextImpl & context)
{
  operatorInfos_.push_back(unique_ptr<OperatorInfo>(new OperatorInfo(context)));
  operContexts_[&context] = operatorInfos_.back().get();
}

void Scheduler::start()
{
  unique_lock<mutex> lock(mutex_);
  for (auto & threadInfoPair : threads_)
    readyThreads_.insert(threadInfoPair.first);
  for (auto & operInfoPair : operContexts_)
    readyOperators_.insert(operInfoPair.first);  
}

void Scheduler::stop()
{
  unique_lock<mutex> lock(mutex_);   
  stopped_ = true;
  // wake up all threads, they will ask scheduler for work,
  // and find out that there is no more work for them
  for (auto & threadInfoPair : threads_) 
    threadInfoPair.second->getCV().notify_one();
}

void Scheduler::markOperatorAsCompleted(OperatorContextImpl & oper)
{
  unique_lock<mutex> lock(mutex_);  
  updateOperatorState(oper, OperatorInfo::Completed); 
  // It is possible that the downstream operators that are currently in Waiting
  // state may need to be put into Ready state, as their input ports may close
  // due to this operator's completion. It is ok to superfluously take out an
  // operator out of Waiting state, as it will check again upon wake up to see
  // if it needs to wait again or not.
  for (size_t i=0, iu=oper.getNumberOfOutputPorts(); i<iu; ++i)  {
    OutputPortImpl & oport = oper.getOutputPortImpl(i);
    for (size_t j=0, ju=oport.getNumberOfSubscribers(); j<ju; ++j)  {
      pair<OperatorContextImpl *, size_t> subscriber = oport.getSubscriber(j);
      OperatorContextImpl & doper = *subscriber.first;
      OperatorInfo & doinfo = *(operContexts_[&doper]);
      if (doinfo.getState()==OperatorInfo::Waiting)
        updateOperatorState(doper, OperatorInfo::Ready);
    }
  }
}

void Scheduler::markOperatorAsWaiting(OperatorContextImpl & oper, 
      std::unordered_map<InputPortImpl *, size_t> const & waitSpec)
{
  {
    unique_lock<mutex> lock(mutex_);  
    OperatorInfo & oinfo = *(operContexts_[&oper]);
    OperatorInfo::WaitCondition & waitCond = oinfo.getWaitCondition();
    for (auto & portSizePair : waitSpec)
      waitCond.setWait(*portSizePair.first, portSizePair.second);
    if (!waitCond.isReady())  
      updateOperatorState(oper, OperatorInfo::Waiting); 
    else 
      updateOperatorState(oper, OperatorInfo::Ready); 
  }
  oper.yieldOper(); // co-routine jumps back to the WorkerThread
}

void Scheduler::markInputPortAsChanged(InputPortImpl & iport)
{
  unique_lock<mutex> lock(mutex_);
  auto it = waitingOperators_.find(&iport); 
  // no operators to switch into ready state
  if (it==waitingOperators_.end()) 
    return;
  OperatorContextImpl & oper = *(it->second);
  OperatorInfo & oinfo = *(operContexts_[&oper]);
  OperatorInfo::WaitCondition & waitCond = oinfo.getWaitCondition();
  if (waitCond.isReady()) 
    updateOperatorState(oper, OperatorInfo::Ready);
}

void Scheduler::checkOperatorForPreemption(OperatorContextImpl & oper)
{
  bool preempt = false;
  {
    unique_lock<mutex> lock(mutex_);  
    preempt = plugin_->checkOperatorForPreemption(*this, oper); 
    if (preempt)
      updateOperatorState(oper, OperatorInfo::Ready); 
  }
  if (preempt)
    oper.yieldOper(); // co-routine jumps back to the WorkerThread
}

OperatorContextImpl * Scheduler::getThreadWork(WorkerThread & thread)
{
  unique_lock<mutex> lock(mutex_);  
  OperatorContextImpl * assignment = nullptr;
  if (!stopped_) {
    updateThreadState(thread, ThreadInfo::Waiting);
    assignment = plugin_->findOperatorToExecute(*this, thread); 
    while (assignment==nullptr) {
      ThreadInfo & info = *(threads_[&thread]);
      info.getCV().wait(lock);
      if (stopped_) 
        break;
      assignment = plugin_->findOperatorToExecute(*this, thread); 
    }
  }
  if (assignment==nullptr) {
    updateThreadState(thread, ThreadInfo::Completed);
  } else  {
    threads_[&thread]->setOperator(*assignment); 
    operContexts_[assignment]->setThread(thread);
    updateThreadState(thread, ThreadInfo::Running);
    updateOperatorState(*assignment, OperatorInfo::Running);
  }
  return assignment;
}

void Scheduler::updateOperatorState(OperatorContextImpl & oper, OperatorInfo::OperatorState state)
{
  OperatorInfo & oinfo = *(operContexts_[&oper]);
  OperatorInfo::OperatorState oldState = oinfo.getState();
  if (oldState==state)
    return; // no change
  oinfo.setState(state);
  if (oldState==OperatorInfo::Waiting) {
    OperatorInfo::WaitCondition & waitCond = oinfo.getWaitCondition();
    for (size_t i=0, iu=oper.getNumberOfInputPorts(); i<iu; ++i)  {
      InputPortImpl & iport = oper.getInputPortImpl(i);
      if (waitCond.getWait(iport)>0) 
        waitingOperators_.erase(&iport);    
    }
  } else if (oldState==OperatorInfo::Ready) {
    readyOperators_.erase(&oper);    
  } else if (oldState==OperatorInfo::Running) {
    oinfo.setEndTime(chrono::high_resolution_clock::now());
  }
  if (state==OperatorInfo::OperatorInfo::Waiting) {
    OperatorInfo::WaitCondition & waitCond = oinfo.getWaitCondition();
    for (size_t i=0, iu=oper.getNumberOfInputPorts(); i<iu; ++i)  {
      InputPortImpl & iport = oper.getInputPortImpl(i);
      if (waitCond.getWait(iport)>0) 
        waitingOperators_[&iport] = &oper;    
    }
  } else if (state==OperatorInfo::OperatorInfo::Ready) {
    readyOperators_.insert(&oper);
    oinfo.getWaitCondition().reset(); 
    if (waitingThreads_.size()>0) {
      // wake one of the threads, as there is more work now
      WorkerThread * thread = *(waitingThreads_.begin());
      threads_[thread]->getCV().notify_one();
    }
  } else if (state==OperatorInfo::OperatorInfo::Running) {
    oinfo.setBeginTime(chrono::high_resolution_clock::now());
  }
}

void Scheduler::updateThreadState(WorkerThread & thread, ThreadInfo::ThreadState state)
{
  ThreadInfo & tinfo = *(threads_[&thread]);
  ThreadInfo::ThreadState oldState = tinfo.getState();
  if (oldState==state)
    return; // no change
  tinfo.setState(state);
  if (oldState==ThreadInfo::Waiting) 
    waitingThreads_.erase(&thread);    
  else if (oldState==ThreadInfo::Ready)
    readyThreads_.erase(&thread);
  if (state==ThreadInfo::Waiting) 
    waitingThreads_.insert(&thread);    
  else if (state==ThreadInfo::Ready)
    readyThreads_.insert(&thread);
}

