#include "streamc/Flow.h"
#include "streamc/FlowRunner.h"
#include "streamc/operators/FileSource.h"
#include "streamc/operators/FileSink.h"
#include "streamc/operators/Filter.h"
#include "streamc/operators/Union.h"

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


class FileUnionSink_test : public ::testing::Test {
public:
  FileUnionSink_test() : flow("simple union") {
    flow.addConnection( (src1,0) >> (0,myUnion));
    flow.addConnection( (src2,0) >> (1,myUnion));
    flow.addConnection( (src3,0) >> (1,myUnion));
    flow.addConnection( (src4,0) >> (2,myUnion));
    flow.addConnection( (myUnion,0) >> (0,snk));
    
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
  .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});
  
  Operator & src2 = flow.createOperator<FileSource>("src2")
    .set_fileName("data/in2.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});
  
  Operator & src3 = flow.createOperator<FileSource>("src3")
    .set_fileName("data/in3.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});
  
  Operator & src4 = flow.createOperator<FileSource>("src4")
    .set_fileName("data/in4.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});

  Operator & myUnion = flow.createOperator<Union>("union", 3);

  Operator & snk = flow.createOperator<FileSink>("snk")
    .set_fileName("data/out.dat")
    .set_fileFormat({{"name",Type::String}, {"grade",Type::String}});

};

TEST_F(FileUnionSink_test, FlowTest) {
  vector<Operator *> operators = flow.getOperators();

  EXPECT_EQ(6, operators.size());

  EXPECT_NE(find(operators.begin(), operators.end(), &src1), operators.end());
  EXPECT_NE(find(operators.begin(), operators.end(), &src2), operators.end());
  EXPECT_NE(find(operators.begin(), operators.end(), &src3), operators.end());
  EXPECT_NE(find(operators.begin(), operators.end(), &src4), operators.end());
  EXPECT_NE(find(operators.begin(), operators.end(), &myUnion), operators.end());
  EXPECT_NE(find(operators.begin(), operators.end(), &snk), operators.end());
}

TEST_F(FileUnionSink_test, TopologyTest_OutConnections) {
  vector<connectors::ToConnection> outConnectionsSrc1 = flow.getOutConnections(src1, 0);
  EXPECT_EQ(1, outConnectionsSrc1.size());
  EXPECT_EQ(&(outConnectionsSrc1.begin()->getOperator()), &myUnion);

  vector<connectors::ToConnection> outConnectionsSrc2 = flow.getOutConnections(src2, 0);
  EXPECT_EQ(1, outConnectionsSrc2.size());
  EXPECT_EQ(&(outConnectionsSrc2.begin()->getOperator()), &myUnion);

  vector<connectors::ToConnection> outConnectionsSrc3 = flow.getOutConnections(src3, 0);
  EXPECT_EQ(1, outConnectionsSrc3.size());
  EXPECT_EQ(&(outConnectionsSrc3.begin()->getOperator()), &myUnion);

  vector<connectors::ToConnection> outConnectionsSrc4 = flow.getOutConnections(src4, 0);
  EXPECT_EQ(1, outConnectionsSrc4.size());
  EXPECT_EQ(&(outConnectionsSrc4.begin()->getOperator()), &myUnion);

  vector<connectors::ToConnection> outConnectionsMyUnion = flow.getOutConnections(myUnion, 0);
  EXPECT_EQ(1, outConnectionsMyUnion.size());
  EXPECT_EQ(&(outConnectionsMyUnion.begin()->getOperator()), &snk);  
}

TEST_F(FileUnionSink_test, TopologyTest_InConnections) {
  vector<connectors::FromConnection> inConnectionsMyUnion0 = flow.getInConnections(myUnion, 0);
  EXPECT_EQ(1, inConnectionsMyUnion0.size());
  EXPECT_EQ(&(inConnectionsMyUnion0.begin()->getOperator()), &src1);
  // todo
  vector<connectors::FromConnection> inConnectionsMyUnion2 = flow.getInConnections(myUnion, 2);
  EXPECT_EQ(1, inConnectionsMyUnion2.size());
  EXPECT_EQ(&(inConnectionsMyUnion2.begin()->getOperator()), &src4);

  vector<connectors::FromConnection> inConnectionsSnk = flow.getInConnections(snk, 0);
  EXPECT_EQ(1, inConnectionsSnk.size());
  EXPECT_EQ(&(inConnectionsSnk.begin()->getOperator()), &myUnion);  
}

TEST_F(FileUnionSink_test, OutputTest) {
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

  EXPECT_EQ(57600, attCounts["Semih"]);
  EXPECT_EQ(57600, attCounts["Bugra"]);
  EXPECT_EQ(57600, attCounts["Ahmet"]);
  EXPECT_EQ(57600, attCounts["Mehnet"]);
  EXPECT_EQ(57600, attCounts["Aslı"]);
  EXPECT_EQ(57600, attCounts["Yasemin"]);
  EXPECT_EQ(57600, attCounts["Vehbi"]);
  EXPECT_EQ(115200, attCounts["A"]);
  EXPECT_EQ(57600, attCounts["B"]);
  EXPECT_EQ(115200, attCounts["C"]);
  EXPECT_EQ(57600, attCounts["D"]);
  EXPECT_EQ(57600, attCounts["F"]);

  EXPECT_EQ(403200, outputLength);
}

int main(int argc, char *argv[])
{ 
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


