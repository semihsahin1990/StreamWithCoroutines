#include "streamc/runtime/OutputPortImpl.h"

#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/InputPortImpl.h"

#include <thread>
#include <mutex>

using namespace std;
using namespace streamc;

OutputPortImpl::OutputPortImpl()
{}

void OutputPortImpl::addSubscriber(OperatorContextImpl * oper, size_t inPort)
{
  subscribers_.push_back(std::make_pair(oper, inPort));
}


void OutputPortImpl::pushTuple(Tuple const & tuple)
{
  // TODO: we need to hook into the scheduler to check whether we should go
  // ahead or otherwise we will be descheduled

  for(auto const & opPortPair : subscribers_) {
    OperatorContextImpl * op = opPortPair.first;
    size_t inPort = opPortPair.second;
    op->getInputPortImpl(inPort).pushTuple(tuple);
  }
}
