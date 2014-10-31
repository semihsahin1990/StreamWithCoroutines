#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc { namespace operators
{

/**
 * File source that reads tuples from a file.
 */
class TupleGenerator : public Operator
{
public:
  /**
   * Construct a file source.
   *
   * @param name name of the operator
   */
  TupleGenerator(std::string const & name, size_t count);

  /**
   * The process function that contains the file source logic.
   *
   * @param context the operator context
   */
  void process(OperatorContext & context) override;

private:
  size_t count_;
};

} } // namespace streamc::operators
 

