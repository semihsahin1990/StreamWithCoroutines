#pragma once

#include <inttypes.h>

namespace streamc
{

class Tuple;

class OutputPort
{
public:
  virtual ~OutputPort() {}
  virtual void pushTuple(Tuple const & tuple) = 0;
};

} // namespace streamc
