#include <iostream>
#include <unordered_map>
#include <vector>

#include "streamc/Operator.h"

namespace streamc
{

class Flow
{
public:
  virtual void addConnection(Operator *op1, uint32_t oport, Operator *op2, uint32_t iport);
  virtual std::vector<Operator *> getNeighbors(Operator *op);
  virtual std::vector<Operator *> getOperators();
  virtual void printTopology();
};

}
