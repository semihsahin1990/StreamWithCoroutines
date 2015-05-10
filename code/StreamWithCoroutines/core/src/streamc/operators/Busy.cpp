#include "streamc/operators/Busy.h"
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;
using namespace streamc;
using namespace streamc::operators;

Busy::Busy(std::string const & name, uint64_t busyTimeMicrosecs, double selectivity)
  : Operator(name, 1, 1), busyTimeMicrosecs_(busyTimeMicrosecs), selectivity_(selectivity)
{}

Busy::Busy(std::string const & name)
  : Busy(name, 100, 1)
{}

void Busy::process(OperatorContext & context)
{
  InputPort & iport = context.getInputPort(0);
  OutputPort & oport = context.getOutputPort(0);

  mt19937_64 randgen_;
  randgen_.seed(mt19937_64::default_seed);
  std::uniform_real_distribution<double> zero_one(0.0, 1.0);

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
    if(zero_one(randgen_) < selectivity_)
      oport.pushTuple(tuple);
    iport.popTuple();
  }
}

Busy * Busy::clone(std::string const & name) 
{
  return new Busy(name, busyTimeMicrosecs_, selectivity_);
}