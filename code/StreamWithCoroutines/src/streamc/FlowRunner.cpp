#include "streamc/FlowRunner.h"
#include "streamc/runtime/FlowRunnerImpl.h"

using namespace streamc;

FlowRunner &FlowRunner::createRunner()
{
    static FlowRunnerImpl runner;
    return static_cast<FlowRunner&>(runner);
}
