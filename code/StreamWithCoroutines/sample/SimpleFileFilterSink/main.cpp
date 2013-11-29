#include "streamc/Flow.h"
#include "streamc/FlowRunner.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/FileSink.h"
#include "streamc/operators/Filter.h"

#include <cstdlib>
#include <thread>
#include <chrono>

using namespace std;
using namespace streamc;

int main()
{
  Flow flow("simple file filtering");
  
  Operator & src = flow.createOperator<FileSource>("src")
    .set_fileName("data/in.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});

  Operator & fltF = flow.createOperator<Filter>("fltF")
    .set_filter(MEXP1( t_.get<Type::String>("grade") != "F" ));

  Operator & fltG = flow.createOperator<Filter>("fltG")
    .set_filter(MEXP1( t_.get<Type::String>("name") != "Bugra" ));

  Operator & snk = flow.createOperator<FileSink>("snk")
    .set_fileName("data/out.dat"); 
  	
  flow.addConnections( (src,0) >> (0,fltF,0) >> (0,fltG,0) >> (0,snk) ); 
  // flow.printTopology(std::cout);

  // alternatives:
  // flow.addConnection( (src,0) >> (0,flt) ); 
  // flow.addConnection( (flt,0) >> (0,snk) ); 
  // flow.addConnection(src, 0, flt, 0);

  FlowRunner & runner = FlowRunner::createRunner();
  runner.setInfrastructureLogLevel(Info);
  runner.setApplicationLogLevel(Trace);
  runner.run(flow, 2);
  runner.wait(flow);
  
  /* 
  // alternative:
  this_thread::sleep_for(chrono::milliseconds(500));
  runner.requestShutdown(flow); 
  runner.wait(flow); 
  */
  
  return EXIT_SUCCESS;
}


