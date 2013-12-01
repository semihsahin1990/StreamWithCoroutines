#pragma once

#include "streamc/Ports.h"

#include <vector>

namespace streamc
{

class PortsImpl: public Ports
{
  public:
  //implemented interface
  static bool waitTupleFromAll(std::vector<InputPort *> ports);
};

} //namespace streamc
