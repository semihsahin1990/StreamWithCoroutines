#pragma once

#include "streamc/Tuple.h"
#include "streamc/InputPort.h"
#include "streamc/OutputPort.h"
#include "streamc/OperatorContext.h"

#include<iostream>
#include<vector>

#define MEXP1(x) [] (Tuple & t_) { return x; }
#define MEXP2(x) [] (Tuple & t0_, Tuple & t1_) { return x; }
#define MEXPn(x) [] (std::vector<Tuple *> const & ts_) { return x; }

namespace streamc
{

class Operator
{

private:
  //properties of an operator
  std::string name_;
  size_t numInputPorts_;
  size_t numOutputPorts_;

public:
  //constructor with name, numInputPorts, numOutputPorts
  Operator(std::string const & name, size_t numInputPorts, size_t numOutputPorts)
    : name_(name), numInputPorts_(numInputPorts), numOutputPorts_(numOutputPorts) 
  {}
  //constructor with numInputPorts, numOutputPorts
  Operator(size_t numInputPorts, size_t numOutputPorts)
    : Operator("op@addr_"+std::to_string(reinterpret_cast<uintptr_t>(this)), 
               numInputPorts, numOutputPorts) 
  {}
  //destructor
  virtual ~Operator() 
  {}
  //get name
  std::string const & getName() const
  {
    return name_;
  }
  //get numInputPorts
  size_t getNumberOfInputPorts() const
  {
    return numInputPorts_;
  }
  //get numOutputPorts
  size_t getNumberOfOutputPorts() const
  {
    return numOutputPorts_;
  }
  //init function with Operator context
  virtual void init(OperatorContext & context) = 0;
  //process function with Operator context
  virtual void process(OperatorContext & context) = 0;
};
 
}
