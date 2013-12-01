#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc { namespace operators
{

/**
 * %Barrier operator that combines tupels from different inputs.
 */
class Barrier : public Operator
{
public:
  /**
   * Construct a barrier operator.
   *
   * @param name name of the operator
   */  
  Barrier(std::string const & name);

  /**
   * The process function that contains the barrier logic.
   *
   * @param context the operator context
   */  
  void process(OperatorContext & context) override;
};

} } // namespace streamc::operators


