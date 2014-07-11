#include "streamc/operators/ResultCollector.h"

#include <iostream>
#include <fstream>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace streamc;
using namespace streamc::operators;

ResultCollector::ResultCollector(std::string const & name, std::string const & fileName)
  : Operator(name, 1, 1), fileName_(fileName), tupleCounter_(0), minLatency_(-1), maxLatency_(0), mean_(0), deviation_(0)
{ }

void ResultCollector::process(OperatorContext & context)
{
  double variance = 0;
  
  InputPort & iport = context.getInputPort(0);
  OutputPort & oport = context.getOutputPort(0);

  high_resolution_clock::time_point firstTupleTime;
  high_resolution_clock::time_point lastTupletime;
  
  while (!context.isShutdownRequested()) {
    bool closed = iport.waitTuple();
    if (closed)
      break;
    Tuple & tuple = iport.getFrontTuple();
    high_resolution_clock::time_point currentTime = high_resolution_clock::now();
    
    if(tupleCounter_ == 0)
      firstTupleTime = currentTime;
    lastTupletime = currentTime;

    high_resolution_clock::time_point timestamp = tuple.getTimestampAttribute("Timestamp");
    high_resolution_clock::duration timeDiff = currentTime - timestamp;
    std::chrono::microseconds timeDiffInMicrosecs = duration_cast<std::chrono::microseconds>(timeDiff);
    uint64_t latency = timeDiffInMicrosecs.count();

    tupleCounter_++;

    if(minLatency_==-1 || latency<minLatency_)
      minLatency_ = latency;
    if(latency>maxLatency_)
      maxLatency_ = latency;

    double oldMean = mean_;
    double oldVariance = variance;

    mean_ = (oldMean * (tupleCounter_-1) + latency) / tupleCounter_;
    variance = ((latency-mean_)*(latency-mean_) + (tupleCounter_-1)*(mean_-oldMean)*(mean_-oldMean) + (tupleCounter_-1)*oldVariance) / tupleCounter_; 

    oport.pushTuple(tuple);
    iport.popTuple();
  }

  deviation_ = sqrt(variance);

  ofstream output;
  output.open(fileName_.c_str());
  
  output<<"counter:\t"<<tupleCounter_<<endl;
  output<<"firstTupleTime:\t"<<duration_cast<std::chrono::milliseconds>(firstTupleTime.time_since_epoch()).count()<<endl;
  output<<"lastTupletime:\t"<<duration_cast<std::chrono::milliseconds>(lastTupletime.time_since_epoch()).count()<<endl;
  output<<"minLatency:\t"<<minLatency_<<endl;
  output<<"maxLatency:\t"<<maxLatency_<<endl;
  output<<"mean:\t"<<mean_<<endl;
  output<<"deviation:\t"<<deviation_<<endl;

  output.close();
}
