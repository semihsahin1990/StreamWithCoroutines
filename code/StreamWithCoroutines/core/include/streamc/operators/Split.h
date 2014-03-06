#include "streamc/Operator.h"

#include <iostream>
#include <vector>
#include <functional>

namespace streamc { namespace operators
{

/**
 * %Split operator that splits tuples with a given hash function.
 */
class Split : public Operator
{
public:
  /**
   * Construct a split operator.
   *
   * @param name name of the operator
   */
  Split(std::string const & name);

  /**
   * Construct a split operator.
   *
   * @param name name of the operator
   * @param numOutputs number of the output ports
   */
  Split(std::string const & name, int numOutputs);

    /**
   * Construct a split operator.
   *
   * @param name name of the operator
   * @param hashFunction the hash function
   */
  Split(std::string const & name, std::function<int (Tuple &)> hashFunction);

    /**
   * Construct a split operator.
   *
   * @param name name of the operator
   * @param numOutputs number of the output ports
   * @param hashFunction the hash function
   */
  Split(std::string const & name, int numOutputs, std::function<int (Tuple &)> hashFunction);

  /**
   * Set the hash function parameter.
   *
   * @param hashFunction the hash function
   * @return self
   */
  Split & set_hashFunction(std::function<int (Tuple &)> hashFunction);

  /**
   * The process function that contains the split logic.
   *
   * @param context the operator context
   */
  void process(OperatorContext & context) override;
private:
  std::function<int (Tuple &)> hashFunction_;
};

} } // namespace streamc::operators


