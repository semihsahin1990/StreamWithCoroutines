#include <vector>
#include "Operator.h"

class Flow
{
	private: 
		vector<Operator *> *ops;

	public:
		Flow();
		void addOperator(Operator *op);
		void connect(Operator *op1, Operator *op2);
		void run(int numberOfThreads);
};
