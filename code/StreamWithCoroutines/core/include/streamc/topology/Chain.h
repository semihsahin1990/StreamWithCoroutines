#include "streamc/Flow.h"
#include "streamc/Operator.h"
#include "vector"

class Chain
{
public:
	Chain(size_t depth, uint64_t cost, double selectivity);
	streamc::Flow & getFlow() { return flow_; }

private:
	size_t depth_;
	uint64_t cost_;
	double selectivity_;
	std::vector<streamc::Operator *> selectiveOps_;
	std::vector<streamc::Operator *> busyOps_;
	streamc::Flow flow_;
};