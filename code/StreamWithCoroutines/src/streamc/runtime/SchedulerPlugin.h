#pragma once

namespace streamc
{

class SchedulerPlugin
{
public:
  // return the operator to execute or nullptr if there is no operator to execute
  virtual OperatorContextImpl * getOperatorToExecute(SchedulerPluginService & service,
                                                     WorkerThread & thread) = 0;  
};

} /* namespace streamc */


