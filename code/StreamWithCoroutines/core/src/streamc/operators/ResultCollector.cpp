#include "streamc/operators/ResultCollector.h"
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace streamc;
using namespace streamc::operators;

ResultCollector::ResultCollector(std::string const & name)
  : Operator(name, 1, 1), tupleCounter_(0), totalLatency_(0)
{}

void ResultCollector::process(OperatorContext & context)
{
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

    totalLatency_ = totalLatency_ + timeDiffInMicrosecs.count();
    maxLatency_ = timeDiffInMicrosecs.count();
    tupleCounter_++;

    oport.pushTuple(tuple);
    iport.popTuple();
  }
}
