#pragma once

#include <inttypes.h>

namespace streamc
{

class Tuple;

class OutputPort
{
public:
  //destructor
  virtual ~OutputPort() {}
  //push tuple to the input port of each subscriber
  virtual void pushTuple(Tuple const & tuple) = 0;
};

} // namespace streamc
