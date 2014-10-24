#include "streamc/operators/Selective.h"

#include <random>

using namespace streamc;
using namespace streamc::operators;
using namespace std;

Selective::Selective(std::string const & name, double selectivity)
  : Operator(name, 1, 1), selectivity_(selectivity)
{}

Selective::Selective(std::string const & name)
  : Selective(name, 1.0)
{}

Selective & Selective::set_selectivity(double selectivity)
{
  selectivity_ = selectivity;
  return *this;
}

void Selective::process(OperatorContext & context)
{
  InputPort & iport = context.getInputPort(0);
  OutputPort & oport = context.getOutputPort(0);

  mt19937_64 randgen_;
  randgen_.seed(mt19937_64::default_seed);
  std::uniform_real_distribution<double> zero_one(0.0, 1.0);

  while (!context.isShutdownRequested()) {
    bool closed = iport.waitTuple();
    if (closed)
      break;
    Tuple & tuple = iport.getFrontTuple();
    if(zero_one(randgen_) < selectivity_)
      oport.pushTuple(tuple);
    iport.popTuple();
  }
}

Selective * Selective::clone(std::string const & name)
{
  return new Selective(name, selectivity_);
}

