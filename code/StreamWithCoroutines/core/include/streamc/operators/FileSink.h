#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc { namespace operators
{

/**
 * File sink that writes tuples to a file.
 */
class FileSink : public Operator
{
public:
  /**
   * Construct a file sink.
   *
   * @param name name of the operator
   */
  FileSink(std::string const & name);

  /**
   * Construct a file sink.
   *
   * @param name name of the operator
   * @param fileName name of the file to write to
   */
  FileSink(std::string const & name, std::string const & fileName);

  /**
   * Set the file name.
   *
   * @param fileName name of the file to write to
   * @return self
   */
  FileSink & set_fileName(std::string const & fileName);

  /**
   * The process function that contains the file sink logic.
   *
   * @param context the operator context
   */
  void process(OperatorContext & context) override;

  /**
   * Initialize file reader state.
   * 
   * @param context the operator context
   */
  void initState(OperatorContext & context) override;

  /**
   * Sace the file reader state.
   *
   * @param context the operator context
   */
  void saveState(OperatorContext & context) override;
private:
  // parameters
  std::string fileName_;
  // state
  std::streampos filePos_;
};

} } // namespace streamc::operators


