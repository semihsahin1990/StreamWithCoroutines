#include "streamc/operators/ResultCollector.h"
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace streamc;
using namespace streamc::operators;

ResultCollector::ResultCollector(std::string const & name)
  : Operator(name, 1, 1), tupleCounter_(0), minLatency_(-1), maxLatency_(0), mean_(0), deviation_(0)
{}

void ResultCollector::process(OperatorContext & context)
{
  double variance = 0;
  InputPort & iport = context.getInputPort(0);
  OutputPort & oport = context.getOutputPort(0);

  while (!context.isShutdownRequested()) {
    bool closed = iport.waitTuple();
    if (closed)
      break;
    Tuple & tuple = iport.getFrontTuple();

    high_resolution_clock::time_point currentTime = high_resolution_clock::now();
    high_resolution_clock::time_point timestamp = tuple.getTimestampAttribute("Timestamp");
    high_resolution_clock::duration timeDiff = currentTime - timestamp;
    std::chrono::microseconds timeDiffInMicrosecs = duration_cast<std::chrono::microseconds>(timeDiff);
    uint64_t latency = timeDiffInMicrosecs.count();

    tupleCounter_++;

    if(minLatency_==-1 || latency<minLatency_)
      minLatency_ = latency;
    if(latency>maxLatency_)
      maxLatency_ = timeDiffInMicrosecs.count();

    double oldMean = mean_;
    double oldVariance = variance;

    mean_ = (oldMean * (tupleCounter_-1) + latency) / tupleCounter_;
    variance = ((latency-mean_)*(latency-mean_) + (tupleCounter_-1)*(mean_-oldMean)*(mean_-oldMean) + (tupleCounter_-1)*oldVariance) / tupleCounter_; 

    oport.pushTuple(tuple);
    iport.popTuple();
  }

  deviation_ = sqrt(variance);
  cout<<"minLatency:\t"<<minLatency_<<endl;
  cout<<"maxLatency:\t"<<maxLatency_<<endl;
  cout<<"mean:\t\t"<<mean_<<endl;
  cout<<"deviation:\t"<<deviation_<<endl;
}
