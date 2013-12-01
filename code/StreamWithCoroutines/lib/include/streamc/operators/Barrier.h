#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc 
{

class Barrier : public Operator
{
public:
  Barrier(std::string const & name);
  void init(OperatorContext & context) override;
  void process(OperatorContext & context) override;

private:
  InputPort *iport1_;
  InputPort *iport2_;

  OutputPort * oport_;
};

} // namespace streamc


