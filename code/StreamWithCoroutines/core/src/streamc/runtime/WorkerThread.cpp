#include "streamc/runtime/WorkerThread.h"

//#include "streamc/runtime/RuntimeLogger.h"
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
  //SC_LOG(Info, "Launched worker thread # " << index_); 
}

void WorkerThread::run()
{
  //SC_LOG(Info, "Worker thread #" << index_ << " started running"); 
  while(!stop_.load()) {
    OperatorContextImpl * oper = scheduler_->getThreadWork(*this);
    if (oper==nullptr) 
      break; // no more work to come
    oper->runOper();
  }
  //SC_LOG(Info, "Worker thread #" << index_ << " completed running"); 
}

void WorkerThread::join()
{
  //SC_LOG(Info, "Joining worker thread #" << index_); 
  thread_->join();
  //SC_LOG(Info, "Joined worker thread #" << index_); 
}


