#pragma once

#include "streamc/Tuple.h"
#include "streamc/OutputPort.h"

#include <mutex>

namespace streamc
{

class OperatorContextImpl;
class FlowContext;
class Scheduler;

class OutputPortImpl : public OutputPort
{  
public:
  OutputPortImpl(OperatorContextImpl & oper, FlowContext & flowContext, Scheduler & scheduler);

  size_t getNumberOfSubscribers();
  void addSubscriber(OperatorContextImpl & oper, size_t inPort); 
  std::pair<OperatorContextImpl *, size_t> getSubscriber(size_t index);

  // implemented interface
  void pushTuple(Tuple const & tuple) override;

private:
  OperatorContextImpl * oper_;
  FlowContext * flowContext_;
  Scheduler * scheduler_;
  std::vector<std::pair<OperatorContextImpl *, size_t>> subscribers_;
  std::mutex mutex_; 
};

} // namespace streamc
