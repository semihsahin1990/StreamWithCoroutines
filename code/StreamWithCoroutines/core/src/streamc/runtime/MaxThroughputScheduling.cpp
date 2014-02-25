#include "streamc/runtime/MaxThroughputScheduling.h"

#include "streamc/runtime/SchedulerPluginService.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
#include "streamc/runtime/FlowContext.h"

using namespace std;
using namespace streamc;
#include <iostream>
#include <unordered_set>
#include <utility>

MaxThroughputScheduling::MaxThroughputScheduling(uint64_t epochMicrosecs/*=1000*/) 
  : epochMicrosecs_(epochMicrosecs) 
{
  constexpr double clockPeriodInMicrosec = 
    (1000 * 1000 * (double) chrono::high_resolution_clock::period::num) 
        / chrono::high_resolution_clock::period::den;
  static_assert(clockPeriodInMicrosec <= 1.0, "The system's highest resolution clock has a period greater than 1 microsecond");
  randgen_.seed(mt19937_64::default_seed);
}

OperatorContextImpl * MaxThroughputScheduling::
    findOperatorToExecute(SchedulerPluginService & service,
                          WorkerThread & thread) 
{ 
  unordered_set<OperatorContextImpl *> const & opers = service.getReadyOperators();

  if (opers.size()==0)
    return nullptr;

  auto it = opers.begin();
  OperatorContextImpl *selected = *it;
  double maxRunningTime = estimateRunningTime(selected);

  for(++it; it!=opers.end(); it++) {
    double runningTime = estimateRunningTime(*it);
    if(runningTime > maxRunningTime)
      maxRunningTime = runningTime;
  } 
  return selected;
}

double MaxThroughputScheduling::estimateRunningTime(OperatorContextImpl *oper) {
  double min = -1;
  OperatorInfo & oinfo = oper->getOperatorInfo();
  
  // input ports
  size_t numberOfInputPorts = oper->getNumberOfInputPorts();

  if(oinfo.isConjuntive()) {
    for(size_t i=0; i<numberOfInputPorts; i++) {
      InputPortImpl & iportImpl = oper->getInputPortImpl(i);
      double profile = oinfo.getIPortProfile(iportImpl);
      double runningTime = iportImpl.getTupleCount() / profile;

      if(min==-1 || runningTime<min)
        min = runningTime;
    }
  }
  else {
    double max = -1;
    for(size_t i=0; i<numberOfInputPorts; i++) {
      InputPortImpl & iportImpl = oper->getInputPortImpl(i);
      double profile = oinfo.getIPortProfile(iportImpl);
      double runningTime = iportImpl.getTupleCount() / profile;

      if(max==-1 || runningTime>max)
        max = runningTime;
    }
    min = max;
  }

  // output ports
  size_t numberOfOutputPorts = oper->getNumberOfOutputPorts();
  size_t capacity = FlowContext::getMaxQueueSize();
  //size_t capacity = 5;
  for(size_t i=0; i<numberOfOutputPorts; i++) {
    OutputPortImpl & oportImpl = oper->getOutputPortImpl(i);
      double profile = oinfo.getOPortProfile(oportImpl);
      pair<OperatorContextImpl *, size_t> subscriber = oportImpl.getSubscriber(0);
      InputPortImpl & iportImpl = (subscriber.first) -> getInputPortImpl(subscriber.second);

      double runningTime = (capacity-iportImpl.getTupleCount()) / profile;

      if(min==-1 || runningTime<min)
        min = runningTime;
  }

  return min;
}

bool MaxThroughputScheduling::
    checkOperatorForPreemption(SchedulerPluginService & service,
                               OperatorContextImpl & oper) 
{
  using namespace std::chrono;
  
  OperatorInfo & opinfo = *(service.getOperators().find(&oper)->second);
  high_resolution_clock::time_point lastTime = opinfo.getBeginTime();
  high_resolution_clock::time_point currentTime = high_resolution_clock::now();
  high_resolution_clock::duration timeDiff = currentTime - lastTime;
  std::chrono::microseconds timeDiffInMicrosecs = duration_cast<std::chrono::microseconds>(timeDiff); 
  return (timeDiffInMicrosecs.count() >= epochMicrosecs_);
}
