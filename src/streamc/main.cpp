#include<iostream>
#include<vector>
#include<queue>

#include "Tuple.h"
#include "InputPort.h"
#include "OutputPort.h"
#include "Operator.h"
#include "Flow.h"
#include "myOp.h"

using namespace std;

int main()
{
	myOp* mop1 = new myOp(1);
	myOp* mop2 = new myOp(2);
	myOp* mop3 = new myOp(3);
	myOp* mop4 = new myOp(4);
	
	Operator *op1 = mop1;
	Operator *op2 = mop2;
	Operator *op3 = mop3;
	Operator *op4 = mop4;

	uint32_t oport, iport;
	
	Flow flow;
	
	flow.connect(op1,5,op3,6);
	flow.connect(op3,11,op4,12);
	flow.connect(op2,7,op3,8);
	flow.connect(op1,9,op4,10);


	flow.printTopology();
	/*
	Tuple t;
	string name;
	int64_t x;
	double y;
	string z;
	
	name = "1"; x = 11;
	t.addAttribute(name,x);
	name = "2"; x = 22;
	t.addAttribute(name,x);
	name = "3"; x = 33;
	t.addAttribute(name,x);
	
	cout<<t.getIntAttribute("1")<<endl;

	queue<Tuple>* portQ = new queue<Tuple>();
	InputPort* iport = new InputPort(portQ);
	OutputPort* oport = new OutputPort(portQ);
	
	cout<<iport->hasTuple()<<endl;
	oport->pushTuple(t);
	
	cout<<iport->hasTuple()<<endl;
	Tuple tt = iport->popTuple();
	*/
	/*
	name = "1"; x = 11;
	t.addAttribute(name,x);
	name = "2"; x = 22;
	t.addAttribute(name,x);
	name = "3"; x = 33;
	t.addAttribute(name,x);


	name = "4"; y = 4.4;
	t.addAttribute(name,y);
	name = "5"; y = 5.5;
	t.addAttribute(name,y);

	name = "6"; z = "66";
	t.addAttribute(name,z);
	name = "7"; z = "77";
	t.addAttribute(name,z);

	cout<< t.getIntAttribute("1")<<endl;
	cout<< t.getIntAttribute("2")<<endl;
	cout<< t.getIntAttribute("3")<<endl;
	cout<< t.getDoubleAttribute("4")<<endl;
	cout<< t.getDoubleAttribute("5")<<endl;
	cout<< t.getStringAttribute("6")<<endl;
	cout<< t.getStringAttribute("7")<<endl;

	vector<int64_t> myVector;
	x = 123;
	myVector.push_back(x);
	x = 456;
	myVector.push_back(x);
	
	name = "8";
	t.addAttribute(name, myVector);

	name = "9";
	x = 789;
	myVector.pop_back();
	myVector.push_back(x);
	t.addAttribute(name, myVector);

	vector<int64_t> vec1 = t.getIntListAttribute("8");
	cout<<vec1.size()<<endl;
	vector<int64_t> vec2 = t.getIntListAttribute("9");
	cout<<vec2.size()<<endl;

	for(int i=0; i<vec1.size(); i++){
		cout<<vec1.at(i)<<" ";
	}
	cout<<endl;
	
	vec1.pop_back();
	vec1 = t.getIntListAttribute("8");
	for(int i=0; i<vec1.size(); i++){
		cout<<vec1.at(i)<<" ";
	}
	cout<<endl;

	for(int i=0; i<vec2.size(); i++){
		cout<<vec2.at(i)<<" ";
	}
	cout<<endl;

	vector<int> a;
	a.push_back(1);
	a.push_back(2);

	vector<int> *b = new vector<int>(a);
	b->push_back(3);

	for(int i=0; i<a.size(); i++){
		cout<<a.at(i)<<" ";
	}
	cout<<endl;
	
	for(int i=0; i<b->size(); i++){
		cout<<b->at(i)<<" ";
	}
	cout<<endl;
	*/	
	return 0;
}
