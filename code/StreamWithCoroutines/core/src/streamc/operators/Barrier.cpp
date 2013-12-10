#include "streamc/operators/Barrier.h"

using namespace streamc;
using namespace streamc::operators;
using namespace std;

Barrier::Barrier(std::string const & name)
  : Operator(name, 2, 1)
{}

Barrier::Barrier(std::string const & name, int numInputs)
  : Operator(name, numInputs, 1)
{}

void Barrier::process(OperatorContext & context)
{
  Tuple resultTuple;
  
  size_t nInputs = getNumberOfInputPorts();
  unordered_map<InputPort*, size_t> waitSpec;
  for (size_t i=0; i<nInputs; ++i)
      waitSpec[&context.getInputPort(i)] = 1;
  
  OutputPort & oport = context.getOutputPort(0);

  while(!context.isShutdownRequested()) {
    bool closed = context.waitOnAllPorts(waitSpec);
    if(closed)
      break;
    
    for (size_t i=0; i<nInputs; ++i) 
      resultTuple.append(context.getInputPort(i).getFrontTuple());

    oport.pushTuple(resultTuple);

    for (size_t i=0; i<nInputs; ++i) 
      context.getInputPort(i).popTuple();
  }
}
