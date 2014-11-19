#pragma once 

#include "streamc/Tuple.h"
#include "streamc/InputPort.h"
#include "streamc/runtime/SpinLock.h"

#include <deque>

namespace streamc 
{

class OperatorContextImpl;
class Scheduler;

class InputPortImpl: public InputPort
{
public:
  InputPortImpl(OperatorContextImpl & oper, Scheduler & scheduler);
  size_t getNumberOfPublishers();
  void addPublisher(OperatorContextImpl & oper, size_t outPort);
  std::pair<OperatorContextImpl *, size_t> getPublisher(size_t index);
  void pushTuple(Tuple const & tuple);
  std::chrono::high_resolution_clock::time_point & getFrontTimestamp();
  void drain();
  OperatorContextImpl & getOperatorContextImpl() { return *oper_; }

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
  typedef std::chrono::high_resolution_clock::time_point Timestamp;
  std::deque<std::pair<Tuple, Timestamp>> portQueue_;
  std::vector<std::pair<OperatorContextImpl *, size_t>> publishers_;
  SpinLock spinlock_; 
};

} // namespace streamc
