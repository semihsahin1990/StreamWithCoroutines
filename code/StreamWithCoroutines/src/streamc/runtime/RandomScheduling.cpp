#include "streamc/runtime/RandomScheduling.h"

#include "streamc/runtime/SchedulerPluginService.h"

using namespace std;
using namespace streamc;

#include <unordered_set>

RandomScheduling::RandomScheduling(uint64_t epochMicrosecs/*=1000*/) 
  : epochMicrosecs_(epochMicrosecs) 
{
  constexpr double clockPeriodInMicrosec = 
    (1000 * 1000 * (double) chrono::high_resolution_clock::period::num) 
        / chrono::high_resolution_clock::period::den;
  static_assert(clockPeriodInMicrosec <= 1.0, "The system's highest resolution clock has a period greater than 1 microsecond");
  randgen_.seed(mt19937_64::default_seed);
}

OperatorContextImpl * RandomScheduling::
    findOperatorToExecute(SchedulerPluginService & service,
                          WorkerThread & thread) 
{ 
  unordered_set<OperatorContextImpl *> const & opers = service.getReadyOperators();
  if (opers.size()==0)
    return nullptr;
  size_t l = randgen_() % opers.size(); 
  auto it = opers.begin();
  for (size_t i=0; i<l; ++i, it++);
  return *it;
}

bool RandomScheduling::
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
