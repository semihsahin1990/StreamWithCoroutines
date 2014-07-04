#include <string>
#include "streamc/topology/ReverseTree.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/Timestamper.h"
#include "streamc/operators/Selective.h"
#include "streamc/operators/Busy.h"
#include "streamc/operators/Union.h"
#include "streamc/operators/ResultCollector.h"
#include "streamc/operators/FileSink.h"

using namespace std;
using namespace streamc;
using namespace streamc::operators;
using namespace streamc::connectors;

// | n^depth-1 x (source+timestamper+selective+busy) + | n^depth-2 x (union+selective+busy) + ... + | 1 x (union+selective+busy) + (resultcollector + sink) 
ReverseTree::ReverseTree(size_t depth, uint64_t cost, double selectivity, size_t n)
	: depth_(depth), cost_(cost), selectivity_(selectivity), n_(n), flow_("reverseTree")
{
	// create sources
	int numberOfSources = pow(n_, depth_-1);
	for(size_t i=0; i<numberOfSources; i++) {
		Operator & src = flow_.createOperator<FileSource>("src"+to_string(i))
	  		.set_fileName("data/in.dat")
	  		.set_fileFormat({{"name",Type::String}, {"grade",Type::String}});
	  	sourceOps_.push_back(&src);

	  	Operator & timestamper = flow_.createOperator<Timestamper>("timestamper"+to_string(i));
	  	timestamperOps_.push_back(&timestamper);
	}

	// create nodes (selective & cost)
	int numberOfNodes = (pow(n_, depth_)-1) / (n_-1) ;
	for(size_t i=0; i<numberOfNodes; i++) {
		Operator & selective = flow_.createOperator<Selective>("selective"+to_string(i), selectivity_);
		selectiveOps_.push_back(&selective);

		Operator & busy = flow_.createOperator<Busy>("busy"+to_string(i), cost_);
		busyOps_.push_back(&busy);
	}

	// create unions
	for(size_t i=numberOfSources; i<numberOfNodes; i++) {
		Operator & myUnion = flow_.createOperator<Union>("union"+to_string(i-numberOfSources), n_);
		unionOps_.push_back(&myUnion);
	}

	// create result collector
	Operator & resultCollector = flow_.createOperator<ResultCollector>("resultCollector", "expData/result.dat");

	// create sink
	Operator & snk = flow_.createOperator<FileSink>("snk")
    	.set_fileName("data/out.dat")
    	.set_fileFormat({{"name",Type::String}, {"grade",Type::String}});

    // connect operators
    for(size_t i=0; i<numberOfSources; i++) {
    	flow_.addConnection((*sourceOps_[i], 0) >> (0, *timestamperOps_[i]));
    	flow_.addConnection((*timestamperOps_[i], 0) >> (0, *selectiveOps_[i]));
    	flow_.addConnection((*selectiveOps_[i], 0) >> (0, *busyOps_[i]));
    //	cout<<"source "<<i<<"\t timestamper "<<i<<endl;
    //	cout<<"timestamper "<<i<<"\t selective "<<i<<endl;
    //	cout<<"selective "<<i<<"\t busy "<<i<<endl;
    }

    for(size_t i=numberOfSources; i<numberOfNodes; i++) {
    	flow_.addConnection((*unionOps_[i-numberOfSources], 0) >> (0, *selectiveOps_[i]));
    	flow_.addConnection((*selectiveOps_[i], 0) >> (0, *busyOps_[i]));
    //	cout<<"union "<<i-numberOfSources<<"\t selective "<<i<<endl;
    //	cout<<"selective "<<i<<"\t busy "<<i<<endl;
    }

	for(size_t i=0; i<numberOfNodes-1; i++) {
		for(size_t j=0; j<n_; j++) {
			flow_.addConnection((*busyOps_[i], 0) >> (j, *unionOps_[i/n_]));
	//		cout<<"busy "<<i<<"\t union "<<(i/n)<<" port "<<j<<endl;
		}
	}

	flow_.addConnection((*busyOps_[numberOfNodes-1],0) >> (0,resultCollector));
	flow_.addConnection((resultCollector, 0) >> (0, snk));
}