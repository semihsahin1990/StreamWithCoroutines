#pragma once

#include <cstddef>

namespace streamc
{

class InputPort;
class OutputPort;

class OperatorContext
{
public:
  virtual ~OperatorContext() {}
  virtual InputPort & getInputPort(size_t inputPort) = 0;
  virtual OutputPort & getOutputPort(size_t outputPort) = 0;
  virtual bool isShutdownRequested() = 0;
};

} // namespace streamc
