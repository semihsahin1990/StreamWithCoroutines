#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc 
{

class FileSink : public Operator
{
private:
  std::string fileName_;
  InputPort * iport_;
public:
  FileSink(std::string const & name, std::string const & fileName);
protected:
  void init(OperatorContext & context);
  void process(OperatorContext & context);
};

}


