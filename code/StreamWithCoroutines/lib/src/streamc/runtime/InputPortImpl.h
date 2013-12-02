#pragma once 

#include "streamc/Tuple.h"
#include "streamc/InputPort.h"

#include <deque>

namespace streamc 
{

class OperatorContextImpl;
class Scheduler;

class InputPortImpl: public InputPort
{
public:
  InputPortImpl(OperatorContextImpl & oper, Scheduler & scheduler);
  void addPublisher(OperatorContextImpl & oper);
  void pushTuple(Tuple const & tuple);
  void drain();

  // implemented interface
  bool isClosed() override;
  bool hasTuple() override;
  size_t getTupleCount() override;  
  bool waitTuple() override;
  bool waitTuple(size_t n) override;
  Tuple & getFrontTuple() override;
  Tuple & getTupleAt(size_t index) override;
  void popTuple() override;

private:
  bool isClosedNoLock();

private:
  OperatorContextImpl * oper_;
  Scheduler * scheduler_;
  bool isClosed_;
  std::deque<Tuple> portQueue_;
  std::vector<OperatorContextImpl *> publishers_;
  std::mutex mutex_; 
};

} // namespace streamc
