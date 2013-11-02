#pragma once

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

namespace streamc
{

class Operator;
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
  Flow(std::string const & name);
  ~Flow();
  std::string const & getName() const { return name_; }
  void addOperator(Operator * op);
  void addConnection(Operator * fromOp, uint32_t fromOutPort, 
                     Operator * toOp, uint32_t toInPort);
  Operator * getOperatorByName(std::string const & opName) const;
  std::vector<Operator *> const & getOperators() const { return opList_; }
  std::vector<OutConnection> const & getOutConnections(Operator *op, size_t outPort) const;
  std::vector<InConnection> const & getInConnections(Operator *op, size_t inPort) const;
  void printTopology(std::ostream & ostream) const; 

private:
  std::string name_;
  std::vector<Operator *> opList_; // to return to users
  std::unordered_map<std::string, std::unique_ptr<Operator>> ops_;
  std::unordered_map<uintptr_t, std::unique_ptr<OperatorConnections>> opConnections_;
};

} // namespace streamc
