#include "streamc/runtime/FlowContext.h"

#include "streamc/Flow.h"

#include "streamc/Operator.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
#include "streamc/runtime/OperatorContextImpl.h"

using namespace std;
using namespace streamc;

//constructor with flow, initially none of the operators is completed and shutdown is not requested.
FlowContext::FlowContext(Flow & flow)
  : flow_(flow), numCompleted_(0), isShutdownRequested_(false)
{}

//destructor 
FlowContext::~FlowContext()
{}

/*
create operator contexts for each operator in the flow
add input and output ports to each operator context
run the flow with numThreads thread
*/
void FlowContext::run(int numThreads)
{
  //get operators
  vector<Operator *> const & opers = flow_.getOperators();

  //create operator context for each operator
  for (Operator * oper : opers) 
  {
    uintptr_t opAddr = reinterpret_cast<uintptr_t>(oper);
    OperatorContextImpl * operatorContext = new OperatorContextImpl(this, oper);
    // add the operator context
    operatorContexts_[opAddr] = unique_ptr<OperatorContextImpl>(operatorContext);
  }
  

  //add input and output ports to each operator context
  for (Operator * oper : opers) 
  {
    uintptr_t opAddr = reinterpret_cast<uintptr_t>(oper);
    OperatorContextImpl * operatorContext = operatorContexts_[opAddr].get();

    // add input ports
    for (size_t i=0; i<oper->getNumberOfInputPorts(); ++i) 
    {
      InputPortImpl * port = new InputPortImpl();
      vector<FromConnection> const & conns = flow_.getInConnections(*oper, i);

      //add publishers
      for (auto const & conn : conns) 
      {
        uintptr_t oopAddr = reinterpret_cast<uintptr_t>(&conn.getOperator());
        port->addPublisher(operatorContexts_[oopAddr].get());
      }
      operatorContext->addInputPort(port);
    }

    // add output ports
    for (size_t i=0; i<oper->getNumberOfOutputPorts(); ++i) 
    {
      OutputPortImpl * port = new OutputPortImpl();
      vector<ToConnection> const & conns = flow_.getOutConnections(*oper, i);

      //add subscribers
      for (auto const & conn : conns)  
      {
        uintptr_t oopAddr = reinterpret_cast<uintptr_t>(&conn.getOperator());
        port->addSubscriber(operatorContexts_[oopAddr].get(), conn.getInputPort());
      }
      operatorContext->addOutputPort(port);
    }
  }
  
  //init all operators
  for (Operator * oper : opers) 
  {
    uintptr_t opAddr = reinterpret_cast<uintptr_t>(oper);
    OperatorContextImpl * operatorContext = operatorContexts_[opAddr].get();
    operatorContext->initOper();
  }
  // TODO: we need to hookup to the scheduler to perform the computation
}

//////////////////////////////////////////////////////////////////////////////
void FlowContext::wait()
{
  unique_lock<mutex> lock(mutex_);
  while(numCompleted_ < flow_.getOperators().size())
    cv_.wait(lock);
}

//increment numCompleted, and mark oper as completed
void FlowContext::markOperatorCompleted(Operator * oper)
{
  unique_lock<mutex> lock(mutex_);
  numCompleted_++;
  if (numCompleted_==flow_.getOperators().size())
    cv_.notify_one();
}

//set isShutDownRequested as true
void FlowContext::requestShutdown()
{
  isShutdownRequested_.store(true);
}

//return isShutDownRequested value
bool FlowContext::isShutdownRequested()
{
  return isShutdownRequested_.load();
}
