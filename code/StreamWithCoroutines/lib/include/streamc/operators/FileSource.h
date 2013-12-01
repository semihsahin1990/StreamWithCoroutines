#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc { namespace operators
{

/**
 * File source that reads tuples from a file.
 */
class FileSource : public Operator
{
public:
  /**
   * Construct a file source.
   *
   * @param name name of the operator
   */
  FileSource(std::string const & name);

  /**
   * Construct a file source.
   *
   * @param name name of the operator
   * @param fileName name of the file to read from
   */
  FileSource(std::string const & name, std::string const & fileName);

  /**
   * Construct a file source.
   *
   * @param name name of the operator
   * @param fileName name of the file to read from
   * @param attributes name and types of the attributes of the tuples 
   */
  FileSource(std::string const & name, std::string const & fileName,
             std::vector<std::pair<std::string, Type>> const & attributes);  

  /**
   * Set the file name.
   *
   * @param fileName name of the file to write to
   * @return self
   */
  FileSource & set_fileName(std::string const & fileName);

  /**
   * Set the file format.
   *
   * @param attributes name and types of the attributes of the tuples 
   * @return self
   */
  FileSource & set_fileFormat(std::vector<std::pair<std::string, Type>> const & attributes);

  /**
   * The process function that contains the file source logic.
   *
   * @param context the operator context
   */
  void process(OperatorContext & context) override;
private:
  std::string fileName_;
  std::vector<std::pair<std::string, Type>> attributes_;
};

} } // namespace streamc::operators
 

