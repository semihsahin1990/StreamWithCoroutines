#include "streamc/Operator.h"

#include <iostream>
#include <vector>
#include <functional>

namespace streamc { namespace operators
{

/**
 * %Merge operator that merges tuples according to the mergeBy.
 */
class Merge : public Operator
{
public:
  /**
   * Construct a merge operator.
   *
   * @param name name of the operator
   */
  Merge(std::string const & name);

  /**
   * Construct a merge operator.
   *
   * @param name name of the operator
   * @param numInputs number of the input ports
   */
  Merge(std::string const & name, int numInputs);

  void process(OperatorContext & context) override;

 /**
   * Clone the operator 
   * 
   * @return the clone of the operator
   */
  Merge * clone(std::string const & name) override;

private:
};

} } // namespace streamc::operators


