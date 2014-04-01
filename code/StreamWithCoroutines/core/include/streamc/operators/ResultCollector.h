#include "streamc/Operator.h"

#include <iostream>
#include <vector>
#include <functional>

namespace streamc { namespace operators
{

/**
 * %ResultCollector operator that evaluates tuples against a given condition.
 */
class ResultCollector : public Operator
{
public:
  /**
   * Construct a ResultCollector operator.
   *
   * @param name name of the operator
   */
  ResultCollector(std::string const & name);


  /**
   * The process function that puts timestamps to tuples.
   *
   * @param context the operator context
   */
  void process(OperatorContext & context) override;
private:
  size_t tupleCounter_;
  uint64_t totalLatency_;
  uint64_t maxLatency_;
};

} } // namespace streamc::operators


