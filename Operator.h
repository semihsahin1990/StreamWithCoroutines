#pragma once

#include<iostream>
#include<vector>

#include "InputPort.h"
#include "OutputPort.h"

// Operator: user visible interface
// OpertorImpl has a Operator: System implementation of an operator

class Operator
{
	private:

        std::vector<InputPort *> inputPorts;
        std::vector<OutputPort *> outputPorts;

	public:

		Operator()
		{
		}

		virtual void process()=0;

		virtual vector<InputPort *> const & getInputPorts() const
		{
			return inputPorts;
		}

		virtual vector<OutputPort *> const & getOutputPorts() const
		{
			return outputPorts;
		}
		
		virtual void addInputPort(InputPort *inputPort)
		{
			inputPorts->push_back(inputPort);
		}

		virtual void addOutputPort(OutputPort *outputPort)
		{
			outputPorts->push_back(outputPort);
		}
};
