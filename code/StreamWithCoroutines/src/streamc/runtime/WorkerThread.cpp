#include "streamc/runtime/WorkerThread.h"

#include "streamc/runtime/Scheduler.h"
#include "streamc/runtime/OperatorContextImpl.h"

#include <functional>

using namespace std;
using namespace streamc;

WorkerThread::WorkerThread(int index, Scheduler & scheduler)
  : index_(index), scheduler_(&scheduler), stop_(false) 
{}

void WorkerThread::start()
{
  thread_.reset(new thread(bind(&WorkerThread::run, this)));
}

void WorkerThread::run()
{
  while(!stop_.load()) {
    OperatorContextImpl * oper = scheduler_->getThreadWork(*this);
    if (oper==nullptr) 
      break; // no more work to come
    oper->runOper();
  }
}

void WorkerThread::join()
{
  thread_->join();
}


