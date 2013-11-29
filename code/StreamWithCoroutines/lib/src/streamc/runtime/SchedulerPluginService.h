#pragma once

#include "streamc/runtime/HashHelpers.h"
#include "streamc/runtime/SchedulerState.h"

#include <unordered_set>
#include <unordered_map>

namespace streamc
{

class SchedulerPluginService
{
public:
  // get the threads of the flow
  virtual std::unordered_map<WorkerThread *, ThreadInfo *> const & getThreads() const = 0;
  // get the opertors of the flow
  virtual std::unordered_map<OperatorContextImpl *, OperatorInfo *> const & getOperators() const = 0;
  // get the threads that are currently in the ready state
  virtual std::unordered_set<WorkerThread *> const & getReadyThreads() const = 0;
  // get the threads that are currently in the ready state
  virtual std::unordered_set<OperatorContextImpl *> const & getReadyOperators() const = 0;
};

} /* namespace streamc */
