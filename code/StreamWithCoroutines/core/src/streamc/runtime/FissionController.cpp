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

vector<OperatorContextImpl *> candidates;

void FissionController::findBottleneckCandidates() {
	vector<OperatorContextImpl *> operators = flowContext_.getOperators();
	for(auto it=operators.begin(); it!=operators.end(); it++) {
		OperatorContextImpl * oper = *it;
		if(oper->getNumberOfInputPorts() ==1 && oper->getNumberOfOutputPorts()==1)
			candidates.push_back(oper);
	}
}

OperatorContextImpl * FissionController::detectBottleneck() {
	if(candidates.size()==0)
		return nullptr;
	
	double threshold = 0.3;
	for(auto it=candidates.begin(); it!=candidates.end(); it++) {
		OperatorContextImpl * oper = *it;
		InputPortImpl & iport = oper->getInputPortImpl(0);
		OutputPortImpl & oport = oper->getOutputPortImpl(0);

		if(iport.isWriteBlocked(threshold) && !oport.isWriteBlocked(threshold)) {
		//	cerr<<"bottleneck:\t"<<oper->getOperator().getName()<<endl;
			return oper;
		}
			
	}
	return nullptr;
}

void FissionController::addFission(OperatorContextImpl * bottleneck, size_t replicaCount) {
	// block publishers
	InputPortImpl & iport = bottleneck->getInputPortImpl(0);
	size_t numberOfPublishers = iport.getNumberOfPublishers();
	for (size_t i=0; i<numberOfPublishers; i++) {
		OperatorContextImpl * publisher = iport.getPublisher(i).first;
		{
			unique_lock<mutex> lock(mutex_);
			if (scheduler_.requestPartialBlock(*publisher) == false)
				requests_.insert(publisher);
		}
	}
	// wait until publishers are blocked
	{
		unique_lock<mutex> lock(mutex_);
		while (requests_.size()>0)
			cv_.wait(lock);
	}

	// block bottleneck operator & wait
	{
		unique_lock<mutex> lock(mutex_);
		if (scheduler_.requestCompleteBlock(*bottleneck) == false)
			requests_.insert(bottleneck);
		while (requests_.size()>0)
			cv_.wait(lock);
	}

	// block request for subscribers
	OutputPortImpl & oport = bottleneck->getOutputPortImpl(0);
	size_t numberOfSubscribers = oport.getNumberOfSubscribers();
	for(size_t i=0; i<numberOfSubscribers; i++) {
		OperatorContextImpl * subscriber = oport.getSubscriber(i).first;
		{
			unique_lock<mutex> lock(mutex_);
			if(scheduler_.requestCompleteBlock(*subscriber) == false)
				requests_.insert(subscriber);
		}
	}
	// wait until subscribers are blocked
	{
		unique_lock<mutex> lock(mutex_);
		while (requests_.size()>0)
			cv_.wait(lock);
	}

	flowContext_.addFission(&(bottleneck->getOperator()), replicaCount);
	replicatedOperators_[bottleneck] = replicaCount;

	scheduler_.unblockOperators();
}

void FissionController::changeFissionLevel(OperatorContextImpl * bottleneck, size_t replicaCount) {
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

	// request block for merge's subscribers
	OutputPortImpl & mergeOPort = merge->getOutputPortImpl(0);
	size_t numberOfMergeSubscribers = mergeOPort.getNumberOfSubscribers();
	for (size_t i=0; i<numberOfMergeSubscribers; i++) {
		OperatorContextImpl * mergeSubscriber = mergeOPort.getSubscriber(i).first;
		{
			unique_lock<mutex> lock(mutex_);
			if (scheduler_.requestCompleteBlock(*mergeSubscriber) == false)
				requests_.insert(mergeSubscriber);
		}
	}
	// wait until merge subscribers blocked
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
	srand(time(NULL));

	while(!isCompleted_.load()) {
		std::chrono::milliseconds duration(2000);
		this_thread::sleep_for(duration);

		OperatorContextImpl * bottleneck = nullptr;
		vector<OperatorContextImpl *> operators = flowContext_.getOperators();
		for(auto it=operators.begin(); it!=operators.end(); it++) {
			if((*it)->getOperator().getName()=="busy") {
				bottleneck = *it;
				break;
			}
		}
		addFission(bottleneck, 10);
		this_thread::sleep_for(duration);
		changeFissionLevel(bottleneck, 5);
		return;
	}
	/*
	srand(time(NULL));
	findBottleneckCandidates();

	while(!isCompleted_.load()) {
		std::chrono::milliseconds duration(2000);
		this_thread::sleep_for(duration);

		OperatorContextImpl * bottleneck = detectBottleneck();
		if(bottleneck == nullptr) {
			this_thread::sleep_for(duration);
			continue;
		}
		cerr<<bottleneck->getOperator().getName()<<endl;
		if(replicatedOperators_.find(bottleneck) == replicatedOperators_.end()) {
			addFission(bottleneck, 7);
			//addFission(bottleneck, rand()%10+2);
		}
		else {
			changeFissionLevel(bottleneck, rand()%3+2);
		}
	}
	*/
}

void FissionController::blockGranted(OperatorContextImpl * oper) 
{
	unique_lock<mutex> lock(mutex_);
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

