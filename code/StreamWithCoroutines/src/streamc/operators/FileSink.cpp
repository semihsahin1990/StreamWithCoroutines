#include "streamc/operators/FileSink.h"

#include <iostream>
#include <fstream>
#include <regex>

using namespace streamc;
using namespace std;

//constructor with name and fileName
FileSink::FileSink(std::string const & name, std::string const & fileName)
  : Operator(name, 1, 0), fileName_(fileName)
{}

//constructor with name, fileName is initialized with ""
FileSink::FileSink(std::string const & name)
  : FileSink(name, "")
{}

//set fileName
FileSink & FileSink::set_fileName(std::string const & fileName)
{
  fileName_ = fileName;
  return *this;
}

//init FileSink with OperatorContext
void FileSink::init(OperatorContext & context)
{
  iport_ = & context.getInputPort(0);
}

//wait for tuple, get it. write its all attribute values to the file.
void FileSink::process(OperatorContext & context)
{
  ofstream output;
  output.open(fileName_.c_str(), ios::out);
  while (!context.isShutdownRequested()) {
    bool closed = iport_->waitTuple();
    if (closed)
      break;
    Tuple & tuple = iport_->getFrontTuple();
    auto const & attributes = tuple.getAttributes();
    if (!attributes.empty()) {
      auto it=attributes.begin();
      output << Value::toString(*(it->second));
      for (++it; it!=attributes.end(); ++it) 
        output << "," << Value::toString(*(it->second));
    }
    output << "\n";
    iport_->popTuple();
  } 
}
