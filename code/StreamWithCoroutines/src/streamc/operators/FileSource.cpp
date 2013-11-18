#include "streamc/operators/FileSource.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

using namespace streamc;
using namespace std;

// constructor with name, fileName and fileFormat
FileSource::FileSource(std::string const & name, std::string const & fileName,
                       std::vector<std::pair<std::string, Type>> const & attributes)
  : Operator(name, 0, 1), fileName_(fileName), attributes_(attributes) 
{}

// constructor with name and fileName, init attributes as {}
FileSource::FileSource(std::string const & name, std::string const & fileName)
  : FileSource(name, fileName, {})
{}

// constructor with name, init fileName as "", attributes as {}
FileSource::FileSource(std::string const & name)
  : FileSource(name, "", {})
{}

// set fileName
FileSource & FileSource::set_fileName(std::string const & fileName)
{
  fileName_ = fileName;
  return *this;
}


// set file format
FileSource & FileSource::set_fileFormat(std::vector<std::pair<std::string, Type>> const & attributes)
{
  attributes_ = attributes;
  return *this;
}

// init FileSource with OperatorContext
void FileSource::init(OperatorContext & context)
{
  oport_ = & context.getOutputPort(0);
}

/*
read line from inputFile until EOF or shutDownRequested
for each line:
  create tuple of attributes
  split line into tokens with separator ','
  for each token: 
    create attribute: (name, Value(token, type) pair
      set name as a corresponding name from atributes_ (file format)
      set token as a value from the file
      set type as a corresponding type from attributes_ (file format)
	 add attribute to the tuple
  push 'tuple' to the output port
*/
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

