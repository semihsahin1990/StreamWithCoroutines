#include "streamc/operators/FileSink.h"

#include "streamc/Tuple.h"
#include "streamc/InputPort.h"
#include "streamc/OperatorContext.h"

#include <iostream>
#include <fstream>
#include <regex>

using namespace streamc;
using namespace std;

FileSink::FileSink(std::string const & name, std::string const & fileName)
  : Operator(name, 1, 0), fileName_(fileName)
{}

void FileSink::init(OperatorContext & context)
{
  iport_ = & context.getInputPort(0);
}

void FileSink::process(OperatorContext & context)
{
  ofstream output;
  output.open(fileName_.c_str(), ios::out);
  while (!context.isShutdownRequested()) {
    iport_->waitTuple();
    Tuple & tuple = iport_->getFrontTuple();
    auto const & attributes = tuple.getAttributes();
    if (!attributes.empty()) {
      auto it=attributes.begin();
      Value::toString(*(it->second));
      for (++it; it!=attributes.end(); ++it) 
        output << "," << Value::toString(*(it->second));
    }
    iport_->popTuple();
  } 
}
