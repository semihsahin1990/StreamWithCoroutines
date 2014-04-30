#include "streamc/Operator.h"

#include <iostream>
#include <vector>
#include <functional>

namespace streamc { namespace operators
{

/**
 * %RoundRobinMerge operator that RoundRobinMerges tuples according to the RoundRobinMergeBy.
 */
class RoundRobinMerge : public Operator
{
public:
  /**
   * Construct a RoundRobinMerge operator.
   *
   * @param name name of the operator
   */
  RoundRobinMerge(std::string const & name);

  /**
   * Construct a RoundRobinMerge operator.
   *
   * @param name name of the operator
   * @param numInputs number of the input ports
   */
  RoundRobinMerge(std::string const & name, int numInputs);

  void process(OperatorContext & context) override;

 /**
   * Clone the operator 
   * 
   * @return the clone of the operator
   */
  RoundRobinMerge * clone() override;

private:
};

} } // namespace streamc::operators


