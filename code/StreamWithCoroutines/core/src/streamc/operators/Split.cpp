#include "streamc/operators/Split.h"

using namespace streamc;
using namespace streamc::operators;
using namespace std;

Split::Split(std::string const & name)
  : Split(name, 1, MEXP1(0))
{}

Split::Split(std::string const & name, int numOutputs)
  : Split(name, numOutputs, MEXP1(0))
{}

Split::Split(std::string const & name, std::function<int (Tuple &)> hashFunction)
  : Operator(name, 1, 1), hashFunction_(hashFunction)
{}

Split::Split(std::string const & name, int numOutputs, std::function<int (Tuple &)> hashFunction)
  : Operator(name, 1, numOutputs), hashFunction_(hashFunction)
{}

Split & Split::set_hashFunction(std::function<int (Tuple &)> hashFunction)
{
  hashFunction_ = hashFunction;
  return *this;
}

void Split::process(OperatorContext & context)
{
  size_t seqNo = 0;
  InputPort & iport = context.getInputPort(0);
  size_t nOutputs = getNumberOfOutputPorts();

  while (!context.isShutdownRequested()) {
    bool closed = iport.waitTuple();
    if(closed)
      break;

    Tuple & tuple = iport.getFrontTuple();
    int hashValue = hashFunction_(tuple);
    tuple.setAttribute("seqNo", (int64_t)seqNo);
    context.getOutputPort(hashValue%nOutputs).pushTuple(tuple);
    iport.popTuple();
    seqNo++;
  }
}
