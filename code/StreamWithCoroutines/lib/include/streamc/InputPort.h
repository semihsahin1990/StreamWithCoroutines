#pragma once

#include <inttypes.h>

namespace streamc
{

class Tuple;

class InputPort
{
public:	
  //destructor
  virtual ~InputPort() {}
  //returns whether port is complete or not
  virtual bool isComplete() = 0;
  //returns whether port has tuple or not
  virtual bool hasTuple() = 0;
  //returns the size of port
  virtual size_t getTupleCount() = 0;
  //returns true if port is complete, returns false if port has tuple
  virtual bool waitTuple() = 0;
  
  //returns next tuple in the port
  virtual Tuple & getFrontTuple() = 0;
  //returns index-th tuple in the port
  virtual Tuple & getTupleAt(size_t index) = 0;
  //removes next tuple in the port
  virtual void popTuple() = 0; 
};

} // namespace streamc

