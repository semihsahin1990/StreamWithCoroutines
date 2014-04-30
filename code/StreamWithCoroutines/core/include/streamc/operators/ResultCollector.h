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
  ResultCollector(std::string const & name, std::string const & fileName);


  /**
   * The process function that puts timestamps to tuples.
   *
   * @param context the operator context
   */
  void process(OperatorContext & context) override;

 /**
   * Clone the operator 
   * 
   * @return the clone of the operator
   */
  ResultCollector * clone() override;

private:
  std::string fileName_;
  size_t tupleCounter_;
  uint64_t minLatency_, maxLatency_;
  double mean_, deviation_;
};

} } // namespace streamc::operators


