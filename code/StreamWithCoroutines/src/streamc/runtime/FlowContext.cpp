#include "streamc/runtime/FlowContext.h"

#include "streamc/Flow.h"

#include "streamc/Operator.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
#include "streamc/runtime/OperatorContextImpl.h"

using namespace std;
using namespace streamc;

FlowContext::FlowContext(Flow & flow)
  : flow_(flow), numCompleted_(0), isShutdownRequested_(false)
{}

FlowContext::~FlowContext()
{}

void FlowContext::run(int numThreads)
{
  vector<Operator *> const & opers = flow_.getOperators();
  for (Operator * oper : opers) {
    uintptr_t opAddr = reinterpret_cast<uintptr_t>(oper);
    OperatorContextImpl * operatorContext = new OperatorContextImpl(this, oper);
    // add the operator context
    operatorContexts_[opAddr] = unique_ptr<OperatorContextImpl>(operatorContext);
  }
  for (Operator * oper : opers) {
    uintptr_t opAddr = reinterpret_cast<uintptr_t>(oper);
    OperatorContextImpl * operatorContext = operatorContexts_[opAddr].get();
    // add input ports
    for (size_t i=0; i<oper->getNumberOfInputPorts(); ++i) {
      InputPortImpl * port = new InputPortImpl();
      vector<InConnection> const & conns = flow_.getInConnections(*oper, i);
      for (InConnection const & conn : conns) {
        uintptr_t oopAddr = reinterpret_cast<uintptr_t>(&conn.getOperator());
        port->addPublisher(operatorContexts_[oopAddr].get());
      }
      operatorContext->addInputPort(port);
    }
    // add output ports
    for (size_t i=0; i<oper->getNumberOfOutputPorts(); ++i) {
      OutputPortImpl * port = new OutputPortImpl();
      vector<OutConnection> const & conns = flow_.getOutConnections(*oper, i);
      for (OutConnection const & conn : conns)  {
        uintptr_t oopAddr = reinterpret_cast<uintptr_t>(&conn.getOperator());
        port->addSubscriber(operatorContexts_[oopAddr].get(), conn.getInputPort());
      }
      operatorContext->addOutputPort(port);
    }
  }
  for (Operator * oper : opers) {
    uintptr_t opAddr = reinterpret_cast<uintptr_t>(oper);
    OperatorContextImpl * operatorContext = operatorContexts_[opAddr].get();
    operatorContext->initOper();
  }
  // TODO: we need to hookup to the scheduler to perform the computation
}

void FlowContext::wait()
{
  unique_lock<mutex> lock(mutex_);
  while(numCompleted_ < flow_.getOperators().size())
    cv_.wait(lock);
}

void FlowContext::markOperatorCompleted(Operator * oper)
{
  unique_lock<mutex> lock(mutex_);
  numCompleted_++;
  if (numCompleted_==flow_.getOperators().size())
    cv_.notify_one();
}

void FlowContext::requestShutdown()
{
  isShutdownRequested_.store(true);
}

bool FlowContext::isShutdownRequested()
{
  return isShutdownRequested_.load();
}
