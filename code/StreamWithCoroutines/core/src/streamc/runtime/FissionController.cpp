#include "streamc/runtime/FissionController.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
#include "streamc/Operator.h"

#include <chrono>

using namespace std;
using namespace streamc;

uint64_t fissionPeriod = 1000; //microsecs

FissionController::FissionController(FlowContext & flowContext, Scheduler & scheduler)
	:	flowContext_(flowContext), scheduler_(scheduler)
{}

FissionController::~FissionController()
{}

void FissionController::start() {
	isCompleted_.store(false);
	thread_.reset(new thread(bind(&FissionController::run, this)));
}

OperatorContextImpl * FissionController::detectBottleneck() {
	vector<OperatorContextImpl *> operators = flowContext_.getOperators();
	vector<OperatorContextImpl *> candidates;
	for(auto it=operators.begin(); it!=operators.end(); it++) {
		OperatorContextImpl * oper = *it;
		if(oper->getNumberOfInputPorts() == 1 && oper->getNumberOfOutputPorts() == 1)
			candidates.push_back(oper);
	}
	if(candidates.size()>0)
		return candidates[0];
	return nullptr;
}

void FissionController::run() {
	while(!isCompleted_.load()) {
		std::chrono::milliseconds duration(100);
		this_thread::sleep_for(duration);

		OperatorContextImpl * bottleneck = detectBottleneck();
		if(bottleneck == nullptr) {
			this_thread::sleep_for(duration);
			continue;
		}
			
		cerr<<"bottleneck operator: "<<bottleneck->getOperator().getName()<<endl;
		// block bottleneck publishers
		InputPortImpl & iport = bottleneck->getInputPortImpl(0);
		size_t numberOfPublishers = iport.getNumberOfPublishers();
		for(size_t i=0; i<numberOfPublishers; i++) {
			OperatorContextImpl * publisher = iport.getPublisher(i).first;
			{
				unique_lock<mutex> lock(mutex_);
				if(scheduler_.blockPublisherOper(*publisher) == false)
					requests_.insert(publisher);
			}
		}

		{
		unique_lock<mutex> lock(mutex_);
		if(requests_.size()>0)
			cv_.wait(lock);
		// block bottleneck operator
		if(scheduler_.blockBottleneckOper(*bottleneck) == false)
			requests_.insert(bottleneck);
		if(requests_.size()>0)
			cv_.wait(lock);
		}

		flowContext_.addFission(&(bottleneck->getOperator()), 3);
		scheduler_.unblockOperators();
		cerr<<"operators unblocked"<<endl;
		break;
	}
}

void FissionController::blockGranted(OperatorContextImpl * oper) {
	unique_lock<mutex> lock(mutex_);
	requests_.erase(oper);
	if(requests_.size()==0)
		cv_.notify_all();
}

void FissionController::join() {
	thread_->join();
}

void FissionController::setCompleted() {
	if (!isCompleted_.load()) 
		isCompleted_.store(true);
	cv_.notify_all();
}