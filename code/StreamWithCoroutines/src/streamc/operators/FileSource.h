#include<iostream>
#include<vector>

#include "streamc/Operator.h"

namespace streamc 
{

class FileSource : public Operator
{
private:
	OutputPort* oport_;
	std::string fileName_;

public:
	FileSource(std::string id, std::string fileName);

	virtual void process();
	virtual void init();
};

}


