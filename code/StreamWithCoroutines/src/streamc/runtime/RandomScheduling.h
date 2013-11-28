#pragma once

namespace streamc
{

class RandomScheduling : public SchedulerPlugin
{
public:
  // return the operator to execute or nullptr if there is no operator to execute
  OperatorContextImpl * getOperatorToExecute(SchedulerPluginService & service,
                                             WorkerThread & thread) 
  { 
    // TODO
    return nullptr;
  }
  // TODO
};

} /* namespace streamc */


