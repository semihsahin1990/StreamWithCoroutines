#include "streamc/Operator.h"

#include <iostream>
#include <unordered_map>

namespace streamc 
{

class FileSource : public Operator
{
public:
  FileSource(std::string const & name);
  FileSource(std::string const & name, std::string const & fileName);
  FileSource(std::string const & name, std::string const & fileName,
             std::unordered_map<std::string, Type> const & attributes);  
  FileSource & set_fileName(std::string const & fileName);
  FileSource & set_fileFormat(std::unordered_map<std::string, Type> const & attributes);
  void init(OperatorContext & context) override;
  void process(OperatorContext & context) override;
private:
  std::string fileName_;
  std::unordered_map<std::string, Type> attributes_;
  OutputPort * oport_;

};

} // namespace streamc
 

