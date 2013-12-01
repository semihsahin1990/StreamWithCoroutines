#pragma once

#include <functional>

/// @cond hidden

namespace streamc
{
  class Operator;
  class Flow;
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
  struct hash<streamc::Flow *> 
  {
  public:
    size_t operator()(streamc::Flow * const & flowAddr) const
    {
      return std::hash<uintptr_t>()(reinterpret_cast<uintptr_t>(flowAddr));
    }
  };
}

/// @endcond
