#pragma once

#include "streamc/Tuple.h"
#include "streamc/OutputPort.h"

namespace streamc
{

class OperatorContextImpl;

class OutputPortImpl : public OutputPort
{  
public:
  //constructor
  OutputPortImpl();
  //add (subscriber operator,portNo) pair to this port
  void addSubscriber(OperatorContextImpl & oper, size_t inPort);
  //push tuple to the input port of each subscriber
  void pushTuple(Tuple const & tuple);

private:
  std::vector<std::pair<OperatorContextImpl *, size_t>> subscribers_;
};

} // namespace streamc
