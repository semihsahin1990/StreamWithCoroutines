#pragma once

#include "streamc/runtime/Scheduler.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace streamc
{

class Scheduler;
class WorkerThread;
class UtilityController
{
public:
	UtilityController(FlowContext & flowContext_, Scheduler & scheduler);
	~UtilityController();
	void start(size_t maxThreads);
	void join();
	void run();
	void setCompleted();
	void threadBlockGranted(WorkerThread * thread);

private:
	bool addThread();
	bool removeThread();

private:
	size_t maxThreads_;
	std::vector<WorkerThread *> threads_;
	FlowContext & flowContext_;
	Scheduler & scheduler_;
	std::unique_ptr<std::thread> thread_;
	std::atomic<bool> isCompleted_;
	std::mutex mutex_;
	std::condition_variable cv_;
	bool blockRequested_;
	WorkerThread * removedThread_;
};
}