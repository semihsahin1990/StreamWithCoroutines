
#include <streamc/experiment/ExpData.h>
#include <streamc/experiment/Run.h>

using namespace std;
using namespace streamc;

class BasicExperiment : public streamc::experiment::Run 
{
public:
  void process() 
  {
    using namespace streamc::experiment;

    ExpData data("BasicExperiment");    
    data.setDescription("This is a simple experiment - throughput as a function of number of threads for different approaches");

    data.addFieldName("num_threads");
    data.addFieldName("throughput_random");
    data.addFieldName("throughput_schedulerA");
    data.addFieldName("throughput_schedulerB");

    data.open();

    size_t const numThreadsMin = 1;
    size_t const numThreadsMax = 4;  
    for (size_t numThreads=numThreadsMin; numThreads<=numThreadsMax; ++numThreads) {
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
  BasicExperiment().run();
  return EXIT_SUCCESS;
}
