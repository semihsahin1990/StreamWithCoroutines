#include "streamc/runtime/Scheduler.h"

#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/WorkerThread.h"

using namespace std;
using namespace streamc;

Scheduler::ThreadInfo & Scheduler::getThreadInfo(WorkerThread & thread)
{
  return threads_[&thread];
}

Scheduler::OperatorInfo & Scheduler::getOperatorInfo(OperatorContextImpl & context)
{
  return operContexts_[&context];
}

void Scheduler::addThread(WorkerThread & thread)
{
  threads_[&thread] = ThreadInfo(&thread);
}

void Scheduler::addOperatorContext(OperatorContextImpl & context)
{
  operContexts_[&context] = OperatorInfo(&context);
}

//////////

void Scheduler::start()
{
  unique_lock<mutex> lock(mutex_);
  // TODO: index initialization
}

void Scheduler::stop()
{
  unique_lock<mutex> lock(mutex_);   
  // TODO: let all waiting threads loose
}

void Scheduler::markOperatorCompleted(OperatorContextImpl & oper)
{
  unique_lock<mutex> lock(mutex_);  
  updateOperatorState(oper, OperatorInfo::Completed); 
}

void Scheduler::updateOperatorState(OperatorContextImpl & oper, OperatorInfo::OperatorState state)
{
  OperatorInfo & oinfo = getOperatorInfo(oper);
  oinfo.setState(state);
  // TODO: update indices
}

