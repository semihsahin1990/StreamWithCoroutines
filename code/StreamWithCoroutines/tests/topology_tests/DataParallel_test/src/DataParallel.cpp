#include <string>
#include "DataParallel.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/Split.h"
#include "streamc/operators/Union.h"
#include "streamc/operators/Filter.h"
#include "streamc/operators/Merge.h"
#include "streamc/operators/FileSink.h"
#include <boost/functional/hash.hpp>

using namespace std;
using namespace streamc;
using namespace streamc::operators;
using namespace streamc::connectors;

DataParallel::DataParallel(size_t n) 
	: n_(n), flow_("data parallel")
{
  // create source
  Operator & src = flow_.createOperator<FileSource>("src")
    .set_fileName("data/in.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}, {"lineNo", Type::Integer}});
  
  // create split
  Operator & split = flow_.createOperator<Split>("split", n_)
    .set_hashFunction(MEXP1( t_.get<Type::Integer>("lineNo") ));
  
  // create unions
  int numberOfUnions = n_;
  for(size_t i=0; i<numberOfUnions; i++) {
    //Operator & myUnion = flow_.createOperator<Union>("union"+to_string(i), 1);
    //unions_.push_back(&myUnion);

    Operator & fltF = flow_.createOperator<Filter>("fltF"+to_string(i))
      .set_filter(MEXP1( t_.get<Type::String>("grade") != "F" ));
    unions_.push_back(&fltF);
  }
  
  // create merge
  Operator & merge = flow_.createOperator<Merge>("merge", n_);

  // create FileSink
  Operator & snk = flow_.createOperator<FileSink>("snk")
    .set_fileName("data/out.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});

  // add connections
  flow_.addConnection((src, 0) >> (0, split));

  for(size_t i=0; i<n_; i++)
    flow_.addConnection((split, i) >> (0,*unions_[i]));

  for(size_t i=0; i<n_; i++)
    flow_.addConnection((*unions_[i],0) >> (i,merge));

  flow_.addConnection((merge,0) >> (0, snk));
}