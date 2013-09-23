#include "../Tuple.h"
#include "../OutputPort.h"

using namespace std;

OutputPort::OutputPort(queue<Tuple> *portQ)
{
	OutputPort::portQ = portQ;
}


void OutputPort::pushTuple(Tuple tuple)
{
	OutputPort::portQ->push(tuple);
}
