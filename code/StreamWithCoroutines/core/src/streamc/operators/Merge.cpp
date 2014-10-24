#include "streamc/operators/Merge.h"
#include "streamc/Logger.h"
#include <queue>
using namespace streamc;
using namespace streamc::operators;
using namespace std;

Merge::Merge(std::string const & name)
  : Operator(name, 1, 1)
{}

Merge::Merge(std::string const & name, int numInputs)
  : Operator(name, numInputs, 1)
{}

void Merge::process(OperatorContext & context)
{
  typedef pair<int, int> pqNode;
  priority_queue<pqNode, vector<pqNode>, greater<pqNode>> pq;

  bool closed;
  size_t nInputs = getNumberOfInputPorts();
  OutputPort & oport = context.getOutputPort(0);

  for(size_t i=0; i<nInputs-1; i++) {
    InputPort & iport = context.getInputPort(i);
    closed = iport.waitTuple();
    if(!closed) {
      int seqNo = iport.getFrontTuple().getIntAttribute("seqNo");
      pq.push(make_pair(seqNo, i));
    }
  }

  int previousPortNo = nInputs-1;
  do {
    InputPort & previousPort = context.getInputPort(previousPortNo);
    closed = previousPort.waitTuple();
    
    if(!closed) {
      int seqNo = previousPort.getFrontTuple().getIntAttribute("seqNo");
      pq.push(make_pair(seqNo, previousPortNo));
    }

    if(pq.size() == 0)
      break;
    
    int selectedPortNo = pq.top().second;
    InputPort & selectedPort = context.getInputPort(selectedPortNo);
    Tuple & tuple = selectedPort.getFrontTuple();
    tuple.removeAttribute("seqNo");
    oport.pushTuple(tuple);

    selectedPort.popTuple();
    pq.pop();
    previousPortNo = selectedPortNo;
  } while(!context.isShutdownRequested());
}

Merge * Merge::clone(std::string const & name)
{
  return new Merge(name, getNumberOfInputPorts());
}

