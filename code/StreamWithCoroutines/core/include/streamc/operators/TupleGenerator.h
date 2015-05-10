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
  TupleGenerator(std::string const & name);

  /**
   * Construct a tuple generator.
   *
   * @param name name of the operator
   * @param attributes name and types of the attributes of the tuples 
   */
  TupleGenerator(std::string const & name,
             std::vector<std::pair<std::string, Type>> const & attributes);  

  /**
   * Set the tuple format.
   *
   * @param attributes name and types of the attributes of the tuples 
   * @return self
   */
  TupleGenerator & set_tupleFormat(std::vector<std::pair<std::string, Type>> const & attributes);

  /**
   * The process function that contains the file source logic.
   *
   * @param context the operator context
   */
  void process(OperatorContext & context) override;

  /**
   * Clone the operator 
   * 
   * @return the clone of the operator
   */
  TupleGenerator * clone(std::string const & name) override;
private:
  // parameters
  std::vector<std::pair<std::string, Type>> attributes_;
};

} } // namespace streamc::operators
 

