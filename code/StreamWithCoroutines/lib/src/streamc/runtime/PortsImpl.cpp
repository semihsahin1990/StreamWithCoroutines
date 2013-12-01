#include "streamc/runtime/PortsImpl.h"
#include "streamc/InputPort.h"

#include<vector>

using namespace std;
using namespace streamc;

// TODO: this class is work under progress

bool PortsImpl::waitTupleFromAll(vector<InputPort *>ports)
{
  bool allAvailable = true;

  for(vector<InputPort *>::iterator it = ports.begin(); it != ports.end(); it++)
    allAvailable = allAvailable & ((*it)->waitTuple());

  return allAvailable;
}
