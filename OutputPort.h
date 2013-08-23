#pragma once

#include<queue>
#include "Port.h"
#include "Tuple.h"

using namespace std;

class OutputPort : public Port
{
	private:
		queue<Tuple *> *portQ;
	
	public:
		OutputPort(queue<Tuple *> *portQ);
		void pushTuple(Tuple *tuple);
};
