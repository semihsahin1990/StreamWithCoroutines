#include "streamc/operators/Filter.h"

using namespace streamc;
using namespace streamc::operators;
using namespace std;

Filter::Filter(std::string const & name, std::function<bool (Tuple &)> filter)
  : Operator(name, 1, 1), filter_(filter)
{}

Filter::Filter(std::string const & name)
  : Filter(name, MEXP1(false))
{}

Filter & Filter::set_filter(std::function<bool (Tuple &)> filter)
{
  filter_ = filter;
  return *this;
}

void Filter::process(OperatorContext & context)
{
  InputPort & iport = context.getInputPort(0);
  OutputPort & oport = context.getOutputPort(0);

  while (!context.isShutdownRequested()) {
    bool closed = iport.waitTuple();
    if (closed)
      break;
    Tuple & tuple = iport.getFrontTuple();
    if (filter_(tuple)) 
      oport.pushTuple(tuple);
    iport.popTuple();
  }
}

Filter * Filter::clone(std::string const & name)
{
  return new Filter(name, filter_);
}
