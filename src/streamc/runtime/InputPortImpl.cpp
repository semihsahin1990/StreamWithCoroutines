#include<queue>
#include "../Tuple.h"
#include "../InputPort.h"

using namespace std;

InputPort::InputPort(queue<Tuple> *portQ)
{
	InputPort::portQ = portQ;
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
