#include "streamc/runtime/OperatorContextImpl.h"

#include "streamc/Operator.h"
#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"

using namespace std;
using namespace streamc;

//constructor with flowContext and operator
OperatorContextImpl::OperatorContextImpl(FlowContext * flowContext, Operator * oper)
  : flowContext_(flowContext), oper_(oper), isComplete_(false)
{}

//destructor
OperatorContextImpl::~OperatorContextImpl()
{}

//returns whether shutdown is requested or not
bool OperatorContextImpl::isShutdownRequested()
{
  return flowContext_->isShutdownRequested();
}

//init operator with this operator context
void OperatorContextImpl::initOper() 
{
  oper_->init(*this);
}

//run the operator. when it returns, set isComplete_ as true and mark operator as completed
void OperatorContextImpl::runOper() 
{
  oper_->process(*this);
  isComplete_.store(true);
  flowContext_->markOperatorCompleted(oper_);
}

//add inputPort to the context
void OperatorContextImpl::addInputPort(InputPortImpl * port)
{
  inputs_.push_back(std::unique_ptr<InputPortImpl>(port));
}

//add outputPort to the context
void OperatorContextImpl::addOutputPort(OutputPortImpl * port)
{
  outputs_.push_back(std::unique_ptr<OutputPortImpl>(port));
}

InputPort & OperatorContextImpl::getInputPort(size_t inputPort) { 
  return getInputPortImpl(inputPort); 
}

OutputPort & OperatorContextImpl::getOutputPort(size_t outputPort) { 
  return getOutputPortImpl(outputPort); 
}
