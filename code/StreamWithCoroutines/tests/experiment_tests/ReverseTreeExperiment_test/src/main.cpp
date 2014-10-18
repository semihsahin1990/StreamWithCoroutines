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


#include <streamc/topology/ReverseTree.h>

using namespace std;
using namespace streamc;

class ReverseTreeExperiment : public streamc::experiment::Run 
{
public:

  void runExperiment(int depth, int numThreads, int cost, double selectivity, SchedulerPlugin * plugin, double & throughput, double & latency) {
    std::chrono::seconds timespan(5);
    std::this_thread::sleep_for(timespan);

    ReverseTree ReverseTree(depth, cost, selectivity, 2);
    Flow & flow = ReverseTree.getFlow();

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
    
    cout<<count<<"\t"<<throughput<<"\t"<<latency<<endl;
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

  void repeatExperiment(int numberOfRuns, int depth, int numThreads, int cost, double selectivity, int schedulerId, int quanta, double & avgThroughput, double & throughputDev, double & avgLatency, double & latencyDev) {
    vector<double> tValues;
    vector<double> lValues;

    avgThroughput = 0;
    avgLatency = 0;

    for(int i=0; i<numberOfRuns; i++) {
      double throughput, latency;
      runExperiment(depth, numThreads, cost, selectivity, getScheduler(schedulerId, quanta), throughput, latency);
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

    cout<<avgThroughput<<"\t"<<throughputDev<<"\t"<<avgLatency<<"\t"<<latencyDev<<endl;
  }

  void process() 
  {
    using namespace streamc::experiment;

    int numberOfRuns = 3;

    size_t defaultThreads = 4;
    size_t defaultDepth = 2;
    int defaultCost = 30;
    double defaultSelectivity = 0.98;
    int defaultQuanta = 50000;

    vector<int> quantaValues =  {500, 1000, 5000, 10000, 50000, 100000, 500000};
    
    double throughput, latency;
    double throughputDev, latencyDev;

    vector<string> schedulers = {"random", "maxThroughput", "maxTupleWait", "leastRecently", "maxQueue"};

    // thread experiment
    cout<<"thread experiment"<<endl;
    size_t const numThreadsMin = 1;
    size_t const numThreadsMax = 12;
    ExpData data("ReverseTreeExperiment-thread");
    data.setDescription("This is a ReverseTree experiment - throughput as a function of quanta for different approaches");
    data.addFieldName("num_threads");
    for(int i=0; i<schedulers.size(); i++) {
      data.addFieldName("t_"+schedulers[i]);
      data.addFieldName("td_"+schedulers[i]);
      data.addFieldName("l_"+schedulers[i]);
      data.addFieldName("ld_"+schedulers[i]);
    }
    data.open();
    for (size_t numThreads=numThreadsMin; numThreads<=numThreadsMax; ++numThreads) {
      data.addNewRecord();
      data.addNewFieldValue("num_threads", numThreads);
      for(size_t j=0; j<schedulers.size(); j++) {
        cout<<schedulers[j]<<endl;
        repeatExperiment(numberOfRuns, defaultDepth, numThreads, defaultCost, defaultSelectivity, j, defaultQuanta, throughput, throughputDev, latency, latencyDev);
        data.addNewFieldValue("t_"+schedulers[j], throughput);
        data.addNewFieldValue("td_"+schedulers[j], throughputDev);
        data.addNewFieldValue("l_"+schedulers[j], latency);
        data.addNewFieldValue("ld_"+schedulers[j], latencyDev);
        cout<<endl;
      }
      cout<<"---------------"<<endl;
    }
    data.close();

    // cost experiment
    cout<<"cost experiment"<<endl;
    int const minCost = 0;
    int const maxCost = 100;
    ExpData data2("ReverseTreeExperiment-cost");
    data2.setDescription("This is a ReverseTree experiment - throughput as a function of cost for different approaches");
    data2.addFieldName("costInMicrosecs");
    for(int i=0; i<schedulers.size(); i++) {
      data2.addFieldName("t_"+schedulers[i]);
      data2.addFieldName("td_"+schedulers[i]);
      data2.addFieldName("l_"+schedulers[i]);
      data2.addFieldName("ld_"+schedulers[i]);
    }
    data2.open();
    for (size_t cost=minCost; cost<=maxCost; cost+=20) {
      data2.addNewRecord();
      data2.addNewFieldValue("costInMicrosecs", cost);
      for(size_t j=0; j<schedulers.size(); j++) {
        cout<<schedulers[j]<<endl;
        repeatExperiment(numberOfRuns, defaultDepth, defaultThreads, cost, defaultSelectivity, j, defaultQuanta, throughput, throughputDev, latency, latencyDev);
        data2.addNewFieldValue("t_"+schedulers[j], throughput);
        data2.addNewFieldValue("td_"+schedulers[j], throughputDev);
        data2.addNewFieldValue("l_"+schedulers[j], latency);
        data2.addNewFieldValue("ld_"+schedulers[j], latencyDev);
        cout<<endl;
      }
      cout<<"---------------"<<endl;
    }
    data2.close();

    /*
    // depth experiment
    cout<<"depth experiments"<<endl;
    int const minDepth = 2;
    int const maxDepth = 20;
    ExpData data3("ReverseTreeExperiment-depth");
    data3.setDescription("This is a ReverseTree experiment - throughput as a function of depth for different approaches");
    data3.addFieldName("depth");
    for(int i=0; i<schedulers.size(); i+=2) {
      data3.addFieldName("t_"+schedulers[i]);
      data3.addFieldName("l_"+schedulers[i]);
      data3.addFieldName("d_"+schedulers[i]);
    }
    data3.open();
    for (size_t depth=minDepth; depth<=maxDepth; depth=depth+2) {
      data3.addNewRecord();
      data3.addNewFieldValue("depth", depth);
      for(size_t j=0; j<schedulers.size(); j++) {
        avgThroughput = avgLatency = avgDeviation = 0;
        cout<<schedulers[j]<<endl;
        for(size_t k=0; k<numberOfRuns; k++) {
          runExperiment(depth, defaultThreads, defaultCost, defaultSelectivity, *getScheduler(j, defaultQuanta), throughput, latency, deviation);
          avgThroughput = avgThroughput + throughput;
          avgLatency = avgLatency + latency;
          avgDeviation = avgDeviation + deviation;
        }
        avgThroughput = avgThroughput / numberOfRuns;
        avgLatency = avgLatency / numberOfRuns;
        avgDeviation = avgDeviation / numberOfRuns;
        data3.addNewFieldValue("t_"+schedulers[j], avgThroughput);
        data3.addNewFieldValue("l_"+schedulers[j], avgLatency);
        data3.addNewFieldValue("d_"+schedulers[j], avgDeviation);
        cout<<endl;
      }
      cout<<"---------------"<<endl;
    }
    data3.close();

    // selectivity experiment
    cout<<"selectivity experiments"<<endl;
    double const minSelectivity = 0.7;
    double const maxSelectivity = 1.0;
    ExpData data4("ReverseTreeExperiment-selectivity");
    data4.setDescription("This is a ReverseTree experiment - throughput as a function of depth for different approaches");
    data4.addFieldName("selectivity");
    for(int i=0; i<schedulers.size(); i++) {
      data4.addFieldName("t_"+schedulers[i]);
      data4.addFieldName("l_"+schedulers[i]);
      data4.addFieldName("d_"+schedulers[i]);
    }
    data4.open();
    for (double selectivity=minSelectivity; selectivity<=maxSelectivity; selectivity+=0.05) {
      data4.addNewRecord();
      data4.addNewFieldValue("selectivity", selectivity);
      for(size_t j=0; j<schedulers.size(); j++) {
        avgThroughput = avgLatency = avgDeviation = 0;
        cout<<schedulers[j]<<endl;
        for(size_t k=0; k<numberOfRuns; k++) {
          runExperiment(defaultDepth, defaultThreads, defaultCost, selectivity, *getScheduler(j, defaultQuanta), throughput, latency, deviation);
          avgThroughput = avgThroughput + throughput;
          avgLatency = avgLatency + latency;
          avgDeviation = avgDeviation + deviation;
        }
        avgThroughput = avgThroughput / numberOfRuns;
        avgLatency = avgLatency / numberOfRuns;
        avgDeviation = avgDeviation / numberOfRuns;
        data4.addNewFieldValue("t_"+schedulers[j], avgThroughput);
        data4.addNewFieldValue("l_"+schedulers[j], avgLatency);
        data4.addNewFieldValue("d_"+schedulers[j], avgDeviation);
        cout<<endl;
      }
      cout<<"---------------"<<endl;
    }
    data4.close();

    // quanta experiment
    cout<<"quanta experiments"<<endl;
    ExpData data5("ReverseTreeExperiment-quanta");
    data5.setDescription("This is a ReverseTree experiment - throughput as a function of quanta for different approaches");
    data5.addFieldName("quanta");
    for(int i=0; i<schedulers.size(); i++) {
      data5.addFieldName("t_"+schedulers[i]);
      data5.addFieldName("l_"+schedulers[i]);
      data5.addFieldName("d_"+schedulers[i]);
    }

    data5.open();
    for (size_t i=0; i<quantaValues.size(); i++) {
      data5.addNewRecord();
      data5.addNewFieldValue("quanta", log(quantaValues[i]));
      for(size_t j=0; j<schedulers.size(); j++) {
        avgThroughput = avgLatency = avgDeviation = 0;
        for(size_t k=0; k<numberOfRuns; k++) {
          runExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *getScheduler(j, quantaValues[i]), throughput, latency, deviation);
          avgThroughput = avgThroughput + throughput;
          avgLatency = avgLatency + latency;
          avgDeviation = avgDeviation + deviation;
        }
        avgThroughput = avgThroughput / numberOfRuns;
        avgLatency = avgLatency / numberOfRuns;
        data5.addNewFieldValue("t_"+schedulers[j], avgThroughput);
        data5.addNewFieldValue("l_"+schedulers[j], avgLatency);
        data5.addNewFieldValue("d_"+schedulers[j], avgDeviation);
        cout<<endl;
      }
      cout<<"---------------"<<endl;
    }
    data5.close();
    */
  }
};

int main(int argc, char *argv[])
{
  ReverseTreeExperiment().run();
  return EXIT_SUCCESS;
}