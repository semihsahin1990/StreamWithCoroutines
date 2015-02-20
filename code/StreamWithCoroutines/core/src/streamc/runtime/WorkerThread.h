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
  long getRunningTime();

private:
  void run();
private:
  int index_;
  Scheduler * scheduler_;
  std::unique_ptr<std::thread> thread_;
  std::atomic<bool> stop_;

  timespec threadBeginTime_, threadEndTime_;
  long threadRunningTime_;
  std::mutex mutex_;
};

} /* namespace streamc */
