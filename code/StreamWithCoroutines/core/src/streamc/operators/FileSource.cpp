#include "streamc/operators/FileSource.h"

#include "streamc/Logger.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

using namespace streamc;
using namespace streamc::operators;
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

void FileSource::initState(OperatorContext & context)
{
  Tuple & store = context.getStateStore();
  filePos_ = 0;
  if (store.hasAttribute("pos")) {
    int64_t pos = store.get<Type::Integer>("pos");
    filePos_ = static_cast<streampos>(pos);
  } 
}

void FileSource::saveState(OperatorContext & context) 
{
  Tuple & store = context.getStateStore();
  int64_t pos = static_cast<streampos>(filePos_);
  store.setAttribute("pos", pos);
}

void FileSource::process(OperatorContext & context)
{
  Tuple tuple;
  string line;
  std::regex sep(",");
  ifstream input;
  input.open(fileName_.c_str(), ios::in);
  if (!input) {
    SC_APPLOG(Error, "Error in opening input file: " << fileName_ << ", details: "<< strerror(errno));
    return;
  }
  input.seekg(filePos_);
  if (!input) {
    SC_APPLOG(Error, "Error in seeking to location: " << filePos_ << ", in input file: " << fileName_ << ", details: "<< strerror(errno));
    return;
  }
  size_t counter = 0;
  OutputPort & oport = context.getOutputPort(0);
  while (!context.isShutdownRequested()) {
    line.clear();
    getline(input, line);
    if (!input && !input.eof()) {
      SC_APPLOG(Error, "Error in reading from input file: " << fileName_ << ", details: "<< strerror(errno));
      break;        
    }
    if(line.size()==0 && input.eof())
      break;   
    SC_APPLOG(Trace, getName() << " Read line: " << counter);
    sregex_token_iterator tokenIt(line.begin(), line.end(), sep, -1);
    bool error = false;
    for (auto it=attributes_.begin(); it!=attributes_.end(); ++it, ++tokenIt) {
      if (tokenIt==sregex_token_iterator()) {
        error = true;
        SC_APPLOG(Error, getName() << "Error in line: " << counter);
        break; // problem with the line
      }
      string const & name = it->first;
      Type type = it->second;
      string const & token = *tokenIt;
      tuple.setAttribute(name, Value::fromString(token, type));
    }
    if (!error)
      oport.pushTuple(tuple);
    filePos_ = input.tellg();
  } 
}

FileSource * FileSource::clone(std::string const & name)
{
  return new FileSource(name, fileName_, attributes_);
}

