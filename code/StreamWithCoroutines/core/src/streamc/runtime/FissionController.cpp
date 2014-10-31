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
		if(oper->getNumberOfInputPorts() == 1 && oper->getNumberOfOutputPorts() == 1 && oper->getOperator().getName()=="busy")
			candidates.push_back(oper);
	}
	if(candidates.size()>0)
		return candidates[0];
	return nullptr;
}

void FissionController::addFission(OperatorContextImpl * bottleneck, size_t replicaCount) {
	cerr<<"add fission\t"<<bottleneck->getOperator().getName()<<"\t"<<replicaCount<<endl;
	InputPortImpl & iport = bottleneck->getInputPortImpl(0);

	// block publishers
	size_t numberOfPublishers = iport.getNumberOfPublishers();
	for (size_t i=0; i<numberOfPublishers; i++) {
		OperatorContextImpl * publisher = iport.getPublisher(i).first;
		{
			unique_lock<mutex> lock(mutex_);
			if (scheduler_.blockPublisherOper(*publisher) == false)
				requests_.insert(publisher);
		}
	}

	{
	  unique_lock<mutex> lock(mutex_);
	  while (requests_.size()>0)
	  	cv_.wait(lock);
	  // block bottleneck operator
	  // TODO: grant block request if input port is empty
	  if (scheduler_.blockBottleneckOper(*bottleneck) == false)
	  	requests_.insert(bottleneck);
	  while (requests_.size()>0)
	  	cv_.wait(lock);
	}

	flowContext_.addFission(&(bottleneck->getOperator()), replicaCount);
	replicatedOperators_[bottleneck] = replicaCount;

	scheduler_.unblockOperators();
}

void FissionController::changeFissionLevel(OperatorContextImpl * bottleneck, size_t replicaCount) {
	cerr<<"change fission\t"<<bottleneck->getOperator().getName()<<"\t"<<replicaCount<<endl;
	InputPortImpl & iport = bottleneck->getInputPortImpl(0);
	// block publishers (split in this case)
	OperatorContextImpl * publisher = iport.getPublisher(0).first;
	{
		unique_lock<mutex> lock(mutex_);
		if (scheduler_.blockPublisherOper(*publisher) == false)
			requests_.insert(publisher);
		while (requests_.size()>0)
			cv_.wait(lock);
	}

	// block replicas
	// TODO: grant block request if input port is empty
	size_t fissionLevel = publisher->getNumberOfOutputPorts();
	for(int i=0; i<fissionLevel; i++) {
		{
			OutputPortImpl & oport = publisher->getOutputPortImpl(i);
			OperatorContextImpl * replica = oport.getSubscriber(0).first;

			unique_lock<mutex> lock(mutex_);
			if (scheduler_.blockBottleneckOper(*replica) == false)
				requests_.insert(replica);
		}
	}

	{
		unique_lock<mutex> lock(mutex_);
		while (requests_.size()>0)
			cv_.wait(lock);
	}
	
	flowContext_.changeFissionLevel(&(bottleneck->getOperator()), replicaCount);
	replicatedOperators_[bottleneck] = replicaCount;

	scheduler_.unblockOperators();
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

		if(replicatedOperators_.find(bottleneck) == replicatedOperators_.end()) {
			addFission(bottleneck, 2);
		}
		else {
			changeFissionLevel(bottleneck, 3);
			break;
		}
	}
}

void FissionController::blockGranted(OperatorContextImpl * oper) 
{
	unique_lock<mutex> lock(mutex_);
	cerr<<"block request in fission controller granted \t"<<oper->getOperator().getName()<<endl;
	requests_.erase(oper);
	if(requests_.size()==0) {
		cerr<<"wake up baby"<<endl;
		cv_.notify_all();
	}
	cerr<<"requests size:\t"<<requests_.size()<<endl;
}

void FissionController::join() {
	thread_->join();
}

void FissionController::setCompleted() {
	if (!isCompleted_.load()) 
		isCompleted_.store(true);
	cv_.notify_all();
}

