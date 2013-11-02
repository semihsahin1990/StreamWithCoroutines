#include "streamc/Flow.h"
#include "streamc/Tuple.h"
#include "streamc/Operator.h"
#include "streamc/InputPort.h"
#include "streamc/OutputPort.h"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>

using namespace std;
using namespace streamc;

namespace streamc {
class OperatorConnections
{
public:
  OperatorConnections(size_t numInputPorts, size_t numOutputPorts)
    : inPortConnections_(numInputPorts, std::vector<InConnection>()),
      outPortConnections_(numOutputPorts, std::vector<OutConnection>())
  {}
  void addOutputConnection(size_t outPort, OutConnection const & ocon) 
  {
    outPortConnections_[outPort].push_back(ocon);
  }
  void addInputConnection(size_t inPort, InConnection const & icon) 
  {
    inPortConnections_[inPort].push_back(icon);
  }
  vector<OutConnection> const & getOutputPortConnections(size_t outPort) const 
  { 
    return outPortConnections_[outPort]; 
  }
  vector<InConnection> const & getInputPortConnections(size_t inPort) const 
  { 
    return inPortConnections_[inPort]; 
  }
private:
  vector<vector<InConnection>> inPortConnections_;
  vector<vector<OutConnection>> outPortConnections_;
};
}

void Flow::addOperator(Operator * op)
{
  if (ops_.count(op->getName())>0)
    throw runtime_error("Operator with name '"+op->getName()+"' exists");
  ops_[op->getName()] = unique_ptr<Operator>(op);
  uintptr_t opAddr = reinterpret_cast<uintptr_t>(op);
  opConnections_[opAddr] = unique_ptr<OperatorConnections>(
    new OperatorConnections(op->getNumberOfInputPorts(), op->getNumberOfOutputPorts()));
}

void Flow::addConnection(Operator * fromOp, uint32_t fromOutPort, 
                         Operator * toOp, uint32_t toInPort)
{
  uintptr_t fromOpAddr = reinterpret_cast<uintptr_t>(fromOp);
  uintptr_t toOpAddr = reinterpret_cast<uintptr_t>(toOp);
  opConnections_[fromOpAddr]->addOutputConnection(fromOutPort, OutConnection(toOp, toInPort)); 
  opConnections_[toOpAddr]->addInputConnection(toInPort, InConnection(fromOp, fromOutPort)); 
}

Operator * Flow::getOperatorByName(std::string const & opName)
{
  auto it = ops_.find(opName);
  if (it==ops_.end())
    return nullptr;
  return it->second.get();
}

std::vector<OutConnection> const & Flow::getOutConnections(Operator *op, size_t outPort)
{
  uintptr_t opAddr = reinterpret_cast<uintptr_t>(op);
  return opConnections_[opAddr]->getOutputPortConnections(outPort);
}

std::vector<InConnection> const & Flow::getInConnections(Operator *op, size_t inPort)
{
  uintptr_t opAddr = reinterpret_cast<uintptr_t>(op);
  return opConnections_[opAddr]->getInputPortConnections(inPort);  
}

void Flow::printTopology(std::ostream & ostream)
{
  for (auto const & nameOpPair : ops_) {
    string const & opName = nameOpPair.first;
    Operator * oper = nameOpPair.second.get();
    uintptr_t opAddr = reinterpret_cast<uintptr_t>(oper);
    ostream << "operator: " << opName << "\n";
    ostream << "\tout conections:\n";
    OperatorConnections const & connections = *(opConnections_[opAddr]); 
    for (size_t oport=0; oport<oper->getNumberOfOutputPorts(); ++oport) {
      ostream << "\t\toutput port #" << oport << ":\n";
      for (auto const & conn : connections.getOutputPortConnections(oport)) 
        ostream << "\t\t\toperator: " << conn.getOperator()->getName() << ", input port: " << conn.getInputPort() << "\n";
    }
    for (size_t iport=0; iport<oper->getNumberOfInputPorts(); ++iport) {
      ostream << "\t\tinput port #" << iport << ":\n";
      for (auto const & conn : connections.getInputPortConnections(iport)) 
        ostream << "\t\t\toperator: " << conn.getOperator()->getName() << ", output port: " << conn.getOutputPort() << "\n";
    }
  }
}


