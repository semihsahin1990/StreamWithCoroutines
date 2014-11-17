#include "streamc/runtime/FissionController.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
#include "streamc/Operator.h"
#include <time.h>

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
			if (scheduler_.requestPartialBlock(*publisher) == false)
				requests_.insert(publisher);
		}
	}

	// block bottleneck operator
	{
	  unique_lock<mutex> lock(mutex_);
	  while (requests_.size()>0)
	  	cv_.wait(lock);
	  if (scheduler_.requestCompleteBlock(*bottleneck) == false)
	  	requests_.insert(bottleneck);
	  while (requests_.size()>0)
	  	cv_.wait(lock);
	}

	flowContext_.addFission(&(bottleneck->getOperator()), replicaCount);
	replicatedOperators_[bottleneck] = replicaCount;

	scheduler_.unblockOperators();
}

void FissionController::changeFissionLevel(OperatorContextImpl * bottleneck, size_t replicaCount) {
	cerr<<endl<<endl<<"CHAGE FISSION\t"<<bottleneck->getOperator().getName()<<"\t"<<replicaCount<<endl;
	
	InputPortImpl & iport = bottleneck->getInputPortImpl(0);
	OutputPortImpl & oport = bottleneck->getOutputPortImpl(0);
	
	OperatorContextImpl * split = iport.getPublisher(0).first;
	OperatorContextImpl * merge = oport.getSubscriber(0).first;

	// request block for split's publishers
	InputPortImpl & splitIPort = split->getInputPortImpl(0);
	size_t numberOfSplitPublishers = splitIPort.getNumberOfPublishers();
	for (size_t i=0; i<numberOfSplitPublishers; i++) {
		OperatorContextImpl * splitPublisher = splitIPort.getPublisher(i).first;
		{
			unique_lock<mutex> lock(mutex_);
			if (scheduler_.requestPartialBlock(*splitPublisher) == false)
				requests_.insert(splitPublisher);
		}
	}
	// wait until split publishers blocked
	{
	  unique_lock<mutex> lock(mutex_);
	  while (requests_.size()>0)
	  	cv_.wait(lock);
	}
	
	// request block for split & wait
	{
		unique_lock<mutex> lock(mutex_);
		if (scheduler_.requestCompleteBlock(*split) == false)
			requests_.insert(split);
		while (requests_.size()>0)
	  		cv_.wait(lock);
	}
	
	// request block for replicas
	size_t numberOfReplicas = split->getNumberOfOutputPorts();
	for (size_t i=0; i<numberOfReplicas; i++) {
		OutputPortImpl & splitOPort = split->getOutputPortImpl(i);
		OperatorContextImpl * replica = splitOPort.getSubscriber(0).first;
		{
			unique_lock<mutex> lock(mutex_);
			if (scheduler_.requestCompleteBlock(*replica) == false)
				requests_.insert(replica);
		}
	}
	// wait until replicas blocked
	{
		unique_lock<mutex> lock(mutex_);
		while (requests_.size()>0)
			cv_.wait(lock);
	}
	
	// request block for merge & wait
	{
		unique_lock<mutex> lock(mutex_);
		if (scheduler_.requestCompleteBlock(*merge) == false)
			requests_.insert(merge);
		while (requests_.size()>0)
	  		cv_.wait(lock);
	}
	
	flowContext_.changeFissionLevel(&(bottleneck->getOperator()), replicaCount);
	replicatedOperators_[bottleneck] = replicaCount;
	scheduler_.unblockOperators();
}

void FissionController::run() {
	srand(time(NULL));
	while(!isCompleted_.load()) {
		std::chrono::milliseconds duration(500);
		this_thread::sleep_for(duration);

		OperatorContextImpl * bottleneck = detectBottleneck();
		if(bottleneck == nullptr) {
			this_thread::sleep_for(duration);
			continue;
		}
		
		if(replicatedOperators_.find(bottleneck) == replicatedOperators_.end()) {
			addFission(bottleneck, rand()%10 + 1);

		}
		else {
			changeFissionLevel(bottleneck, rand()%10+1);
		}
	}
}

void FissionController::blockGranted(OperatorContextImpl * oper) 
{
	unique_lock<mutex> lock(mutex_);
	cerr<<"block granted in fission controller:  \t"<<oper->getOperator().getName()<<"\t"<<oper->getInputPortImpl(0).getTupleCount()<<endl;
	requests_.erase(oper);
	if(requests_.size()==0) {
		cv_.notify_all();
	}
}

void FissionController::join() {
	thread_->join();
}

void FissionController::setCompleted() {
	if (!isCompleted_.load()) 
		isCompleted_.store(true);
	cv_.notify_all();
}

