#include "streamc/runtime/FissionController.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
#include "streamc/Operator.h"
#include "streamc/runtime/RuntimeLogger.h"
#include <time.h>

#include <chrono>

#define BOTTLENECK_THRESHOLD 0.01
#define FISSION_PERIOD 10000 //milliseconds

using namespace std;
using namespace streamc;

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
	candidates.clear();
	vector<OperatorContextImpl *> operators = flowContext_.getOperators();
	for(auto it=operators.begin(); it!=operators.end(); it++) {
		OperatorContextImpl * oper = *it;
		if(oper->getNumberOfInputPorts() ==1 && oper->getNumberOfOutputPorts()==1)
			candidates.push_back(oper);
	}
}

void printCandidates() {
	SC_LOG(Info, "----");
	SC_LOG(Info, candidates.size());
	int counter = 0;
	for(auto it=candidates.begin(); it!=candidates.end(); it++) {
		
		SC_LOG(Info, counter++<<":\t"<<(*it)->getOperator().getName());
		SC_LOG(Info, "----");
	}
}

OperatorContextImpl * FissionController::detectBottleneck() {
	if(candidates.size()==0)
		return nullptr;
	/**/
	vector<OperatorContextImpl *> operators = flowContext_.getOperators();
	for(auto it=operators.begin(); it!=operators.end(); it++) {
		OperatorContextImpl * oper = *it;
		if(oper->getNumberOfInputPorts() ==1 && oper->getNumberOfOutputPorts()==1) {
			InputPortImpl & iport = oper->getInputPortImpl(0);
			OutputPortImpl & oport = oper->getOutputPortImpl(0);
			if(oper->isComplete() || oper->getOperator().getName().compare("resultCollector")==0)
				continue;

			SC_LOG(Info,"Check bottleneck:\t"<<oper->getOperator().getName());
			SC_LOG(Info, "iport:\t"<<iport.getOperatorContextImpl().getOperator().getName()<<"\t"<<iport.getWriteBlockedRatio());
			SC_LOG(Info, "oport:\t"<<oport.getOperatorContextImpl().getOperator().getName()<<"\t"<<oport.getWriteBlockedRatio());
		}
	}
	SC_LOG(Info,"----------------------------------------------\n")
	/**/
	for(auto it=candidates.begin(); it!=candidates.end(); it++) {
		OperatorContextImpl * oper = *it;
		InputPortImpl & iport = oper->getInputPortImpl(0);
		OutputPortImpl & oport = oper->getOutputPortImpl(0);

		if(oper->isComplete() || oper->getOperator().getName().compare("resultCollector")==0)
			continue;

		// non-replicated operator
		if(replicatedOperators_.find(oper) == replicatedOperators_.end()) {
			SC_LOG(Info, "Checking:\t"<<oper->getOperator().getName()<<"\tratio:\t"<<iport.getWriteBlockedRatio());
			SC_LOG(Info, "Checking:\t"<<oper->getOperator().getName()<<"\tratio:\t"<<oport.getWriteBlockedRatio());
			if(iport.getWriteBlockedRatio()>=BOTTLENECK_THRESHOLD && oport.getWriteBlockedRatio()<BOTTLENECK_THRESHOLD) {
				return oper;
			}
		}

		// replicated operator
		else {
			OperatorContextImpl * split = iport.getPublisher(0).first;
			size_t replicaCount = split->getNumberOfOutputPorts();
			double avgWriteBlockedRatio = 0;
			for(size_t i=0; i<replicaCount; i++) {
				avgWriteBlockedRatio = avgWriteBlockedRatio + split->getOutputPortImpl(i).getWriteBlockedRatio();
			}
			avgWriteBlockedRatio = avgWriteBlockedRatio / replicaCount;

			SC_LOG(Info, "Checking:\t"<<oper->getOperator().getName()<<"\tratio:\t"<<avgWriteBlockedRatio);
			SC_LOG(Info, "Checking:\t"<<oper->getOperator().getName()<<"\tratio:\t"<<oport.getWriteBlockedRatio());
			if(avgWriteBlockedRatio>=BOTTLENECK_THRESHOLD && oport.getWriteBlockedRatio()<BOTTLENECK_THRESHOLD) {
				return oper;
			}
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

void FissionController::resetPortsWaitingClock() {
	for(auto it=candidates.begin(); it!=candidates.end(); it++) {
		// reset candidate's clocl
		OperatorContextImpl * oper = *it;
		InputPortImpl & iport = oper->getInputPortImpl(0);
		iport.resetBeginTime();

		OutputPortImpl & oport = oper->getOutputPortImpl(0);
		oport.resetBeginTime();

		// reset replicas' clock if any
		if(replicatedOperators_.find(oper)!=replicatedOperators_.end()) {
			OperatorContextImpl * split = iport.getPublisher(0).first;
			size_t replicaCount = split->getNumberOfOutputPorts();
			for(size_t i=0; i<replicaCount; i++) {
				split->getOutputPortImpl(i).resetBeginTime();
			}
		}
	}
}

void FissionController::run() {
	return;
	srand(time(NULL));
	findBottleneckCandidates();
	std::chrono::milliseconds duration(FISSION_PERIOD);

	while(!isCompleted_.load()) {
		this_thread::sleep_for(duration);

		OperatorContextImpl * bottleneck = detectBottleneck();
		if(bottleneck == nullptr)
			continue;
		
		cerr<<"replicate:\t"<<bottleneck->getOperator().getName()<<endl;
		SC_LOG(Info,"replicate:\t"<<bottleneck->getOperator().getName());
		if(replicatedOperators_.find(bottleneck) == replicatedOperators_.end()) {
			addFission(bottleneck, 2);
		}
		else {
			changeFissionLevel(bottleneck, replicatedOperators_[bottleneck]+1);
		}
		resetPortsWaitingClock();
	}
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

