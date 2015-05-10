#include <string>
#include "streamc/topology/Chain.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/TupleGenerator.h"
#include "streamc/operators/Timestamper.h"
#include "streamc/operators/Busy.h"
#include "streamc/operators/ResultCollector.h"
#include "streamc/operators/FileSink.h"

using namespace std;
using namespace streamc;
using namespace streamc::operators;
using namespace streamc::connectors;

// source + timestamper + depth x (busy) + resultcollector + sink
Chain::Chain(size_t depth, vector<double> costList, double selectivity) 
	: depth_(depth), selectivity_(selectivity), flow_("chain")
{
  // create source
  /*
  Operator & src = flow_.createOperator<FileSource>("src")
    .set_fileName("data/in.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}, {"lineNo", Type::Integer}});
  */
  
  Operator & src = flow_.createOperator<TupleGenerator>("src")
    .set_tupleFormat({{"name",Type::String}, {"grade",Type::String}, {"lineNo", Type::Integer}});

  Operator & timestamper = flow_.createOperator<Timestamper>("timestamper");

  // create busy operators
  for(size_t i=0; i<depth_; i++) {
    Operator & busy = flow_.createOperator<Busy>("busy"+to_string(i), costList[i], selectivity_);
    busyOps_.push_back(&busy);
  }

  // create result collector
  Operator & resultCollector = flow_.createOperator<ResultCollector>("resultCollector", "expData/result.dat");

  // create FileSink
  Operator & snk = flow_.createOperator<FileSink>("snk")
    .set_fileName("data/out.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});

  // add connections
  flow_.addConnection((src, 0) >> (0, timestamper));
  flow_.addConnection((timestamper, 0) >> (0, *busyOps_[0]));
  for(size_t i=0; i<depth_-1; i++)
    flow_.addConnection((*busyOps_[i], 0) >> (0, *busyOps_[i+1]));
  flow_.addConnection((*busyOps_[depth_-1], 0) >> (0, resultCollector));
  flow_.addConnection((resultCollector, 0) >> (0, snk));

}