#include "streamc/operators/FileSink.h"

#include <iostream>
#include <fstream>
#include <regex>

using namespace streamc;
using namespace std;

FileSink::FileSink(std::string const & name, std::string const & fileName)
  : Operator(name, 1, 0), fileName_(fileName)
{}

FileSink::FileSink(std::string const & name)
  : FileSink(name, "")
{}

FileSink & FileSink::set_fileName(std::string const & fileName)
{
  fileName_ = fileName;
  return *this;
}

void FileSink::init(OperatorContext & context)
{
  iport_ = & context.getInputPort(0);
}

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