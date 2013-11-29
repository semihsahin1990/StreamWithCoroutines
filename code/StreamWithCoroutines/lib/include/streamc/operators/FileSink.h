#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc 
{

class FileSink : public Operator
{
public:
  FileSink(std::string const & name);
  FileSink(std::string const & name, std::string const & fileName);
  FileSink & set_fileName(std::string const & fileName);

  void init(OperatorContext & context) override;
  void process(OperatorContext & context) override;

private:
  std::string fileName_;
  InputPort * iport_;
};

} // namespace streamc


