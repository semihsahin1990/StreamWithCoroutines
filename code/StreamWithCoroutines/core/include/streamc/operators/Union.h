#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc { namespace operators
{

/**
 * %Union operator that receives tuples from different inputs.
 */
class Union : public Operator
{
public:
  /**
   * Construct a union operator.
   *
   * By default, the union will have two input ports.
   * @param name name of the operator
   */  
  Union(std::string const & name);

  /**
   * Construct a union operator.
   *
   * @param name name of the operator
   * @param numInputs the number of input ports 
   */  
  Union(std::string const & name, int numInputs);

  /**
   * The process function that contains the union logic.
   *
   * @param context the operator context
   */  
  void process(OperatorContext & context) override;

 /**
   * Clone the operator 
   * 
   * @return the clone of the operator
   */
  Union * clone() override;

};

} } // namespace streamc::operators


