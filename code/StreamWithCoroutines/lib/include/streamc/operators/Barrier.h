#include "streamc/Operator.h"

#include <iostream>
#include <vector>

namespace streamc { namespace operators
{

class Barrier : public Operator
{
public:
  Barrier(std::string const & name);
  void process(OperatorContext & context) override;
private:
};

} } // namespace streamc::operators


