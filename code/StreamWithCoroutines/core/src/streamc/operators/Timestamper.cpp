#include "streamc/operators/Timestamper.h"
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace streamc;
using namespace streamc::operators;

Timestamper::Timestamper(std::string const & name)
  : Operator(name, 1, 1)
{}

void Timestamper::process(OperatorContext & context)
{
  InputPort & iport = context.getInputPort(0);
  OutputPort & oport = context.getOutputPort(0);

  while (!context.isShutdownRequested()) {
    bool closed = iport.waitTuple();
    if (closed)
      break;
    Tuple & tuple = iport.getFrontTuple();
    tuple.setAttribute("Timestamp", high_resolution_clock::now());
    oport.pushTuple(tuple);
    iport.popTuple();
  }
}

Timestamper * Timestamper::clone(std::string const & name)
{
  return new Timestamper(name);
}

