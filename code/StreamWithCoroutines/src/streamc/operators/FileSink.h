#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc 
{

class FileSink : public Operator
{
public:
  //constructor with name
  FileSink(std::string const & name);
  //constructor with name and fileName
  FileSink(std::string const & name, std::string const & fileName);
  //set fileName
  FileSink & set_fileName(std::string const & fileName);

  //virtual init and process functions of base 'Operator' class
  void init(OperatorContext & context) override;
  void process(OperatorContext & context) override;

private:
  //properties of FileSink
  std::string fileName_;
  InputPort * iport_;
};

} // namespace streamc


