#include "streamc/Tuple.h"
#include "streamc/OutputPort.h"

using namespace std;
using namespace streamc;

class OutputPortImpl : public OutputPort
{
	private:
		queue<Tuple> *portQ_;

	public:
		OutputPortImpl(queue<Tuple> *portQ){
			portQ_ = portQ;
		}

		void pushTuple(Tuple &tuple){
			portQ_->push(tuple);
		}
};
