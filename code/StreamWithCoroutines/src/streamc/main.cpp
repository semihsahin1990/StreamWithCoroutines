#include<iostream>
#include<sstream>
#include<queue>

#include "streamc/Tuple.h"
#include "streamc/InputPort.h"
#include "streamc/OutputPort.h"
#include "streamc/Operator.h"
#include "streamc/FlowRunner.h"
#include "streamc/runtime/InputPortImpl.h"
#include "streamc/runtime/OutputPortImpl.h"
#include "streamc/operators/FileSource.h"

using namespace std;
using namespace streamc;

int main()
{
  // operator instances
  Flow flow("test flow");
  FileSource * src = new FileSource("src", "src.dat", 
      {{"name",Type::String}, {"grade",Type::String}});
  Filter * flt = new Filter("flt", [] (Tuple & t) { 
      return t.getStringAttribute("grade") != "F"; 
  });
  FileSink * snk = new FileSource("snk", "snk.dat");
  
  // connections
  flow.addOperator(src);
  flow.addOperator(flt);
  flow.addOperator(snk);
  flow.connectOperators(src,0, flt,0);
  flow.connectOperators(flt,0, snk,0);
	
  flow.printTopology();
	  
  // run it!
  FlowRunner const & runner = FlowRunner::createFlowRunner();
  runner.run(flow, 5);
  runner.wait(flow);
  
  // alternative: sleep(10); runner.shutdown(flow); runner.wait(flow);        
}
