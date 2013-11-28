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
  virtual std::unordered_map<WorkerThread *, ThreadInfo *> const & getThreads() const = 0;
  virtual std::unordered_map<OperatorContextImpl *, OperatorInfo *> const & getOperators() const = 0;
  virtual std::unordered_set<WorkerThread *> const & getWaitingThreads() const = 0;
  virtual std::unordered_set<WorkerThread *> const & getReadyThreads() const = 0;
  virtual std::unordered_set<OperatorContextImpl *> const & getReadyOperators() const = 0;
  virtual std::unordered_map<InputPortImpl *, OperatorContextImpl *> const & getWaitingOperators() const = 0;
};

} /* namespace streamc */
