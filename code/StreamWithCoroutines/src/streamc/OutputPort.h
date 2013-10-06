#pragma once

#include<queue>
#include "Tuple.h"

namespace streamc
{

class OutputPort
{
	public:
		static OutputPort* create(std::queue<Tuple> *portQ);
		virtual void pushTuple(Tuple tuple) = 0;

	protected:
		OutputPort(){}
};

}
