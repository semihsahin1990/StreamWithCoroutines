#include <string>
#include "streamc/topology/Tree.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/Timestamper.h"
#include "streamc/operators/Selective.h"
#include "streamc/operators/Busy.h"
#include "streamc/operators/RoundRobinSplit.h"
#include "streamc/operators/ResultCollector.h"
#include "streamc/operators/FileSink.h"
#include <vector>

using namespace std;
using namespace streamc;
using namespace streamc::operators;
using namespace streamc::connectors;

// src + timestamper + | 1 x (selective + busy + RRsplit) + ... + | n^(depth-2) x (selective + busy + RRSplit) + | n^(depth-1) x (selective + busy) + | n^(depth-1) x (resultCollector+sink)
Tree::Tree(size_t depth, vector<double> costList, double selectivity, size_t n) 
	: depth_(depth), selectivity_(selectivity), n_(n), flow_("tree")
{
	// create source
	Operator & src = flow_.createOperator<FileSource>("src")
	    .set_fileName("data/in.dat")
	    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}, {"lineNo", Type::Integer}});

	// create timestamper
	Operator & timestamper = flow_.createOperator<Timestamper>("timestamper");

	// create Nodes (selective-cost)
	size_t numberOfNodes = (pow(n_, depth_)-1) / (n_-1);
	for(size_t i=0; i<numberOfNodes; i++) {
    	Operator & selective = flow_.createOperator<Selective>("selective"+to_string(i), selectivity_);
    	selectiveOps_.push_back(&selective);

    	Operator & busy = flow_.createOperator<Busy>("busy"+to_string(i), costList[i], selectivity_);
    	busyOps_.push_back(&busy);
	}

	// create splits
	size_t numberOfSplits = (pow(n_, depth_-1)-1) / (n_-1);
	for(size_t i=0; i<numberOfSplits; i++) {
		Operator & split = flow_.createOperator<RoundRobinSplit>("rrSplit"+to_string(i), n_);
		splitOps_.push_back(&split);
	}

	// create result collectors & sinks
	size_t numberOfLeaves = pow(n_, depth-1);
	for(size_t i=0; i<numberOfLeaves; i++) {
		Operator & resultCollector = flow_.createOperator<ResultCollector>("resultCollector"+to_string(i),"expData/result"+to_string(i)+".dat");
		resultCollectorOps_.push_back(&resultCollector);

		Operator & snk = flow_.createOperator<FileSink>("snk"+to_string(i))
  			.set_fileName("data/out"+to_string(i)+".dat")
  			.set_fileFormat({{"name",Type::String}, {"grade",Type::String}});
  		sinkOps_.push_back(&snk);
	}
	
	// connect operators
	flow_.addConnection((src, 0) >> (0, timestamper));
  flow_.addConnection((timestamper, 0) >> (0, *selectiveOps_[0]));

  for(size_t i=0; i<numberOfNodes; i++) {
	  flow_.addConnection((*selectiveOps_[i], 0) >> (0, *busyOps_[i]));
	  // cout<<"selective "<<i<<" port 0\t busy "<<i<<"port 0"<<endl;
	}

  for(size_t i=0; i<numberOfSplits; i++) {
    flow_.addConnection((*busyOps_[i], 0) >> (0, *splitOps_[i]));
    //	cout<<"busy "<<i<<" port 0\t split "<<i<<"port 0"<<endl;
  }

	size_t nNodeToNode = numberOfNodes - numberOfLeaves;
	for(size_t i=0; i<nNodeToNode; i++) {
		for(size_t j=0; j<n_; j++) {
			flow_.addConnection((*splitOps_[i], j) >> (0, *selectiveOps_[i*n_+j+1]));
      	//	cout<<"split "<<i<<" port "<<j<<"\t"<<"selective "<<(i*n+j+1)<<endl;
		}
	}
	
	for(size_t i=0; i<numberOfLeaves; i++) {
		flow_.addConnection((*busyOps_[i+nNodeToNode], 0) >> (0, *resultCollectorOps_[i]));
		flow_.addConnection((*resultCollectorOps_[i], 0) >> (0 , *sinkOps_[i]));
		//cout<<"busy "<<i+nNodeToNode<<" port "<<0<<"\t"<<" resultCollector "<<i<<endl;
	}
}


