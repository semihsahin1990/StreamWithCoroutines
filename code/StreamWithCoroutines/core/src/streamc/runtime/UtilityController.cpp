#include "streamc/runtime/UtilityController.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/WorkerThread.h"
#include <time.h>

#include <chrono>

using namespace std;
using namespace streamc;

UtilityController::UtilityController(FlowContext & flowContext, Scheduler & scheduler)
	:	flowContext_(flowContext), scheduler_(scheduler)
{}

UtilityController::~UtilityController()
{}

void UtilityController::start() {
	isCompleted_.store(false);
	thread_.reset(new thread(bind(&UtilityController::run, this)));
}

void UtilityController::run() {
	auto threads = scheduler_.getThreads();

	while(!isCompleted_.load()) {
		std::chrono::milliseconds duration(2000);
		this_thread::sleep_for(duration);

		// measure utilities
		for(auto it=threads.begin(); it!=threads.end(); it++) {
			double utility = it->first->getUtility();
			//cerr<<utility<<endl;
		}
		//cerr<<endl;
	}
}

void UtilityController::join() {
	thread_->join();
}

void UtilityController::setCompleted() {
	if (!isCompleted_.load()) 
		isCompleted_.store(true);
}

