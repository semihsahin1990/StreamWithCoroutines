#include <iostream>
#include <unordered_map>
#include <vector>

#include "streamc/Operator.h"

namespace streamc
{
 
class Flow
{
private:

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

private:
	std::unordered_map<Operator*, std::vector<Node>> adjList;

public:
	void connect(Operator *op1, uint32_t oport, Operator *op2, uint32_t iport)
        {
		Node node(op2, oport, iport);
		adjList[op1].push_back(node);
	}

	void printTopology(){
		for(std::unordered_map<Operator*, std::vector<Node>>::iterator it = adjList.begin(); it!=adjList.end(); it++){
			std::cout<<it->first->getId()<<"->";
			for(std::vector<Node>::iterator nIt = it->second.begin(); nIt!=it->second.end(); nIt++){
				std::cout<< " " << nIt->getOperator()->getId();
			}
			std::cout<<std::endl;
		}
	}

	void run(int numberOfThreads);

};

}

