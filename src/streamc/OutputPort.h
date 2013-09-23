#pragma once

#include<queue>
#include "Tuple.h"

class OutputPort
{
	private:
		std::queue<Tuple> *portQ;
	
	public:
		OutputPort(std::queue<Tuple> *portQ);
		void pushTuple(Tuple tuple);
};
