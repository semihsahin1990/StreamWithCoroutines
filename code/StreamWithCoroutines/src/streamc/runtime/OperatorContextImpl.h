#pragma once

#include "streamc/OperatorContext.h"

#include <memory>
#include <mutex>
#include <atomic>
#include <vector>

#include <boost/coroutine/coroutine.hpp>

namespace streamc
{

class Operator;
class FlowContext;
class InputPortImpl;
class OutputPortImpl;

class OperatorContextImpl : public OperatorContext
{
public:
  OperatorContextImpl(FlowContext * flowContext, Operator * oper);
  ~OperatorContextImpl();
  Operator & getOperator() { return *oper_; }
  void addInputPort(InputPortImpl * port);
  void addOutputPort(OutputPortImpl * port);
  void initOper();
  void runOper();
  void yieldOper();

  bool isComplete() const { return isComplete_.load(); }
  size_t getNumberOfInputPorts() { return inputs_.size(); }
  size_t getNumberOfOutputPorts() { return outputs_.size(); }
  InputPortImpl & getInputPortImpl(size_t inputPort) { return *inputs_[inputPort]; }
  OutputPortImpl & getOutputPortImpl(size_t outputPort) { return *outputs_[outputPort]; }

  // interface to be implemented
  InputPort & getInputPort(size_t inputPort);
  OutputPort & getOutputPort(size_t outputPort);
  bool isShutdownRequested();

private:
  typedef boost::coroutines::coroutine<void()> coro_t;
  void coroBody(coro_t::caller_type & caller);
  bool coroStarted_;
  coro_t coroCallee_;
  coro_t::caller_type * coroCaller_;

private:
  FlowContext * flowContext_;
  Operator * oper_;
  std::atomic<bool> isComplete_;
  std::vector<std::unique_ptr<InputPortImpl>> inputs_;
  std::vector<std::unique_ptr<OutputPortImpl>> outputs_;
  std::mutex mutex_;
};

} // namespace streamc
