#pragma once

#include <inttypes.h>

namespace streamc
{

class Tuple;

/**
 * Output port that provides tuple sending services to operators.
 *
 */
class OutputPort
{
public:
  virtual ~OutputPort() {}

  /**
   * Send the tuple to downstream opertors.
   *
   * The operation may block if downstream input ports are full
   * or if the system decides to preempt the operator.
   */
  virtual void pushTuple(Tuple const & tuple) = 0;
};

} // namespace streamc
