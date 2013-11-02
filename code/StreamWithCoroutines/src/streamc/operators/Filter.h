#include "streamc/Operator.h"

#include <iostream>
#include <vector>
#include <functional>

namespace streamc 
{

class Filter : public Operator
{
public:
  Filter(std::string const & name, std::function<bool (Tuple &)> filter);
  void init(OperatorContext & context);
  void process(OperatorContext & context);
private:
  InputPort * iport_;
  OutputPort * oport_;
  std::function<bool (Tuple &)> filter_;
};

} // namespace streamc


