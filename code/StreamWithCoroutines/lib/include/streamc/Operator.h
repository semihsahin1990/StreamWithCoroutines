#pragma once

#include "streamc/Tuple.h"
#include "streamc/InputPort.h"
#include "streamc/OutputPort.h"
#include "streamc/OperatorContext.h"
#include "streamc/HashHelpers.h"

#include <iostream>
#include <vector>

namespace streamc
{
  class Operator;
}

#define MEXP1(x) [] (Tuple & t_) { return x; }
#define MEXP2(x) [] (Tuple & t0_, Tuple & t1_) { return x; }
#define MEXPn(x) [] (std::vector<Tuple *> const & ts_) { return x; }

namespace streamc
{

/**
 * Base class for implementing streaming operators.
 */
class Operator
{
protected:
  /**
   * Construct with explicit name.
   *
   * @param name name of the operator
   * @param numInputPorts number of input ports of the operator
   * @param numOutputPorts number of output ports of the operator
   */
  Operator(std::string const & name, size_t numInputPorts, size_t numOutputPorts)
    : name_(name), numInputPorts_(numInputPorts), numOutputPorts_(numOutputPorts) 
  {}

  /**
   * Construct with implicit name.
   *
   * @param numInputPorts number of input ports of the operator
   * @param numOutputPorts number of output ports of the operator
   */
  Operator(size_t numInputPorts, size_t numOutputPorts)
    : Operator("op@addr_"+std::to_string(reinterpret_cast<uintptr_t>(this)), 
               numInputPorts, numOutputPorts) 
  {}
public:
  virtual ~Operator() {}

  /**
   * Get the name of the operator.
   *
   * @return the name of the operator
   */
  std::string const & getName() const
  {
    return name_;
  }

  /**
   * Get the number of input ports of the operator.
   *
   * @return the number of input ports of the operato
   */
  size_t getNumberOfInputPorts() const
  {
    return numInputPorts_;
  }

  /**
   * Get the number of output ports of the operator.
   *
   * @return the number of output ports of the operato
   */
  size_t getNumberOfOutputPorts() const
  {
    return numOutputPorts_;
  }

  /**
   * Perform the main processing of the operator.
   *
   * This function is overriden to provide the implementation for the operator's
   * core logic. The <code>context</code> object is used to provide runtime
   * services, such as accessing the ports.
   * @param context operator context
   */
  virtual void process(OperatorContext & context) = 0;

private:
  std::string name_;
  size_t numInputPorts_;
  size_t numOutputPorts_;
};
 
}


