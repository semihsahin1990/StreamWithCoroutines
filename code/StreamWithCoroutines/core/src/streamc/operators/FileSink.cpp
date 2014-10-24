#include "streamc/operators/FileSink.h"

#include "streamc/Logger.h"

#include <iostream>
#include <fstream>
#include <regex>

using namespace streamc;
using namespace streamc::operators;
using namespace std;

FileSink::FileSink(std::string const & name, std::string const & fileName,
                       std::vector<std::pair<std::string, Type>> const & attributes)
  : Operator(name, 1, 0), fileName_(fileName), attributes_(attributes) 
{}

FileSink::FileSink(std::string const & name, std::string const & fileName)
  : FileSink(name, fileName, {})
{}

FileSink::FileSink(std::string const & name)
  : FileSink(name, "", {})
{}

FileSink & FileSink::set_fileName(std::string const & fileName)
{
  fileName_ = fileName;
  return *this;
}

FileSink & FileSink::set_fileFormat(std::vector<std::pair<std::string, Type>> const & attributes)
{
  attributes_ = attributes;
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
  output.open(fileName_.c_str(), (filePos_>0) ? (ios::out|ios::app) : ios::out);
  if (!output) {
//   SC_APPLOG(Error, "Error in opening output file: " << fileName_ << ", details: " << strerror(errno));
    return;
  }
  output.seekp(filePos_);
  if (!output) {
  //  SC_APPLOG(Error, "Error in seeking to location: " << filePos_ << ", in output file: " << fileName_ << ", details: "<< strerror(errno));
    return;
  }
  InputPort & iport = context.getInputPort(0);
  while (!context.isShutdownRequested()) {
    bool closed = iport.waitTuple();
    if (closed)
      break;

    Tuple & tuple = iport.getFrontTuple();
    auto const & tupleAttributes = tuple.getAttributes();
    if (!tupleAttributes.empty()) {
      auto it = attributes_.begin();
      output << Value::toString(*(tupleAttributes.at((*it).first)));
      for(++it; it!=attributes_.end(); ++it)
        output << "," << Value::toString(*(tupleAttributes.at((*it).first)));
    }
    output << "\n";
    iport.popTuple();
    filePos_ = output.tellp();
  } 
}

FileSink * FileSink::clone(std::string const & name) 
{
  return new FileSink(name, fileName_, attributes_);
}


