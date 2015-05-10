#include <streamc/FlowRunner.h>

#include "streamc/operators/FileSource.h"
#include "streamc/operators/Timestamper.h"
#include "streamc/operators/Busy.h"
#include "streamc/operators/ResultCollector.h"
#include "streamc/operators/FileSink.h"

#include "streamc/topology/Chain.h"
#include "streamc/runtime/RandomScheduling.h"
#include <vector>

using namespace std;
using namespace streamc;
using namespace std::chrono;
using namespace streamc::operators;
using namespace streamc::connectors;

vector<double> generateCosts(size_t baseCost, size_t length) {
  mt19937_64 randgen;
  unsigned seed = system_clock::now().time_since_epoch().count();
  randgen.seed(seed);
  uniform_real_distribution<> dist(0.5, 1.5);

  vector<double> costList;
  for(size_t i=0; i<length; i++)
    costList.push_back(dist(randgen) * baseCost);

  return costList;
}

int main(int argc, char *argv[]) {

  int depth = 10;
  double selectivity = 1;
  vector<double> costList = generateCosts(40, depth);

  for(int i=0; i<depth; i++) {
    cerr<<i<<"\t"<<costList[i]<<endl;
  }

  Chain chain(depth, costList, selectivity);
  Flow & flow = chain.getFlow();

  FlowRunner & runner = FlowRunner::createRunner();
  runner.setInfrastructureLogLevel(Info);
  runner.setApplicationLogLevel(Trace);

  runner.run(flow, 4, new RandomScheduling());
  runner.wait(flow);
  return 0;
/*
  Flow flow("GeneralPurposeTest");

  Operator & src = flow.createOperator<FileSource>("src")
    .set_fileName("data/in.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}, {"lineNo", Type::Integer}});

  Operator & timestamper = flow.createOperator<Timestamper>("timestamper");

  Operator & busy = flow.createOperator<Busy>("busy", 100);
  //Operator & busy2 = flow.createOperator<Busy>("busy2", 100);

  Operator & resultCollector = flow.createOperator<ResultCollector>("resultCollector", "expData/result.dat");

  Operator & snk = flow.createOperator<FileSink>("snk")
    .set_fileName("data/out.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}, {"lineNo", Type::Integer}});

  flow.addConnection((src, 0) >> (0,timestamper));
  flow.addConnection((timestamper, 0) >> (0,busy));
  flow.addConnection((busy, 0) >> (0,resultCollector));
  flow.addConnection((resultCollector, 0) >> (0,snk));
*/
/*
  flow.addConnection((src, 0) >> (0,timestamper));
  flow.addConnection((timestamper, 0) >> (0,busy));
  flow.addConnection((busy, 0) >> (0,busy2));
  flow.addConnection((busy2, 0) >> (0,resultCollector));
  flow.addConnection((resultCollector, 0) >> (0,snk));
*/
/*
  FlowRunner & runner = FlowRunner::createRunner();
  runner.setInfrastructureLogLevel(Error);
  runner.setApplicationLogLevel(Error);

  runner.run(flow, 5, new RandomScheduling());
  runner.wait(flow);
*/
  return 0;
}

