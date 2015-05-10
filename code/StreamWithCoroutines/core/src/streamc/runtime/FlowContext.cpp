#include "streamc/runtime/FlowContext.h"

#include "streamc/Flow.h"
#include "streamc/Operator.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
#include "streamc/runtime/Scheduler.h"
#include "streamc/runtime/WorkerThread.h"
#include "streamc/runtime/SchedulerPlugin.h"
#include "streamc/runtime/RuntimeLogger.h"
#include "streamc/runtime/RandomScheduling.h"
#include "streamc/runtime/FissionController.h"
#include "streamc/runtime/UtilityController.h"

#include "streamc/operators/RoundRobinSplit.h"
#include "streamc/operators/RoundRobinMerge.h"

using namespace std;
using namespace streamc;
using namespace streamc::operators;

size_t FlowContext::maxQueueSize_ = 1000; // TODO: make changable, and perhaps per port?

FlowContext::FlowContext(Flow & flow, SchedulerPlugin * plugin)
  : flow_(flow), numCompleted_(0), isShutdownRequested_(false)
{
  SC_LOG(Info, "\n\n\n\n\n\n\n\nNEW EXPERIMENT\n");
  // create the scheduler
  if (plugin!=nullptr)
    scheduler_.reset(new Scheduler(*this, plugin));
  else
     scheduler_.reset(new Scheduler(*this, new RandomScheduling()));

  // create fissionController
  fissionController_ = new FissionController(*this, *scheduler_);
  scheduler_->setFissionController(fissionController_);

  // create utilityController
  utilityController_ = new UtilityController(*this, *scheduler_);
  scheduler_->setUtilityController(utilityController_);

  // get operators
  vector<Operator *> const & opers = flow_.getOperators();

  // create operator context for each operator
  for (Operator * oper : opers) {
    OperatorContextImpl * operatorContext = new OperatorContextImpl(this, oper, *scheduler_);
    operatorContexts_[oper] = unique_ptr<OperatorContextImpl>(operatorContext);
  }
  
  // add input and output ports to each operator context
  for (Operator * oper : opers) {
    OperatorContextImpl * operatorContext = operatorContexts_[oper].get();

    // add input ports
    for (size_t i=0; i<oper->getNumberOfInputPorts(); ++i) {
      InputPortImpl * port = new InputPortImpl(*operatorContext, *scheduler_);
      vector<connectors::FromConnection> const & conns = flow_.getInConnections(*oper, i);

      // add publishers
      for (auto const & conn : conns) {
        Operator * ooper = &conn.getOperator();
        port->addPublisher(*operatorContexts_[ooper], conn.getOutputPort());
      }
      operatorContext->addInputPort(port);
    }

    // add output ports
    for (size_t i=0; i<oper->getNumberOfOutputPorts(); ++i) {
      OutputPortImpl * port = new OutputPortImpl(*operatorContext, *this, *scheduler_);
      vector<connectors::ToConnection> const & conns = flow_.getOutConnections(*oper, i);

      //add subscribers
      for (auto const & conn : conns) {
        Operator * ooper = &conn.getOperator();
        port->addSubscriber(*operatorContexts_[ooper], conn.getInputPort());
      }
      operatorContext->addOutputPort(port);
    }
  }
  // add operator contexts to the schduler
  for (Operator * oper : opers) {
    auto & opcPtr = operatorContexts_[oper];
    scheduler_->addOperatorContext(*opcPtr);
  }
}

FlowContext::~FlowContext()
{}
/*
void FlowContext::run(int numThreads)
{
  // reset the shutdown requested (in case we are being rerun)
  isShutdownRequested_.store(false);
  
  // create threads and add threads to the scheduler 
  scheduler_->removeThreads();
  threads_.clear();
  for (int i=0; i<numThreads; ++i) { 
    auto thread = new WorkerThread(i, *scheduler_);
    threads_.push_back(unique_ptr<WorkerThread>(thread));
    scheduler_->addThread(*threads_[i]);
  }
  
  utilityController_->start();
  fissionController_->start();

  // start the scheduler and all the threads
  scheduler_->start();
  for (int i=0; i<numThreads; ++i) 
    threads_[i]->start();
}
*/
void FlowContext::run(int maxThreads)
{
  // reset the shutdown requested (in case we are being rerun)
  isShutdownRequested_.store(false);
  
  // create threads and add threads to the scheduler 
  //scheduler_->removeThreads();
  //threads_.clear();
  /*
  for (int i=0; i<numThreads; ++i) { 
    auto thread = new WorkerThread(i, *scheduler_);
    threads_.push_back(unique_ptr<WorkerThread>(thread));
    scheduler_->addThread(*threads_[i]);
  }
  */
  utilityController_->start(maxThreads);
  fissionController_->start();

  // start the scheduler and all the threads
  /*
  scheduler_->start();
  for (int i=0; i<numThreads; ++i) 
    threads_[i]->start();
  */
}

// wait for completion
/*
void FlowContext::wait()
{
  unique_lock<mutex> lock(mutex_);
  while(numCompleted_ < operatorContexts_.size())
    cv_.wait(lock);
  // join all threads 
  for (auto & threadPtr : threads_) 
    threadPtr->join();
  fissionController_->setCompleted();
  fissionController_->join();

  utilityController_->setCompleted();
  utilityController_->join();
}
*/
void FlowContext::wait()
{
  unique_lock<mutex> lock(mutex_);
  while(numCompleted_ < operatorContexts_.size())
    cv_.wait(lock);
  // join all threads 
  //for (auto & threadPtr : threads_) 
  //  threadPtr->join();
  fissionController_->setCompleted();
  fissionController_->join();

  utilityController_->setCompleted();
  utilityController_->join();
}

// called by a worker thread
// increment numCompleted, and mark oper as completed
void FlowContext::markOperatorCompleted(Operator * oper)
{
  SC_LOG(Info, "Operator Completed:\t"<<oper->getName());
  
  unique_lock<mutex> lock(mutex_);
  numCompleted_++;
  { // notify scheduler about the operator completion
    OperatorContextImpl * opc = operatorContexts_[oper].get();
    scheduler_->markOperatorAsCompleted(*opc); 
  }

  if (numCompleted_==operatorContexts_.size()) {
    // tell the scheduler that there is no more work
    scheduler_->stop();
    // wake up clients waiting for completion
    cv_.notify_all();
  }
}

// set isShutDownRequested as true
void FlowContext::requestShutdown()
{
  if (!isShutdownRequested_.load()) 
    isShutdownRequested_.store(true);
}

// return isShutDownRequested value
bool FlowContext::isShutdownRequested()
{
  return isShutdownRequested_.load();
}

vector<OperatorContextImpl *> FlowContext::getOperators() {
  vector<OperatorContextImpl *> operators;

  for(auto it=operatorContexts_.begin(); it!=operatorContexts_.end(); it++) {
    operators.push_back(it->second.get());
  }
  return operators;
}

void FlowContext::printTopology() {
  size_t numberOfOperators = operatorContexts_.size();
  cout<<"numberOfOperators\t"<<numberOfOperators<<endl;

  for(auto it=operatorContexts_.begin(); it!=operatorContexts_.end(); ++it) {
    Operator * oper = it->first;
    OperatorContextImpl * operatorContext = operatorContexts_[oper].get();
    cout<<oper->getName()<<endl;
    for(size_t i=0; i<oper->getNumberOfInputPorts(); i++) {
      cout<<"\tinput port: "<<i<<"\t"<<endl;
      cout<<"\tpublishers:"<<endl;
      InputPortImpl & iport = operatorContext->getInputPortImpl(i);
      size_t numberOfPublishers = iport.getNumberOfPublishers();
      for(size_t j=0; j<numberOfPublishers; j++) {
        auto publisher = iport.getPublisher(j);
        cout<<"\t\t"<<publisher.first->getOperator().getName()<<"\toutput port: "<<publisher.second<<endl;
      }
    }

    for(size_t i=0; i<oper->getNumberOfOutputPorts(); i++) {
      cout<<"\toutput port: "<<i<<"\t"<<endl;
      cout<<"\tsubscribers:"<<endl;
      OutputPortImpl & oport = operatorContext->getOutputPortImpl(i);
      size_t numberOfSubscribers = oport.getNumberOfSubscribers();
      for(size_t j=0; j<numberOfSubscribers; j++) {
        auto subscriber = oport.getSubscriber(j);
        cout<<"\t\t"<<subscriber.first->getOperator().getName()<<"\tinput port: "<<subscriber.second<<endl;
      }
    }
  }
  cout<<endl;
}

void FlowContext::addFission(Operator *oper, size_t replicaCount) {
  // create RRSplit
  Operator & split = *(new RoundRobinSplit(oper->getName()+"_fissionSplit"+to_string(replicaCount), replicaCount));
  OperatorContextImpl * splitContext = new OperatorContextImpl(this, &split, *scheduler_);
  {
    operatorContexts_[&split] = unique_ptr<OperatorContextImpl>(splitContext);
    InputPortImpl * iport = new InputPortImpl(*splitContext, *scheduler_);
    splitContext->addInputPort(iport);

    for(size_t j=0; j<replicaCount; j++) {
      OutputPortImpl * oport = new OutputPortImpl(*splitContext, *this, *scheduler_);
      splitContext->addOutputPort(oport);
    }
  }
  // create RRMerge
  Operator & merge = *(new RoundRobinMerge(oper->getName()+"_fissionMerge"+to_string(replicaCount), replicaCount));
  OperatorContextImpl * mergeContext = new OperatorContextImpl(this, &merge, *scheduler_);
  {
    operatorContexts_[&merge] = unique_ptr<OperatorContextImpl>(mergeContext);

    for(size_t j=0; j<replicaCount; j++) {
      InputPortImpl * iport = new InputPortImpl(*mergeContext, *scheduler_);
      mergeContext->addInputPort(iport);  
    }

    OutputPortImpl * oport = new OutputPortImpl(*mergeContext, *this, *scheduler_);
    mergeContext->addOutputPort(oport);
  }
  
  // create Replicas
  vector<OperatorContextImpl *> replicas;

  OperatorContextImpl * operatorContext = operatorContexts_[oper].get();
  replicas.push_back(operatorContext);
  for(size_t i=1; i<replicaCount; i++) {
    Operator & replica = *(oper->clone(oper->getName()+"_replica_"+to_string(i)));

    // create replica context
    OperatorContextImpl * replicaContext = new OperatorContextImpl(this, &replica, *scheduler_);
    operatorContexts_[&replica] = unique_ptr<OperatorContextImpl>(replicaContext);
    
    // create input port
    InputPortImpl * iport = new InputPortImpl(*replicaContext, *scheduler_);
    replicaContext->addInputPort(iport);  

    // create output port
    OutputPortImpl * oport = new OutputPortImpl(*replicaContext, *this, *scheduler_);
    replicaContext->addOutputPort(oport);

    replicas.push_back(replicaContext);
  }

  // get publishers
  InputPortImpl & iport = operatorContext->getInputPortImpl(0);
  size_t numberOfPublishers = iport.getNumberOfPublishers();
  
  vector<pair<OperatorContextImpl *, size_t>> publishers;
  for(size_t i=0; i<numberOfPublishers; i++)
    publishers.push_back(iport.getPublisher(i));

  // disconnect publisher-oper, connect publisher-rrsplit
  {
    // remove all publishers from oper's iport
    for(size_t i=0; i<numberOfPublishers; i++)
      iport.removePublisher(0);

    // for each publisher
    for(size_t i=0; i<numberOfPublishers; i++) {
      OperatorContextImpl * publisherOper = publishers[i].first;
      size_t publisherPortNo = publishers[i].second;
      OutputPortImpl & publisherOPort = publisherOper->getOutputPortImpl(publisherPortNo);

      // remove oper from publisher oport's subscriber list
      size_t numberOfPublisherSubscribers = publisherOPort.getNumberOfSubscribers();
      for(size_t j=0; j<numberOfPublisherSubscribers; j++) {
        OperatorContextImpl * publisherSubscriber = publisherOPort.getSubscriber(j).first;
        if(publisherSubscriber == operatorContext) {
          publisherOPort.removeSubscriber(j);
          break;
        }
      }

      // insert publisher to rrSplit iport's publisher list
      InputPortImpl & splitIPort = splitContext->getInputPortImpl(0);
      splitIPort.addPublisher(*publisherOper, publisherPortNo);

      // insert rrSplit to publisher oport's subscriber list
      publisherOPort.addSubscriber(*splitContext, 0);
    }
  }

  // get subscribers
  OutputPortImpl & oport = operatorContext->getOutputPortImpl(0);
  size_t numberOfSubscribers = oport.getNumberOfSubscribers();

  vector<pair<OperatorContextImpl *, size_t>> subscribers;
  for(size_t i=0; i<numberOfSubscribers; i++)
    subscribers.push_back(oport.getSubscriber(i));

  // disconnect oper-subscriber, connect rrmerge-subscriber
  {
    // remove all subscribers from oper's oport
    for(size_t i=0; i<numberOfSubscribers; i++)
      oport.removeSubscriber(0);

    // for each subscriber
    for(size_t i=0; i<numberOfSubscribers; i++) {
      OperatorContextImpl * subscriberOper = subscribers[i].first;
      size_t subscriberPortNo = subscribers[i].second;
      InputPortImpl & subscriberIPort = subscriberOper->getInputPortImpl(subscriberPortNo);

      // remove oper from subscriber iport;s publisher list
      size_t numberOfSubscriberPublishers = subscriberIPort.getNumberOfPublishers();
      for(size_t j=0; j<numberOfSubscriberPublishers; j++) {
        OperatorContextImpl * subscriberPublisher = subscriberIPort.getPublisher(j).first;
        if(subscriberPublisher == operatorContext) {
          subscriberIPort.removePublisher(j);
          break;
        }
      }

      // insert subscriber to rrMerge oport's subscriber list
      OutputPortImpl & mergeOPort = mergeContext->getOutputPortImpl(0);
      mergeOPort.addSubscriber(*subscriberOper, subscriberPortNo);

      // insert rrMerge to subscriber iport's publishere list
      subscriberIPort.addPublisher(*mergeContext, 0);
    }
  }

  // connect rrSplit to replicas
  for(size_t i=0; i<replicaCount; i++) {
    splitContext->getOutputPortImpl(i).addSubscriber(*replicas[i], 0);
    replicas[i]->getInputPortImpl(0).addPublisher(*splitContext, i);
  }

  // connect replicas to rrMerge
  for(size_t i=0; i<replicaCount; i++) {
    replicas[i]->getOutputPortImpl(0).addSubscriber(*mergeContext, i);
    mergeContext->getInputPortImpl(i).addPublisher(*replicas[i], 0);
  }

  // add new opers to scheduler
  scheduler_->addOperatorContext(*operatorContexts_[&merge]);
  scheduler_->addOperatorContext(*operatorContexts_[&split]);
  for(int i=1; i<replicaCount; i++) {
    scheduler_->addOperatorContext(*operatorContexts_[&(replicas[i]->getOperator())]);
  }
}

void FlowContext::disconnect(OperatorContextImpl *upStreamOper, OperatorContextImpl *downStreamOper) {
  size_t numberOfOutputPorts = upStreamOper->getNumberOfOutputPorts();
  for(size_t i=0; i<numberOfOutputPorts; i++) {
    OutputPortImpl & oport = upStreamOper->getOutputPortImpl(i);
    size_t numberOfSubscribers = oport.getNumberOfSubscribers();
    for(size_t j=0; j<numberOfSubscribers; j++) {
      auto subscriberPair = oport.getSubscriber(j);
      OperatorContextImpl * subscriberOper = subscriberPair.first;

      if(subscriberOper == downStreamOper) {
        InputPortImpl & subscriberIPort = subscriberOper->getInputPortImpl(subscriberPair.second);
        subscriberIPort.removePublisher(*upStreamOper);
        oport.removeSubscriber(*downStreamOper);
        break;
      }
    }
  }
}

void FlowContext::connect(OperatorContextImpl *upStreamOper, size_t oportNo, OperatorContextImpl *downStreamOper, size_t iportNo) {
  OutputPortImpl & oport = upStreamOper->getOutputPortImpl(oportNo);
  oport.addSubscriber(*downStreamOper, iportNo);

  InputPortImpl & iport = downStreamOper->getInputPortImpl(iportNo);
  iport.addPublisher(*upStreamOper, oportNo);
}

void FlowContext::changeFissionLevel(Operator *oper, size_t newReplicaCount) {
  OperatorContextImpl * operatorContext = operatorContexts_[oper].get();
  InputPortImpl & iport = operatorContext->getInputPortImpl(0);
  OutputPortImpl & oport = operatorContext->getOutputPortImpl(0);

  // get oldSplit & oldMerge
  OperatorContextImpl * oldSplit = iport.getPublisher(0).first;
  OperatorContextImpl * oldMerge = oport.getSubscriber(0).first;

  // get oldReplicas
  vector<OperatorContextImpl *> oldReplicas;
  size_t oldReplicaCount = oldSplit->getNumberOfOutputPorts();
  for (size_t i=0; i<oldReplicaCount; i++) {
    OutputPortImpl & splitOPort = oldSplit->getOutputPortImpl(i);
    oldReplicas.push_back(splitOPort.getSubscriber(0).first);
  }

  // get publishers
  vector<pair<OperatorContextImpl *, size_t>> publishers;
  InputPortImpl  & oldSplitIPort = oldSplit->getInputPortImpl(0);
  size_t numberOfPublishers = oldSplitIPort.getNumberOfPublishers();
  for(size_t i=0; i<numberOfPublishers; i++) {
    publishers.push_back(oldSplitIPort.getPublisher(i));
  }

  // get subscribers
  vector<pair<OperatorContextImpl *, size_t>> subscribers;
  OutputPortImpl & oldMergeOPort = oldMerge->getOutputPortImpl(0);
  size_t numberOfSubscribers = oldMergeOPort.getNumberOfSubscribers();
  for(size_t i=0; i<numberOfSubscribers; i++) {
    subscribers.push_back(oldMergeOPort.getSubscriber(i));
  }

  // disconnect publishers-oldSplit
  for(auto it=publishers.begin(); it!=publishers.end(); it++) {
    disconnect(it->first, oldSplit);
  }

  // disconnect oldSplit-oldReplicas-oldMerge
  for(auto it=oldReplicas.begin(); it!=oldReplicas.end(); it++) {
    disconnect(oldSplit, *it);
    disconnect(*it, oldMerge);
  }

  // disconnect oldMerge-subscribers
  for(auto it=subscribers.begin(); it!=subscribers.end(); it++) {
    disconnect(oldMerge, it->first);
  }

  // remove dangling operators from scheduler_ & flowContext_
  scheduler_->removeOperatorContext(*operatorContexts_[&(oldSplit->getOperator())]);
  scheduler_->removeOperatorContext(*operatorContexts_[&(oldMerge->getOperator())]);

  operatorContexts_.erase(&(oldSplit->getOperator()));
  operatorContexts_.erase(&(oldMerge->getOperator()));

  for(size_t i=newReplicaCount; i<oldReplicaCount; i++) {
    scheduler_->removeOperatorContext(*operatorContexts_[&(oldReplicas[i]->getOperator())]);
    operatorContexts_.erase(&(oldReplicas[i]->getOperator()));
  }

  // create newSplit
  Operator * newSplitOper = new RoundRobinSplit(oper->getName()+"_fissionSplit"+to_string(newReplicaCount), newReplicaCount);
  OperatorContextImpl * newSplit = new OperatorContextImpl(this, newSplitOper, *scheduler_);
  {
    operatorContexts_[newSplitOper] = unique_ptr<OperatorContextImpl>(newSplit);
    InputPortImpl * iport = new InputPortImpl(*newSplit, *scheduler_);
    newSplit->addInputPort(iport);

    for(size_t j=0; j<newReplicaCount; j++) {
      OutputPortImpl * oport = new OutputPortImpl(*newSplit, *this, *scheduler_);
      newSplit->addOutputPort(oport);
    }
  }
  // create newMerge
  Operator * newMergeOper = new RoundRobinMerge(oper->getName()+"_fissionMerge"+to_string(newReplicaCount), newReplicaCount);
  OperatorContextImpl * newMerge = new OperatorContextImpl(this, newMergeOper, *scheduler_);
  {
    operatorContexts_[newMergeOper] = unique_ptr<OperatorContextImpl>(newMerge);

    for(size_t j=0; j<newReplicaCount; j++) {
      InputPortImpl * iport = new InputPortImpl(*newMerge, *scheduler_);
      newMerge->addInputPort(iport);
    }

    OutputPortImpl * oport = new OutputPortImpl(*newMerge, *this, *scheduler_);
    newMerge->addOutputPort(oport);
  }

    // get replicas
  vector<OperatorContextImpl *> replicas;
  if(newReplicaCount<oldReplicaCount) {
    for(size_t i=0; i<newReplicaCount; i++)
      replicas.push_back(oldReplicas[i]);
  }
  else {
    for(size_t i=0; i<oldReplicaCount; i++)
      replicas.push_back(oldReplicas[i]);

    for(size_t i=oldReplicaCount; i<newReplicaCount; i++) {
      Operator & replicaOper = *(oper->clone(oper->getName()+"_replica_"+to_string(i)));

      // create replica context
      OperatorContextImpl * replica = new OperatorContextImpl(this, &replicaOper, *scheduler_);
      operatorContexts_[&replicaOper] = unique_ptr<OperatorContextImpl>(replica);
      
      // create input port
      InputPortImpl * iport = new InputPortImpl(*replica, *scheduler_);
      replica->addInputPort(iport);

      // create output port
      OutputPortImpl * oport = new OutputPortImpl(*replica, *this, *scheduler_);
      replica->addOutputPort(oport);

      replicas.push_back(replica);
    }
  }

  // connect publishers-newSplit
  for(auto it=publishers.begin(); it!=publishers.end(); it++) {
    connect(it->first, it->second, newSplit, 0);
  }

  // connect newSplit-replicas-newMerge
  for(size_t i=0; i<replicas.size(); i++) {
    connect(newSplit, i, replicas[i], 0);
    connect(replicas[i], 0, newMerge, i);
  }

  // connect newMerge-subscribers
  for(auto it=subscribers.begin(); it!=subscribers.end(); it++) {
    connect(newMerge, 0, it->first, it->second);
  }
  
  // add newSplit and newMerge to scheduler
  scheduler_->addOperatorContext(*operatorContexts_[newMergeOper]);
  scheduler_->addOperatorContext(*operatorContexts_[newSplitOper]);

  // add new replicas to scheduler
  for(size_t i=oldReplicaCount; i<newReplicaCount; i++) {
    scheduler_->addOperatorContext(*operatorContexts_[&(replicas[i]->getOperator())]);
  }
  return;
  
  // remove oldSplit and oldMerge from scheduler
  /*
  scheduler_->removeOperatorContext(*operatorContexts_[&(oldSplit->getOperator())]);
  scheduler_->removeOperatorContext(*operatorContexts_[&(oldMerge->getOperator())]);
  
  // remove replicas from scheduler
  
  for(size_t i=newReplicaCount; i<oldReplicaCount; i++) {
    scheduler_->removeOperatorContext(*operatorContexts_[&(oldReplicas[i]->getOperator())]);
  }
  */
  /*  */
  /*
  size_t numberOfPublishers = newSplit->getInputPortImpl(0).getNumberOfPublishers();
  for(size_t i=0; i<numberOfPublishers; i++) {
    OperatorContextImpl * publisherOper = newSplit->getInputPortImpl(0).getPublisher(i).first;
    scheduler_->reInitOperatorInfo(*publisherOper);
  }
  */
  /* */

  // remove oldSplit and oldMerge from FlowContext
  /*
  operatorContexts_.erase(&(oldSplit->getOperator()));
  operatorContexts_.erase(&(oldMerge->getOperator()));

  // remove replicas from FlowContext
  for (size_t i=newReplicaCount; i<oldReplicaCount; i++)
    operatorContexts_.erase(&(oldReplicas[i]->getOperator()));
  */
}