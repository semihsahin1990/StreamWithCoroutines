#include "streamc/Tuple.h"
#include "streamc/InputPort.h"

using namespace std;
using namespace streamc;

class InputPortImpl: public InputPort
{
	private:
		queue<Tuple> *portQ_;

	public:
		InputPortImpl(queue<Tuple> *portQ){
			portQ_ = portQ;
		}

		bool hasTuple(){
			return !portQ_->empty();
		}
	
		Tuple popTuple(){
			Tuple returned = portQ_->front();
			portQ_->pop();
			return returned;
		}
};

InputPort* InputPort::create(queue<Tuple> *portQ){
	return new InputPortImpl(portQ);
}
