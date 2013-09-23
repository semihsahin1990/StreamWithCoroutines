#pragma once

#include<iostream>
#include<vector>

#include "InputPort.h"
#include "OutputPort.h"

// Operator: user visible interface
// OpertorImpl implements a Operator: System implementation of an operator

class Operator
{
private:
	int id;
	std::vector<InputPort *> inputPorts;
	std::vector<OutputPort *> outputPorts;

public:
	Operator(int id){
		this->id = id;
	}
	int getId(){
		return id;
	}

	virtual void process()=0;
	
	virtual void const addInputPort(InputPort *inputPort)
	{
		inputPorts.push_back(inputPort);
	}
	
	virtual void const addOutputPort(OutputPort *outputPort)
	{
		outputPorts.push_back(outputPort);
	}
	
	virtual std::vector<InputPort *> const & getInputPorts()
	{
		return inputPorts;
	}
	
	virtual std::vector<OutputPort *> const & getOutputPorts()
	{
		return outputPorts;
	}
};
