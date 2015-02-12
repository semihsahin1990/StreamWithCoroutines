#pragma once

#include "streamc/runtime/Scheduler.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace streamc
{

class Scheduler;
class UtilityController
{
public:
	UtilityController(FlowContext & flowContext_, Scheduler & scheduler);
	~UtilityController();
	void start();
	void join();
	void run();
	void setCompleted();

private:
	FlowContext & flowContext_;
	Scheduler & scheduler_;
	std::unique_ptr<std::thread> thread_;
	std::atomic<bool> isCompleted_;
};
}