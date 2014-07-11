
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
#include <streamc/topology/DataParallel.h>
#include <streamc/topology/Tree.h>
#include <streamc/topology/ReverseTree.h>

using namespace std;
using namespace streamc;

class QuantaExperiment : public streamc::experiment::Run 
{
public:

   double runChainExperiment(int depth, int numThreads, int cost, double selectivity, SchedulerPlugin & plugin) {
    remove("data/out.dat");
    std::chrono::seconds timespan(5);
    std::this_thread::sleep_for(timespan);

    Chain chain(depth, cost, selectivity);
    Flow & flow = chain.getFlow();

    cout<<"started"<<endl;
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

  double runDataParallelExperiment(int width, int numThreads, int cost, double selectivity, SchedulerPlugin & plugin) {
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

  double runReverseTreeExperiment(int depth, int numThreads, int cost, double selectivity, SchedulerPlugin & plugin) {
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
    string temp;

    input>>temp>>count;
    input>>temp>>firstTupleTime;
    input>>temp>>lastTupleTime;

    cout<<count<<"\t"<<firstTupleTime<<"\t"<<lastTupleTime<<"\t"<<(double)(count)/(double)(lastTupleTime-firstTupleTime)<<endl;

    return (double)count/(double)(lastTupleTime-firstTupleTime);
  }

  double runTreeExperiment(int depth, int numThreads, int cost, double selectivity, SchedulerPlugin & plugin) {
    std::chrono::seconds timespan(5);
    std::this_thread::sleep_for(timespan);

    int width = 2;
    Tree Tree(depth, cost, selectivity, width);
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
      string temp;

      input>>temp>>partialCount;
      input>>temp>>firstTupleTime;
      input>>temp>>lastTupleTime;
      input.close();
      
      count = count + partialCount;
      if(minFirstTupleTime == -1 || firstTupleTime < minFirstTupleTime)
        minFirstTupleTime = firstTupleTime;
      if(maxLastTupleTime == -1 || lastTupleTime > maxLastTupleTime)
        maxLastTupleTime = lastTupleTime;
    }
    cout<<count<<"\t"<<minFirstTupleTime<<"\t"<<maxLastTupleTime<<"\t"<<(double)(count)/(double)(maxLastTupleTime-minFirstTupleTime)<<endl;

    return (double)count/(double)(maxLastTupleTime-minFirstTupleTime);
  }

  void process() 
  {
    using namespace streamc::experiment;

    size_t defaultThreads = 4;
    size_t defaultDepth = 10;
    size_t defaultWidth = 10;
    int defaultCost = 50;
    double defaultSelectivity = 1.0;

    size_t quantaValues[] =  {50, 100, 500, 1000, 5000, 10000, 50000, 100000};
    size_t size = 8;

    ExpData data("QuantaChainExperiment");
    data.setDescription("This is a Quanta experiment - throughput as a function of quanta for different approaches");
    data.addFieldName("quantaInMicroSecs");
    data.addFieldName("throughput_random");
    data.addFieldName("throughput_MTS");
    data.addFieldName("throughput_MTW");
    data.addFieldName("throughput_LRS");
    data.addFieldName("throughput_MQL");
    
    data.open();
    for (size_t i = 0; i<size; i++) {
        data.addNewRecord();
        data.addNewFieldValue("quantaInMicroSecs", quantaValues[i]);
        double throughput = runChainExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new RandomScheduling(quantaValues[i])));
        data.addNewFieldValue("throughput_random", throughput);
        throughput = runChainExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new MaxThroughputScheduling(quantaValues[i])));
        data.addNewFieldValue("throughput_MTS", throughput);
        throughput = runChainExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new MinLatencyScheduling(quantaValues[i])));
        data.addNewFieldValue("throughput_MTW", throughput);
        throughput = runChainExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new LeastRecentlyScheduling(quantaValues[i])));
        data.addNewFieldValue("throughput_LRS", throughput);
        throughput = runChainExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new MaxQueueLengthScheduling(quantaValues[i])));
        data.addNewFieldValue("throughput_MQL", throughput);
        cout<<endl;
    }
    data.close();

    ExpData data2("QuantaDataParallelExperiment");
    data2.setDescription("This is a Quanta experiment - throughput as a function of quanta for different approaches");
    data2.addFieldName("quantaInMicroSecs");
    data2.addFieldName("throughput_random");
    data2.addFieldName("throughput_MTS");
    data2.addFieldName("throughput_MTW");
    data2.addFieldName("throughput_LRS");
    data2.addFieldName("throughput_MQL");
    data2.open();

    for (size_t i = 0; i<size; i++) {
        data2.addNewRecord();
        data2.addNewFieldValue("quantaInMicroSecs", quantaValues[i]);
        double throughput = runDataParallelExperiment(defaultWidth, defaultThreads, defaultCost, defaultSelectivity, *(new RandomScheduling(quantaValues[i])));
        data2.addNewFieldValue("throughput_random", throughput);
        throughput = runDataParallelExperiment(defaultWidth, defaultThreads, defaultCost, defaultSelectivity, *(new MaxThroughputScheduling(quantaValues[i])));
        data2.addNewFieldValue("throughput_MTS", throughput);
        throughput = runDataParallelExperiment(defaultWidth, defaultThreads, defaultCost, defaultSelectivity, *(new MinLatencyScheduling(quantaValues[i])));
        data2.addNewFieldValue("throughput_MTW", throughput);
        throughput = runDataParallelExperiment(defaultWidth, defaultThreads, defaultCost, defaultSelectivity, *(new LeastRecentlyScheduling(quantaValues[i])));
        data2.addNewFieldValue("throughput_LRS", throughput);
        throughput = runDataParallelExperiment(defaultWidth, defaultThreads, defaultCost, defaultSelectivity, *(new MaxQueueLengthScheduling(quantaValues[i])));
        data2.addNewFieldValue("throughput_MQL", throughput);
        cout<<endl;
    }
    data2.close();

    defaultDepth = 3;
    ExpData data3("QuantaReverseTreeExperiment");
    data3.setDescription("This is a Quanta experiment - throughput as a function of quanta for different approaches");
    data3.addFieldName("quantaInMicroSecs");
    data3.addFieldName("throughput_random");
    data3.addFieldName("throughput_MTS");
    data3.addFieldName("throughput_MTW");
    data3.addFieldName("throughput_LRS");
    data3.addFieldName("throughput_MQL");

    data3.open();
    for (size_t i = 0; i<size; i++) {
        data3.addNewRecord();
        data3.addNewFieldValue("quantaInMicroSecs", quantaValues[i]);
        double throughput = runReverseTreeExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new RandomScheduling(quantaValues[i])));  
        data3.addNewFieldValue("throughput_random", throughput);
        throughput = runReverseTreeExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new MaxThroughputScheduling(quantaValues[i])));
        data3.addNewFieldValue("throughput_MTS", throughput);
        throughput = runReverseTreeExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new MinLatencyScheduling(quantaValues[i])));
        data3.addNewFieldValue("throughput_MTW", throughput);
        throughput = runReverseTreeExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new LeastRecentlyScheduling(quantaValues[i])));
        data3.addNewFieldValue("throughput_LRS", throughput);
        throughput = runReverseTreeExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new MaxQueueLengthScheduling(quantaValues[i])));
        data3.addNewFieldValue("throughput_MQL", throughput);
        cout<<endl;
    }
    data3.close();

    ExpData data4("QuantaTreeExperiment");
    data4.setDescription("This is a Quanta experiment - throughput as a function of quanta for different approaches");
    data4.addFieldName("quantaInMicroSecs");
    data4.addFieldName("throughput_random");
    data4.addFieldName("throughput_MTS");
    data4.addFieldName("throughput_MTW");
    data4.addFieldName("throughput_LRS"); 
    data4.addFieldName("throughput_MQL");

    data4.open();
    for (size_t i = 0; i<size; i++) {
        data4.addNewRecord();
        data4.addNewFieldValue("quantaInMicroSecs", quantaValues[i]);
        double throughput = runTreeExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new RandomScheduling(quantaValues[i])));
        data4.addNewFieldValue("throughput_random", throughput);
        throughput = runTreeExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new MaxThroughputScheduling(quantaValues[i])));
        data4.addNewFieldValue("throughput_MTS", throughput);
        throughput = runTreeExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new MinLatencyScheduling(quantaValues[i])));
        data4.addNewFieldValue("throughput_MTW", throughput);
        throughput = runTreeExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new LeastRecentlyScheduling(quantaValues[i])));
        data4.addNewFieldValue("throughput_LRS", throughput);
        throughput = runTreeExperiment(defaultDepth, defaultThreads, defaultCost, defaultSelectivity, *(new MaxQueueLengthScheduling(quantaValues[i])));
        data4.addNewFieldValue("throughput_MQL", throughput);
        cout<<endl;
    }
    data4.close();

  }
};

int main(int argc, char *argv[])
{
  QuantaExperiment().run();
  return EXIT_SUCCESS;
}
