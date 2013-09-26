#include<iostream>
#include<vector>
#include<queue>

#include "streamc/Flow.h"
#include "streamc/Operator.h"
#include "streamc/InputPort.h"
#include "streamc/OutputPort.h"

using namespace streamc;


/*
Flow::Flow()
{
	ops = new vector<Operator *>();
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
*/

void Flow::run(int numberOfThreads)
{

}
