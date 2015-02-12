#include "streamc/runtime/WorkerThread.h"

#include "streamc/runtime/RuntimeLogger.h"
#include "streamc/runtime/Scheduler.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/Operator.h"

#include <functional>

using namespace std;
using namespace streamc;
using namespace std::chrono;

WorkerThread::WorkerThread(int index, Scheduler & scheduler)
  : index_(index), scheduler_(&scheduler), stop_(false), utility_(0), threadRunningTime_(0)
{}

void WorkerThread::start()
{
  thread_.reset(new thread(bind(&WorkerThread::run, this)));
  SC_LOG(Info, "Launched worker thread # " << index_); 
}

void WorkerThread::run()
{
  SC_LOG(Info, "Worker thread #" << index_ << " started running"); 
  globalBeginTime_ = high_resolution_clock::now();

  while(!stop_.load()) {
    SC_LOG(Info, "Worker thread #" << index_ << " waiting for a ready operator");
    OperatorContextImpl * oper = scheduler_->getThreadWork(*this);
    if (oper==nullptr) 
      break; // no more work to come
    SC_LOG(Info, "Worker thread #" << index_ << " running operator:\t"<<oper->getOperator().getName());

    setBeginTime();
    oper->runOper();
    setEndtime();
    updateUtility();
  }
  SC_LOG(Info, "Worker thread #" << index_ << " completed running"); 
}

void WorkerThread::join()
{
  SC_LOG(Info, "Joining worker thread #" << index_); 
  thread_->join();
  SC_LOG(Info, "Joined worker thread #" << index_); 
}

void WorkerThread::setBeginTime() {
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &threadBeginTime_);
}

void WorkerThread::setEndtime() {
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &threadEndTime_);
  globalEndTime_ = high_resolution_clock::now();
}

void WorkerThread::updateUtility() {
  long threadDifference = (threadEndTime_.tv_sec - threadBeginTime_.tv_sec) * 1000000 + (threadEndTime_.tv_nsec - threadBeginTime_.tv_nsec)/1000;
  long globalDifference = duration_cast<microseconds>(globalEndTime_ - globalBeginTime_).count();

  threadRunningTime_ = threadRunningTime_ + threadDifference;
  utility_ = ((100 * threadRunningTime_) / globalDifference) / 100.0;
}

double WorkerThread::getUtility() {
  return utility_;
}