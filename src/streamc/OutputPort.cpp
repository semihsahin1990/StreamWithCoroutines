#include<iostream>
#include "Tuple.h"
#include "OutputPort.h"

OutputPort::OutputPort(queue<Tuple *> *portQ)
{
	OutputPort::portQ = portQ;
}


void OutputPort::pushTuple(Tuple *tuple)
{
	OutputPort::portQ->push(tuple);
}
