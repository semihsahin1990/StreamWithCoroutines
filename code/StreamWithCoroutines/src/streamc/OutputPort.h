#pragma once

#include<queue>
#include "Tuple.h"

namespace streamc
{

class OutputPort
{
	public:
		virtual void pushTuple(Tuple &tuple) = 0;

	protected:
		OutputPort(){}
};

}
