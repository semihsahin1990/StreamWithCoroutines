#include<iostream>
#include<vector>
#include<queue>

#include "Flow.h"
#include "Operator.h"
#include "InputPort.h"
#include "OutputPort.h"

Flow::Flow()
{
	Flow::ops = new vector<Operator *>();
}

void Flow::addOperator(Operator *op)
{
	ops->push_back(op);
}

void Flow::connect(Operator *op1, Operator *op2)
{
	queue<Tuple *> *portQ = new queue<Tuple*>();
	
	InputPort *iPort = new InputPort(portQ);
	OutputPort *oPort = new OutputPort(portQ);

	op1->addOutputPort(oPort);
	op2->addInputPort(iPort);
}

void Flow::run(int numberOfThreads)
{

}
