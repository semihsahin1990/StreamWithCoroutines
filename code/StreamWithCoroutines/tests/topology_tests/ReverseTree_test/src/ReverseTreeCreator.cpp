#include <string>
#include "ReverseTreeCreator.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/Union.h"
#include "streamc/operators/Barrier.h"
#include "streamc/operators/FileSink.h"

using namespace std;
using namespace streamc;
using namespace streamc::operators;
using namespace streamc::connectors;

ReverseTreeCreator::ReverseTreeCreator(size_t depth) 
	: depth_(depth), flow_("reverse tree")
{
	// create sources
	int numberOfSources = pow(2, depth_-1);
	for(size_t i=0; i<numberOfSources; i++) {
		Operator & src = flow_.createOperator<FileSource>("src"+to_string(i))
	  		.set_fileName("data/in.dat")
	  		.set_fileFormat({{"name",Type::String}, {"grade",Type::String}});

	  	sources_.push_back(&src);
	}

	// create unions
	int numberOfUnions = pow(2, depth_-1) - 1;
	for(size_t i=0; i<numberOfUnions; i++) {
		Operator & myUnion = flow_.createOperator<Union>("union"+to_string(i), 2);
		unions_.push_back(&myUnion);
	}

	// create sink
	Operator & snk = flow_.createOperator<FileSink>("snk")
    .set_fileName("data/out.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});

	//flow.addConnection( (src1,0) >> (0,myUnion));
	for(size_t i=0; i<numberOfSources; i+=2) {
		flow_.addConnection((*sources_[i],0) >> (0,*unions_[i/2]));
		flow_.addConnection((*sources_[i+1],0) >> (1,*unions_[i/2]));
	}

	size_t target = pow(2, depth_-2);
	for(size_t i=0; i<numberOfUnions-1; i+=2, target++) {
		cout<<i<<" "<<i+1<<" --> "<<target<<endl;
		flow_.addConnection((*unions_[i],0) >> (0,*unions_[target]));
		flow_.addConnection((*unions_[i+1],0) >> (1,*unions_[target]));
	}
	cout<<numberOfUnions-1<<" "<<numberOfUnions-1<<" --> snk"<<endl;
	flow_.addConnection((*unions_[numberOfUnions-1],0) >> (0,snk));
}