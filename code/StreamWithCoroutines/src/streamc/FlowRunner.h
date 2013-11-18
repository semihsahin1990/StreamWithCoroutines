#pragma once

namespace streamc
{

class Flow;

class FlowRunner
{
public:	
  //static FlowRunner creater
  static FlowRunner & createRunner();

  //runs flow with numThreads threads
  virtual void run(Flow & flow, int numThreads) = 0;
  
  //pauses the main thread of flow
  virtual void wait(Flow & flow) = 0;

  //shutdowns the flow
  virtual void requestShutdown(Flow & flow) = 0;
};

} // namespace streamc
