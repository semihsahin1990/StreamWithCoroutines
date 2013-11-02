#include "streamc/Operator.h"

#include <iostream>
#include <vector>
#include <functional>

namespace streamc 
{

class Filter : public Operator
{
private:
  InputPort * iport_;
  OutputPort * oport_;
  std::function<bool (Tuple &)> filter_;
public:
  Filter(std::string const & name, std::function<bool (Tuple &)> filter);
       
protected:
  void init(OperatorContext & context);
  void process(OperatorContext & context);
};

}


