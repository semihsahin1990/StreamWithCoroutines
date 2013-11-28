#include "streamc/runtime/WorkerThread.h"

#include "streamc/runtime/Scheduler.h"

using namespace std;
using namespace streamc;

WorkerThread::WorkerThread(int index, Scheduler & scheduler)
  : index_(index), scheduler_(&scheduler), stop_(false) 
{}

void WorkerThread::start()
{
  thread_.reset(new thread());
  while(!stop_.load()) {
    OperatorContextImpl * oper = scheduler_->getThreadWork(*this);
    if (oper==nullptr) // no more work to come
      break; 
    // TODO: execute operator
  }
}

void WorkerThread::join()
{
  thread_->join();
}


