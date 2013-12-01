#include "streamc/Operator.h"

#include <iostream>
#include <vector>
#include <functional>

namespace streamc 
{

class Filter : public Operator
{
public:
  Filter(std::string const & name);
  Filter(std::string const & name, std::function<bool (Tuple &)> filter);
  Filter & set_filter(std::function<bool (Tuple &)> filter);
  void process(OperatorContext & context) override;
private:
  std::function<bool (Tuple &)> filter_;
};

} // namespace streamc


