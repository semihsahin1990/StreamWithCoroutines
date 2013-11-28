#include "streamc/runtime/SchedulerState.h"

#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/InputPortImpl.h"

using namespace std;
using namespace streamc;

OperatorInfo::WaitCondition::WaitCondition(OperatorContextImpl & oper)
{
  for (size_t i=0, iu=oper.getNumberOfInputPorts(); i<iu; ++i) 
    portWaits_[&oper.getInputPortImpl(i)] = 0;
}

void OperatorInfo::WaitCondition::setWait(InputPortImpl & iport, size_t count)
{ 
  portWaits_[&iport] = count; 
}

size_t OperatorInfo::WaitCondition::getWait(InputPortImpl & iport)
{
  return portWaits_[&iport];
}

bool OperatorInfo::WaitCondition::isReady()
{
  bool ready = true;
  for (auto & portCountPair : portWaits_) {
    InputPortImpl * iport = portCountPair.first;
    size_t count = portCountPair.second;
    if (count>0 && iport->getTupleCount()<count) {
      ready = false;
      break;
    }
  }
  return ready;
}

void OperatorInfo::WaitCondition::reset()
{
  for (auto & portCountPair : portWaits_)
    portCountPair.second = 0;
}

OperatorInfo::WaitCondition & OperatorInfo::getWaitCondition()
{
  return cond_;
}

