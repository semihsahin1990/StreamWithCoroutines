#include "streamc/operators/FileSource.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

using namespace streamc;
using namespace std;

FileSource::FileSource(std::string const & name, std::string const & fileName,
                       std::unordered_map<std::string, Type> const & attributes)
  : Operator(name, 0, 1), fileName_(fileName), attributes_(attributes) 
{}

void FileSource::init(OperatorContext & context)
{
  oport_ = & context.getOutputPort(0);
}

void FileSource::process(OperatorContext & context)
{
  Tuple tuple;
  string line;
  ifstream input;
  input.open(fileName_.c_str(), ios::in);
  std::regex sep(",");
  while (!context.isShutdownRequested()) {
    input >> line;
    if(input.eof())
      break;   
    sregex_token_iterator tokenIt(line.begin(), line.end(), sep, -1);
    for (auto it=attributes_.begin(); it!=attributes_.end(); ++it, ++tokenIt) {
      string const & name = it->first;
      Type type = it->second;
      string const & token = *tokenIt;
      tuple.setAttribute(name, Value::fromString(token, type));
    }
    oport_->pushTuple(tuple);
  } 
}
