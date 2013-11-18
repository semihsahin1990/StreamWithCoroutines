#include "streamc/Operator.h"

#include <iostream>
#include <vector>
#include <functional>

namespace streamc 
{

class Filter : public Operator
{
public:
  //constructor with name
  Filter(std::string const & name);
  //constructor with name, and filtering function
  Filter(std::string const & name, std::function<bool (Tuple &)> filter);
  //set filtering function
  Filter & set_filter(std::function<bool (Tuple &)> filter);
  //virtual init and process method of base 'Operator'
  void init(OperatorContext & context) override;
  void process(OperatorContext & context) override;

private:
  //properties of filter
  InputPort * iport_;
  OutputPort * oport_;
  std::function<bool (Tuple &)> filter_;
};

} // namespace streamc


