#include <queue>
#include "streamc/Tuple.h"
#include "streamc/InputPort.h"

using namespace std;
using namespace streamc;

InputPort::InputPort(queue<Tuple> *portQ)
{
	InputPort::portQ = portQ;
}

InputPort::~InputPort(){
	delete InputPort::portQ;
}

bool InputPort::hasTuple()
{
	return !(InputPort::portQ)->empty();
}

Tuple InputPort::popTuple()
{
	Tuple returned = InputPort::portQ->front();
	InputPort::portQ->pop();
	return returned;
}
