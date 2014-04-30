#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc { namespace operators
{

/**
 * %Barrier operator that combines tuples from different inputs.
 */
class Barrier : public Operator
{
public:
  /**
   * Construct a barrier operator.
   *
   * By default, the barrier will have two input ports.
   * @param name name of the operator
   */  
  Barrier(std::string const & name);

  /**
   * Construct a barrier operator.
   *
   * @param name name of the operator
   * @param numInputs the number of input ports 
   */  
  Barrier(std::string const & name, int numInputs);

  /**
   * The process function that contains the barrier logic.
   *
   * @param context the operator context
   */  
  void process(OperatorContext & context) override;

  /**
   * Clone the operator 
   * 
   * @return the clone of the operator
   */
  Barrier * clone() override;
};

} } // namespace streamc::operators


