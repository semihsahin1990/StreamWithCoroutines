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

class Experiment : public streamc::experiment::Run 
{
public:
  void process() 
  {
    using namespace streamc::experiment;

    int depth = 4;
    int cost = 30;
    double selectivity = 1.0;
    int width = 2;
    int numThreads = 1;
    int quanta = 50000;

    ReverseTree tree(depth, cost, selectivity, width);
    Flow & flow = tree.getFlow();
    FlowRunner & runner = FlowRunner::createRunner();
    runner.run(flow, numThreads, new RandomScheduling(quanta));
    runner.wait(flow);
  }
};

int main(int argc, char *argv[])
{
  Experiment().run();
  return EXIT_SUCCESS;
}
