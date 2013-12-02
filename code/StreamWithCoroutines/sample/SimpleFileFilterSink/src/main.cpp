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
using namespace streamc::operators;
using namespace streamc::connectors;

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

  // run with two threads
  runner.run(flow, 2);
  // wait for quarter of a second
  this_thread::sleep_for(chrono::milliseconds(250)); 
  // request the flow to be shutdown
  runner.requestShutdown(flow); 
  // wait until shutdown
  runner.wait(flow); 

  // let's see how many lines are there
  system("wc -l data/out.dat");

  // run the flow again (it will resume from where it left)
  // this time, use three threads
  runner.run(flow, 3);
  // wait for completion (this time we don't explicity ask for shutdwon, it
  // completes when there is no more work to do)
  runner.wait(flow);
  
  // let's see how many lines are there now (should be a total of 72000)
  system("wc -l data/out.dat");

  return EXIT_SUCCESS;
}


