#include <string>
#include "TreeCreator.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/Split.h"
#include "streamc/operators/FileSink.h"
#include <boost/functional/hash.hpp>

using namespace std;
using namespace streamc;
using namespace streamc::operators;
using namespace streamc::connectors;

TreeCreator::TreeCreator(size_t depth, size_t n) 
	: depth_(depth), n_(n), flow_("tree")
{
	//create source
	Operator & src = flow_.createOperator<FileSource>("src")
  		.set_fileName("data/in.dat")
  		.set_fileFormat({{"name",Type::String}, {"grade",Type::String}});

  	// create splits
  	size_t numberOfSplits = (pow(n_, depth_-2)-1) / (n_-1);
  	for(size_t i=0; i<numberOfSplits; i++) {
  		Operator & split = flow_.createOperator<Split>("split"+to_string(i), n_)
  			.set_hashFunction(MEXP1( t_.get<Type::String>("name") != "Bugra" ? 0 : 1));
  		splits_.push_back(&split);
  	}
  	// create sinks
  	size_t numberOfSinks = pow(n_, depth_-2);
  	for(size_t i=0; i<numberOfSinks; i++) {
  		Operator & snk = flow_.createOperator<FileSink>("snk"+to_string(i))
    		.set_fileName("data/out"+to_string(i)+".dat")
    		.set_fileFormat({{"name",Type::String}, {"grade",Type::String}});
    	sinks_.push_back(&snk);
  	}

  	// connect operators
  	flow_.addConnection((src, 0) >> (0, *splits_[0]));

  	size_t nSplitToSplit = (pow(n_, depth_-3)-1)/(n_-1);
  	for(size_t i=0; i<nSplitToSplit; i++) {
  		for(size_t j=0; j<n_; j++) {
  			cout<<"id: "<<i<<" port: "<<j<<" -->> "<<"id: "<<i*n_+j+1<<endl;
  			flow_.addConnection((*splits_[i], j) >> (0, *splits_[i*n_+j+1]));
  		}
  	}

  	cout<<"splits to sinks"<<endl;
  	size_t sinkID = 0;
  	for(size_t i=nSplitToSplit; i<numberOfSplits; i++) {
  		for(size_t j=0; j<n_; j++, sinkID++) {
  			cout<<"id: "<<i<<" port: "<<j<<" -->> "<<"id: "<<sinkID<<endl;
  			flow_.addConnection((*splits_[i], j) >> (0, *sinks_[sinkID]));
  		}
  	}
}