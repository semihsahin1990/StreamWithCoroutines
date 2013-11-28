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

  // public interface
  bool isComplete();
  bool hasTuple();
  size_t getTupleCount();  
  bool waitTuple();
  Tuple & getFrontTuple();
  Tuple & getTupleAt(size_t index);
  void popTuple();

private:
  bool isCompleteNoLock();

private:
  OperatorContextImpl * oper_;
  Scheduler * scheduler_;
  bool isComplete_;
  std::deque<Tuple> portQueue_;
  std::vector<OperatorContextImpl *> publishers_;
  std::mutex mutex_; 
};

} // namespace streamc
