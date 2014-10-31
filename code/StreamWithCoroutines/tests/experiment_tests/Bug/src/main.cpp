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


#include <streamc/topology/DataParallel.h>

using namespace std;
using namespace streamc;

class Experiment : public streamc::experiment::Run 
{
public:

  void runExperiment(int width, int numThreads, int cost, double selectivity, SchedulerPlugin * plugin, double & throughput, double & latency) {
    std::chrono::seconds timespan(5);
    std::this_thread::sleep_for(timespan);

    DataParallel DataParallel(cost, selectivity, width);
    Flow & flow = DataParallel.getFlow();

    FlowRunner & runner = FlowRunner::createRunner();
    runner.run(flow, numThreads, plugin);
    runner.wait(flow);

    string fileName_ = "expData/result.dat";
    ifstream input;
    input.open(fileName_.c_str(), ios::in);

    int count;
    long long firstTupleTime;
    long long lastTupleTime;
    double minLatency;
    double maxLatency;
    string temp;

    input>>temp>>count;
    input>>temp>>firstTupleTime;
    input>>temp>>lastTupleTime;
    input>>temp>>minLatency;
    input>>temp>>maxLatency;
    input>>temp>>latency;

    throughput = (double)count/(double)(lastTupleTime-firstTupleTime);
  }

  void repeatExperiment(int numberOfRuns, int width, int numThreads, int cost, double selectivity, int schedulerId, int quanta, double & avgThroughput, double & throughputDev, double & avgLatency, double & latencyDev) {
    vector<double> tValues;
    vector<double> lValues;

    avgThroughput = 0;
    avgLatency = 0;

    for(int i=0; i<numberOfRuns; i++) {
      double throughput, latency;
      runExperiment(width, numThreads, cost, selectivity, getScheduler(schedulerId, quanta), throughput, latency);
      tValues.push_back(throughput);
      lValues.push_back(latency);
      avgThroughput = avgThroughput + throughput;
      avgLatency = avgLatency + latency;
    }

    avgThroughput = avgThroughput / numberOfRuns;
    avgLatency = avgLatency / numberOfRuns;

    throughputDev = 0;
    latencyDev = 0;

    for(int i=0; i<numberOfRuns; i++) {
      throughputDev += pow(avgThroughput - tValues[i], 2);
      latencyDev += pow(avgLatency - lValues[i], 2);

    }

    throughputDev = sqrt(throughputDev/numberOfRuns);
    latencyDev = sqrt(latencyDev/numberOfRuns);
  }

  SchedulerPlugin * getScheduler(int i, size_t quanta) {
    if(i == 0)
      return new RandomScheduling(quanta);
    if(i == 1)
      return new MaxThroughputScheduling(quanta);
    if(i == 2)
      return new MinLatencyScheduling(quanta);
    if(i == 3)
      return new LeastRecentlyScheduling(quanta);
    if(i == 4)
      return new MaxQueueLengthScheduling(quanta);
    return nullptr;
  }

  void process() 
  {
    using namespace streamc::experiment;

    int numberOfRuns = 3;
    size_t defaultWidth = 15;
    int defaultCost = 30;
    double defaultSelectivity = 1;
    int defaultQuanta = 50000;
    
    double throughput, latency;
    double throughputDev, latencyDev;

    vector<string> schedulers = {"random", "maxThroughput", "maxTupleWait", "leastRecently", "maxQueue"};

    // thread experiment
    cout<<"thread experiment"<<endl;
    size_t const numThreadsMin = 1;
    size_t const numThreadsMax = 12;
    
    ExpData data("DataParallelExperiment-thread");
    data.setDescription("This is a DataParallel experiment - throughput as a function of number of threads for different approaches");
    data.addFieldName("num_threads");
    for(int i=0; i<schedulers.size(); i++) {
      data.addFieldName("t_"+schedulers[i]);
      data.addFieldName("td_"+schedulers[i]);
      data.addFieldName("l_"+schedulers[i]);
      data.addFieldName("ld_"+schedulers[i]);
    }

    data.open();

    for (size_t numThreads=numThreadsMin; numThreads<=numThreadsMax; ++numThreads) {
      cerr<<numThreads<<endl;
      data.addNewRecord();
      data.addNewFieldValue("num_threads", numThreads);
      for(size_t j=0; j<schedulers.size(); j++) {
        repeatExperiment(numberOfRuns, defaultWidth, numThreads, defaultCost, defaultSelectivity, j, defaultQuanta, throughput, throughputDev, latency, latencyDev);
        data.addNewFieldValue("t_"+schedulers[j], throughput);
        data.addNewFieldValue("td_"+schedulers[j], throughputDev);
        data.addNewFieldValue("l_"+schedulers[j], latency);
        data.addNewFieldValue("ld_"+schedulers[j], latencyDev);
      }
    }
    data.close();
  }
};

int main(int argc, char *argv[])
{
  Experiment().run();
  return EXIT_SUCCESS;
}
