#include "streamc/Flow.h"
#include "streamc/Operator.h"
#include "vector"

class ReverseTree
{
public:
	ReverseTree(size_t depth, uint64_t cost, double selectivity, size_t n);
	size_t getDepth() { return depth_; }
	streamc::Flow & getFlow() { return flow_; }

private:
	size_t depth_;
	uint64_t cost_;
	double selectivity_;
	size_t n_;
	std::vector<streamc::Operator *> sourceOps_;
	std::vector<streamc::Operator *> timestamperOps_;
	std::vector<streamc::Operator *> selectiveOps_;
	std::vector<streamc::Operator *> busyOps_;
	std::vector<streamc::Operator *> barrierOps_;
	streamc::Flow flow_;
};