#pragma once

#include<iostream>
#include<vector>

#include "InputPort.h"
#include "OutputPort.h"

// Operator: user visible interface
// OpertorImpl implements a Operator: System implementation of an operator

namespace streamc
{

class Operator
{
private:
	std::string id_;
	std::vector<InputPort *> inputPorts;
	std::vector<OutputPort *> outputPorts;

public:
	Operator(std::string id){
		id_ = id;
	}

	virtual ~Operator(){
		inputPorts.clear();
		outputPorts.clear();
	}

	std::string getId(){
		return id_;
	}

	virtual void init() = 0;
	virtual void process() = 0;
	
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

	virtual bool isShutdownRequested(){
		return false;
	}
};

}
