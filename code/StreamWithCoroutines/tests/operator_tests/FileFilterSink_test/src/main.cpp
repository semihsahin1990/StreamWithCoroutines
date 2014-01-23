#include "streamc/Flow.h"
#include "streamc/FlowRunner.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/FileSink.h"
#include "streamc/operators/Filter.h"

#include "gtest/gtest.h"

#include <cstdlib>
#include <thread>
#include <chrono>
#include <fstream>
#include <regex>

using namespace std;
using namespace streamc;
using namespace streamc::operators;
using namespace streamc::connectors;

class FileFilterSink_test : public ::testing::Test {
public:
  FileFilterSink_test() : flow("simple file filtering") {
    flow.addConnections( (src,0) >> (0,fltF,0) >> (0,fltG,0) >> (0,snk) ); 
    // flow.printTopology(std::cout);
   
    // alternatives:
    // flow.addConnection( (src,0) >> (0,flt) ); 
    // flow.addConnection( (flt,0) >> (0,snk) ); 
    // flow.addConnection(src, 0, flt, 0);
  }

  void runFlow() {
    FlowRunner & runner = FlowRunner::createRunner();
    runner.setInfrastructureLogLevel(Info);
    runner.setApplicationLogLevel(Trace);

    // run with two threads
    runner.run(flow, 2);
    // wait for quarter of a second
    this_thread::sleep_for(chrono::milliseconds(250)); 
    // request the flow to be shutdown
    runner.requestShutdown(flow); 
    // wait until shutdown
    runner.wait(flow); 

    // let's see how many lines are there
    system("wc -l data/out.dat");

    // run the flow again (it will resume from where it left)
    // this time, use three threads
    runner.run(flow, 3);
    // wait for completion (this time we don't explicity ask for shutdwon, it
    // completes when there is no more work to do)
    runner.wait(flow);
    system("wc -l data/out.dat");
  }

protected:
  Flow flow;
  Operator & src = flow.createOperator<FileSource>("src")
    .set_fileName("data/in.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});

  Operator & fltF = flow.createOperator<Filter>("fltF")
    .set_filter(MEXP1( t_.get<Type::String>("grade") != "F" ));
  
  Operator & fltG = flow.createOperator<Filter>("fltG")
    .set_filter(MEXP1( t_.get<Type::String>("name") != "Bugra" ));

  Operator & snk = flow.createOperator<FileSink>("snk")
    .set_fileName("data/out.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});
};

TEST_F(FileFilterSink_test, FlowTest) {
  vector<Operator *> operators = flow.getOperators();

  EXPECT_EQ(4, operators.size());

  EXPECT_NE(find(operators.begin(), operators.end(), &src), operators.end());
  EXPECT_NE(find(operators.begin(), operators.end(), &fltF), operators.end());
  EXPECT_NE(find(operators.begin(), operators.end(), &fltG), operators.end());
  EXPECT_NE(find(operators.begin(), operators.end(), &snk), operators.end());
}

TEST_F(FileFilterSink_test, TopologyTest_OutConnections) {
  vector<connectors::ToConnection> outConnectionsSrc = flow.getOutConnections(src, 0);
  EXPECT_EQ(1, outConnectionsSrc.size());
  EXPECT_EQ(&(outConnectionsSrc.begin()->getOperator()), &fltF);

  vector<connectors::ToConnection> outConnectionsFltF = flow.getOutConnections(fltF, 0);
  EXPECT_EQ(1, outConnectionsFltF.size());
  EXPECT_EQ(&(outConnectionsFltF.begin()->getOperator()), &fltG);

  vector<connectors::ToConnection> outConnectionsFltG = flow.getOutConnections(fltG, 0);
  EXPECT_EQ(1, outConnectionsFltG.size());
  EXPECT_EQ(&(outConnectionsFltG.begin()->getOperator()), &snk);
}

TEST_F(FileFilterSink_test, TopologyTest_InConnections) {
  vector<connectors::FromConnection> inConnectionsFltF = flow.getInConnections(fltF, 0);
  EXPECT_EQ(1, inConnectionsFltF.size());
  EXPECT_EQ(&(inConnectionsFltF.begin()->getOperator()), &src);

  vector<connectors::FromConnection> inConnectionsFltG = flow.getInConnections(fltG, 0);
  EXPECT_EQ(1, inConnectionsFltG.size());
  EXPECT_EQ(&(inConnectionsFltG.begin()->getOperator()), &fltF);

  vector<connectors::FromConnection> inConnectionsSnk = flow.getInConnections(snk, 0);
  EXPECT_EQ(1, inConnectionsSnk.size());
  EXPECT_EQ(&(inConnectionsSnk.begin()->getOperator()), &fltG);
}


TEST_F(FileFilterSink_test, OutputTest) {
  runFlow();

  string line;
  std::regex sep(",");
  sregex_token_iterator end;

  int outputLength = 0;
  int lineLength;

  ifstream input;
  string outputFile = "data/out.dat";
  input.open(outputFile.c_str(), ios::in);
  unordered_map<string, int> attCounts;

  while(true) {
    line.clear();
    getline(input, line);

    if(line.size()==0 && input.eof())
        break;

    lineLength = 0;
    sregex_token_iterator tokenIt(line.begin(), line.end(), sep, -1);
    while(tokenIt != end) {
      if(attCounts.find(*tokenIt) == attCounts.end())
        attCounts[*tokenIt] = 0;
      attCounts[*tokenIt]++;
      lineLength++;
      tokenIt++;
    }

    EXPECT_EQ(2, lineLength);
    outputLength++;
  }
  input.close();

  EXPECT_EQ(14400, attCounts["Semih"]);
  EXPECT_EQ(0, attCounts["Bugra"]);
  EXPECT_EQ(14400, attCounts["Ahmet"]);
  EXPECT_EQ(14400, attCounts["Mehnet"]);
  EXPECT_EQ(0, attCounts["Aslı"]);
  EXPECT_EQ(14400, attCounts["Yasemin"]);
  EXPECT_EQ(14400, attCounts["Vehbi"]);
  EXPECT_EQ(28800, attCounts["A"]);
  EXPECT_EQ(0, attCounts["B"]);
  EXPECT_EQ(28800, attCounts["C"]);
  EXPECT_EQ(14400, attCounts["D"]);
  EXPECT_EQ(0, attCounts["F"]);

  EXPECT_EQ(72000, outputLength);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();

}


