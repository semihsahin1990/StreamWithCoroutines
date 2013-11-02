#pragma once

#include "streamc/Operator.h"

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

namespace streamc
{

class OperatorConnections;

class OutConnection
{
public:
  OutConnection(Operator * downstreamOp, size_t inPort)
    : downstreamOp_(downstreamOp), inPort_(inPort) {}
  size_t getInputPort() const { return inPort_; }
  Operator * getOperator() const { return downstreamOp_; }
private:
  Operator * downstreamOp_;
  size_t inPort_;
};

class InConnection
{
public:
  InConnection(Operator * upstreamOp, size_t outPort)
    : upstreamOp_(upstreamOp), outPort_(outPort) {}
  size_t getOutputPort() const { return outPort_; }
  Operator * getOperator() const { return upstreamOp_; }
private:
  Operator * upstreamOp_;
  size_t outPort_;
};

class Flow
{
public:
  void addOperator(Operator * op);
  void addConnection(Operator * fromOp, uint32_t fromOutPort, 
                     Operator * toOp, uint32_t toInPort);
  Operator * getOperatorByName(std::string const & opName);
  std::vector<Operator *> const & getOperators();
  std::vector<OutConnection> const & getOutConnections(Operator *op, size_t outPort);
  std::vector<InConnection> const & getInConnections(Operator *op, size_t inPort);
  void printTopology(std::ostream & ostream);
private: 
  std::unordered_map<std::string, std::unique_ptr<Operator>> ops_;
  std::unordered_map<uintptr_t, std::unique_ptr<OperatorConnections>> opConnections_;
};

} // namespace streamc
