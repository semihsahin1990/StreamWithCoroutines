#pragma once

namespace streamc
{

class Flow;

class FlowRunner
{
public:	
  static FlowRunner & createRunner();
  virtual void run(Flow & flow, int numThreads) = 0;
  virtual void wait(Flow & flow) = 0;
  virtual void requestShutdown(Flow & flow) = 0;
};

} // namespace streamc
