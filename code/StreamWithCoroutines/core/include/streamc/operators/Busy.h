#include "streamc/Operator.h"

#include <iostream>
#include <vector>
#include <functional>

namespace streamc { namespace operators
{

/**
 * %busy operator
 */
class Busy : public Operator
{
public:
  /**
   * Construct a busy operator.
   *
   * @param name name of the operator
   */
  Busy(std::string const & name);

  /**
   * Construct a busy operator.
   *
   * @param name name of the operator
   * @param busyTimeMicrosecs
   */
  Busy(std::string const & name, uint64_t busyTimeMicrosecs);

  /**
   * The process function that contains the busy logic.
   *
   * @param context the operator context
   */
  void process(OperatorContext & context) override;

  /**
   * Clone the operator 
   * 
   * @return the clone of the operator
   */
  Busy * clone(std::string const & name) override;

private:
  uint64_t busyTimeMicrosecs_;
};

} } // namespace streamc::operators


