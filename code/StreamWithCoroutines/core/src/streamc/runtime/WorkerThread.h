#pragma once

#include <atomic>
#include <thread>
#include <time.h>

namespace streamc
{

class Scheduler;

class WorkerThread
{
public:
  WorkerThread(int index, Scheduler & scheduler);
  void start();
  void join();
  void setBeginTime();
  void setEndtime();
  void updateUtility();
  double getUtility();

private:
  void run();
private:
  int index_;
  Scheduler * scheduler_;
  std::unique_ptr<std::thread> thread_;
  std::atomic<bool> stop_;

  timespec threadBeginTime_, threadEndTime_;
  std::chrono::high_resolution_clock::time_point globalBeginTime_, globalEndTime_;
  long threadRunningTime_;
  double utility_;
  
  
};

} /* namespace streamc */
