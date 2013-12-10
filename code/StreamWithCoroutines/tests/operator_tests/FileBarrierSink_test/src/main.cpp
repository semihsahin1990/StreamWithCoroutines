#include "streamc/Flow.h"
#include "streamc/FlowRunner.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/FileSink.h"
#include "streamc/operators/Filter.h"
#include "streamc/operators/Barrier.h"

#include <cstdlib>
#include <thread>
#include <chrono>

using namespace std;
using namespace streamc;
using namespace streamc::operators;
using namespace streamc::connectors;

int main()
{
  Flow flow("simple barrier");
  
  Operator & src1 = flow.createOperator<FileSource>("src1")
    .set_fileName("data/in1.dat")
    .set_fileFormat({{"name1",Type::String}, {"grade1",Type::String}});

  Operator & src2 = flow.createOperator<FileSource>("src2")
    .set_fileName("data/in2.dat")
    .set_fileFormat({{"name2",Type::String}, {"grade2",Type::String}});

  Operator & barrier = flow.createOperator<Barrier>("barrier");

  Operator & snk = flow.createOperator<FileSink>("snk")
    .set_fileName("data/out.dat"); 
  
  flow.addConnections( (src1,0) >> (0,barrier,0) >> (0,snk) ); 
  flow.addConnection( (src2,0) >> (1,barrier));
  
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


