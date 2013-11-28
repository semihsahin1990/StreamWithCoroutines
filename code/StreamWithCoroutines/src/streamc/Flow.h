#pragma once

#include "streamc/Operator.h"
#include "streamc/runtime/HashHelpers.h"

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

namespace streamc
{

class OperatorConnections;

//ToConnection class: stores down stream part of connection: operator and its inPort
class ToConnection
{
public:
  ToConnection(Operator & downstreamOp, size_t inPort) 
    : downstreamOp_(downstreamOp), inPort_(inPort) 
  {}

  //returns inPort
  size_t getInputPort() const { return inPort_; }

  //returns operator
  Operator & getOperator() const { return downstreamOp_; }

private:
  Operator & downstreamOp_;
  size_t inPort_;
};

inline ToConnection operator,(size_t inPort, Operator & downstreamOp)
{
  return ToConnection(downstreamOp, inPort);
}

//FromConnection class: stores up stream part of the connection: operator and its outPort
class FromConnection
{
public:
  FromConnection(Operator & upstreamOp, size_t outPort) 
    : upstreamOp_(upstreamOp), outPort_(outPort) 
  {}

  //returns outPort
  size_t getOutputPort() const { return outPort_; }

  //returns operator
  Operator & getOperator() const { return upstreamOp_; }

private:
  Operator & upstreamOp_;
  size_t outPort_;
};

inline FromConnection operator,(Operator & upstreamOp, size_t outPort)
{
  return FromConnection(upstreamOp, outPort);
}

//Connection class: stores FromConnection and ToConnection parts of the connection
class Connection
{
public:
  //constructor with FromConnection and ToConnection
  Connection (FromConnection const & fromConn, ToConnection const & toConn)
    : fromConn_(fromConn), toConn_(toConn)
  {}

  //constructor with upstream operator, its outport, downstream operator and its inport
  Connection (Operator & upstreamOp, size_t outPort, Operator & downstreamOp, size_t inPort)
    : Connection(FromConnection(upstreamOp, outPort), ToConnection(downstreamOp, inPort))
  {}
  
  //returns the up stream part of the connection
  FromConnection const & getInConnection() const { return fromConn_; }

  //returns the down stream part of the connection
  ToConnection const & getOutConnection() const { return toConn_; }

private:
  FromConnection fromConn_;
  ToConnection toConn_;
};

inline Connection operator>>(FromConnection const & fromConn, ToConnection const & toConn)
{
  return Connection(fromConn, toConn);
}


//ConnectionKnot class: stores middle part of the connection, inPort, operator, outputPort
class ConnectionKnot
{
public:
  //constructor with inPort, operator, outPort
  ConnectionKnot(size_t inPort, Operator & op, size_t outPort)
    : inPort_(inPort), op_(op), outPort_(outPort)
  {}

  //returns inPort
  size_t getInputPort() const { return inPort_; }

  //returns operator
  Operator & getOperator() const { return op_; }

  //returns outPort
  size_t getOutputPort() const { return outPort_; }

private:
  size_t inPort_;
  Operator & op_;
  size_t outPort_;
};

inline ConnectionKnot operator,(ToConnection const & to, size_t outPort)
{
  return ConnectionKnot(to.getInputPort(), to.getOperator(), outPort);
}

class OpenConnectionChain;
class ConnectionChain
{
public:
  //construtor
  ConnectionChain() {}

  //constructor with OpenConnectionChain
  ConnectionChain(OpenConnectionChain && other);

  //add new connection to the chain
  void addConnection(Connection const & conn) { connections_.push_back(conn); }

  //return all connections
  std::vector<Connection> const & getConnections() const { return connections_; }

private:
  std::vector<Connection> connections_;
};

//open connection: stores incomleteConnection with open outputPort
class OpenConnectionChain : public ConnectionChain
{
public:
  //constructor
  OpenConnectionChain() {}

  //constructor with OpenConnectionChain
  OpenConnectionChain(OpenConnectionChain && other)
    : ConnectionChain(std::move(other)), outputPort_(other.outputPort_)
  {}

  //sets/changes open output port
  void setOpenOutputPort(size_t outputPort) { outputPort_ = outputPort; }

  //returns open output port
  size_t getOpenOutputPort() const { return outputPort_; }
private:
  size_t outputPort_;
};

inline ConnectionChain::ConnectionChain(OpenConnectionChain && other) 
  : connections_(std::move(other.connections_))
{}

//ties FromConnection and ConnectionKnot, which results in OpenConnection
inline OpenConnectionChain operator>>(FromConnection const & fromConn, ConnectionKnot const & knot)
{
  OpenConnectionChain chain;
  Connection conn(fromConn.getOperator(), fromConn.getOutputPort(), knot.getOperator(), knot.getInputPort());
  chain.addConnection(conn);
  chain.setOpenOutputPort(knot.getOutputPort());
  return chain;
}

//ties OpenConnection and ConnectionKnot, which results in OpenConnection
inline OpenConnectionChain operator>>(OpenConnectionChain && chain, ConnectionKnot const & knot)
{
  Connection conn(chain.getConnections().rbegin()->getOutConnection().getOperator(), 
                  chain.getOpenOutputPort(), knot.getOperator(), knot.getInputPort());
  OpenConnectionChain newChain(std::move(chain));
  newChain.addConnection(conn);
  newChain.setOpenOutputPort(knot.getOutputPort());
  return newChain;
}

//ties OpenConnection and ToConnection, which results in ConnectionChain
inline ConnectionChain operator>>(OpenConnectionChain && chain, ToConnection const & to)
{
  Connection conn(chain.getConnections().rbegin()->getOutConnection().getOperator(), 
                  chain.getOpenOutputPort(), to.getOperator(), to.getInputPort());
  ConnectionChain newChain(std::move(chain));
  newChain.addConnection(conn);
  return newChain;
}

class Flow
{
public:
  //constructor with name
  Flow(std::string const & name);

  //destructor
  ~Flow();

  //returns name of the flow
  std::string const & getName() const { return name_; }

  //adds operator to the flow
  void addOperator(Operator & op);

  //adds connection from 'fromOutPort' of Operator 'fromOp' to 'toInPort' of Operator 'toOp'
  void addConnection(Operator & fromOp, uint32_t fromOutPort, Operator & toOp, uint32_t toInPort);

  //adds connection from FromConnection from to ToConnection to
  void addConnection(FromConnection const & from, ToConnection const & to);

  //adds connection with Connection
  void addConnection(Connection const & conn);

  //adds connections of ConnectionChain conns
  void addConnections(ConnectionChain const & conns);

  //returns operator with name = opName
  Operator & getOperatorByName(std::string const & opName) const;

  //returns operators in the flow
  std::vector<Operator *> const & getOperators() const { return opList_; }

  //returns outConnections of Operator op
  std::vector<ToConnection> const & getOutConnections(Operator & op, size_t outPort) const;

  //returns inConnections of Operator op
  std::vector<FromConnection> const & getInConnections(Operator & op, size_t inPort) const;

  //prints the flow topology
  void printTopology(std::ostream & ostream) const; 

  template <typename T, typename... Args>
  T & createOperator(std::string const & name, Args&&... args)
  {
    T * op = new T(name, std::forward<Args>(args)...);
    addOperator(*op);
    return *op;
  }

private:
  //name of the flow
  std::string name_;

  //operators in the flow
  std::vector<Operator *> opList_;

  //maps name to operator
  std::unordered_map<std::string, std::unique_ptr<Operator>> ops_;

  // maps operator to operator connections
  std::unordered_map<Operator *, std::unique_ptr<OperatorConnections>> opConnections_;
};

} // namespace streamc
