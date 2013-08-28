#include <vector>
#include "Operator.h"

class Flow
{
	private: 
		vector<Operator *> *ops;
        // topology object
	public:
		Flow();
		void addOperator(Operator *op);
		void connect(Operator *op1, uint32_t oport, Operator *op2, uint32_t iport);
		void run(int numberOfThreads);
};
