#pragma once

#include <functional>

namespace streamc 
{ 
  class Operator; 
  class OperatorContextImpl;
  class WorkerThread;
}
namespace std 
{
  template<>
  struct hash<streamc::Operator *> 
  {
  public:
    size_t operator()(streamc::Operator * const & opAddr) const
    {
      return std::hash<uintptr_t>()(reinterpret_cast<uintptr_t>(opAddr));
    }
  };
}

namespace std 
{
  template<>
  struct hash<streamc::OperatorContextImpl *> 
  {
  public:
    size_t operator()(streamc::OperatorContextImpl * const & opcAddr) const
    {
      return std::hash<uintptr_t>()(reinterpret_cast<uintptr_t>(opcAddr));
    }
  };
}

namespace std 
{
  template<>
  struct hash<streamc::WorkerThread *> 
  {
  public:
    size_t operator()(streamc::WorkerThread * const & threadAddr) const
    {
      return std::hash<uintptr_t>()(reinterpret_cast<uintptr_t>(threadAddr));
    }
  };
}
