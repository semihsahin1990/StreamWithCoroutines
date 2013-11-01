#include <iostream>
#include <unordered_map>
#include <vector>

#include "streamc/Flow.h"
#include "streamc/Tuple.h"
#include "streamc/Operator.h"
#include "streamc/InputPort.h"
#include "streamc/OutputPort.h"

using namespace std;
using namespace streamc;

class Node{
private:
	Operator *op;
	uint32_t oport, iport;

public:
	Node(Operator *op, uint32_t oport, uint32_t iport){
		this->op = op;
		this->oport = oport;
		this->iport = iport;
	}
	
	Operator *getOperator(){
		return op;
	}
	
	uint32_t getOPort(){
		return oport;
	}
	
	uint32_t getIPort(){
		return iport;
	}
};

unordered_map<Operator*, vector<Node>> adjList_;

void Flow::addConnection(Operator *op1, uint32_t oport, Operator *op2, uint32_t iport){
	Node node(op2, oport, iport);
	adjList_[op1].push_back(node);
}

vector<Operator *> Flow::getOperators(){
	vector<Operator*> operators;
	
	for(auto it = adjList_.begin(); it!=adjList_.end(); it++){
		operators.push_back(it->first);
	}

	return operators;
}

vector<Operator*> Flow::getNeighbors(Operator *op){
	vector<Operator*> neighbors;
	
	for(auto it = adjList_.begin(); it!=adjList_.end(); it++){
		if(it->first == op){
			for(auto nIt = it->second.begin(); nIt!=it->second.end(); nIt++)
				neighbors.push_back(nIt->getOperator());

			return neighbors;
		}
	}

	return neighbors;
}

void Flow::printTopology(){
	for(auto it = adjList_.begin(); it!=adjList_.end(); it++){
		cout<<it->first->getId()<<"->";
		for(auto nIt = it->second.begin(); nIt!=it->second.end(); nIt++){
			cout<< " " << nIt->getOperator()->getId();
		}
		cout<<std::endl;
	}
}
