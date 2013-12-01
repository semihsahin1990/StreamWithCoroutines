#pragma once

// TODO: This class is work under progress

#include <vector>

namespace streamc
{

class InputPort;

class PortsImpl
{
  public:
  //implemented interface
  static bool waitTupleFromAll(std::vector<InputPort *> ports);
};

} //namespace streamc
