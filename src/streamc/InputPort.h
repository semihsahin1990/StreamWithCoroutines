#pragma once

#include<queue>
#include "Tuple.h"

class InputPort
{
	private:
		std::queue<Tuple> *portQ;
	
	public:
		InputPort(std::queue<Tuple> *portQ);
		bool hasTuple();
		Tuple popTuple();
};
