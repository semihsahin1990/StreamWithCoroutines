#pragma once

namespace streamc
{

class OperatorContextImpl;
class SchedulerPluginService;
class WorkerThread;

class SchedulerPlugin
{
public:
  // return the operator to execute or nullptr if there is no operator to execute
  virtual OperatorContextImpl * findOperatorToExecute(SchedulerPluginService & service,
                                                      WorkerThread & thread) = 0;  
  // return true of the operator is to be preempted
  virtual bool checkOperatorForPreemption(SchedulerPluginService & service,
                                          OperatorContextImpl & oper) = 0;
};

} /* namespace streamc */


