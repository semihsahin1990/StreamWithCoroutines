
#include <streamc/experiment/ExpData.h>
#include <streamc/experiment/Run.h>
#include <streamc/Flow.h>
#include <streamc/FlowRunner.h>

#include <streamc/topology/Chain.h>

using namespace std;
using namespace streamc;

class ChainExperiment : public streamc::experiment::Run 
{
public:
  void process() 
  {
    using namespace streamc::experiment;

    ExpData data("ChainExperiment");    
    data.setDescription("This is a simple experiment - throughput as a function of number of threads for different approaches");

    data.addFieldName("num_threads");
    data.addFieldName("throughput_random");
    data.addFieldName("throughput_schedulerA");
    data.addFieldName("throughput_schedulerB");

    data.open();

    size_t defaultDepth = 10;
    int defaultCost = 100;
    double defaultSelectivity = 1.0;

    size_t const numThreadsMin = 1;
    size_t const numThreadsMax = 4;

    for (size_t numThreads=numThreadsMin; numThreads<=numThreadsMax; ++numThreads) {

      Chain chain(defaultDepth, defaultCost, defaultSelectivity);
      Flow & flow = chain.getFlow();

      FlowRunner & runner = FlowRunner::createRunner();
      runner.setInfrastructureLogLevel(Info);
      runner.setApplicationLogLevel(Trace);
      runner.run(flow, numThreads);
      runner.wait(flow);

      data.addNewRecord();
      
      data.addNewFieldValue("num_threads", numThreads);

      double throughput = random();
      data.addNewFieldValue("throughput_random", throughput);
      
      throughput = random();
      data.addNewFieldValue("throughput_schedulerA", throughput);

      throughput = random();
      data.addNewFieldValue("throughput_schedulerB", throughput);
    }

    data.close();
  }
};

int main(int argc, char *argv[])
{
  ChainExperiment().run();
  return EXIT_SUCCESS;
}
