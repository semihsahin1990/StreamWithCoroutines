#include <streamc/experiment/ExpData.h>
#include <streamc/experiment/Run.h>
#include <streamc/Flow.h>
#include <streamc/FlowRunner.h>
#include <thread>

#include "streamc/runtime/SchedulerPlugin.h"
#include "streamc/runtime/RandomScheduling.h"
#include "streamc/runtime/MaxThroughputScheduling.h"
#include "streamc/runtime/MinLatencyScheduling.h"
#include "streamc/runtime/LeastRecentlyScheduling.h"
#include "streamc/runtime/MaxQueueLengthScheduling.h"

#include <fstream>
#include <streamc/topology/Chain.h>
#include <streamc/topology/DataParallel.h>

using namespace std;
using namespace streamc;

ofstream output;
/*
void runExperiment(int numThreads, int depth, int cost) {
  std::chrono::seconds timespan(5);
  std::this_thread::sleep_for(timespan);

  Chain chain(depth, cost, 1.0);
  Flow & flow = chain.getFlow();

  FlowRunner & runner = FlowRunner::createRunner();
  runner.setInfrastructureLogLevel(Error);
  runner.setApplicationLogLevel(Error);
  
  runner.run(flow, numThreads, new RandomScheduling());
  runner.wait(flow);
  
  ifstream input;
  input.open("expData/utilityResults.dat", ios::in);

  vector<double> sum = vector<double>(numThreads);
  vector<double> first = vector<double>(numThreads);
  vector<double> last = vector<double>(numThreads);
  vector<double> max = vector<double>(numThreads);
  vector<double> min = vector<double>(numThreads);

  bool firstTime = true;
  int count = 0;

  while(!input.eof()) {
    for(size_t i=0; i<numThreads; i++) {
      input>>last[i];

      if(firstTime) {
        sum[i] = first[i] = min[i] = max[i] = last[i];
      }
      else{
        sum[i] += last[i];
        if(last[i] < min[i])
          min[i] = last[i];
        if(last[i] > max[i])
          max[i] = last[i];
      }
    }
    count++;
    firstTime = false;
  }
  input.close();


  output<<"depth:\t"<<depth<<endl;
  output<<"cost:\t"<<cost<<endl;
  output<<"numThreads:\t"<<numThreads<<endl;

  output<<"first util:\t";
  for(size_t i=0; i<numThreads; i++) 
    output<<first[i]<<", ";
  output<<endl;

  output<<"last util:\t";
  for(size_t i=0; i<numThreads; i++) 
    output<<last[i]<<", ";
  output<<endl;

  output<<"min util:\t";
  for(size_t i=0; i<numThreads; i++) 
    output<<min[i]<<", ";
  output<<endl;

  output<<"max util:\t";
  for(size_t i=0; i<numThreads; i++) 
    output<<max[i]<<", ";
  output<<endl;

  output<<"avg util:\t";
  for(size_t i=0; i<numThreads; i++) 
    output<<(double)sum[i]/count<<", ";
  output<<endl;

  input.open("expData/result.dat", ios::in);

  int tupleCount;
  long long firstTupleTime;
  long long lastTupleTime;
  string temp;
  
  input>>temp>>tupleCount;
  input>>temp>>firstTupleTime;
  input>>temp>>lastTupleTime;
  output<<"throughput:\t"<<(double)(tupleCount)/(lastTupleTime-firstTupleTime)<<endl<<endl;
}

*/
void runExperiment(int numThreads, int width, int cost) {
  std::chrono::seconds timespan(5);
  std::this_thread::sleep_for(timespan);

  DataParallel dataParallel(cost, 1.0, width);
  Flow & flow = dataParallel.getFlow();

  FlowRunner & runner = FlowRunner::createRunner();
  runner.setInfrastructureLogLevel(Error);
  runner.setApplicationLogLevel(Error);
  
  runner.run(flow, numThreads, new RandomScheduling());
  runner.wait(flow);
  
  ifstream input;
  input.open("expData/utilityResults.dat", ios::in);

  vector<double> sum = vector<double>(numThreads);
  vector<double> first = vector<double>(numThreads);
  vector<double> last = vector<double>(numThreads);
  vector<double> max = vector<double>(numThreads);
  vector<double> min = vector<double>(numThreads);

  bool firstTime = true;
  int count = 0;

  while(!input.eof()) {
    for(size_t i=0; i<numThreads; i++) {
      input>>last[i];

      if(firstTime) {
        sum[i] = first[i] = min[i] = max[i] = last[i];
      }
      else{
        sum[i] += last[i];
        if(last[i] < min[i])
          min[i] = last[i];
        if(last[i] > max[i])
          max[i] = last[i];
      }
    }
    count++;
    firstTime = false;
  }
  input.close();


  output<<"width:\t"<<width<<endl;
  output<<"cost:\t"<<cost<<endl;
  output<<"numThreads:\t"<<numThreads<<endl;

  output<<"first util:\t";
  for(size_t i=0; i<numThreads; i++) 
    output<<first[i]<<", ";
  output<<endl;

  output<<"last util:\t";
  for(size_t i=0; i<numThreads; i++) 
    output<<last[i]<<", ";
  output<<endl;

  output<<"min util:\t";
  for(size_t i=0; i<numThreads; i++) 
    output<<min[i]<<", ";
  output<<endl;

  output<<"max util:\t";
  for(size_t i=0; i<numThreads; i++) 
    output<<max[i]<<", ";
  output<<endl;

  output<<"avg util:\t";
  for(size_t i=0; i<numThreads; i++) 
    output<<(double)sum[i]/count<<", ";
  output<<endl;

  input.open("expData/result.dat", ios::in);

  int tupleCount;
  long long firstTupleTime;
  long long lastTupleTime;
  string temp;
  
  input>>temp>>tupleCount;
  input>>temp>>firstTupleTime;
  input>>temp>>lastTupleTime;
  output<<"throughput:\t"<<(double)(tupleCount)/(lastTupleTime-firstTupleTime)<<endl<<endl;
}

void process() 
{
  using namespace streamc::experiment;

  size_t defaultThreads = 4;
  size_t defaultDepth = 6;
  int defaultCost = 50;
  
  output.open("expData/depth_utility_test.dat");
  size_t const depthMin = 4;
  size_t const depthMax = 10;
  for (size_t depth=depthMin; depth<=depthMax; ++depth) {
      runExperiment(defaultThreads, depth, defaultCost);
  }
  output.close();

  output.open("expData/cost_utility_test.dat");
  size_t const costMin = 0;
  size_t const costMax = 200;
  for (size_t cost=costMin; cost<=costMax; cost = cost+50) {
      runExperiment(defaultThreads, defaultDepth, cost);
  }
  output.close();

  output.open("expData/numThreads_utility_test.dat");
  size_t const numThreadsMin = 1;
  size_t const numThreadsMax = 4;
  for (size_t numThreads=numThreadsMin; numThreads<=numThreadsMax; ++numThreads) {
      runExperiment(numThreads, defaultDepth, defaultCost);
  }
  output.close();


}
int main(int argc, char *argv[])
{
  process();
}