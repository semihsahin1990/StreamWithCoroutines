
#include <streamc/experiment/ExpData.h>
#include <streamc/experiment/Run.h>
#include <streamc/Flow.h>
#include <streamc/FlowRunner.h>
#include <thread>

#include "streamc/runtime/SchedulerPlugin.h"
#include "streamc/runtime/RandomScheduling.h"
#include "streamc/runtime/MaxThroughputScheduling.h"
#include "streamc/runtime/MinLatencyScheduling.h"


#include <streamc/topology/DataParallel.h>

using namespace std;
using namespace streamc;

class DataParallelExperiment : public streamc::experiment::Run 
{
public:

  double runExperiment(int width, int numThreads, int cost, double selectivity, SchedulerPlugin & plugin) {
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
    string temp;

    input>>temp>>count;
    input>>temp>>firstTupleTime;
    input>>temp>>lastTupleTime;

    cout<<count<<"\t"<<firstTupleTime<<"\t"<<lastTupleTime<<"\t"<<(double)(count)/(double)(lastTupleTime-firstTupleTime)<<endl;

    return (double)count/(double)(lastTupleTime-firstTupleTime);
  }

  void process() 
  {
    using namespace streamc::experiment;

    size_t defaultThreads = 4;
    size_t defaultWidth = 10;
    int defaultCost = 50;
    double defaultSelectivity = 1.0;

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
  }
};

int main(int argc, char *argv[])
{
  DataParallelExperiment().run();
  return EXIT_SUCCESS;
}
