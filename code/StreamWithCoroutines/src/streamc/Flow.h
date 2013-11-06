#pragma once

#include "streamc/Operator.h"

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

namespace streamc
{

class OperatorConnections;

class ToConnection
{
public:
  ToConnection(Operator & downstreamOp, size_t inPort) 
    : downstreamOp_(downstreamOp), inPort_(inPort) {}
  size_t getInputPort() const { return inPort_; }
  Operator & getOperator() const { return downstreamOp_; }
private:
  Operator & downstreamOp_;
  size_t inPort_;
};

inline ToConnection operator,(size_t inPort, Operator & downstreamOp)
{
  return ToConnection(downstreamOp, inPort);
}

class FromConnection
{
public:
  FromConnection(Operator & upstreamOp, size_t outPort) 
    : upstreamOp_(upstreamOp), outPort_(outPort) {}
  size_t getOutputPort() const { return outPort_; }
  Operator & getOperator() const { return upstreamOp_; }
private:
  Operator & upstreamOp_;
  size_t outPort_;
};

inline FromConnection operator,(Operator & upstreamOp, size_t outPort)
{
  return FromConnection(upstreamOp, outPort);
}

class Connection
{
public:
  Connection (FromConnection const & fromConn, ToConnection const & toConn)
    : fromConn_(fromConn), toConn_(toConn) {}
  Connection (Operator & upstreamOp, size_t outPort, Operator & downstreamOp, size_t inPort)
    : Connection(FromConnection(upstreamOp, outPort), ToConnection(downstreamOp, inPort)) {}
  FromConnection const & getInConnection() const { return fromConn_; }
  ToConnection const & getOutConnection() const { return toConn_; }
private:
  FromConnection fromConn_;
  ToConnection toConn_;
};

inline Connection operator>>(FromConnection const & fromConn, ToConnection const & toConn)
{
  return Connection(fromConn, toConn);
}

class ConnectionKnot
{
public:
  ConnectionKnot(size_t inPort, Operator & op, size_t outPort)
    : inPort_(inPort), op_(op), outPort_(outPort)
  {}
  size_t getInputPort() const { return inPort_; }
  Operator & getOperator() const { return op_; }
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
  ConnectionChain() {}
  ConnectionChain(OpenConnectionChain && other);
  void addConnection(Connection const & conn) { connections_.push_back(conn); }
  std::vector<Connection> const & getConnections() const { return connections_; }
private:
  std::vector<Connection> connections_;
};

class OpenConnectionChain : public ConnectionChain
{
public:
  OpenConnectionChain() {}
  OpenConnectionChain(OpenConnectionChain && other)
    : ConnectionChain(std::move(other)), outputPort_(other.outputPort_)
  {}
  void setOpenOutputPort(size_t outputPort) { outputPort_ = outputPort; }
  size_t getOpenOutputPort() const { return outputPort_; }
private:
  size_t outputPort_;
};

inline ConnectionChain::ConnectionChain(OpenConnectionChain && other) 
  : connections_(std::move(other.connections_))
{}

inline OpenConnectionChain operator>>(FromConnection const & fromConn, ConnectionKnot const & knot)
{
  OpenConnectionChain chain;
  Connection conn(fromConn.getOperator(), fromConn.getOutputPort(), knot.getOperator(), knot.getInputPort());
  chain.addConnection(conn);
  chain.setOpenOutputPort(knot.getOutputPort());
  return chain;
}

inline OpenConnectionChain operator>>(OpenConnectionChain && chain, ConnectionKnot const & knot)
{
  Connection conn(chain.getConnections().rbegin()->getOutConnection().getOperator(), 
                  chain.getOpenOutputPort(), knot.getOperator(), knot.getInputPort());
  OpenConnectionChain newChain(std::move(chain));
  newChain.addConnection(conn);
  newChain.setOpenOutputPort(knot.getOutputPort());
  return newChain;
}

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
  Flow(std::string const & name);
  ~Flow();
  std::string const & getName() const { return name_; }
  void addOperator(Operator & op);
  void addConnection(Operator & fromOp, uint32_t fromOutPort, Operator & toOp, uint32_t toInPort);
  void addConnection(FromConnection const & from, ToConnection const & to);
  void addConnection(Connection const & conn);
  void addConnections(ConnectionChain const & conns);
  Operator & getOperatorByName(std::string const & opName) const;
  std::vector<Operator *> const & getOperators() const { return opList_; }
  std::vector<ToConnection> const & getOutConnections(Operator & op, size_t outPort) const;
  std::vector<FromConnection> const & getInConnections(Operator & op, size_t inPort) const;
  void printTopology(std::ostream & ostream) const; 

  template <typename T, typename... Args>
  T & createOperator(std::string const & name, Args&&... args)
  {
    T * op = new T(name, std::forward<Args>(args)...);
    addOperator(*op);
    return *op;
  }
private:
  std::string name_;
  std::vector<Operator *> opList_; // to return to users
  std::unordered_map<std::string, std::unique_ptr<Operator>> ops_;
  std::unordered_map<uintptr_t, std::unique_ptr<OperatorConnections>> opConnections_;
};

} // namespace streamc
