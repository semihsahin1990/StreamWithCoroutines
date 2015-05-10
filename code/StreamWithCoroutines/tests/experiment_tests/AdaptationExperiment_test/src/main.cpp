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

#include <streamc/topology/Chain.h>

using namespace std;
using namespace streamc;
using namespace std::chrono;

class AdaptationExperiment : public streamc::experiment::Run 
{
public:
  void runExperiment(int depth, int numThreads, vector<double> costList, double selectivity, SchedulerPlugin * plugin, double & throughput, double & latency) {
    std::chrono::seconds timespan(5);
    std::this_thread::sleep_for(timespan);

    Chain chain(depth, costList, selectivity);
    Flow & flow = chain.getFlow();

    FlowRunner & runner = FlowRunner::createRunner();
    runner.setInfrastructureLogLevel(Trace);
    runner.setApplicationLogLevel(Trace);

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
    
    cout<<count<<"\t"<<throughput<<"\t"<<latency<<endl<<endl<<endl;
  }

  void repeatExperiment(int numberOfRuns, int depth, int numThreads, vector<double> costList, double selectivity, SchedulerPlugin * plugin, double & avgThroughput, double & throughputDev, double & avgLatency, double & latencyDev) {
    vector<double> tValues;
    vector<double> lValues;

    avgThroughput = 0;
    avgLatency = 0;

    for(int i=0; i<numberOfRuns; i++) {
      double throughput, latency;
      runExperiment(depth, numThreads, costList, selectivity, plugin, throughput, latency);
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

    cout<<avgThroughput<<"\t"<<throughputDev<<"\t"<<avgLatency<<"\t"<<latencyDev<<endl<<endl<<endl<<endl;
  }

  void process() 
  {
    using namespace streamc::experiment;

    size_t numberOfRuns = 1;
    size_t defaultDepth = 1;
    size_t defaultThreads = 12;
    double defaultSelectivity = 1;
    int defaultQuanta = 50000;

    double throughput, latency;
    double throughputDev, latencyDev;
    
    vector<double> costList;

    size_t const minCost = 20;
    size_t const maxCost = 140;
    
    ExpData data("AdaptationExperiment");
    data.setDescription("This is an adaptation experiment");
    data.addFieldName("cost");
    data.addFieldName("#channels");
    data.addFieldName("#threads");
    data.addFieldName("throughput");
    data.addFieldName("throughput_dev");
    data.addFieldName("latency");
    data.addFieldName("latency_dev");

    data.open();
    for (size_t cost = minCost; cost <= maxCost; cost+=20) {
      costList.clear();
      costList.push_back(cost);
      cerr<<"cost:\t"<<costList[0]<<endl;
      data.addNewRecord();
      data.addNewFieldValue("cost", cost);
      repeatExperiment(numberOfRuns, defaultDepth, defaultThreads, costList, defaultSelectivity, new RandomScheduling(defaultQuanta), throughput, throughputDev, latency, latencyDev);
      data.addNewFieldValue("#channels", throughput);
      data.addNewFieldValue("#threads", throughput);
      data.addNewFieldValue("throughput", throughput);
      data.addNewFieldValue("throughput_dev", throughput);
      data.addNewFieldValue("latency", throughput);
      data.addNewFieldValue("latency_dev", throughput);
    }
    data.close();
  }
};

int main(int argc, char *argv[])
{
  AdaptationExperiment().run();
  return EXIT_SUCCESS;
}