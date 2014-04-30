#include "streamc/operators/Busy.h"
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
  InputPort & iport = context.getInputPort(0);
  OutputPort & oport = context.getOutputPort(0);

  while (!context.isShutdownRequested()) {
    bool closed = iport.waitTuple();
    if (closed)
      break;
    Tuple & tuple = iport.getFrontTuple();
    high_resolution_clock::time_point beginTime = high_resolution_clock::now();
    while(true){
      high_resolution_clock::time_point currentTime = high_resolution_clock::now();
      high_resolution_clock::duration timeDiff = currentTime - beginTime;
      std::chrono::microseconds timeDiffInMicrosecs = duration_cast<std::chrono::microseconds>(timeDiff); 
      if(timeDiffInMicrosecs.count()>=busyTimeMicrosecs_)
        break;
    }
    oport.pushTuple(tuple);
    iport.popTuple();
  }
}

Busy * Busy::clone() 
{
  return new Busy(getName(), busyTimeMicrosecs_);
}

