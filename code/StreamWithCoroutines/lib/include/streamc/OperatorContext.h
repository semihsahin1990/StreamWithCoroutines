#pragma once

#include <cstddef>

namespace streamc
{

class InputPort;
class OutputPort;

/**
 * %Operator context that provides runtime services for operators.
 *
 * These services include acccessing input and output ports, and
 * checking whethere shutdown is requested.
 */
class OperatorContext
{
public:
  virtual ~OperatorContext() {}

  /**
   * Get the input port at a given index
   *
   * @param inputPort the input port index
   * @return the input port object
   */
  virtual InputPort & getInputPort(size_t inputPort) = 0;

  /**
   * Get the output port at a given index
   *
   * @param outputPort the input port index
   * @return the input port object
   */
  virtual OutputPort & getOutputPort(size_t outputPort) = 0;

  /**
   * Check whether shutdown is requested
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
};

} // namespace streamc
