#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc 
{

class InputPort;

class FileSink : public Operator
{
public:
  FileSink(std::string const & name, std::string const & fileName);
  void init(OperatorContext & context);
  void process(OperatorContext & context);
private:
  std::string fileName_;
  InputPort * iport_;
};

} // namespace streamc


