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
  //constructor with flowContext and operator
  OperatorContextImpl(FlowContext * flowContext, Operator * oper);
  //destructor
  ~OperatorContextImpl();
  //add inputPort
  void addInputPort(InputPortImpl * port);
  //add outputPort
  void addOutputPort(OutputPortImpl * port);
  //init operator
  void initOper();
  //run operator
  void runOper();

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
  FlowContext * flowContext_;
  Operator * oper_;
  std::atomic<bool> isComplete_;
  std::vector<std::unique_ptr<InputPortImpl>> inputs_;
  std::vector<std::unique_ptr<OutputPortImpl>> outputs_;
};

} // namespace streamc
