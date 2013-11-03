#include "streamc/Flow.h"
#include "streamc/FlowRunner.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/FileSink.h"
#include "streamc/operators/Filter.h"

using namespace std;
using namespace streamc;

int main()
{
  // operator graph
  Flow flow("simple file filtering");
  
  // a source operator that reads from a file
  unordered_map<string,Type> format = {{"name",Type::String}, {"grade",Type::String}};
  Operator & src = flow.createOperator<FileSource>("src",    // op name
                                                   "in.dat", // in file
                                                   format);  // file format

  // a filter operator that drops F grades 
  auto filter = [] (Tuple & t) { return get<Type::String>(t, "grade") != "F"; };
  Operator & flt = flow.createOperator<Filter>("flt",   // op name
                                               filter); // filter

  // a sink operator that writes to a file
  Operator & snk = flow.createOperator<FileSink>("snk",      // op name 
                                                 "out.dat"); // out file
  
  // connections

  // output 0 of src connects to intput 0 of flt
  flow.addConnection( (src,0) >> (0,flt) ); 

  // output 0 of flt connects to input 0 of snk
  flow.addConnection( (flt,0) >> (0,snk) ); 
	
  flow.printTopology(std::cout);
	  
  // run it!
  FlowRunner & runner = FlowRunner::createRunner();
  runner.run(flow, 5);
  runner.wait(flow);
  
  /*alternative:
  sleep(10); 
  runner.requestShutdown(flow); 
  runner.wait(flow);        
  */
}


