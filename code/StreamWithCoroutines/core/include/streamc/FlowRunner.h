#pragma once

#include "streamc/LogLevel.h"

namespace streamc
{

class Flow;
class SchedulerPlugin;
class Operator;

/**
 * %Flow runner that can be used to run flows.
 */
class FlowRunner
{
public:	
  /**
   * Create the global instance of the flow runner.
   *
   * @return the global instance of the flow runner
   */
  static FlowRunner & createRunner();

  /**
   * Run the given flow using the given number of threads.
   *
   * @param flow the flow to run
   * @param numThreads the number of threads to use
   * @param plugin optional scheduler plugin (owned by the runtime)
   */
  virtual void run(Flow & flow, int numThreads, SchedulerPlugin * plugin=nullptr) = 0;
  
  /**
   * Wait for the flow to complete.
   *
   * The execution will block until the flow completes.
   * @param flow the flow to wait for
   */
  virtual void wait(Flow & flow) = 0;

  virtual void addFission(Flow & flow, Operator * oper, size_t replica) = 0;

  /**
   * Request the flow to be shut down.
   *
   * This function will not block. After the request is sent, the
   * caller can use the @link FlowRunner::wait wait @endlink function
   * to wait for the completion of the flow. 
   * @param flow the flow to request to be shut down
   */
  virtual void requestShutdown(Flow & flow) = 0;

  /**
   * Set the infrastructure log level.
   *
   * @param level level of the infrastructure logs.
   */
  virtual void setInfrastructureLogLevel(LogLevel level) = 0;

  /**
   * Set the application log level.
   *
   * @param level level of the application logs.
   */
  virtual void setApplicationLogLevel(LogLevel level) = 0;
};

} // namespace streamc


