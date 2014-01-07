#include "gtest/gtest.h"
#include <cstdlib>
#include <fstream>

using namespace std;

int outputSize = 0;

void init() {
	string outputFile = "data/out.dat";
	string line;

	ifstream input;

	input.open(outputFile.c_str(), ios::in);
	
	while(true) {
		line.clear();
		getline(input, line);

		if(line.size()==0 && input.eof())
  			break; 

  		outputSize++;
	}
	input.close();
}

TEST (FileBarrierSink_test, CheckOutputSize) {
	 EXPECT_EQ(100800, outputSize);
}

int main(int argc, char *argv[]) {
	init();
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}