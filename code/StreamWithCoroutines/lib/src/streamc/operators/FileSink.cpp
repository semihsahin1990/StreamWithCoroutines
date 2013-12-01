#include "streamc/operators/FileSink.h"

#include "streamc/Logger.h"

#include <iostream>
#include <fstream>
#include <regex>

using namespace streamc;
using namespace streamc::operators;
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

void FileSink::process(OperatorContext & context)
{
  ofstream output;
  output.open(fileName_.c_str(), ios::out);
  if (!output) {
    SC_APPLOG(Error, "Error in opening output file: " << fileName_ << ", details: " << strerror(errno));
    return;
  }
  InputPort & iport = context.getInputPort(0);
  while (!context.isShutdownRequested()) {
    bool closed = iport.waitTuple();
    if (closed)
      break;
    Tuple & tuple = iport.getFrontTuple();
    auto const & attributes = tuple.getAttributes();
    if (!attributes.empty()) {
      auto it=attributes.begin();
      output << Value::toString(*(it->second));
      for (++it; it!=attributes.end(); ++it) 
        output << "," << Value::toString(*(it->second));
    }
    output << "\n";
    iport.popTuple();
  } 
}
