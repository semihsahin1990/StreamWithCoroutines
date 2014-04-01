#include "streamc/Operator.h"

#include <iostream>
#include <vector>
#include <functional>

namespace streamc { namespace operators
{

/**
 * %Timestamper operator that evaluates tuples against a given condition.
 */
class Timestamper : public Operator
{
public:
  /**
   * Construct a Timestamper operator.
   *
   * @param name name of the operator
   */
  Timestamper(std::string const & name);


  /**
   * The process function that puts timestamps to tuples.
   *
   * @param context the operator context
   */
  void process(OperatorContext & context) override;
};

} } // namespace streamc::operators


