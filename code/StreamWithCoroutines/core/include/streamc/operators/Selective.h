#include "streamc/Operator.h"

#include <iostream>
#include <vector>
#include <functional>

namespace streamc { namespace operators
{

/**
 * %Selective operator.
 */
class Selective : public Operator
{
public:
  /**
   * Construct a Selective operator.
   *
   * @param name name of the operator
   */
  Selective(std::string const & name);

  /**
   * Construct a Selective operator.
   *
   * @param name name of the operator
   * @param selectivity selectivity value
   */
  Selective(std::string const & name, double selectivity);

  /**
   * Set the selectivity paramter.
   *
   * @param selectivity selectivity value
   * @return self
   */
  Selective & set_selectivity(double selectivity);

  /**
   * The process function that contains the Selective logic.
   *
   * @param context the operator context
   */
  void process(OperatorContext & context) override;

 /**
   * Clone the operator 
   * 
   * @return the clone of the operator
   */
  Selective * clone() override;


private:
  double selectivity_;
};

} } // namespace streamc::operators


