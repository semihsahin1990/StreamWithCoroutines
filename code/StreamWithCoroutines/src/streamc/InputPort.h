#pragma once

#include "streamc/Tuple.h"

namespace streamc
{

class InputPort
{
public:	
  virtual bool isComplete() = 0;
  virtual bool hasTuple() = 0;
  virtual size_t getTupleCount() = 0;
  virtual bool waitTuple() = 0;

  virtual Tuple const & getFrontTuple() = 0;
  virtual Tuple const & getTupleAt(size_t index) = 0;
  virtual void popTuple() = 0; 
};

}

