#include "streamc/Flow.h"
#include "streamc/FlowRunner.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/FileSink.h"
#include "streamc/operators/Filter.h"

using namespace std;
using namespace streamc;

int main()
{
  Flow flow("simple file filtering");
  
  Operator & src = flow.createOperator<FileSource>("src")
    .set_fileName("in.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});

  Operator & fltF = flow.createOperator<Filter>("fltF")
    .set_filter(MEXP1( t_.get<Type::String>("grade") != "F" ));

  Operator & fltG = flow.createOperator<Filter>("fltG")
    .set_filter(MEXP1( t_.get<Type::String>("name") != "Gil" ));

  Operator & snk = flow.createOperator<FileSink>("snk")
    .set_fileName("out.dat"); 
  	
  flow.addConnections( (src,0) >> (0,fltF,0) >> (0,fltG,0) >> (0,snk) ); 

  // alternative:
  // flow.addConnection( (src,0) >> (0,flt) ); 
  // flow.addConnection( (flt,0) >> (0,snk) ); 
  // flow.addConnection(src, 0, flt, 0);

  flow.printTopology(std::cout);
	  
  FlowRunner & runner = FlowRunner::createRunner();
  runner.run(flow, 5);
  runner.wait(flow);
  
  /*alternative:
  sleep(10); 
  runner.requestShutdown(flow); 
  runner.wait(flow);        
  */
}


