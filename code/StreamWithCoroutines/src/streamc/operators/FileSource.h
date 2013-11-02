#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc 
{

class FileSource : public Operator
{
private:
  std::string fileName_;
  std::unordered_map<std::string, Type> attributes_;
  OutputPort * oport_;
public:
  FileSource(std::string const & name, std::string const & fileName,
             std::unordred_map<std::string, Type> const & attributes);  
protected:
  void init(OperatorContext & context);
  void process(OperatorContext & context);
};

}


