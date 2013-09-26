#include<iostream>
#include "Operator.h"
#include "InputPort.h"

namespace streamc
{

class myOp : public Operator
{
public:
    myOp(int id) : Operator(id)
    {
    }

    void process()
    {
        std::cout << "processing" << std::endl;	
    }
};

}
