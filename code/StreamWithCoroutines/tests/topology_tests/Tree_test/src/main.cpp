#include "streamc/Flow.h"
#include "streamc/FlowRunner.h"
#include "TreeCreator.h"

using namespace std;
using namespace streamc;

int main(int argc, char *argv[]) {
	TreeCreator creator(4,3);

	Flow &flow = creator.getFlow();

	cout<<creator.getDepth()<<endl;

	FlowRunner & runner = FlowRunner::createRunner();
    runner.setInfrastructureLogLevel(Info);
    runner.setApplicationLogLevel(Trace);
    runner.run(flow, 2);
    runner.wait(flow);

	return 0;
}