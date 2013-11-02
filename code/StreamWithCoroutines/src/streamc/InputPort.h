#pragma once

#include <inttypes.h>

namespace streamc
{

class Tuple;

class InputPort
{
public:	
  virtual ~InputPort() {}
  virtual bool isComplete() = 0;
  virtual bool hasTuple() = 0;
  virtual size_t getTupleCount() = 0;
  virtual bool waitTuple() = 0;

  virtual Tuple & getFrontTuple() = 0;
  virtual Tuple & getTupleAt(size_t index) = 0;
  virtual void popTuple() = 0; 
};

} // namespace streamc

