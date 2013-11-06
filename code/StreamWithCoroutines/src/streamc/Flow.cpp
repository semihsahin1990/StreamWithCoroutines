#include "streamc/Flow.h"

#include "streamc/InputPort.h"
#include "streamc/Operator.h"
#include "streamc/OutputPort.h"
#include "streamc/Tuple.h"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>

using namespace std;
using namespace streamc;

namespace streamc  
{

class OperatorConnections 
{
public:
  OperatorConnections(size_t numInputPorts, size_t numOutputPorts)
    : inPortConnections_(numInputPorts, vector<FromConnection>()),
      outPortConnections_(numOutputPorts, vector<ToConnection>())
  {}
  void addOutputConnection(size_t outPort, ToConnection const & ocon) 
  {
    outPortConnections_[outPort].push_back(ocon);
  }
  void addInputConnection(size_t inPort, FromConnection const & icon) 
  {
    inPortConnections_[inPort].push_back(icon);
  }
  vector<ToConnection> const & getOutputPortConnections(size_t outPort) const 
  { 
    return outPortConnections_[outPort]; 
  }
  vector<FromConnection> const & getInputPortConnections(size_t inPort) const 
  { 
    return inPortConnections_[inPort]; 
  }
private:
  vector<vector<FromConnection>> inPortConnections_;
  vector<vector<ToConnection>> outPortConnections_;
};

} // namespace streamc

Flow::Flow(string const & name)
  : name_(name)
{}

Flow::~Flow() 
{}

void Flow::addOperator(Operator & op)
{
  string const & name = op.getName();
  if (ops_.count(name)>0)
    throw runtime_error("Operator with name '"+name+"' already exists in flow '"+name_+"'.");
  ops_[name] = unique_ptr<Operator>(&op);
  uintptr_t opAddr = reinterpret_cast<uintptr_t>(&op);
  opConnections_[opAddr] = unique_ptr<OperatorConnections>(
    new OperatorConnections(op.getNumberOfInputPorts(), op.getNumberOfOutputPorts()));
  opList_.push_back(&op);
}

void Flow::addConnection(Operator & fromOp, uint32_t fromOutPort, 
                         Operator & toOp, uint32_t toInPort)
{
  uintptr_t fromOpAddr = reinterpret_cast<uintptr_t>(&fromOp);
  uintptr_t toOpAddr = reinterpret_cast<uintptr_t>(&toOp);
  opConnections_[fromOpAddr]->addOutputConnection(fromOutPort, ToConnection(toOp, toInPort)); 
  opConnections_[toOpAddr]->addInputConnection(toInPort, FromConnection(fromOp, fromOutPort)); 
}

void Flow::addConnection(FromConnection const & from, ToConnection const & to)
{
  addConnection(from.getOperator(), from.getOutputPort(),
                to.getOperator(), to.getInputPort()); 
}

void Flow::addConnection(Connection const & conn)
{
  addConnection(conn.getInConnection(), conn.getOutConnection());
}

void Flow::addConnections(ConnectionChain const & conns)
{
  for (auto const & conn : conns.getConnections())
    addConnection(conn);
}

Operator & Flow::getOperatorByName(std::string const & opName) const
{
  auto it = ops_.find(opName);
  if (it==ops_.end())
    throw runtime_error("Operator with name '"+opName+"' does not exist in flow '"+name_+"'.");
  return *(it->second.get());
}

std::vector<ToConnection> const & Flow::getOutConnections(Operator & op, size_t outPort) const
{
  uintptr_t opAddr = reinterpret_cast<uintptr_t>(&op);
  return opConnections_.find(opAddr)->second->getOutputPortConnections(outPort);
}

std::vector<FromConnection> const & Flow::getInConnections(Operator & op, size_t inPort) const
{
  uintptr_t opAddr = reinterpret_cast<uintptr_t>(&op);
  return opConnections_.find(opAddr)->second->getInputPortConnections(inPort);  
}

void Flow::printTopology(std::ostream & ostream) const
{
  for (auto oper : opList_) {
    string const & opName = oper->getName();
    uintptr_t opAddr = reinterpret_cast<uintptr_t>(oper);
    ostream << "operator: " << opName << "\n";
    ostream << "\tout conections:\n";
    OperatorConnections const & connections = *(opConnections_.find(opAddr)->second); 
    for (size_t iport=0; iport<oper->getNumberOfInputPorts(); ++iport) {
      ostream << "\t\tinput port #" << iport << ":\n";
      for (auto const & conn : connections.getInputPortConnections(iport)) 
        ostream << "\t\t\toperator: " << conn.getOperator().getName() << ", output port: " << conn.getOutputPort() << "\n";
    }
    for (size_t oport=0; oport<oper->getNumberOfOutputPorts(); ++oport) {
      ostream << "\t\toutput port #" << oport << ":\n";
      for (auto const & conn : connections.getOutputPortConnections(oport)) 
        ostream << "\t\t\toperator: " << conn.getOperator().getName() << ", input port: " << conn.getInputPort() << "\n";
    }
  }
}


