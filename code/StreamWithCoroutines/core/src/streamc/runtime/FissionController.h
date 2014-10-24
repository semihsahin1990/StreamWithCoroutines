#pragma once

#include "streamc/runtime/Scheduler.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace streamc
{

class Scheduler;
class FissionController
{
public:
	FissionController(FlowContext & flowContext_, Scheduler & scheduler);
	~FissionController();
	void start();
	void join();
	void run();
	void setCompleted();
	OperatorContextImpl * detectBottleneck();
	void blockGranted(OperatorContextImpl * oper);
private:
	FlowContext & flowContext_;
	Scheduler & scheduler_;
	std::unordered_map<OperatorContextImpl *, size_t> replicatedOperators_;
	std::unordered_set<OperatorContextImpl *> requests_;
	std::unique_ptr<std::thread> thread_;
	std::atomic<bool> isCompleted_;
	std::mutex mutex_;
	std::condition_variable cv_;
};
}