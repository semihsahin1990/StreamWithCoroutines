
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


#include <streamc/topology/Tree.h>

using namespace std;
using namespace streamc;
using namespace std::chrono;

class TreeExperiment : public streamc::experiment::Run 
{
public:

  void runExperiment(int depth, int numThreads, vector<double> costList, double selectivity, SchedulerPlugin * plugin, double & throughput, double & latency) {
    std::chrono::seconds timespan(5);
    std::this_thread::sleep_for(timespan);

    int width = 2;
    Tree Tree(depth, costList, selectivity, width);
    Flow & flow = Tree.getFlow();

    FlowRunner & runner = FlowRunner::createRunner();
    runner.run(flow, numThreads, plugin);
    runner.wait(flow);

    long long minFirstTupleTime = -1;
    long long maxLastTupleTime = -1;
    int count = 0;
    int numberOfSinks = pow(width, depth-1);

    for(int i=0; i<numberOfSinks; i++) {
      string fileName_ = "expData/result"+to_string(i)+".dat";
      ifstream input;
      input.open(fileName_.c_str(), ios::in);

      int partialCount;
      long long firstTupleTime;
      long long lastTupleTime;
      double minLatency;
      double maxLatency;
      double partialAvgLatency;
      double partialDeviation;
      string temp;

      input>>temp>>partialCount;
      input>>temp>>firstTupleTime;
      input>>temp>>lastTupleTime;
      input>>temp>>minLatency;
      input>>temp>>maxLatency;
      input>>temp>>partialAvgLatency;
      input>>temp>>partialDeviation;
      input.close();
      
      count = count + partialCount;
      if(minFirstTupleTime == -1 || firstTupleTime < minFirstTupleTime)
        minFirstTupleTime = firstTupleTime;
      if(maxLastTupleTime == -1 || lastTupleTime > maxLastTupleTime)
        maxLastTupleTime = lastTupleTime;

      latency = latency + partialAvgLatency * partialCount;
    }

    throughput = (double)count/(double)(maxLastTupleTime-minFirstTupleTime);
    latency = latency / count;
    cout<<count<<"\t"<<throughput<<"\t"<<latency<<endl;
  }

  void repeatExperiment(int numberOfRuns, int depth, int numThreads, vector<double> costList, double selectivity, int schedulerId, int quanta, double & avgThroughput, double & throughputDev, double & avgLatency, double & latencyDev) {
    vector<double> tValues;
    vector<double> lValues;

    avgThroughput = 0;
    avgLatency = 0;

    for(int i=0; i<numberOfRuns; i++) {
      double throughput, latency;
      runExperiment(depth, numThreads, costList, selectivity, getScheduler(schedulerId, quanta), throughput, latency);
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

  void process() 
  {
    using namespace streamc::experiment;

    int numberOfRuns = 3;

    size_t defaultThreads = 4;
    size_t defaultDepth = 3;
    int defaultCost = 40;
    double defaultSelectivity = 1;
    int defaultQuanta = 50000;

    vector<int> quantaValues =  {500, 1000, 5000, 10000, 50000, 100000, 500000};
    
    double throughput, latency;
    double throughputDev, latencyDev;

    vector<string> schedulers = {"random", "maxThroughput", "maxTupleWait", "leastRecently", "maxQueue"};
    vector<double> costList = generateCosts(defaultCost, (pow(2, defaultDepth)-1));

    // thread experiment
    cout<<"thread experiment"<<endl;
    size_t const numThreadsMin = 1;
    size_t const numThreadsMax = 12;
    ExpData data("TreeExperiment-thread");
    data.setDescription("This is a Tree experiment - throughput as a function of number of threads for different approaches");
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
        repeatExperiment(numberOfRuns, defaultDepth, numThreads, costList, defaultSelectivity, j, defaultQuanta, throughput, throughputDev, latency, latencyDev);
        data.addNewFieldValue("t_"+schedulers[j], throughput);
        data.addNewFieldValue("td_"+schedulers[j], throughputDev);
        data.addNewFieldValue("l_"+schedulers[j], latency);
        data.addNewFieldValue("ld_"+schedulers[j], latencyDev);
        cout<<endl;
      }
      cout<<"---------------"<<endl;
    }
    data.close();
/*    
    // cost experiment
    cout<<"cost experiment"<<endl;
    int const minCost = 0;
    int const maxCost = 100;
    ExpData data2("TreeExperiment-cost");
    data2.setDescription("This is a Tree experiment - throughput as a function of cost for different approaches");
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
*/
    /*
    // depth experiment
    cout<<"depth experiments"<<endl;
    int const mindepth = 2;
    int const maxdepth = 5;
    ExpData data3("TreeExperiment-depth");
    data3.setDescription("This is a Tree experiment - throughput as a function of depth for different approaches");
    data3.addFieldName("depth");
    for(int i=0; i<schedulers.size(); i++) {
      data3.addFieldName("t_"+schedulers[i]);
      data3.addFieldName("l_"+schedulers[i]);
      data3.addFieldName("d_"+schedulers[i]);
    }

    data3.open();
    for (size_t depth=mindepth; depth<=maxdepth; ++depth) {
      data3.addNewRecord();
      data3.addNewFieldValue("depth", depth);
      for(size_t j=0; j<schedulers.size(); j++) {
        avgThroughput = avgLatency = avgDeviation = 0;
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
    ExpData data4("TreeExperiment-selectivity");
    data4.setDescription("This is a Tree experiment - throughput as a function of depth for different approaches");
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
    
    cout<<"quanta experiments"<<endl;

    ExpData data5("TreeExperiment-quanta");
    data5.setDescription("This is a ree experiment - throughput as a function of quanta for different approaches");
    data5.addFieldName("quanta");
    for(int i=0; i<schedulers.size(); i++) {
      data5.addFieldName("t_"+schedulers[i]);
      data5.addFieldName("l_"+schedulers[i]);
      data5.addFieldName("d_"+schedulers[i]);
    }

    data5.open();
    for (size_t i=0; i<quantaValues.size(); i++) {
      data5.addNewRecord();
      data5.addNewFieldValue("quanta", log(quantaValues[i])/log(10));
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
        avgDeviation = avgDeviation / numberOfRuns;
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
  TreeExperiment().run();
  return EXIT_SUCCESS;
}
