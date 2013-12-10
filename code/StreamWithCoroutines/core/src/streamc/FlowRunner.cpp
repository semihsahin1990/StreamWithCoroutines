#include "streamc/FlowRunner.h"

#include "streamc/Flow.h"
#include "streamc/runtime/FlowRunnerImpl.h"

using namespace streamc;

//creates FlowRunnerImpl and returns it
FlowRunner & FlowRunner::createRunner()
{
    static FlowRunnerImpl runner;
    return static_cast<FlowRunner&>(runner);
}
