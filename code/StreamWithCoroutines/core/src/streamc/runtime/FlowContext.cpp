#include "streamc/runtime/FlowContext.h"

#include "streamc/Flow.h"
#include "streamc/Operator.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
#include "streamc/runtime/Scheduler.h"
#include "streamc/runtime/WorkerThread.h"
#include "streamc/runtime/SchedulerPlugin.h"

#include "streamc/operators/RoundRobinSplit.h"
#include "streamc/operators/RoundRobinMerge.h"
#include "streamc/operators/Busy.h"

using namespace std;
using namespace streamc;
using namespace streamc::operators;

size_t FlowContext::maxQueueSize_ = 10000; // TODO: make changable, and perhaps per port?

FlowContext::FlowContext(Flow & flow, SchedulerPlugin & plugin)
  : flow_(flow), numCompleted_(0), isShutdownRequested_(false)
{
  // create the scheduler
  scheduler_.reset(new Scheduler(*this, plugin));

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

  // start the scheduler and all the threads
  scheduler_->start();
  for (int i=0; i<numThreads; ++i) 
    threads_[i]->start();
}

// wait for completion
void FlowContext::wait()
{
  unique_lock<mutex> lock(mutex_);
  while(numCompleted_ < flow_.getOperators().size())
    cv_.wait(lock);
  // join all threads 
  for (auto & threadPtr : threads_) 
    threadPtr->join();
}

// called by a worker thread
// increment numCompleted, and mark oper as completed
void FlowContext::markOperatorCompleted(Operator * oper)
{
  unique_lock<mutex> lock(mutex_);
  numCompleted_++;
  { // notify scheduler about the operator completion
    OperatorContextImpl * opc = operatorContexts_[oper].get();  
    scheduler_->markOperatorAsCompleted(*opc); 
  }
  if (numCompleted_==flow_.getOperators().size()) {
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
  printTopology();
  // create splits
  Flow fflow("fission_flow");
  vector<OperatorContextImpl *> splits;
  for(size_t i=0; i<oper->getNumberOfInputPorts(); i++) {
    Operator & split = fflow.createOperator<RoundRobinSplit>(oper->getName()+"_fissionSplit_"+to_string(i), replicaCount);
    
    // create operator context
    OperatorContextImpl * operatorContext = new OperatorContextImpl(this, &split, *scheduler_);
    operatorContexts_[&split] = unique_ptr<OperatorContextImpl>(operatorContext);
    splits.push_back(operatorContext);
    
    // create input port
    InputPortImpl * port = new InputPortImpl(*operatorContext, *scheduler_);
    operatorContext->addInputPort(port);

    // create output ports
    for(size_t j=0; j<replicaCount; j++) {
      OutputPortImpl * port = new OutputPortImpl(*operatorContext, *this, *scheduler_);
      operatorContext->addOutputPort(port);
    }
  }

  // create merges
  vector<OperatorContextImpl *> merges;
  for(size_t i=0; i<oper->getNumberOfOutputPorts(); i++) {
    Operator & merge = fflow.createOperator<RoundRobinMerge>(oper->getName()+"_fissionMerge_"+to_string(i), replicaCount);

    // create operator context
    OperatorContextImpl * operatorContext = new OperatorContextImpl(this, &merge, *scheduler_);
    operatorContexts_[&merge] = unique_ptr<OperatorContextImpl>(operatorContext);
    merges.push_back(operatorContext);
    
    // create input ports
    for(size_t j=0; j<replicaCount; j++) {
      InputPortImpl * port = new InputPortImpl(*operatorContext, *scheduler_);
      operatorContext->addInputPort(port);  
    }

    // create output port
    OutputPortImpl * port = new OutputPortImpl(*operatorContext, *this, *scheduler_);
    operatorContext->addOutputPort(port);
  }

  // create replicas
  vector<OperatorContextImpl *> replicas;
  replicas.push_back(operatorContexts_[oper].get());
  for(size_t i=1; i<replicaCount; i++) {
    // TODO: make generic
    Operator & replica = fflow.createOperator<Busy>(oper->getName()+"_replica_"+to_string(i));
    

    // create operator context
    OperatorContextImpl * operatorContext = new OperatorContextImpl(this, &replica, *scheduler_);
    operatorContexts_[&replica] = unique_ptr<OperatorContextImpl>(operatorContext);
    replicas.push_back(operatorContext);
    
    // create input ports
    for(size_t j=0; j<oper->getNumberOfInputPorts(); j++) {
      InputPortImpl * port = new InputPortImpl(*operatorContext, *scheduler_);
      operatorContext->addInputPort(port);  
    }

    // create output port
    for(size_t j=0; j<oper->getNumberOfOutputPorts(); j++) {
      OutputPortImpl * port = new OutputPortImpl(*operatorContext, *this, *scheduler_);
      operatorContext->addOutputPort(port);
    }
  }
  
  OperatorContextImpl * operatorContext = operatorContexts_[oper].get();

  // get all publishers
  vector<vector<pair<OperatorContextImpl *, size_t>>> publishers(oper->getNumberOfInputPorts(), vector<pair<OperatorContextImpl *, size_t>>());
  for(size_t i=0; i<oper->getNumberOfInputPorts(); ++i) {
    InputPortImpl & iport = operatorContext->getInputPortImpl(i);
    size_t numberOfPublishers = iport.getNumberOfPublishers();
    for(size_t j=0; j<numberOfPublishers; j++) {
      publishers[i].push_back(iport.getPublisher(0));
      iport.removePublisher(0);
    }
  }
  
  // get all subscribers
  vector<vector<pair<OperatorContextImpl *, size_t>>> subscribers(oper->getNumberOfOutputPorts(), vector<pair<OperatorContextImpl *, size_t>>());
  for(size_t i=0; i<oper->getNumberOfOutputPorts(); i++) {
    OutputPortImpl & oport = operatorContext->getOutputPortImpl(i);
    size_t numberOfSubscribers = oport.getNumberOfSubscribers();
    for(size_t j=0; j<numberOfSubscribers; j++) {
      subscribers[i].push_back(oport.getSubscriber(0));
      oport.removeSubscriber(0);
    }
  }

  // remove oper from publishers' subscribers
  for(size_t i=0; i<oper->getNumberOfInputPorts(); i++) {
    size_t numberOfPublishers = publishers[i].size();
    for(size_t j=0; j<numberOfPublishers; j++) {
      OperatorContextImpl * publisherOper = publishers[i][j].first;
      OutputPortImpl & publisherOPort = publisherOper->getOutputPortImpl(publishers[i][j].second);
      size_t numberOfSubscribers = publisherOPort.getNumberOfSubscribers();
      for(size_t k=0; k<numberOfSubscribers; k++) {
        auto subscriber = publisherOPort.getSubscriber(k);
        if(subscriber.first == operatorContext && subscriber.second == i) {
          publisherOPort.removeSubscriber(k);
          break;
        }
      }
    }
  }

  // remove oper from subscribers' publishers
  for(size_t i=0; i<oper->getNumberOfOutputPorts(); i++) {
    size_t numberOfSubscribers = subscribers.size();
    for(size_t j=0; j<numberOfSubscribers; j++) {
      OperatorContextImpl * subscriberOper = subscribers[i][j].first;
      InputPortImpl & subscriberIPort = subscriberOper->getInputPortImpl(subscribers[i][j].second);
      size_t numberOfPublishers = subscriberIPort.getNumberOfPublishers();
      for(size_t k=0; k<numberOfPublishers; k++) {
        auto publisher = subscriberIPort.getPublisher(k);
        if(publisher.first == operatorContext && publisher.second == i) {
          subscriberIPort.removePublisher(k);
          break;
        }
      }
    }
  }

  // connect publishers to splits
  for(size_t i=0; i<oper->getNumberOfInputPorts(); i++) {
    size_t numberOfPublishers = publishers[i].size();
    for(size_t j=0; j<numberOfPublishers; j++) {
      OperatorContextImpl * publisherOper = publishers[i][j].first;
      OutputPortImpl & publisherOPort = publisherOper->getOutputPortImpl(publishers[i][j].second);
      publisherOPort.addSubscriber(*splits[i], 0);
      splits[i]->getInputPortImpl(0).addPublisher(*publisherOper, publishers[i][j].second);
    }
  }

  // connect splits to replicas
  for(size_t i=0; i<oper->getNumberOfInputPorts(); i++) {
    for(size_t j=0; j<replicaCount; j++) {
      splits[i]->getOutputPortImpl(j).addSubscriber(*replicas[j], i);
      replicas[j]->getInputPortImpl(i).addPublisher(*splits[i], j);
    }
  }

  // connect replicas to merges
  for(size_t i=0; i<replicaCount; i++) {
    for(size_t j=0; j<oper->getNumberOfOutputPorts(); j++) {
      replicas[i]->getOutputPortImpl(j).addSubscriber(*merges[j], i);
      merges[j]->getInputPortImpl(i).addPublisher(*replicas[i], j);
    }
  }

  // connect merges to subscribers
  for(size_t i=0; i<oper->getNumberOfOutputPorts(); i++) {
    size_t numberOfSubscribers = subscribers[i].size();
    for(size_t j=0; j<numberOfSubscribers; j++) {
      OperatorContextImpl * subscriberOper = subscribers[i][j].first;
      InputPortImpl & subscriberIPort = subscriberOper->getInputPortImpl(subscribers[i][j].second);
      merges[i]->getOutputPortImpl(0).addSubscriber(*subscriberOper, subscribers[i][j].second);
      subscriberIPort.addPublisher(*merges[i], 0);
    }
  }

  printTopology();
}