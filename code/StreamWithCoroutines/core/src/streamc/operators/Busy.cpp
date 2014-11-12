#include "streamc/operators/Busy.h"
#include <random>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace streamc;
using namespace streamc::operators;

Busy::Busy(std::string const & name, uint64_t busyTimeMicrosecs)
  : Operator(name, 1, 1), busyTimeMicrosecs_(busyTimeMicrosecs)
{}

Busy::Busy(std::string const & name)
  : Busy(name, 1000)
{}

void Busy::process(OperatorContext & context)
{
  mt19937_64 randgen;
  unsigned seed = system_clock::now().time_since_epoch().count();
  randgen.seed(seed*reinterpret_cast<uintptr_t>(&context));  
  uniform_real_distribution<> dist(0.5, 1.5);

  InputPort & iport = context.getInputPort(0);
  OutputPort & oport = context.getOutputPort(0);

  while (!context.isShutdownRequested()) {
    bool closed = iport.waitTuple();
    if (closed)
      break;
    Tuple & tuple = iport.getFrontTuple();
    double currentBusyTime = dist(randgen) * busyTimeMicrosecs_;
    high_resolution_clock::time_point beginTime = high_resolution_clock::now();
    while(true){
      high_resolution_clock::time_point currentTime = high_resolution_clock::now();
      high_resolution_clock::duration timeDiff = currentTime - beginTime;
      std::chrono::microseconds timeDiffInMicrosecs = duration_cast<std::chrono::microseconds>(timeDiff); 
      if(timeDiffInMicrosecs.count()>=currentBusyTime)
        break;
    }
    oport.pushTuple(tuple);
    iport.popTuple();
  }
}
