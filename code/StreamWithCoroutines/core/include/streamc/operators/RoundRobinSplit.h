#include "streamc/Operator.h"

#include <iostream>
#include <vector>
#include <functional>

namespace streamc { namespace operators
{

/**
 * %RoundRobinSplit operator that RoundRobinSplits tuples with a given hash function.
 */
class RoundRobinSplit : public Operator
{
public:
  /**
   * Construct a RoundRobinSplit operator.
   *
   * @param name name of the operator
   */
  RoundRobinSplit(std::string const & name);

  /**
   * Construct a RoundRobinSplit operator.
   *
   * @param name name of the operator
   * @param numOutputs number of the output ports
   */
  RoundRobinSplit(std::string const & name, int numOutputs);

  /**
   * The process function that contains the RoundRobinSplit logic.
   *
   * @param context the operator context
   */
  void process(OperatorContext & context) override;

 /**
   * Clone the operator 
   * 
   * @return the clone of the operator
   */
  RoundRobinSplit * clone(std::string const & name) override;


private:
  std::function<int (Tuple &)> hashFunction_;
};

} } // namespace streamc::operators


