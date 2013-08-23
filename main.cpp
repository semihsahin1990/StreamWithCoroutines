#include<iostream>
#include<queue>

#include "TupleWith1.h"
#include "TupleWith2.h"
#include "InputPort.h"
#include "OutputPort.h"
#include "FileSource.h"

using namespace std;

int main()
{

	queue<Tuple *> *portQ = new queue<Tuple *>();
	InputPort *iport = new InputPort(portQ);

	FileSource *fs = new FileSource();
	
	fs->addInputPort(iport);

	fs->process();
/*
	queue<Tuple *> *portQ = new queue<Tuple *>();

	InputPort *iport = new InputPort(portQ);
	OutputPort *oport = new OutputPort(portQ);

	for(int i=0; i<10; i++)
	{
		TupleWith1 *tuple = new TupleWith1();
		tuple->x = i;
		oport->pushTuple(tuple);
		cout<< ((TupleWith1 *)(iport->popTuple())) -> x << endl;
	}
*/

	return 0;
}
