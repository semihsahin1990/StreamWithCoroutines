
#include <streamc/experiment/ExpData.h>
#include <streamc/experiment/Run.h>
#include <streamc/Flow.h>
#include <streamc/FlowRunner.h>

#include <streamc/topology/DataParallel.h>

using namespace std;
using namespace streamc;

class DataParallelExperiment : public streamc::experiment::Run 
{
public:
  void process() 
  {
    using namespace streamc::experiment;

    ExpData data("DataParallelExperiment");    
    data.setDescription("This is a simple experiment - throughput as a function of number of threads for different approaches");

    data.addFieldName("num_threads");
    data.addFieldName("throughput_random");
    data.addFieldName("throughput_schedulerA");
    data.addFieldName("throughput_schedulerB");

    data.open();

    size_t const numThreadsMin = 1;
    size_t const numThreadsMax = 1;

    for (size_t numThreads=numThreadsMin; numThreads<=numThreadsMax; ++numThreads) {

      DataParallel dataParallel(4, 100, 1.0);
      Flow & flow = dataParallel.getFlow();

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
  DataParallelExperiment().run();
  return EXIT_SUCCESS;
}
