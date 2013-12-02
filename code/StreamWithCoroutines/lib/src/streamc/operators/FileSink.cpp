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

void FileSink::initState(OperatorContext & context)
{
  Tuple & store = context.getStateStore();
  filePos_ = 0;
  if (store.hasAttribute("pos")) {
    int64_t pos = store.get<Type::Integer>("pos");
    filePos_ = static_cast<streampos>(pos);
  }
}

void FileSink::saveState(OperatorContext & context) 
{
  Tuple & store = context.getStateStore();
  int64_t pos = static_cast<streampos>(filePos_);
  store.setAttribute("pos", pos);
}

void FileSink::process(OperatorContext & context)
{
  ofstream output;
  output.open(fileName_.c_str(), ios::out);
  if (!output) {
    SC_APPLOG(Error, "Error in opening output file: " << fileName_ << ", details: " << strerror(errno));
    return;
  }
  output.seekp(filePos_);
  if (!output) {
    SC_APPLOG(Error, "Error in seeking to location: " << filePos_ << ", in output file: " << fileName_ << ", details: "<< strerror(errno));
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
    filePos_ = output.tellp();
  } 
}
