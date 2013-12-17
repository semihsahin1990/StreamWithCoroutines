#include "streamc/Flow.h"
#include "streamc/FlowRunner.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/FileSink.h"
#include "streamc/operators/Filter.h"
#include "streamc/operators/Union.h"

#include <cstdlib>
#include <thread>
#include <chrono>

using namespace std;
using namespace streamc;
using namespace streamc::operators;
using namespace streamc::connectors;

int main()
{
  Flow flow("simple union");
  
  Operator & src1 = flow.createOperator<FileSource>("src1")
    .set_fileName("data/in1.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});
  
  Operator & src2 = flow.createOperator<FileSource>("src2")
    .set_fileName("data/in2.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});
  
  Operator & src3 = flow.createOperator<FileSource>("src3")
    .set_fileName("data/in3.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});
  
  Operator & src4 = flow.createOperator<FileSource>("src4")
    .set_fileName("data/in4.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});

  Operator & myUnion = flow.createOperator<Union>("union", 3);

  Operator & snk = flow.createOperator<FileSink>("snk")
    .set_fileName("data/out.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});
  
  flow.addConnection( (src1,0) >> (0,myUnion));
  flow.addConnection( (src2,0) >> (1,myUnion));
  flow.addConnection( (src3,0) >> (1,myUnion));
  flow.addConnection( (src4,0) >> (2,myUnion));
  flow.addConnection( (myUnion,0) >> (0,snk));
  
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


