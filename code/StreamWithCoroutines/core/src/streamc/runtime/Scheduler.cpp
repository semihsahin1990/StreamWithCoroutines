#include "streamc/runtime/Scheduler.h"

#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/WorkerThread.h"
#include "streamc/runtime/SchedulerPlugin.h"
#include "streamc/runtime/RandomScheduling.h"
#include "streamc/runtime/MinLatencyScheduling.h"
#include "streamc/runtime/MaxThroughputScheduling.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
#include "streamc/Operator.h"
#include "streamc/runtime/FissionController.h"
#include <iostream>

using namespace std;
using namespace streamc;

Scheduler::Scheduler(FlowContext & flowContext, SchedulerPlugin * plugin) 
  : stopped_(false), flowContext_(flowContext), plugin_(plugin) 
{}

Scheduler::~Scheduler() 
{}

void Scheduler::addThread(WorkerThread & thread)
{
  unique_lock<mutex> lock(mutex_);
  threadInfos_.push_back(unique_ptr<ThreadInfo>(new ThreadInfo(&thread)));
  threads_[&thread] = threadInfos_.back().get();
}

void Scheduler::removeThreads()
{
  unique_lock<mutex> lock(mutex_);
  assert(waitingThreads_.empty());
  assert(readyThreads_.empty());
  threads_.clear();
  threadInfos_.clear();
}
void Scheduler::addOperatorContext(OperatorContextImpl & context)
{
  unique_lock<mutex> lock(mutex_);
  cerr<<"ADD:\t"<<context.getOperator().getName()<<endl;
  operatorInfos_.push_back(unique_ptr<OperatorInfo>(new OperatorInfo(context)));
  operContexts_[&context] = operatorInfos_.back().get();

  context.init();
  operatorInfos_.back().get()->init();
  readyOperators_.insert(&context);
}

void Scheduler::removeOperatorContext(OperatorContextImpl & context)
{
  unique_lock<mutex> lock(mutex_);
  cerr<<"REMOVE:\t"<<context.getOperator().getName()<<endl;

  size_t numberOfOperators = operatorInfos_.size();
  for(size_t i=0; i<numberOfOperators; i++) {
    if(&operatorInfos_[i]->getOperatorContext() == &context) {
      operatorInfos_.erase(operatorInfos_.begin() + i);
      break;
    }
  }
  operContexts_.erase(&context);
  outOfServiceOperators_.erase(&context);
}

void Scheduler::start()
{
  unique_lock<mutex> lock(mutex_);
  for (auto & threadInfoPair : threads_) 
    readyThreads_.insert(threadInfoPair.first);
  /*
  for (auto & operInfoPair : operContexts_) {
    operInfoPair.first->init();
    operInfoPair.second->init();
    readyOperators_.insert(operInfoPair.first);
  }
  */
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
  // It is possible that the downstream operators that are currently in blocked
  // state may need to be put into Ready state, as their input ports may close
  // due to this operator's completion. It is ok to superfluously take out an
  // operator out of blocked state, as it will check again upon wake up to see
  // if it needs to wait again or not.
  for (size_t i=0, iu=oper.getNumberOfOutputPorts(); i<iu; ++i)  {
    OutputPortImpl & oport = oper.getOutputPortImpl(i);
    for (size_t j=0, ju=oport.getNumberOfSubscribers(); j<ju; ++j)  {
      pair<OperatorContextImpl *, size_t> subscriber = oport.getSubscriber(j);
      OperatorContextImpl & doper = *subscriber.first;
      OperatorInfo & doinfo = *(operContexts_[&doper]);
      if (doinfo.getState()==OperatorInfo::ReadBlocked)
        updateOperatorState(doper, OperatorInfo::Ready);
    }
  }
  // It is possible that the upstream operators that are currently in blocked
  // state may need to be put into Ready state, as this operator's completion
  // may be due to a global shutdown, and the upstream operator cannot see the
  // global shutdown unless it is put into ready state. It is ok to superfluously 
  // take out an operator out of blocked state, as it will check again upon wake 
  // up to see if it needs to wait again or not, and that code already has a 
  // special check for a global shutdown.
  if (flowContext_.isShutdownRequested()) {
    for (size_t i=0, iu=oper.getNumberOfInputPorts(); i<iu; ++i)  {
      InputPortImpl & iport = oper.getInputPortImpl(i);
      for (size_t j=0, ju=iport.getNumberOfPublishers(); j<ju; ++j)  {
        pair<OperatorContextImpl *, size_t> publisher = iport.getPublisher(j);
        OperatorContextImpl & uoper = *publisher.first;
        OperatorInfo & uoinfo = *(operContexts_[&uoper]);
        if (uoinfo.getState()==OperatorInfo::WriteBlocked)
          updateOperatorState(uoper, OperatorInfo::Ready);
      }
    }
  }
}

void Scheduler::markOperatorAsReadBlocked(OperatorContextImpl & oper, 
      std::unordered_map<InputPortImpl *, size_t> const & waitSpec, bool conjunctive)
{
  checkOperatorForBlocking(oper);
  {
    unique_lock<mutex> lock(mutex_);  
    OperatorInfo & oinfo = *(operContexts_[&oper]);
    OperatorInfo::ReadWaitCondition & waitCond = oinfo.getReadWaitCondition();
    if (conjunctive) {
      waitCond.makeConjunctive();
      // It is possible that the input port is closed (upstream operators of it 
      // are all complete). In this case, we return back, so that the 
      // calling routine will find out about the closed input port.
      for (auto & portSizePair : waitSpec) 
        if (portSizePair.first->isClosed()) 
          return;
    } else {
      waitCond.makeDisjunctive();
      // Same as above, but this time we only return if all ports are closed.
      bool allClosed = true;
      for (auto & portSizePair : waitSpec) 
        if (!portSizePair.first->isClosed()) {
          allClosed = false;
          break;
        }
      if (allClosed) return;
    }
    for (auto & portSizePair : waitSpec) 
      waitCond.setWaitThreshold(*portSizePair.first, portSizePair.second);
    if (!waitCond.computeReadiness())  
      updateOperatorState(oper, OperatorInfo::ReadBlocked); 
    else 
      updateOperatorState(oper, OperatorInfo::Ready); 
  }
  oper.yieldOper(); // co-routine jumps back to the WorkerThread
}

void Scheduler::markOperatorAsWriteBlocked(OperatorContextImpl & oper, 
      std::unordered_map<InputPortImpl *, size_t> const & waitSpec)
{
  {
    unique_lock<mutex> lock(mutex_);  
    OperatorInfo & oinfo = *(operContexts_[&oper]);
    OperatorInfo::WriteWaitCondition & waitCond = oinfo.getWriteWaitCondition();
    for (auto & portSizePair : waitSpec) {
      // It is possible that the shutdown is requested and the downstream 
      // operator owning the input port is complete. In this case, we return
      // back, in which case the calling routine will find about the shutdown.
      if (flowContext_.isShutdownRequested() &&
        portSizePair.first->getOperatorContextImpl().isComplete())
        return;
      waitCond.setWaitThreshold(*portSizePair.first, portSizePair.second);
    }
    if (!waitCond.computeReadiness())  
      updateOperatorState(oper, OperatorInfo::WriteBlocked); 
    else 
      updateOperatorState(oper, OperatorInfo::Ready); 
  }
  oper.yieldOper(); // co-routine jumps back to the WorkerThread
}

// the port has more tuples, which may impact operators in ReadBlocked state
void Scheduler::markInputPortAsWritten(InputPortImpl & iport)
{
  unique_lock<mutex> lock(mutex_);
  auto it = readBlockedOperators_.find(&iport); 
  // no operators to switch into ready state
  if (it==readBlockedOperators_.end()) 
    return;
  OperatorContextImpl & oper = *(it->second);
  OperatorInfo & oinfo = *(operContexts_[&oper]);
  OperatorInfo::ReadWaitCondition & waitCond = oinfo.getReadWaitCondition();
  if (waitCond.computeReadiness())
    updateOperatorState(oper, OperatorInfo::Ready);
  else if(waitCond.isReady(iport)) 
    readBlockedOperators_.erase(&iport); // not waiting on this port anymore
}

// the port has less tuples, which may impact operators in WriteBlocked state
void Scheduler::markInputPortAsRead(InputPortImpl & iport)
{
  unique_lock<mutex> lock(mutex_);
  auto it = writeBlockedOperators_.find(&iport); 
  // no operators to switch into ready state
  if (it==writeBlockedOperators_.end()) 
    return;
  unordered_set<OperatorContextImpl *> & opers = it->second;
  for (auto oit=opers.begin(); oit!=opers.end();) {
    auto cit = oit++; // updateOperatorState may erase the current item
    OperatorContextImpl & oper = *(*cit);
    OperatorInfo & oinfo = *(operContexts_[&oper]);
    OperatorInfo::WriteWaitCondition & waitCond = oinfo.getWriteWaitCondition();
    if (waitCond.computeReadiness()) 
      updateOperatorState(oper, OperatorInfo::Ready);
    else if(waitCond.isReady(iport)) 
      opers.erase(cit); // not waiting on this input port any more
  }
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

OperatorInfo & Scheduler::getOperatorInfo(OperatorContextImpl *oper) {
  return *(operContexts_[oper]);
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

int counter = 0;
void Scheduler::updateOperatorState(OperatorContextImpl & oper, OperatorInfo::OperatorState state)
{
/*  if(counter == 2)
    cerr<<"State:\t"<<oper.getOperator().getName()<<"\t"<<state<<endl;;  
 */ OperatorInfo & oinfo = *(operContexts_[&oper]);
  OperatorInfo::OperatorState oldState = oinfo.getState();
  if (oldState==state)
    return; // no change
  
  oinfo.setState(state);
  if (oldState==OperatorInfo::ReadBlocked) { // must be moving into ready state
    // remove the operator from the read waiting list of its input ports
    OperatorInfo::ReadWaitCondition & waitCond = oinfo.getReadWaitCondition();
    for(auto const & wait : waitCond.getWaitList())
        readBlockedOperators_.erase(wait.first);    
  } else if (oldState==OperatorInfo::WriteBlocked) { // must be moving into ready state
    // remove the operator from the write waiting list of its downstream input ports
    OperatorInfo::WriteWaitCondition & waitCond = oinfo.getWriteWaitCondition();
    for(auto const & wait : waitCond.getWaitList()) {
        auto it = writeBlockedOperators_.find(wait.first);
        if (it!=writeBlockedOperators_.end()) 
          (it->second).erase(&oper);
    } 
  } else if (oldState==OperatorInfo::Ready) {
    readyOperators_.erase(&oper);    
  } else if (oldState==OperatorInfo::Running) {
    oinfo.setEndTime(chrono::high_resolution_clock::now());
    size_t numberOfInputPorts = oper.getNumberOfInputPorts();
    for(int i=0; i<numberOfInputPorts; i++) 
      oinfo.updateIPortProfile(oper.getInputPortImpl(i));
    size_t numberOfOutputPorts = oper.getNumberOfOutputPorts();
    for(int i=0; i<numberOfOutputPorts; i++) 
      oinfo.updateOPortProfile(oper.getOutputPortImpl(i));
  }
  if (state==OperatorInfo::OperatorInfo::ReadBlocked) {
    OperatorInfo::ReadWaitCondition & waitCond = oinfo.getReadWaitCondition();
    for(auto const & cond : waitCond.getWaitList()) {
      if (!waitCond.isReady(*cond.first)) 
        readBlockedOperators_[cond.first] = &oper;  
    }
  } else if (state==OperatorInfo::OperatorInfo::WriteBlocked) {
    OperatorInfo::WriteWaitCondition & waitCond = oinfo.getWriteWaitCondition();
    for(auto const & cond : waitCond.getWaitList()) {
      if (!waitCond.isReady(*cond.first)) 
        writeBlockedOperators_[cond.first].insert(&oper);
    }
  } else if(state==OperatorInfo::OperatorInfo::Ready) { // must be moving out of Read/WriteBlocked
    readyOperators_.insert(&oper);
    if (oldState==OperatorInfo::OperatorInfo::ReadBlocked) 
      oinfo.getReadWaitCondition().reset(); 
    if (waitingThreads_.size()>0) {
      // wake one of the threads, as there is more work now
      WorkerThread * thread = *(waitingThreads_.begin());
        threads_[thread]->getCV().notify_one();
    }
  } else if (state==OperatorInfo::OperatorInfo::Running) {
    size_t numberOfInputPorts = oper.getNumberOfInputPorts();
    for(int i=0; i<numberOfInputPorts; i++) 
      oinfo.resetIPortCounter(oper.getInputPortImpl(i));
    size_t numberOfOutputPorts = oper.getNumberOfOutputPorts();
    for(int i=0; i<numberOfOutputPorts; i++) 
      oinfo.resetOPortCounter(oper.getOutputPortImpl(i));
    oinfo.setBeginTime(chrono::high_resolution_clock::now());
  } else if(state==OperatorInfo::OperatorInfo::OutOfService) {
    outOfServiceOperators_.insert(&oper);
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

bool Scheduler::requestPartialBlock(OperatorContextImpl & oper) {
  unique_lock<mutex> lock(mutex_);

  if(operContexts_[&oper]->getState() != OperatorInfo::Running) {
    cerr<<"block publisher\t"<<oper.getOperator().getName()<<"\t granted"<<endl;
    updateOperatorState(oper, OperatorInfo::OutOfService);
    return true;
  }
  else {
    cerr<<"block publisher\t"<<oper.getOperator().getName()<<"\t waiting"<<endl;
    partialBlockRequestedOperators_.insert(&oper);
    return false;
  }
}

bool Scheduler::requestCompleteBlock(OperatorContextImpl & oper) {
  unique_lock<mutex> lock(mutex_);
  
  bool allEmpty = true;
  size_t numberOfInputPorts = oper.getNumberOfInputPorts();

  for(size_t i=0; i<numberOfInputPorts; i++) {
    if(oper.getInputPortImpl(i).getTupleCount() != 0)
      allEmpty = false;
  }

  if(operContexts_[&oper]->getState() != OperatorInfo::Running && allEmpty) {
    cerr<<"block bottleneck\t"<<oper.getOperator().getName()<<"\t granted"<<endl;
    updateOperatorState(oper, OperatorInfo::OutOfService);
    return true; 
  }
  else {
    cerr<<"block bottleneck\t"<<oper.getOperator().getName()<<"\t waiting"<<endl;
    completeBlockRequestedOperators_.insert(&oper);
    return false;
  }
}

void Scheduler::checkOperatorForBlocking(OperatorContextImpl & oper) {
  bool requested = false;
  {
    unique_lock<mutex> lock(mutex_);
    if(partialBlockRequestedOperators_.find(&oper)!=partialBlockRequestedOperators_.end()) {
      requested = true;
      partialBlockRequestedOperators_.erase(&oper);
      updateOperatorState(oper, OperatorInfo::OutOfService);
      fissionController_->blockGranted(&oper);
    }

    if(completeBlockRequestedOperators_.find(&oper)!=completeBlockRequestedOperators_.end()) {
      bool allEmpty = true;
      size_t numberOfInputPorts = oper.getNumberOfInputPorts();
      for (size_t i=0; i<numberOfInputPorts; i++) {
        if(oper.getInputPortImpl(i).getTupleCount() != 0)
          allEmpty = false;
      }

      if(allEmpty) {
        requested = true;
        completeBlockRequestedOperators_.erase(&oper);
        updateOperatorState(oper, OperatorInfo::OutOfService);
        fissionController_->blockGranted(&oper);  
      }
    }
  }

  if(requested)
    oper.yieldOper();
}

void Scheduler::unblockOperators() {
  counter++;
  unique_lock<mutex> lock(mutex_);
  for(OperatorContextImpl * oper : outOfServiceOperators_) {
    cerr<<"unblockOperator: "<<oper->getOperator().getName()<<endl;
    updateOperatorState(*oper, OperatorInfo::Ready);
  }
  outOfServiceOperators_.clear();
}