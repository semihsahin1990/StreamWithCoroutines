#include "streamc/runtime/Scheduler.h"

#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/WorkerThread.h"
#include "streamc/runtime/SchedulerPlugin.h"
#include "streamc/runtime/RandomScheduling.h"

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

void Scheduler::markOperatorCompleted(OperatorContextImpl & oper)
{
  unique_lock<mutex> lock(mutex_);  
  updateOperatorState(oper, OperatorInfo::Completed); 
}

void Scheduler::markChangeInInputPortSize(InputPortImpl & iport)
{
  auto it = waitingOperators_.find(&iport); 
  // no operators to switch into ready state
  if (it==waitingOperators_.end()) 
    return;
  OperatorContextImpl & oper = *(it->second);
  OperatorInfo & oinfo = *(operContexts_[&oper]);
  OperatorInfo::WaitCondition & waitCond = oinfo.getWaitCondition();
  if (waitCond.isReady()) {
    waitCond.reset();
    updateOperatorState(oper, OperatorInfo::Ready);
    if (waitingThreads_.size()>0) {
      // wake one of the threads, as there is more work now
      WorkerThread * thread = *(waitingThreads_.begin());
      threads_[thread]->getCV().notify_one();
    }
  }
}

OperatorContextImpl * Scheduler::getThreadWork(WorkerThread & thread)
{
  unique_lock<mutex> lock(mutex_);  
  OperatorContextImpl * assignment = nullptr;
  if (!stopped_) {
    updateThreadState(thread, ThreadInfo::Waiting);
    assignment = plugin_->getOperatorToExecute(*this, thread); 
    while (assignment==nullptr) {
      ThreadInfo & info = *(threads_[&thread]);
      info.getCV().wait(lock);
      if (stopped_)
        break;
      assignment = plugin_->getOperatorToExecute(*this, thread); 
    }
  }
  if (assignment==NULL) {
    updateThreadState(thread, ThreadInfo::Completed);
    threads_[&thread]->setOperator(*assignment); 
    operContexts_[assignment]->setThread(thread);
  } else  {
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
    for (size_t i=0, iu=oper.getNumberOfInputPorts(); i<iu; ++i)  {
      InputPortImpl & iport = oper.getInputPortImpl(i);
      waitingOperators_.erase(&iport);    
    }
  } else if (oldState==OperatorInfo::Ready) {
    readyOperators_.erase(&oper);    
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

