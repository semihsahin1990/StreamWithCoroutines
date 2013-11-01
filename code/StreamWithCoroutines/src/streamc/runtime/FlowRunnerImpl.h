#pragma once

#include "streamc/FlowRunner.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
namespace streamc
{

class FlowRunnerImpl : public FlowRunner
{
private:

	void createPorts(Flow &flow){
		
		std::vector<Operator *> operators = flow.getOperators();

		for(auto it= operators.begin(); it!=operators.end(); it++){
			std::vector<Operator *> neighbors = flow.getNeighbors(*it);

			for(auto nIt = neighbors.begin(); nIt!=neighbors.end(); nIt++){
				std::queue<Tuple>* shared = new std::queue<Tuple>();
				
				InputPortImpl *iport = new InputPortImpl(shared);
				OutputPortImpl *oport = new OutputPortImpl(shared);

				(*it)->addOutputPort(oport);
				(*nIt)->addInputPort(iport);
			}
		}
	}

public:
	void run(Flow &flow, int numThreads)
	{
		createPorts(flow);
	}


};

}
