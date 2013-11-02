#include "streamc/Tuple.h"
#include "streamc/Flow.h"
#include "streamc/FlowRunner.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/FileSink.h"
#include "streamc/operators/Filter.h"

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
  FileSink * snk = new FileSink("snk", "snk.dat");
  
  // connections
  flow.addOperator(src);
  flow.addOperator(flt);
  flow.addOperator(snk);
  flow.addConnection(src,0, flt,0);
  flow.addConnection(flt,0, snk,0);
	
  flow.printTopology(std::cout);
	  
  // run it!
  FlowRunner & runner = FlowRunner::createRunner();
  runner.run(flow, 5);
  runner.wait(flow);
  
  // alternative: sleep(10); runner.shutdown(flow); runner.wait(flow);        
}
