#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc 
{

class FileSource : public Operator
{
public:
  // constructor with name
  FileSource(std::string const & name);
  // constructor with name and fileName
  FileSource(std::string const & name, std::string const & fileName);
  // constructor with name, fileName and fileFormat (fileFormat indicates 'name' and 'type' of a Tuple attribute)
  FileSource(std::string const & name, std::string const & fileName,
             std::vector<std::pair<std::string, Type>> const & attributes);  
  // set fileName
  FileSource & set_fileName(std::string const & fileName);
  FileSource & set_fileFormat(std::vector<std::pair<std::string, Type>> const & attributes);
  // init and process functions of base 'Operator' class
  void init(OperatorContext & context) override;
  void process(OperatorContext & context) override;
private:
  // properties of FileSource
  std::string fileName_;
  std::vector<std::pair<std::string, Type>> attributes_;
  OutputPort * oport_;
};

} // namespace streamc
 

