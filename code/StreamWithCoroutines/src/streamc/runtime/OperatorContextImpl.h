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

  //returns the inputPortImpl with port no = inputPort
  InputPortImpl & getInputPortImpl(size_t inputPort);
  
  //returns the outputPortImpl with port no = outputPort
  OutputPortImpl & getOutputPortImpl(size_t outputPort);
  bool isComplete() const;

  //interface to be implemented
  //returns the inputPort with port no = inputPort
  InputPort & getInputPort(size_t inputPort);
  //returns the outputPort with port no = outputPort
  OutputPort & getOutputPort(size_t outputPort);
  //returns whether shutdown is requested or not
  bool isShutdownRequested();

private:
  FlowContext * flowContext_;
  Operator * oper_;
  std::atomic<bool> isComplete_;
  std::vector<std::unique_ptr<InputPortImpl>> inputs_;
  std::vector<std::unique_ptr<OutputPortImpl>> outputs_;
};

} // namespace streamc
