#include<iostream>
#include<vector>

#include "InputPort.h"
#include "OutputPort.h"

using namespace std;
class Operator
{	
	private:

		vector<InputPort *> *inputPorts;
		vector<OutputPort *> *outputPorts;

	public:

		Operator()
		{
			inputPorts = new vector<InputPort *>();
			outputPorts = new vector<OutputPort *>();
		}

		virtual void process()=0;

		virtual vector<InputPort *>* getInputPorts()
		{
			return inputPorts;
		}

		virtual vector<OutputPort *>* getOutputPorts()
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
