#include "streamc/runtime/OperatorContextImpl.h"

#include "streamc/Operator.h"
#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"

using namespace std;
using namespace streamc;

OperatorContextImpl::OperatorContextImpl(FlowContext * flowContext, Operator * oper)
  : flowContext_(flowContext), oper_(oper), isComplete_(false)
{}

OperatorContextImpl::~OperatorContextImpl()
{}

bool OperatorContextImpl::isShutdownRequested()
{
  return flowContext_->isShutdownRequested();
}

void OperatorContextImpl::initOper() 
{
  oper_->init(*this);
}

void OperatorContextImpl::runOper() 
{
  oper_->process(*this);
  isComplete_.store(true);
  flowContext_->markOperatorCompleted(oper_);
}

InputPort & OperatorContextImpl::getInputPort(size_t inputPort) 
{
  return *inputs_[inputPort];
}

OutputPort & OperatorContextImpl::getOutputPort(size_t outputPort) 
{
  return *outputs_[outputPort];
}

void OperatorContextImpl::addInputPort(InputPortImpl * port)
{
  inputs_.push_back(std::unique_ptr<InputPortImpl>(port));
}

void OperatorContextImpl::addOutputPort(OutputPortImpl * port)
{
  outputs_.push_back(std::unique_ptr<OutputPortImpl>(port));
}

InputPortImpl & OperatorContextImpl::getInputPortImpl(size_t inputPort) 
{
  return *inputs_[inputPort];
}

OutputPortImpl & OperatorContextImpl::getOutputPortImpl(size_t outputPort) 
{
  return *outputs_[outputPort];
}  

bool OperatorContextImpl::isComplete() const
{
  return isComplete_.load();
}
