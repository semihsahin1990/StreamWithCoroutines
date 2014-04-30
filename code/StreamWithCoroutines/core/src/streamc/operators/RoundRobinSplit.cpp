#include "streamc/operators/RoundRobinSplit.h"

using namespace streamc;
using namespace streamc::operators;
using namespace std;

RoundRobinSplit::RoundRobinSplit(std::string const & name)
  : Operator(name, 1, 2)
{}

RoundRobinSplit::RoundRobinSplit(std::string const & name, int numOutputs)
  : Operator(name, 1, numOutputs)
{}

void RoundRobinSplit::process(OperatorContext & context)
{
  InputPort & iport = context.getInputPort(0);
  size_t nOutputs = getNumberOfOutputPorts();
  size_t oPortNo = 0;

  while (!context.isShutdownRequested()) {
    bool closed = iport.waitTuple();
    if(closed)
      break;

    Tuple & tuple = iport.getFrontTuple();
    context.getOutputPort(oPortNo).pushTuple(tuple);
    iport.popTuple();
    oPortNo = (oPortNo+1) % nOutputs;
  }
}

RoundRobinSplit * RoundRobinSplit::clone()
{
  return new RoundRobinSplit(getName(), getNumberOfOutputPorts());
}
