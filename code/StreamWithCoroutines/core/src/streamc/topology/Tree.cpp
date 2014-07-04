#include <string>
#include "streamc/topology/Tree.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/Timestamper.h"
#include "streamc/operators/Selective.h"
#include "streamc/operators/Busy.h"
#include "streamc/operators/Split.h"
#include "streamc/operators/ResultCollector.h"
#include "streamc/operators/FileSink.h"

using namespace std;
using namespace streamc;
using namespace streamc::operators;
using namespace streamc::connectors;

// src + timestamper + | 1 x (selective + busy + RRsplit) + ... + | n^(depth-1) x (selective + busy + RRSplit) + | n^(depth-1) x (resultCollector+sink)
Tree::Tree(size_t depth, uint64_t cost, double selectivity, size_t n) 
	: depth_(depth), cost_(cost), selectivity_(selectivity), n_(n), flow_("tree")
{
	// create source
	Operator & src = flow_.createOperator<FileSource>("src")
  		.set_fileName("data/in.dat")
  		.set_fileFormat({{"name",Type::String}, {"grade",Type::String}, {"id", Type::Integer}});

  	// create timestamper
  	Operator & timestamper = flow_.createOperator<Timestamper>("timestamper");

	// create Nodes (selective-cost-split)
	size_t numberOfNodes = (pow(n_, depth_)-1) / (n_-1);
	for(size_t i=0; i<numberOfNodes; i++) {
    	Operator & selective = flow_.createOperator<Selective>("selective"+to_string(i), selectivity_);
    	selectiveOps_.push_back(&selective);

    	Operator & busy = flow_.createOperator<Busy>("busy"+to_string(i), cost_);
    	busyOps_.push_back(&busy);

		Operator & split = flow_.createOperator<Split>("split"+to_string(i), n_)
			.set_hashFunction(MEXP1( t_.get<Type::Integer>("id")));
		splitOps_.push_back(&split);
	}

	// create sinks
	size_t numberOfLeaves = pow(n_, depth-1);
	for(size_t i=0; i<numberOfLeaves; i++) {
		Operator & snk = flow_.createOperator<FileSink>("snk"+to_string(i))
  			.set_fileName("data/out"+to_string(i)+".dat")
  			.set_fileFormat({{"name",Type::String}, {"grade",Type::String}});
  		sinkOps_.push_back(&snk);
	}

	// connect operators
	flow_.addConnection((src, 0) >> (0, timestamper));
  	flow_.addConnection((src, 0) >> (0, *selectiveOps_[0]));

  	for(size_t i=0; i<numberOfNodes; i++) {
	    flow_.addConnection((*selectiveOps_[i], 0) >> (0, *busyOps_[i]));
    	flow_.addConnection((*busyOps_[i], 0) >> (0, *splitOps_[i]));
  	}

	size_t nNodeToNode = numberOfNodes - numberOfLeaves;
	for(size_t i=0; i<nNodeToNode; i++) {
		for(size_t j=0; j<n_; j++) {
			flow_.addConnection((*splitOps_[i], j) >> (0, *selectiveOps_[i*n_+j+1]));
      		//cout<<"split "<<i<<" port "<<j<<"\t"<<"selective "<<(i*n+j+1)<<endl;
		}
	}
  
	for(size_t i=0; i<numberOfLeaves; i++) {
		for(size_t j=0; j<n_; j++) {
			flow_.addConnection((*splitOps_[i+nNodeToNode], j) >> (0, *sinkOps_[i]));
    		//cout<<"split "<<i+nNodeToNode<<" port "<<j<<"\t"<<" sink "<<i<<endl;
		}
	}
}