#include "streamc/operators/Filter.h"

using namespace streamc;
using namespace std;

//constructor with name and filtering function
Filter::Filter(std::string const & name, std::function<bool (Tuple &)> filter)
  : Operator(name, 1, 1), filter_(filter)
{}

//constructor with name, initialize filtering function as MEXP1(false)
Filter::Filter(std::string const & name)
  : Filter(name, MEXP1(false))
{}

//set filtering function
Filter & Filter::set_filter(std::function<bool (Tuple &)> filter)
{
  filter_ = filter;
  return *this;
}

//init Filter with OperatorContext
void Filter::init(OperatorContext & context)
{
  iport_ = & context.getInputPort(0);
  oport_ = & context.getOutputPort(0);
}


//wait for tuple, get it. if it passes the filter, push it to the output port
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
