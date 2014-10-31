#include "streamc/operators/TupleGenerator.h"

#include "streamc/Logger.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

using namespace streamc;
using namespace streamc::operators;
using namespace std;

TupleGenerator::TupleGenerator(std::string const & name, size_t count)
  : Operator(name, 0, 1), count_(count)
{}

void TupleGenerator::process(OperatorContext & context)
{
  OutputPort & oport = context.getOutputPort(0);
  vector<string> names = {"Semih", "Bugra", "Ahmet", "Mehnet", "Aslı", "Yasemin", "Vehbi"};
  vector<string> grades = {"A", "B", "C", "D", "F", "A", "C"};

  for(size_t i=0; i<count_; i++) {
    Tuple tuple;
    tuple.setAttribute("name", names[i%7]);
    tuple.setAttribute("grade", grades[i%7]);
    tuple.setAttribute("lineNo", (int64_t)i);
    oport.pushTuple(tuple);
  }
}
