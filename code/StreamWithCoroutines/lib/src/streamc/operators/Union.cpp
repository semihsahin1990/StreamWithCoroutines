#include "streamc/operators/Union.h"

using namespace streamc;
using namespace streamc::operators;
using namespace std;

Union::Union(std::string const & name)
  : Operator(name, 2, 1)
{}

Union::Union(std::string const & name, int numInputs)
  : Operator(name, numInputs, 1)
{}

void Union::process(OperatorContext & context)
{
  size_t nInputs = getNumberOfInputPorts();
  unordered_map<InputPort*, size_t> waitSpec;
  for (size_t i=0; i<nInputs; ++i)
      waitSpec[&context.getInputPort(i)] = 1;
  
  OutputPort & oport = context.getOutputPort(0);

  while(!context.isShutdownRequested()) {
    bool closed = context.waitOnAnyPort(waitSpec);
    if(closed)
      break;
    
    for (size_t i=0; i<nInputs; ++i){
      if(context.getInputPort(i).hasTuple()){
        oport.pushTuple(context.getInputPort(i).getFrontTuple());
        context.getInputPort(i).popTuple();
      }
    }
  }
}
