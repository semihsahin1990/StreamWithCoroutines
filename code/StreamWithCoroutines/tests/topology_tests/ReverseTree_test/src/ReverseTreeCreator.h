#include "streamc/Flow.h"
#include "streamc/Operator.h"
#include "vector"

class ReverseTreeCreator
{
public:
	ReverseTreeCreator(size_t depth);
	size_t getDepth() { return depth_; }
	streamc::Flow & getFlow() { return flow_; }

private:
	size_t depth_;
	std::vector<streamc::Operator *> sources_;
	std::vector<streamc::Operator *> unions_;
	streamc::Flow flow_;
};