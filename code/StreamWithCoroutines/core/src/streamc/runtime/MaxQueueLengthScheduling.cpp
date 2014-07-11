#include "streamc/runtime/MaxQueueLengthScheduling.h"

#include "streamc/runtime/SchedulerPluginService.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/Operator.h"

using namespace std;
using namespace streamc;
#include <iostream>
#include <unordered_set>

MaxQueueLengthScheduling::MaxQueueLengthScheduling(uint64_t epochMicrosecs/*=1000*/) 
  : epochMicrosecs_(epochMicrosecs) 
{
  constexpr double clockPeriodInMicrosec = 
    (1000 * 1000 * (double) chrono::high_resolution_clock::period::num) 
        / chrono::high_resolution_clock::period::den;
  static_assert(clockPeriodInMicrosec <= 1.0, "The system's highest resolution clock has a period greater than 1 microsecond");
  randgen_.seed(mt19937_64::default_seed);
}

OperatorContextImpl * MaxQueueLengthScheduling::
    findOperatorToExecute(SchedulerPluginService & service,
                          WorkerThread & thread) 
{ 
 
  unordered_set<OperatorContextImpl *> const & opers = service.getReadyOperators();

  if (opers.size()==0)
    return nullptr;

  OperatorContextImpl * selected = nullptr;
  int maxQueueLength = -1;

  for(auto it = opers.begin(); it!=opers.end(); ++it) {
    OperatorContextImpl *oper = *it;
    size_t numberOfInputPorts = oper->getNumberOfInputPorts();

    // check if it is source op
    if(numberOfInputPorts == 0) {
      selected = *it;
      break;
    }

    // if it is not source op, check front tuple of each input port
    for(size_t i=0; i<numberOfInputPorts; i++) {
      InputPortImpl & iportImpl = oper->getInputPortImpl(i);
      int tupleCount = iportImpl.getTupleCount();
      if(tupleCount > maxQueueLength) {
        maxQueueLength = tupleCount;
        selected = *it;
      }
    }
  }

  if(selected == nullptr)
    cout<<"wtf!"<<endl;
  return selected;
}

bool MaxQueueLengthScheduling::
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

