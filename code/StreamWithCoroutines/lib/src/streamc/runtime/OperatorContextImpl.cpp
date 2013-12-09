#include "streamc/runtime/OperatorContextImpl.h"

#include "streamc/Operator.h"
#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
#include "streamc/runtime/Scheduler.h"

#include <functional>

using namespace std;
using namespace streamc;

//constructor with flowContext and operator
OperatorContextImpl::OperatorContextImpl(FlowContext * flowContext, Operator * oper, Scheduler & scheduler)
  : flowContext_(flowContext), oper_(oper), scheduler_(&scheduler), isComplete_(false), stateStore_(new Tuple())
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

// return true if will never be satisfied
bool OperatorContextImpl::waitOnAllPorts(unordered_map<InputPort *, size_t> const & spec)
{
  // TODO: Improve this code to use scheduler's multi-port wait capabilities
  // Look at InputPortImpl's waitTuple implementation to get the main idea

  unordered_map<InputPortImpl *, size_t> waitSpec;
  for(auto const & portCountPair : spec){
        waitSpec[static_cast<InputPortImpl *>(portCountPair.first)] = portCountPair.second;
  }

  bool needToWait = true;
  while(needToWait){
    {
      bool allAvailable = true;
      for(auto const & portCountPair : spec){
        bool portAvailable = portCountPair.first->getTupleCount() >= portCountPair.second;

        if(!portAvailable){
          allAvailable = false;
          break;
        }
      }

      if(allAvailable){
        needToWait = false;
      } else{
        for(auto const & portCountPair : spec){
          bool portClosed = static_cast<InputPortImpl *>(portCountPair.first)->isClosed();
          if(portClosed)
            return true;
        }
      }
    }

    if(needToWait){
      scheduler_->markOperatorAsReadBlocked(*this, waitSpec, true);
    } else {
      scheduler_->checkOperatorForPreemption(*this);
    }
  }

  return false;
}

bool OperatorContextImpl::waitOnAnyPort(unordered_map<InputPort *, size_t> const & spec)
{
  unordered_map<InputPortImpl *, size_t> waitSpec;
  for(auto const & portCountPair : spec){
        waitSpec[static_cast<InputPortImpl *>(portCountPair.first)] = portCountPair.second;
  }

  bool needToWait = true;
  while(needToWait){
    {
      bool oneAvailable = false;
      for(auto const & portCountPair : spec){
        bool portAvailable = portCountPair.first->getTupleCount() >= portCountPair.second;

        if(portAvailable){
          oneAvailable = true;
          break;
        }
      }

      if(oneAvailable){
        needToWait = false;
      } else{
        bool allClosed = true;
        for(auto const & portCountPair : spec){
            bool portClosed = static_cast<InputPortImpl *>(portCountPair.first)->isClosed();
            if(!portClosed){
              allClosed = false;
              break;
            }
        }

        if(allClosed)
          return true;
      }
    }

    if(needToWait){
      scheduler_->markOperatorAsReadBlocked(*this, waitSpec, true);
    } else {
      scheduler_->checkOperatorForPreemption(*this);
    }
  }

  return false;
}

