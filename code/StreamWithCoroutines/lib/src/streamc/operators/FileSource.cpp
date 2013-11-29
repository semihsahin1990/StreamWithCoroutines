#include "streamc/operators/FileSource.h"

#include "streamc/Logger.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

using namespace streamc;
using namespace std;

FileSource::FileSource(std::string const & name, std::string const & fileName,
                       std::vector<std::pair<std::string, Type>> const & attributes)
  : Operator(name, 0, 1), fileName_(fileName), attributes_(attributes) 
{}

FileSource::FileSource(std::string const & name, std::string const & fileName)
  : FileSource(name, fileName, {})
{}

FileSource::FileSource(std::string const & name)
  : FileSource(name, "", {})
{}

FileSource & FileSource::set_fileName(std::string const & fileName)
{
  fileName_ = fileName;
  return *this;
}

FileSource & FileSource::set_fileFormat(std::vector<std::pair<std::string, Type>> const & attributes)
{
  attributes_ = attributes;
  return *this;
}

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
    getline(input, line);
    SC_APPLOG(Trace, "Read line: " << line);
    if(input.eof())
      break;   
    sregex_token_iterator tokenIt(line.begin(), line.end(), sep, -1);
    bool error = false;
    for (auto it=attributes_.begin(); it!=attributes_.end(); ++it, ++tokenIt) {
      if (tokenIt==sregex_token_iterator()) {
        error = true;
        SC_APPLOG(Error, "Error in line: " << line);
        break; // problem with the line        
      }
      string const & name = it->first;
      Type type = it->second;
      string const & token = *tokenIt;
      tuple.setAttribute(name, Value::fromString(token, type));
    }
    oport_->pushTuple(tuple);
  } 
}

