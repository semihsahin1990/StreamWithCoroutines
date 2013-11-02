#pragma once

#include "streamc/InputPort.h"
#include "streamc/OutputPort.h"

namespace streamc
{

class OperatorContext
{
  virtual InputPort & getInputPort(size_t inputPort) = 0;
  virtual OutputPort & getOutputPort(size_t outputPort) = 0;
  virtual bool isShutdownRequested() = 0;
};

}
