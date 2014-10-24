#pragma once

#include "streamc/Tuple.h"
#include "streamc/InputPort.h"
#include "streamc/OutputPort.h"
#include "streamc/OperatorContext.h"
#include "streamc/HashHelpers.h"
#include "streamc/Lambdas.h"

#include <iostream>
#include <vector>

namespace streamc
{
  class Operator;
}

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
   * Clone this operator. 
   *
   * This functiuon is overriden to provide object cloning.
   * @return the clone of the operator
   */
  virtual Operator * clone(std::string const & name) = 0;

  /**
   * Perform the main processing of the operator.
   *
   * This function is overriden to provide the implementation for the operator's
   * core logic. The <code>context</code> object is used to provide runtime
   * services, such as accessing the ports. For instance, a filter can be
   * implemented as follows:
   * @code{.cpp}
     void Filter::process(OperatorContext & context) {
         InputPort & iport = context.getInputPort(0);
         OutputPort & oport = context.getOutputPort(0);
         while (!context.isShutdownRequested()) {
             bool closed = iport.waitTuple();
             if (closed)
                 break;
             Tuple & tuple = iport.getFrontTuple();
             if (passesFilter(tuple)) 
                 oport.pushTuple(tuple);
             iport.popTuple();
         }
     }
     @endcode
   * @param context operator context
   */
  virtual void process(OperatorContext & context) = 0;

  /**
    Initialize the operator state.
    
    This function can be overriden to initialize the operator state, in case the
    operator is being restarted, rather than starting afresh.
    @param context the operator context
   */
  virtual void initState(OperatorContext & context) {}

  /**
    Save the operator state.
    
    This function can be overriden to initialize save operator state, so that 
    the operator can be restarted later.
    @param context the operator context
   */
  virtual void saveState(OperatorContext & context) {}

private:
  std::string name_;
  size_t numInputPorts_;
  size_t numOutputPorts_;
};
 
}


