#include "streamc/runtime/UtilityController.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/WorkerThread.h"
#include "streamc/runtime/RuntimeLogger.h"
#include <time.h>
#include <fstream>

#include <chrono>
#define ALPHA 0.83
#define ADD_THREAD_THRESHOLD 0.85
#define REMOVE_THREAD_THRESHOLD 0.75

using namespace std;
using namespace streamc;
using namespace std::chrono;

UtilityController::UtilityController(FlowContext & flowContext, Scheduler & scheduler)
	:	flowContext_(flowContext), scheduler_(scheduler)
{}

UtilityController::~UtilityController()
{}

void UtilityController::start(size_t maxThreads) {
	isCompleted_.store(false);
	maxThreads_ = maxThreads;
	thread_.reset(new thread(bind(&UtilityController::run, this)));
}

void UtilityController::addThread() {
	int numOfThreads = threads_.size();

	if(numOfThreads == maxThreads_)
		return;

	WorkerThread * newThread = new WorkerThread(numOfThreads, scheduler_);
	threads_.push_back(newThread);
	scheduler_.addThread(*newThread);
}

void UtilityController::removeThread() {
	int numOfThreads = threads_.size();

	if(numOfThreads == 1)
		return;

	{
		unique_lock<mutex> lock(mutex_);
		scheduler_.requestThreadBlock();
		blockRequested_ = true;
		while (blockRequested_ && !isCompleted_.load())
			cv_.wait(lock);
	}
	bool found = false;
	for(int i=0; i<threads_.size(); i++) {
		if(threads_[i] == removedThread_) {
			threads_.erase(threads_.begin() + i);
			found = true;
			break;
		}
	}
}

void UtilityController::run() {
	threads_.clear();
    addThread();
    
	//vector<long> prevRunningTimes(threads_.size());
	vector<long> prevRunningTimes(maxThreads_);
	vector<long> currRunningTimes(maxThreads_);

	vector<double> prevUtilites(maxThreads_);
	vector<double> currUtilities(maxThreads_);

	bool firstTime;
	bool changed = true;
	milliseconds duration(2000);

	while(!isCompleted_.load()) {
		if(changed) {
			for(size_t i=0; i<threads_.size(); i++) {
				prevRunningTimes[i] = 0;
				threads_[i]->resetRunningTime();
			}
			firstTime = true;
			changed = false;
		}

		high_resolution_clock::time_point periodBeginTime = high_resolution_clock::now();
		this_thread::sleep_for(duration);
		high_resolution_clock::time_point periodEndTime = high_resolution_clock::now();

		auto timeDifference = duration_cast<microseconds>(periodEndTime - periodBeginTime).count();

		for(size_t i=0; i<threads_.size(); i++) {
			currRunningTimes[i] = threads_[i]->getRunningTime();
			currUtilities[i] = (double) (currRunningTimes[i] - prevRunningTimes[i]) / timeDifference;
		}

		double avgUtility = 0;
		for(size_t i=0; i<threads_.size(); i++) {
			prevRunningTimes[i] = currRunningTimes[i];

			if(firstTime)
				prevUtilites[i] = currUtilities[i];
			else
				prevUtilites[i] = ALPHA * prevUtilites[i] + (1-ALPHA) * currUtilities[i];

			avgUtility = avgUtility + prevUtilites[i];
		}
		firstTime = false;
		avgUtility = avgUtility / threads_.size();

		// check increase/decrease required
		if(avgUtility>=ADD_THREAD_THRESHOLD) {
			addThread();
			changed = true;
		}
		if(avgUtility<REMOVE_THREAD_THRESHOLD) {
			removeThread();
			changed = true;
		}
	}
}

void UtilityController::join() {
	SC_LOG(Info, "UtilityController Joining");
	thread_->join();

	for(auto it=threads_.begin(); it!=threads_.end(); it++)
		(*it)->join();
}

void UtilityController::setCompleted() {
	if (!isCompleted_.load()) 
		isCompleted_.store(true);
	cv_.notify_all();
}

void UtilityController::threadBlockGranted(WorkerThread * thread) {
	removedThread_ = thread;
	unique_lock<mutex> lock(mutex_);
	blockRequested_ = false;
	cv_.notify_all();
}

