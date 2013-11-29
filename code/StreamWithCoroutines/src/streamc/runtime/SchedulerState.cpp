#include "streamc/runtime/SchedulerState.h"

#include "streamc/runtime/FlowContext.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OperatorContextImpl.h"
#include "streamc/runtime/OutputPortImpl.h"

using namespace std;
using namespace streamc;

OperatorInfo::ReadWaitCondition::ReadWaitCondition(OperatorContextImpl & oper)
{
  for (size_t i=0, iu=oper.getNumberOfInputPorts(); i<iu; ++i) 
    portWaits_[&oper.getInputPortImpl(i)] = ThresholdAndCount(0, 0);
}

OperatorInfo::WriteWaitCondition::WriteWaitCondition(OperatorContextImpl & oper)
{
  size_t nOports = oper.getNumberOfOutputPorts();
  for (size_t o=0; o<nOports; ++o) {
    OutputPortImpl & oport = oper.getOutputPortImpl(o);
    size_t nSubs = oport.getNumberOfSubscribers();
    for (size_t s=0; s<nSubs; ++s) {
      pair<OperatorContextImpl *, size_t> subs = oport.getSubscriber(s);
      InputPortImpl & iport = subs.first->getInputPortImpl(subs.second);
      portWaits_[&iport] = ThresholdAndCount(FlowContext::getMaxQueueSize(), 0);
    }
  }
}

void OperatorInfo::ReadWaitCondition::setWaitThreshold(InputPortImpl & iport, size_t thresh) 
{ 
  portWaits_[&iport].threshold = thresh; 
}

void OperatorInfo::WriteWaitCondition::setWaitThreshold(InputPortImpl & iport, size_t thresh) 
{ 
  portWaits_[&iport].threshold = thresh; 
}

size_t OperatorInfo::ReadWaitCondition::getWaitThreshold(InputPortImpl & iport) 
{ 
  return portWaits_[&iport].threshold; 
}

size_t OperatorInfo::WriteWaitCondition::getWaitThreshold(InputPortImpl & iport) 
{ 
  return portWaits_[&iport].threshold; 
}

bool OperatorInfo::ReadWaitCondition::computeReadiness()
{
  bool ready = true;
  for (auto & portCondPair : portWaits_) {
    InputPortImpl * iport = portCondPair.first;
    size_t thresh = portCondPair.second.threshold;
    size_t & count = portCondPair.second.currentCount;
    count = iport->getTupleCount();
    if (count < thresh) 
      ready = false;    
  }
  return ready;
}

bool OperatorInfo::WriteWaitCondition::computeReadiness()
{
  bool ready = true;
  for (auto & portCondPair : portWaits_) {
    InputPortImpl * iport = portCondPair.first;
    size_t thresh = portCondPair.second.threshold;
    size_t & count = portCondPair.second.currentCount;
    count = iport->getTupleCount();
    if (count >= thresh) 
      ready = false;    
  }
  return ready;
}

bool OperatorInfo::ReadWaitCondition::isReady(InputPortImpl & iport)
{
  ThresholdAndCount & cond = portWaits_[&iport];
  return (cond.currentCount >= cond.threshold);
}

bool OperatorInfo::WriteWaitCondition::isReady(InputPortImpl & iport)
{
  ThresholdAndCount & cond = portWaits_[&iport];
  return (cond.currentCount < cond.threshold);
}

void OperatorInfo::ReadWaitCondition::reset()
{
  for (auto & portCondPair : portWaits_)
    portCondPair.second.threshold = 0;
}


