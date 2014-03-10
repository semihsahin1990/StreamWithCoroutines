#include "streamc/Flow.h"
#include "streamc/FlowRunner.h"
#include "DataParallel.h"

using namespace std;
using namespace streamc;

int main(int argc, char *argv[]) {
	DataParallel creator(15);

	Flow &flow = creator.getFlow();

	FlowRunner & runner = FlowRunner::createRunner();
    runner.setInfrastructureLogLevel(Info);
    runner.setApplicationLogLevel(Trace);
    runner.run(flow, 10);
    runner.wait(flow);

	return 0;
}