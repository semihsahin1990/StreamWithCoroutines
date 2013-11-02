#pragma once

#include "streamc/Tuple.h"

namespace streamc
{

class OutputPort
{
public:
  virtual void pushTuple(Tuple const & tuple) = 0;
};

}
