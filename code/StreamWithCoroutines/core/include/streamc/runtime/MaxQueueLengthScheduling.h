#pragma once

#include "streamc/runtime/SchedulerPlugin.h"

#include <chrono>
#include <random>
#include <queue>

namespace streamc
{

class MaxQueueLengthScheduling : public SchedulerPlugin
{
public:
  MaxQueueLengthScheduling(uint64_t epochMicrosecs=1000);

  // return the operator to execute or nullptr if there is no operator to execute
  OperatorContextImpl * findOperatorToExecute(SchedulerPluginService & service,
                                              WorkerThread & thread);
  // return true of the operator is to be preempted
  virtual bool checkOperatorForPreemption(SchedulerPluginService & service,
                                          OperatorContextImpl & oper);
private:
  uint64_t epochMicrosecs_;
  std::mt19937_64 randgen_;
};

} /* namespace streamc */