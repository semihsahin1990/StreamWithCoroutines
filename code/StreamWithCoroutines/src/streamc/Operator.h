#pragma once

#include<iostream>
#include<vector>

#include "OperatorContext.h"

namespace streamc
{

class Operator
{
private:
  std::string id_;
  OperatorContext * context_;

public:
  Operator(std::string id)
    : id_(id), completed_(false)
  {
  }
  
  virtual ~Operator() 
  {
  }
  
  std::string getId() 
  {
    return id_;
  }
  
  bool isCompleted() 
  {
    return context_->isCompleted();
  }
  
  Context const & getContext() 
  {
    return context_;
  } 
  
protected:
  virtual void init() = 0;
  virtual void process() = 0;
  
private:
  friend class FlowRunnerImpl;
  void setContext(OperatorContext * context) 
  {
    context_ = context;
  }
};
 
}
