#pragma once

#include <cstddef>

namespace streamc
{

class InputPort;
class OutputPort;

class OperatorContext
{
public:
  //destructor
  virtual ~OperatorContext() {}
  //returns the InputPort with port no = inputPort
  virtual InputPort & getInputPort(size_t inputPort) = 0;
  //returns the OutputPort with port no = outputPort
  virtual OutputPort & getOutputPort(size_t outputPort) = 0;
  //returns whether shutdown is requested or not
  virtual bool isShutdownRequested() = 0;
};

} // namespace streamc
