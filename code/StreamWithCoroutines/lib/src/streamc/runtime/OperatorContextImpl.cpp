#include "streamc/runtime/OperatorContextImpl.h"

#include "streamc/Operator.h"
#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"

#include <functional>

using namespace std;
using namespace streamc;

//constructor with flowContext and operator
OperatorContextImpl::OperatorContextImpl(FlowContext * flowContext, Operator * oper)
  : flowContext_(flowContext), oper_(oper), isComplete_(false), stateStore_(new Tuple())
{}

//destructor
OperatorContextImpl::~OperatorContextImpl()
{}

void OperatorContextImpl::init()
{
  isComplete_.store(false);
  coroStarted_ = false;
}

//returns whether shutdown is requested or not
bool OperatorContextImpl::isShutdownRequested()
{
  return flowContext_->isShutdownRequested();
}

// run the operator. when it returns, set isComplete_ as true and mark operator
// as completed.
void OperatorContextImpl::coroBody(coro_t::caller_type & caller)
{
  coroCaller_ = & caller;
  oper_->process(*this);
  for (auto & iportPtr : inputs_) 
    iportPtr->drain();
  isComplete_.store(true);
  flowContext_->markOperatorCompleted(oper_);  
}

void OperatorContextImpl::initOper() 
{
  oper_->initState(*this);
}

void OperatorContextImpl::saveOper() 
{
  oper_->saveState(*this);
}

Tuple & OperatorContextImpl::getStateStore() 
{
  return *stateStore_;
}

void OperatorContextImpl::runOper() 
{
  unique_lock<mutex> lock(mutex_);  
  using placeholders::_1;
  if (!coroStarted_) {
    coroStarted_ = true;
    coroCallee_ = coro_t(bind(&OperatorContextImpl::coroBody, this, _1));    
  } else {
    coroCallee_();
  }
}

void OperatorContextImpl::yieldOper()
{
  (*coroCaller_)();
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
