#pragma once

#include "streamc/Flow.h"

namespace streamc
{

class FlowRunner
{
public:	
    static FlowRunner & createRunner();
    virtual void run(Flow &flow, int numThreads) = 0;
};

}
