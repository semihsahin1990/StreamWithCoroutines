#include "streamc/Flow.h"
#include "streamc/Operator.h"
#include "vector"

class TreeCreator
{
public:
	TreeCreator(size_t depth, size_t n);
	size_t getDepth() { return depth_; }
	streamc::Flow & getFlow() { return flow_; }

private:
	size_t depth_;
	size_t n_;
	std::vector<streamc::Operator *> sinks_;
	std::vector<streamc::Operator *> splits_;
	streamc::Flow flow_;
};