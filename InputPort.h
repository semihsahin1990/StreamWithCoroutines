#pragma once

#include<queue>
#include "Port.h"
#include "Tuple.h"

using namespace std;

class InputPort : public Port
{
	private:
		queue<Tuple *> *portQ;
	
	public:
		InputPort(queue<Tuple *> *portQ);
		bool hasTuple();
		Tuple* popTuple();
};
