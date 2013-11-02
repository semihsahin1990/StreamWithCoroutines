#include "streamc/Operator.h"

#include <iostream>
#include <unordered_map>

namespace streamc 
{

class FileSource : public Operator
{
public:
  FileSource(std::string const & name, std::string const & fileName,
             std::unordered_map<std::string, Type> const & attributes);  
  void init(OperatorContext & context);
  void process(OperatorContext & context);
private:
  std::string fileName_;
  std::unordered_map<std::string, Type> attributes_;
  OutputPort * oport_;

};

} // namespace streamc
 

