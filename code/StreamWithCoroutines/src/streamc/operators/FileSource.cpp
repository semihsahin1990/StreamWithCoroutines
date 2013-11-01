#include<iostream>
#include<fstream>
#include<sstream>
#include "streamc/operators/FileSource.h"

using namespace streamc;
using namespace std;


FileSource::FileSource(string id, string fileName) : Operator(id){
	fileName_ = fileName;
}

void FileSource::init(){
	oport_ = getOutputPorts()[0];
}

void FileSource::process(){
	int64_t value;
	int counter=0;
	stringstream key;

	ifstream input;
	input.open(fileName_.c_str(), ios::in);

	while(!isShutdownRequested()){
		input>>value;
		if(input.eof())
			break;

		key.str("");
		key<<"key"<<counter++;

		Tuple t;
		t.addAttribute(key.str(), value);
		oport_->pushTuple(t);
	}
	input.close();
}
