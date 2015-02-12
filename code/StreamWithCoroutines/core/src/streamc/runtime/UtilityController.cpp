#include "streamc/runtime/UtilityController.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/WorkerThread.h"
#include <time.h>

#include <chrono>
#define ALPHA 0.83

using namespace std;
using namespace streamc;
using namespace std::chrono;

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

	vector<long> prevRunningTimes(threads.size());
	vector<long> currRunningTimes(threads.size());

	vector<double> prevUtilites(threads.size());
	vector<double> currUtilities(threads.size());

	bool firstTime = true;
	milliseconds duration(2000);
	while(!isCompleted_.load()) {

		high_resolution_clock::time_point periodBeginTime = high_resolution_clock::now();
		this_thread::sleep_for(duration);
		high_resolution_clock::time_point periodEndTime = high_resolution_clock::now();

		auto timeDifference = duration_cast<microseconds>(periodEndTime - periodBeginTime).count();

		size_t index = 0;
		for(auto it=threads.begin(); it!=threads.end(); it++, index++) {
			currRunningTimes[index] = it->first->getRunningTime();
			currUtilities[index] = (double) (currRunningTimes[index] - prevRunningTimes[index]) / timeDifference;
		}

		for(size_t i=0; i<threads.size(); i++) {
			prevRunningTimes[i] = currRunningTimes[i];

			if(firstTime)
				prevUtilites[i] = currUtilities[i];
			else
				prevUtilites[i] = ALPHA * prevUtilites[i] + (1-ALPHA) * currUtilities[i];
			cerr<<"utility:\t"<<prevUtilites[i]<<endl;
		}
		cerr<<endl;
		firstTime = false;
	}
}

void UtilityController::join() {
	thread_->join();
}

void UtilityController::setCompleted() {
	if (!isCompleted_.load()) 
		isCompleted_.store(true);
}

