#include "streamc/runtime/OutputPortImpl.h"

#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/InputPortImpl.h"

#include <thread>
#include <mutex>

using namespace std;
using namespace streamc;

OutputPortImpl::OutputPortImpl()
{}

// add (subscriber operator(operator context), portNo) pair to this port
void OutputPortImpl::addSubscriber(OperatorContextImpl * oper, size_t inPort)
{
  subscribers_.push_back(std::make_pair(oper, inPort));
}


// push tuple to the input port of each subscriber
void OutputPortImpl::pushTuple(Tuple const & tuple)
{
  // TODO: hook into the scheduler to see if we need to be preempted

  for(auto const & opPortPair : subscribers_)
  {
    OperatorContextImpl * op = opPortPair.first;
    size_t inPort = opPortPair.second;
    op->getInputPortImpl(inPort).pushTuple(tuple);
  }
}
