#include "streamc/runtime/FlowContext.h"

#include "streamc/Flow.h"
#include "streamc/Operator.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
#include "streamc/runtime/Scheduler.h"
#include "streamc/runtime/WorkerThread.h"

using namespace std;
using namespace streamc;

size_t FlowContext::maxQueueSize_ = 10000; // TODO: make changable, and perhaps per port?

FlowContext::FlowContext(Flow & flow)
  : flow_(flow), numCompleted_(0), isShutdownRequested_(false)
{
  // create the scheduler
  scheduler_.reset(new Scheduler());

  // get operators
  vector<Operator *> const & opers = flow_.getOperators();

  // create operator context for each operator
  for (Operator * oper : opers) {
    OperatorContextImpl * operatorContext = new OperatorContextImpl(this, oper);
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
        port->addPublisher(*operatorContexts_[ooper]);
      }
      operatorContext->addInputPort(port);
    }

    // add output ports
    for (size_t i=0; i<oper->getNumberOfOutputPorts(); ++i) {
      OutputPortImpl * port = new OutputPortImpl(*operatorContext, *scheduler_);
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
