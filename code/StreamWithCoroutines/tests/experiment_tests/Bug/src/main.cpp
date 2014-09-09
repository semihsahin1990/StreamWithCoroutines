
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

class Experiment : public streamc::experiment::Run 
{
public:
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

    int numberOfRuns = 100;
    int depth = 3;
    int cost = 30;
    double selectivity = 0.98;
    int width = 2;
    int numThreads = 7;
    int quanta = 50000;

    for(int i=0; i<numberOfRuns; i++) {
      cout<<"run: "<<i<<endl;
      for(int j=0; j<5; j++) {
        switch(j) {
          case 0: cout<<"\tRandomScheduling"<<endl; break;
          case 1: cout<<"\tMaxThroughputScheduling"<<endl; break;
          case 2: cout<<"\tMinLatencyScheduling"<<endl; break;
          case 3: cout<<"\tLeastRecentlyScheduling"<<endl; break;
          case 4: cout<<"\tMaxQueueLengthScheduling"<<endl; break;
        }
        Tree Tree(depth, cost, selectivity, width);
        Flow & flow = Tree.getFlow();

        FlowRunner & runner = FlowRunner::createRunner();
        runner.run(flow, numThreads, *getScheduler(j, quanta));
        runner.wait(flow);
      }
    }
  }
};

int main(int argc, char *argv[])
{
  Experiment().run();
  return EXIT_SUCCESS;
}
