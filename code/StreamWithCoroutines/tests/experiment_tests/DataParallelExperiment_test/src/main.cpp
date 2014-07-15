
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

class DataParallelExperiment : public streamc::experiment::Run 
{
public:

  void runExperiment(int width, int numThreads, int cost, double selectivity, SchedulerPlugin & plugin, double & throughput, double & avgLatency) {
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
    input>>temp>>avgLatency;


    throughput = (double)count/(double)(lastTupleTime-firstTupleTime);

    cout<<count<<"\t"<<(double)(count)/(double)(lastTupleTime-firstTupleTime)<<"\t"<<avgLatency<<endl;
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

    size_t defaultThreads = 4;
    size_t defaultWidth = 10;
    int defaultCost = 50;
    double defaultSelectivity = 0.98;
    int defaultQuanta = 100000;

    vector<int> quantaValues =  {500, 1000, 5000, 10000, 50000, 100000, 500000};
    
    double avgThroughput, avgLatency;
    double throughput, latency;

    vector<string> schedulers = {"random", "maxThroughput", "maxTupleWait", "leastRecently", "maxQueue"};

    /*
    // thread experiment
    cout<<"thread experiment"<<endl;
    size_t const numThreadsMin = 1;
    size_t const numThreadsMax = 4;
    ExpData data("DataParallelExperiment-thread");
    data.setDescription("This is a DataParallel experiment - throughput as a function of number of threads for different approaches");
    data.addFieldName("num_threads");
    data.addFieldName("throughput_random");
    data.addFieldName("throughput_schedulerA");
    data.addFieldName("throughput_schedulerB");
    data.open();
    
    for (size_t numThreads=numThreadsMin; numThreads<=numThreadsMax; ++numThreads) {
      data.addNewRecord();
      data.addNewFieldValue("num_threads", numThreads);
      double throughput = runExperiment(defaultWidth, numThreads, defaultCost, defaultSelectivity, *(new RandomScheduling()));  
      data.addNewFieldValue("throughput_random", throughput);
      throughput = runExperiment(defaultWidth, numThreads, defaultCost, defaultSelectivity, *(new MaxThroughputScheduling()));
      data.addNewFieldValue("throughput_schedulerA", throughput);
      throughput = runExperiment(defaultWidth, numThreads, defaultCost, defaultSelectivity, *(new MinLatencyScheduling()));
      data.addNewFieldValue("throughput_schedulerB", throughput);
    }
    data.close();
    
    // cost experiment
    cout<<"cost experiment"<<endl;
    int const minCost = 0;
    int const maxCost = 200;
    ExpData data2("DataParallelExperiment-cost");
    data2.setDescription("This is a DataParallel experiment - throughput as a function of cost for different approaches");
    data2.addFieldName("costInMicrosecs");
    data2.addFieldName("throughput_random");
    data2.addFieldName("throughput_schedulerA");
    data2.addFieldName("throughput_schedulerB");
    data2.open();
    for (size_t cost=minCost; cost<=maxCost; cost+=20) {
      data2.addNewRecord();
      data2.addNewFieldValue("costInMicrosecs", cost);
      double throughput = runExperiment(defaultWidth, defaultThreads, cost, defaultSelectivity, *(new RandomScheduling()));  
      data2.addNewFieldValue("throughput_random", throughput);
      throughput = runExperiment(defaultWidth, defaultThreads, cost, defaultSelectivity, *(new MaxThroughputScheduling()));
      data2.addNewFieldValue("throughput_schedulerA", throughput);
      throughput = runExperiment(defaultWidth, defaultThreads, cost, defaultSelectivity, *(new MinLatencyScheduling()));
      data2.addNewFieldValue("throughput_schedulerB", throughput);
    }
    data2.close();
    
    // depth experiment
    cout<<"width experiments"<<endl;
    int const minWidth = 1;
    int const maxWidth = 20;
    ExpData data3("DataParallelExperiment-width");
    data3.setDescription("This is a DataParallel experiment - throughput as a function of width for different approaches");
    data3.addFieldName("width");
    data3.addFieldName("throughput_random");
    data3.addFieldName("throughput_schedulerA");
    data3.addFieldName("throughput_schedulerB");
    data3.open();
    for (size_t width=minWidth; width<=maxWidth; ++width) {
      data3.addNewRecord();
      data3.addNewFieldValue("width", width);
      double throughput = runExperiment(width, defaultThreads, defaultCost, defaultSelectivity, *(new RandomScheduling()));  
      data3.addNewFieldValue("throughput_random", throughput);
      throughput = runExperiment(width, defaultThreads, defaultCost, defaultSelectivity, *(new MaxThroughputScheduling()));
      data3.addNewFieldValue("throughput_schedulerA", throughput);
      throughput = runExperiment(width, defaultThreads, defaultCost, defaultSelectivity, *(new MinLatencyScheduling()));
      data3.addNewFieldValue("throughput_schedulerB", throughput);
    }
    data3.close();

    // selectivity experiment
    cout<<"selectivity experiments"<<endl;
    double const minSelectivity = 0.5;
    double const maxSelectivity = 1.0;
    ExpData data4("DataParallelExperiment-selectivity");
    data4.setDescription("This is a DataParallel experiment - throughput as a function of depth for different approaches");
    data4.addFieldName("selectivity");
    data4.addFieldName("throughput_random");
    data4.addFieldName("throughput_schedulerA");
    data4.addFieldName("throughput_schedulerB");
    data4.open();
    for (double selectivity=minSelectivity; selectivity<=maxSelectivity; selectivity+=0.1) {
      data4.addNewRecord();
      data4.addNewFieldValue("selectivity", selectivity);
      double throughput = runExperiment(defaultWidth, defaultThreads, defaultCost, selectivity, *(new RandomScheduling()));  
      data4.addNewFieldValue("throughput_random", throughput);
      throughput = runExperiment(defaultWidth, defaultThreads, defaultCost, selectivity, *(new MaxThroughputScheduling()));
      data4.addNewFieldValue("throughput_schedulerA", throughput);
      throughput = runExperiment(defaultWidth, defaultThreads, defaultCost, selectivity, *(new MinLatencyScheduling()));
      data4.addNewFieldValue("throughput_schedulerB", throughput);
    }
    data4.close();
    */

    cout<<"quanta experiments"<<endl;

    ExpData data5("DataParallelExperiment-quanta");
    data5.setDescription("This is a dataParallel experiment - throughput as a function of quanta for different approaches");
    data5.addFieldName("quanta");
    for(int i=0; i<schedulers.size(); i++) {
      data5.addFieldName("throughput_"+schedulers[i]);
      data5.addFieldName("latency_"+schedulers[i]);
    }

    data5.open();
    for (size_t i=0; i<quantaValues.size(); i++) {
      data5.addNewRecord();
      data5.addNewFieldValue("quanta", log(quantaValues[i])/log(10));
      for(size_t j=0; j<schedulers.size(); j++) {
        avgThroughput = avgLatency = 0;
        for(size_t k=0; k<numberOfRuns; k++) {
          runExperiment(defaultWidth, defaultThreads, defaultCost, defaultSelectivity, *getScheduler(j, quantaValues[i]), throughput, latency);
          avgThroughput = avgThroughput + throughput;
          avgLatency = avgLatency + latency;
        }
        avgThroughput = avgThroughput / numberOfRuns;
        avgLatency = avgLatency / numberOfRuns;
        data5.addNewFieldValue("throughput_"+schedulers[j], avgThroughput);
        data5.addNewFieldValue("latency_"+schedulers[j], avgLatency);
        cout<<endl;
      }
      cout<<"---------------"<<endl;
    }
    data5.close();
  }
};

int main(int argc, char *argv[])
{
  DataParallelExperiment().run();
  return EXIT_SUCCESS;
}
