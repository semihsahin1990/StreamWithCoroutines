#pragma once

#include <cstddef>
#include <unordered_map>

namespace streamc
{

class InputPort;
class OutputPort;
class Tuple;

/**
 * %Operator context that provides runtime services for operators.
 *
 * These services include acccessing input and output ports, operator
 * state store, and checking whethere shutdown is requested.
 */
class OperatorContext
{
public:
  virtual ~OperatorContext() {}

  /**
   * Get the input port at a given index.
   *
   * @param inputPort the input port index
   * @return the input port object
   */
  virtual InputPort & getInputPort(size_t inputPort) = 0;

  /**
   * Get the output port at a given index.
   *
   * @param outputPort the input port index
   * @return the input port object
   */
  virtual OutputPort & getOutputPort(size_t outputPort) = 0;

  /**
   * Get the state store of the operator.
   *
   * This store is used to @link Operator::initState init @endlink and 
   * @link Operator::saveState save @endlink state to support restarts. 
   * @return the state store of the operator 
   */
  virtual Tuple & getStateStore() = 0;

  /**
   * Check whether shutdown is requested.
   *
   * An operator's @link Operator::process process method @endlink typically
   * sits within a loop that checks the shutdown condition.
   * @code{.cpp}
     void process(OperatorContext & context) {
       while(!context.isShutdownRequested()) {
          // do work
       }
     }
     @endcode
   * @return <code>true</code> if the shutdown is requested, <code>false</code> otherwise
   */
  virtual bool isShutdownRequested() = 0;

  /**
   * Wait until given number of tuples are available from each given port.
   *
   * This function will block until the wait condition is satisfied, that is 
   * the required number of input tuples are awailable in the port queues of 
   * each of the specified input ports. It will return true, if one or more of 
   * the ports are closed and the wait condition cannot be satisfied anymore. 
   * @param spec the wait spec, that is the required number of tuples from the 
   * specified ports
   * @return true if the wait spec can never be satisfied, false if it is 
   * currently satisfied
   */
   virtual bool waitOnAllPorts(std::unordered_map<InputPort *, size_t> const & spec) = 0;

   /**
    *
    *
   */
   virtual bool waitOnAnyPort(std::unordered_map<InputPort *, size_t> const & spec) = 0;

};

} // namespace streamc
