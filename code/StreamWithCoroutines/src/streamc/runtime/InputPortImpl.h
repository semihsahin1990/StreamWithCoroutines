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
	
		Tuple frontTuple(){
			return portQ_->front();
		}

		void popTuple(){
			portQ_->pop();
		}

		void pop(){
			portQ_->pop();
		}
};
