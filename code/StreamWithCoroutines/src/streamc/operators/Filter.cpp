#include "streamc/operators/Filter.h"

using namespace streamc;
using namespace std;

Filter::Filter(std::string const & name, std::function<bool (Tuple &)> filter)
  : Operator(name, 1, 1), filter_(filter)
{}

void Filter::init(OperatorContext & context)
{
  iport_ = & context.getInputPort(0);
  oport_ = & context.getOutputPort(0);
}

void Filter::process(OperatorContext & context)
{
  while (!context.isShutdownRequested()) {
    bool closed = iport_->waitTuple();
    if (closed)
      break;
    Tuple & tuple = iport_->getFrontTuple();
    if (filter_(tuple)) 
      oport_->pushTuple(tuple);
    iport_->popTuple();
  }
}
