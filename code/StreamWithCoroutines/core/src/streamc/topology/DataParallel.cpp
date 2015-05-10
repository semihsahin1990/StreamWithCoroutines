#include <string>
#include "streamc/topology/DataParallel.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/Timestamper.h"
#include "streamc/operators/Split.h"
#include "streamc/operators/Selective.h"
#include "streamc/operators/Busy.h"
#include "streamc/operators/Merge.h"
#include "streamc/operators/ResultCollector.h"
#include "streamc/operators/FileSink.h"
#include <vector>

using namespace std;
using namespace streamc;
using namespace streamc::operators;
using namespace streamc::connectors;

// source + timestamper + split + | n x (selective+busy) + +merge + result collector + sink
DataParallel::DataParallel(vector<double> costList, double selectivity, size_t n) 
	: selectivity_(selectivity), n_(n), flow_("DataParallel")
{
  // create source
  Operator & src = flow_.createOperator<FileSource>("src")
    .set_fileName("data/in.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}, {"lineNo", Type::Integer}});

  // create timestamper
  Operator & timestamper = flow_.createOperator<Timestamper>("timestamper");

  // create split
  Operator & split = flow_.createOperator<Split>("split", n_)
    .set_hashFunction(MEXP1( t_.get<Type::Integer>("lineNo") ));
  
  // create nodes
  int numberOfNodes = n_;
  for(size_t i=0; i<numberOfNodes; i++) {
    Operator & selective = flow_.createOperator<Selective>("selective"+to_string(i), selectivity_);
    selectiveOps_.push_back(&selective);

    Operator & busy = flow_.createOperator<Busy>("busy"+to_string(i), costList[i], selectivity_);
    busyOps_.push_back(&busy);
  }
  
  // create merge
  Operator & merge = flow_.createOperator<Merge>("merge", n_);

  // create result collector
  Operator & resultCollector = flow_.createOperator<ResultCollector>("ResultCollector", "expData/result.dat");

  // create sink
  Operator & snk = flow_.createOperator<FileSink>("snk")
    .set_fileName("data/out.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});

  // add connections
  flow_.addConnection((src, 0) >> (0, timestamper));
  flow_.addConnection((timestamper, 0) >> (0, split));
  for(size_t i=0; i<n_; i++) {
    flow_.addConnection((split, i) >> (0,*selectiveOps_[i]));
    flow_.addConnection((*selectiveOps_[i], 0) >> (0,*busyOps_[i]));
    flow_.addConnection((*busyOps_[i], 0) >> (i,merge));
  }

  flow_.addConnection((merge,0) >> (0, resultCollector));
  flow_.addConnection((resultCollector,0) >> (0, snk));
}