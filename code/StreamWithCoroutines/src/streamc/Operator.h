#pragma once

#include "streamc/OperatorContext.h"

#include<iostream>
#include<vector>

namespace streamc
{

class OperatorContext;

class Operator
{
private:
  std::string name_;
  size_t numInputPorts_;
  size_t numOutputPorts_;

public:
  Operator(std::string const & name, size_t numInputPorts, size_t numOutputPorts)
    : name_(name), numInputPorts_(numInputPorts), numOutputPorts_(numOutputPorts) 
  {}
  Operator(size_t numInputPorts, size_t numOutputPorts)
    : Operator("op@addr_"+std::to_string(reinterpret_cast<uintptr_t>(this)), 
               numInputPorts, numOutputPorts) 
  {}
  virtual ~Operator() 
  {}  
  std::string const & getName() const
  {
    return name_;
  }
  size_t getNumberOfInputPorts() const
  {
    return numInputPorts_;
  }
  size_t getNumberOfOutputPorts() const
  {
    return numOutputPorts_;
  }
protected:
  virtual void init(OperatorContext & context) = 0;
  virtual void process(OperatorContext & context) = 0;
};
 
}
