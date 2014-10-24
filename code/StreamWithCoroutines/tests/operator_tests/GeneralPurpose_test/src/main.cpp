#include <streamc/FlowRunner.h>

#include "streamc/operators/FileSource.h"
#include "streamc/operators/Busy.h"
#include "streamc/operators/FileSink.h"

#include "streamc/topology/Chain.h"
#include "streamc/runtime/RandomScheduling.h"

using namespace std;
using namespace streamc;
using namespace streamc::operators;
using namespace streamc::connectors;

int main(int argc, char *argv[]) {

  Flow flow("GeneralPurposeTest");

  Operator & src = flow.createOperator<FileSource>("src")
    .set_fileName("data/in.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}, {"lineNo", Type::Integer}});

  Operator & busy = flow.createOperator<Busy>("busy", 10);
  
  Operator & snk = flow.createOperator<FileSink>("snk")
    .set_fileName("data/out.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}, {"lineNo", Type::Integer}});

  flow.addConnection((src, 0) >> (0,busy));
  flow.addConnection((busy, 0) >> (0,snk));

  FlowRunner & runner = FlowRunner::createRunner();
  runner.setInfrastructureLogLevel(Info);
  runner.setApplicationLogLevel(Trace);

  runner.run(flow, 1, new RandomScheduling());
  runner.wait(flow);

  /*
  Chain chain(5, 30, 1);
  Flow & flow = chain.getFlow();

  FlowRunner & runner = FlowRunner::createRunner();
  runner.setInfrastructureLogLevel(Info);
  runner.setApplicationLogLevel(Trace);

  runner.run(flow, 4, new RandomScheduling());
  runner.wait(flow);
  */
  return 0;
}


