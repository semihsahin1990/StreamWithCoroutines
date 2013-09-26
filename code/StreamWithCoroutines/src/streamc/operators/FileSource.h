#include<iostream>
#include<vector>

#include "streamc/Operator.h"

namespace streamc 
{

class FileSource : public Operator
{
public:
    virtual void process();
};

}


