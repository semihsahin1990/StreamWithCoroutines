#pragma once

#include "streamc/OperatorContext.h"

#include <memory>
#include <atomic>
#include <vector>

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
  void addInputPort(InputPortImpl * port);
  void addOutputPort(OutputPortImpl * port);
  void initOper();
  void runOper();
  InputPortImpl & getInputPortImpl(size_t inputPort);
  OutputPortImpl & getOutputPortImpl(size_t outputPort);
  bool isComplete() const;
  // interface to be implemented
  InputPort & getInputPort(size_t inputPort);
  OutputPort & getOutputPort(size_t outputPort);
  bool isShutdownRequested();
private:
  FlowContext * flowContext_;
  Operator * oper_;
  std::atomic<bool> isComplete_;
  std::vector<std::unique_ptr<InputPortImpl>> inputs_;
  std::vector<std::unique_ptr<OutputPortImpl>> outputs_;
};

} // namespace streamc
