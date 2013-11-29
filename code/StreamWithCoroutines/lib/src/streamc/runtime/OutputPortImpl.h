#pragma once

#include "streamc/Tuple.h"
#include "streamc/OutputPort.h"

#include <mutex>

namespace streamc
{

class OperatorContextImpl;
class Scheduler;

class OutputPortImpl : public OutputPort
{  
public:
  OutputPortImpl(OperatorContextImpl & oper, Scheduler & scheduler);
  // add (subscriber operator,portNo) pair to this port
  void addSubscriber(OperatorContextImpl & oper, size_t inPort); 

  size_t getNumberOfSubscribers();
  std::pair<OperatorContextImpl *, size_t> getSubscriber(size_t index);

  // push tuple to the input port of each subscriber
  void pushTuple(Tuple const & tuple);

private:
  OperatorContextImpl * oper_;
  Scheduler * scheduler_;
  std::vector<std::pair<OperatorContextImpl *, size_t>> subscribers_;
  std::mutex mutex_; 
};

} // namespace streamc
