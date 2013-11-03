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
