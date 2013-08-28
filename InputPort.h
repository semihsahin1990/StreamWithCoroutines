#pragma once

#include<queue>
#include "Port.h"
#include "Tuple.h"

using namespace std;


// InputPort: User visible interface
// InputPortImph implements InputPort: System implementation of a port

class InputPort : public Port
{
	private:
		queue<Tuple *> *portQ;
	
	public:
		InputPort(queue<Tuple *> *portQ);
		bool hasTuple();
		Tuple* popTuple();
};
