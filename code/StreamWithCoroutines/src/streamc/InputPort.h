#pragma once

#include<queue>
#include "Tuple.h"

namespace streamc
{

class InputPort
{
	public:	
		virtual bool hasTuple() = 0;
		virtual Tuple popTuple() = 0;

	protected:
		InputPort() {}
};

}
