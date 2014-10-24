#include "streamc/operators/RoundRobinMerge.h"
#include "streamc/Logger.h"
#include <queue>
using namespace streamc;
using namespace streamc::operators;
using namespace std;

RoundRobinMerge::RoundRobinMerge(std::string const & name)
  : Operator(name, 1, 1)
{}

RoundRobinMerge::RoundRobinMerge(std::string const & name, int numInputs)
  : Operator(name, numInputs, 1)
{}

void RoundRobinMerge::process(OperatorContext & context)
{
  size_t iPortNo = 0;
  size_t nInputs = getNumberOfInputPorts();
  OutputPort & oport = context.getOutputPort(0);

  while(!context.isShutdownRequested()) {
    bool closed = context.getInputPort(iPortNo).waitTuple();
    if(closed)
      break;
    
    Tuple & tuple = context.getInputPort(iPortNo).getFrontTuple();
    oport.pushTuple(tuple);
    context.getInputPort(iPortNo).popTuple();
    iPortNo = (iPortNo+1) % nInputs;
  }
}

RoundRobinMerge * RoundRobinMerge::clone(std::string const & name)
{
  return new RoundRobinMerge(name, getNumberOfInputPorts());
}
