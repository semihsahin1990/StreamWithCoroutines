#pragma once

#include "streamc/LogLevel.h"

namespace streamc
{

class Flow;

class FlowRunner
{
public:	
  static FlowRunner & createRunner();

  //runs flow with numThreads threads
  virtual void run(Flow & flow, int numThreads) = 0;
  
  //pauses the main thread of flow
  virtual void wait(Flow & flow) = 0;

  //shutdowns the flow
  virtual void requestShutdown(Flow & flow) = 0;

  // set the log level of the runtime infrastructure
  virtual void setInfrastructureLogLevel(LogLevel level) = 0;

  // set the log level of the application
  virtual void setApplicationLogLevel(LogLevel level) = 0;
};

} // namespace streamc
