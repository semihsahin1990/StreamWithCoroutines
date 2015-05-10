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
  size_t getNumberOfPublishers();
  void addPublisher(OperatorContextImpl & oper, size_t outPort);
  void removePublisher(size_t index);
  void removePublisher(OperatorContextImpl & oper);
  std::pair<OperatorContextImpl *, size_t> getPublisher(size_t index);
  void pushTuple(Tuple const & tuple);
  std::chrono::high_resolution_clock::time_point & getFrontTimestamp();
  void drain();
  OperatorContextImpl & getOperatorContextImpl() { return *oper_; }
  void resetBeginTime();
  void markAsWriteBlocked();
  void unmarkAsWriteBlocked();
  double getWriteBlockedRatio();

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
  std::mutex mutex_; 

  bool inWriteBlockedState_;
  std::chrono::high_resolution_clock::time_point createdAt_;
  std::chrono::high_resolution_clock::time_point writeBlockedBeginTime_;
  std::chrono::microseconds totalWriteBlockedDuration_;
};

} // namespace streamc
