#include<iostream>
#include "Operator.h"
#include "InputPort.h"

using namespace std;

class myOp : public Operator{
public:
	myOp(int id) : Operator(id){
	}

	void process(){
		cout<<"processing"<<endl;	
	}
};
