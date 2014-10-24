#include "streamc/Operator.h"

#include <iostream>
#include <vector>
#include <functional>

namespace streamc { namespace operators
{

/**
 * %Filter operator that evaluates tuples against a given condition.
 */
class Filter : public Operator
{
public:
  /**
   * Construct a filter operator.
   *
   * @param name name of the operator
   */
  Filter(std::string const & name);

  /**
   * Construct a filter operator.
   *
   * @param name name of the operator
   * @param filter the filter condition
   */
  Filter(std::string const & name, std::function<bool (Tuple &)> filter);

  /**
   * Set the filter paramter.
   *
   * @param filter the filter paramter
   * @return self
   */
  Filter & set_filter(std::function<bool (Tuple &)> filter);

  /**
   * The process function that contains the filter logic.
   *
   * @param context the operator context
   */
  void process(OperatorContext & context) override;

/**
   * Clone the operator 
   * 
   * @return the clone of the operator
   */
  Filter * clone(std::string const & name) override;

private:
  std::function<bool (Tuple &)> filter_;
};

} } // namespace streamc::operators


