#include "streamc/operators/TupleGenerator.h"

#include "streamc/Logger.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

using namespace streamc;
using namespace streamc::operators;
using namespace std;

TupleGenerator::TupleGenerator(std::string const & name,
                       std::vector<std::pair<std::string, Type>> const & attributes)
  : Operator(name, 0, 1), attributes_(attributes) 
{}

TupleGenerator::TupleGenerator(std::string const & name)
  : TupleGenerator(name, {})
{}

TupleGenerator & TupleGenerator::set_tupleFormat(std::vector<std::pair<std::string, Type>> const & attributes)
{
  attributes_ = attributes;
  return *this;
}

void TupleGenerator::process(OperatorContext & context)
{
  Tuple tuple;
 
  size_t counter = 0;
  size_t runCounter = 0;
  OutputPort & oport = context.getOutputPort(0);
  while (!context.isShutdownRequested()) {
    /*
    for (auto it=attributes_.begin(); it!=attributes_.end(); ++it, ++tokenIt) {
      if (tokenIt==sregex_token_iterator()) {
        error = true;
        SC_APPLOG(Error, getName() << "Error in line: " << counter);
        break; // problem with the line
      }
      string const & name = it->first;
      Type type = it->second;
      string const & token = "value";
      tuple.setAttribute(name, Value::fromString(token, type));
      */
      
//      .set_tupleFormat({{"name",Type::String}, {"grade",Type::String}, {"lineNo", Type::Integer}});
    //}
    
    tuple.setAttribute("name", "semih");
    tuple.setAttribute("grade", "A");
    tuple.setAttribute("lineNo", (int64_t)counter++);
    oport.pushTuple(tuple);
    if(counter == 100000) {
      counter = 0;
      runCounter++;
    }

    if(runCounter == 400)
      break;
  } 
}

TupleGenerator * TupleGenerator::clone(std::string const & name)
{
  return new TupleGenerator(name, attributes_);
}

