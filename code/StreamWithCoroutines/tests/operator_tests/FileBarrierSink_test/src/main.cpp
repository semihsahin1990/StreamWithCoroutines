#include "streamc/Flow.h"
#include "streamc/FlowRunner.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/FileSink.h"
#include "streamc/operators/Filter.h"
#include "streamc/operators/Barrier.h"

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

class FileBarrierSink_test : public ::testing::Test {
public:
  FileBarrierSink_test() : flow("simple barrier") {
    flow.addConnection( (src1,0) >> (0,barrier));
    flow.addConnection( (src2,0) >> (1,barrier));
    flow.addConnection( (src3,0) >> (1,barrier));
    flow.addConnection( (src4,0) >> (2,barrier));
    flow.addConnection( (barrier,0) >> (0,snk));

    //alternatives:
    //flow.addConnection( (src,0) >> (0,flt) ); 
    //flow.addConnection( (flt,0) >> (0,snk) ); 
    //flow.addConnection(src, 0, flt, 0);
    //flow.printTopology(std::cout);
  }

  void runFlow() {
    FlowRunner & runner = FlowRunner::createRunner();
    runner.setInfrastructureLogLevel(Info);
    runner.setApplicationLogLevel(Trace);
    runner.run(flow, 2);
    runner.wait(flow);

    /* 
    // alternative:
    this_thread::sleep_for(chrono::milliseconds(500));
    runner.requestShutdown(flow); 
    runner.wait(flow); 
    */
  }

protected:
  Flow flow;
  Operator & src1 = flow.createOperator<FileSource>("src1")
    .set_fileName("data/in1.dat")
    .set_fileFormat({{"name1",Type::String}, {"grade1",Type::String}});
  
  Operator & src2 = flow.createOperator<FileSource>("src2")
    .set_fileName("data/in2.dat")
    .set_fileFormat({{"name2",Type::String}, {"grade2",Type::String}});
  
  Operator & src3 = flow.createOperator<FileSource>("src3")
    .set_fileName("data/in3.dat")
    .set_fileFormat({{"name2",Type::String}, {"grade2",Type::String}});
  
  Operator & src4 = flow.createOperator<FileSource>("src4")
    .set_fileName("data/in4.dat")
    .set_fileFormat({{"name3",Type::String}, {"grade3",Type::String}});

  Operator & barrier = flow.createOperator<Barrier>("barrier", 3);

  Operator & snk = flow.createOperator<FileSink>("snk")
    .set_fileName("data/out.dat")
    .set_fileFormat({{"name1",Type::String}, {"grade1",Type::String}, {"name2",Type::String}, {"grade2",Type::String}, {"name3",Type::String}, {"grade3",Type::String}});
};

TEST_F(FileBarrierSink_test, FlowTest) {
  vector<Operator *> operators = flow.getOperators();

  EXPECT_EQ(6, operators.size());

  EXPECT_NE(find(operators.begin(), operators.end(), &src1), operators.end());
  EXPECT_NE(find(operators.begin(), operators.end(), &src2), operators.end());
  EXPECT_NE(find(operators.begin(), operators.end(), &src3), operators.end());
  EXPECT_NE(find(operators.begin(), operators.end(), &src4), operators.end());
  EXPECT_NE(find(operators.begin(), operators.end(), &barrier), operators.end());
  EXPECT_NE(find(operators.begin(), operators.end(), &snk), operators.end());
}

TEST_F(FileBarrierSink_test, TopologyTest_OutConnections) {
  vector<connectors::ToConnection> outConnectionsSrc1 = flow.getOutConnections(src1, 0);
  EXPECT_EQ(1, outConnectionsSrc1.size());
  EXPECT_EQ(&(outConnectionsSrc1.begin()->getOperator()), &barrier);

  vector<connectors::ToConnection> outConnectionsSrc2 = flow.getOutConnections(src2, 0);
  EXPECT_EQ(1, outConnectionsSrc2.size());
  EXPECT_EQ(&(outConnectionsSrc2.begin()->getOperator()), &barrier);

  vector<connectors::ToConnection> outConnectionsSrc3 = flow.getOutConnections(src3, 0);
  EXPECT_EQ(1, outConnectionsSrc3.size());
  EXPECT_EQ(&(outConnectionsSrc3.begin()->getOperator()), &barrier);

  vector<connectors::ToConnection> outConnectionsSrc4 = flow.getOutConnections(src4, 0);
  EXPECT_EQ(1, outConnectionsSrc4.size());
  EXPECT_EQ(&(outConnectionsSrc4.begin()->getOperator()), &barrier);

  vector<connectors::ToConnection> outConnectionsBarrier = flow.getOutConnections(barrier, 0);
  EXPECT_EQ(1, outConnectionsBarrier.size());
  EXPECT_EQ(&(outConnectionsBarrier.begin()->getOperator()), &snk);  
}

TEST_F(FileBarrierSink_test, TopologyTest_InConnections) {
  vector<connectors::FromConnection> inConnectionsBarrier0 = flow.getInConnections(barrier, 0);
  EXPECT_EQ(1, inConnectionsBarrier0.size());
  EXPECT_EQ(&(inConnectionsBarrier0.begin()->getOperator()), &src1);
  // todo
  vector<connectors::FromConnection> inConnectionsBarrier2 = flow.getInConnections(barrier, 2);
  EXPECT_EQ(1, inConnectionsBarrier2.size());
  EXPECT_EQ(&(inConnectionsBarrier2.begin()->getOperator()), &src4);

  vector<connectors::FromConnection> inConnectionsSnk = flow.getInConnections(snk, 0);
  EXPECT_EQ(1, inConnectionsSnk.size());
  EXPECT_EQ(&(inConnectionsSnk.begin()->getOperator()), &barrier);  
}

TEST_F(FileBarrierSink_test, OutputTest) {
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

    EXPECT_EQ(6, lineLength);
    outputLength++;
  }
  input.close();

  EXPECT_EQ(43200, attCounts["Semih"]);
  EXPECT_EQ(43200, attCounts["Bugra"]);
  EXPECT_EQ(43200, attCounts["Ahmet"]);
  EXPECT_EQ(43200, attCounts["Mehnet"]);
  EXPECT_EQ(43200, attCounts["Aslı"]);
  EXPECT_EQ(43200, attCounts["Yasemin"]);
  EXPECT_EQ(43200, attCounts["Vehbi"]);
  EXPECT_EQ(86400, attCounts["A"]);
  EXPECT_EQ(43200, attCounts["B"]);
  EXPECT_EQ(86400, attCounts["C"]);
  EXPECT_EQ(43200, attCounts["D"]);
  EXPECT_EQ(43200, attCounts["F"]);

  EXPECT_EQ(100800, outputLength);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}