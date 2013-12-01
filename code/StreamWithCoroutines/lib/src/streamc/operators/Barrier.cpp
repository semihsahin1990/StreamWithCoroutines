#include "streamc/operators/Barrier.h"
#include "streamc/runtime/PortsImpl.h"

using namespace streamc;
using namespace std;

Barrier::Barrier(std::string const & name)
  : Operator(name, 2, 1)
{}

void Barrier::process(OperatorContext & context)
{
  iport1_ = & context.getInputPort(0);
  iport2_ = & context.getInputPort(1);
  oport_ = & context.getOutputPort(0);
  vector<InputPort *> iports;

  iports.push_back(iport1_);
  iports.push_back(iport2_);

  while(!context.isShutdownRequested())
  {
    bool closed = PortsImpl::waitTupleFromAll(iports);

    if(closed)
      break;

    Tuple resultTuple;
    resultTuple.append(iport1_->getFrontTuple());
    resultTuple.append(iport2_->getFrontTuple());
    
    oport_->pushTuple(resultTuple);

    iport1_->popTuple();
    iport2_->popTuple();
  }
}
