#include<iostream>
#include<sstream>
#include<queue>

#include "streamc/Tuple.h"
#include "streamc/InputPort.h"
#include "streamc/OutputPort.h"
#include "streamc/Operator.h"
#include "streamc/FlowRunner.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
#include "streamc/operators/FileSource.h"

using namespace std;
using namespace streamc;

int main()
{
	FileSource* fs1 = new FileSource("fs1", "");
	FileSource* fs2 = new FileSource("fs2", "");
	FileSource* fs3 = new FileSource("fs3", "");
	FileSource* fs4 = new FileSource("fs4", "");
	
	Flow flow;

	flow.addConnection(fs1,1,fs3,1);
	flow.addConnection(fs1,2,fs4,1);
	flow.addConnection(fs2,1,fs3,2);
	flow.addConnection(fs3,1,fs4,2);
	
	flow.printTopology();
	
	vector<Operator *> operators = flow.getOperators();
	for(auto it=operators.begin(); it!=operators.end(); it++){
		cout<< (*it)->getId() <<endl;
	}
	
	cout<<endl;
	vector<Operator *> neighbors = flow.getNeighbors(fs1);
	for(auto it=neighbors.begin(); it!=neighbors.end(); it++){
		cout<< (*it)->getId() <<endl;
	}

	
/*
	queue<Tuple>* q = new queue<Tuple>();
	OutputPortImpl* fs1OP = new OutputPortImpl(q);

	FileSource fs("fs1", "input1.txt");
	
	fs.addOutputPort(fs1OP);
	fs.init();
	fs.process();

	cout<<q->size()<<endl;
	Tuple t = q->front();
	cout<<t.getIntAttribute("key0")<<endl;
	//q->pop();
*/
	/*
	queue<Tuple>* shared = new queue<Tuple>();

	InputPortImpl* ipImpl = new InputPortImpl(shared);
	OutputPortImpl* opImpl = new OutputPortImpl(shared);

	Tuple t1;
	t1.addAttribute("key1", (int64_t)3);
	t1.addAttribute("key2", (int64_t)4);
	opImpl->pushTuple(t1);
	
	Tuple t2;
	t2.addAttribute("key1", (double)1.1);
	t2.addAttribute("key2", (double)2.2);
	opImpl->pushTuple(t2);
	
	Tuple t = ipImpl->frontTuple();
	cout<<"int values "<<t.getIntAttribute("key1")<<" "<<t.getIntAttribute("key2")<<endl;
	ipImpl->popTuple();

	Tuple x = ipImpl->frontTuple();
	cout<<"double values "<<x.getDoubleAttribute("key1")<<" "<<x.getDoubleAttribute("key2")<<endl;
	ipImpl->popTuple();
	*/
	return 0;
}
