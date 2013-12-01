#include "streamc/operators/Barrier.h"
#include "streamc/runtime/PortsImpl.h"

using namespace streamc;
using namespace std;

Barrier::Barrier(std::string const & name)
  : Operator(name, 2, 1)
{}

void Barrier::process(OperatorContext & context)
{
  Tuple resultTuple;
  InputPort & iport0 = context.getInputPort(0);
  InputPort & iport1 = context.getInputPort(1);
  OutputPort & oport = context.getOutputPort(0);

  while(!context.isShutdownRequested()) {
    /*
      TODO: will be replaced by 
      bool closed = context.waitOnPorts({iport0, 1}, {iport1, 1});
    */

    bool closed = iport0.waitTuple();   
    if(closed)
      break;
    closed = iport1.waitTuple();   
    if(closed)
      break;
    
    resultTuple.append(iport0.getFrontTuple());
    resultTuple.append(iport1.getFrontTuple());
    
    oport.pushTuple(resultTuple);

    iport0.popTuple();
    iport1.popTuple();
  }
}
