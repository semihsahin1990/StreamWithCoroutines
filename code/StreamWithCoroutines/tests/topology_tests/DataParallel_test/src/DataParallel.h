#include "streamc/Flow.h"
#include "streamc/Operator.h"
#include "vector"

class DataParallel
{
public:
	DataParallel(size_t n);
	streamc::Flow & getFlow() { return flow_; }

private:
	size_t n_;
	std::vector<streamc::Operator *> unions_;
	streamc::Flow flow_;
};