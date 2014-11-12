#include "streamc/runtime/WorkerThread.h"

#include "streamc/runtime/RuntimeLogger.h"
#include "streamc/runtime/Scheduler.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/Operator.h"
#include <functional>

#define _GNU_SOURCE 
#include <sched.h>

using namespace std;
using namespace streamc;

WorkerThread::WorkerThread(int index, Scheduler & scheduler)
  : index_(index), scheduler_(&scheduler), stop_(false) 
{}

void WorkerThread::start()
{
  thread_.reset(new thread(bind(&WorkerThread::run, this)));
  SC_LOG(Info, "Launched worker thread # " << index_);
  unsigned int nCores = std::thread::hardware_concurrency();
  unsigned int coreIndex = ((index_ * 2) % nCores) + ((index_ / (nCores/2)) % 2);
  cpu_set_t cpuSet;
  CPU_ZERO(&cpuSet);
  CPU_SET(coreIndex, &cpuSet);
  pthread_setaffinity_np(0, sizeof(cpu_set_t), &cpuSet);
}

void WorkerThread::run()
{
  SC_LOG(Info, "Worker thread #" << index_ << " started running"); 
  while(!stop_.load()) {
    OperatorContextImpl * oper = scheduler_->getThreadWork(*this);
    if (oper==nullptr) 
      break; // no more work to come
    oper->runOper();
  }
  SC_LOG(Info, "Worker thread #" << index_ << " completed running"); 
}

void WorkerThread::join()
{
  SC_LOG(Info, "Joining worker thread #" << index_); 
  thread_->join();
  SC_LOG(Info, "Joined worker thread #" << index_); 
}


