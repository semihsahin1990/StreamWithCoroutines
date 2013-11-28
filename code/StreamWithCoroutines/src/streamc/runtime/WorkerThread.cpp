#include "streamc/runtime/WorkerThread.h"

#include "streamc/runtime/Scheduler.h"

using namespace std;
using namespace streamc;

WorkerThread::WorkerThread(int index, Scheduler * scheduler)
  : index_(index), scheduler_(scheduler), stop_(false) 
{}

void WorkerThread::start()
{
  while(!stop_.load()) {
    // TODO: hook into the scheduler    
  }
}

void WorkerThread::stop()
{
  stop_.store(true);
}

void WorkerThread::join()
{
  thread_->join();
}


