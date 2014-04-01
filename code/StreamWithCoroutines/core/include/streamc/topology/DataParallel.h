#include "streamc/Flow.h"
#include "streamc/Operator.h"
#include "vector"

class DataParallel
{
public:
	DataParallel(uint64_t cost, double selectivity, size_t n);
	streamc::Flow & getFlow() { return flow_; }

private:
	uint64_t cost_;
	double selectivity_;
	size_t n_;
	std::vector<streamc::Operator *> selectiveOps_;
	std::vector<streamc::Operator *> busyOps_;
	streamc::Flow flow_;
};