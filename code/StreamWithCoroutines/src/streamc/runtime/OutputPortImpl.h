#pragma once

#include "streamc/Tuple.h"
#include "streamc/OutputPort.h"

namespace streamc
{

class OperatorContextImpl;

class OutputPortImpl : public OutputPort
{  
public:
  OutputPortImpl();
  void addSubscriber(OperatorContextImpl * oper, size_t inPort);
  void pushTuple(Tuple const & tuple);
private:
  std::vector<std::pair<OperatorContextImpl *, size_t>> subscribers_;
};

} // namespace streamc
