#pragma once

#include <atomic>
#include <thread>
	
namespace streamc
{

class Scheduler;

class WorkerThread
{
public:
  WorkerThread(int index, Scheduler & scheduler);
  void start();
  void join();
  int index() const { return index_; }
private:
  void run(); 
private:
  int index_;
  Scheduler * scheduler_;
  std::unique_ptr<std::thread> thread_;
  std::atomic<bool> stop_;
};

} /* namespace streamc */
