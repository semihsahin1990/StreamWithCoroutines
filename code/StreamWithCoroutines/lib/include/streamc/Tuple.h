#pragma once

#include "streamc/Value.h"

#include<unordered_map>

namespace streamc
{

class Tuple
{
private:
  std::unordered_map<std::string, Value *> values_;
  
public:
  Tuple() {}
  Tuple(Tuple const & other) 
  {
    for(auto it=other.values_.begin(); it!=other.values_.end(); ++it) 
      values_[it->first] = new Value(*(it->second));
  }
  ~Tuple() {
    for (auto it=values_.begin(); it!=values_.end(); ++it)
      delete it->second;
  };
  std::unordered_map<std::string, Value *> const & getAttributes() const
  {
    return values_;
  }
  // TODO: check correctness of iterator stuff
  Tuple & operator=(Tuple const & other)
  {
    for (auto it=values_.begin(); it!=values_.end();) {
      auto curr = it++;
      if (other.values_.count(curr->first)==0) {
        delete curr->second;
        values_.erase(curr);
      } 
    }
    for (auto it=other.values_.begin(); it!=other.values_.end(); ++it) {
      auto vit = values_.find(it->first);
      if (vit==values_.end()) 
        values_[it->first] = new Value(*(it->second));
      else
        *(vit->second) = *(it->second);
    }
    return *this;
  }
  void setAttribute(std::string const & name, Value const & value) 
  {
    auto it = values_.find(name);
    if (it==values_.end()) 
      values_[name] = new Value(value);
    else 
      *(it->second) = value;
  }
  void setAttribute(std::string const & name, int64_t const & value) 
  {
    setAttributeImpl(name, value);
  }
  void setAttribute(std::string const & name, double const & value) 
  {
    setAttributeImpl(name, value);
  }  
  void setAttribute(std::string const & name, std::string const & value) 
  {
    setAttributeImpl(name, value);
  }
  void setAttribute(std::string const & name, std::vector<int64_t> const & value) 
  {
    setAttributeImpl(name, value);
  }
  void setAttribute(std::string const & name, std::vector<double> const & value) 
  {
    setAttributeImpl(name, value);
  }
  void setAttribute(std::string const & name, std::vector<std::string> const & value) 
  {
    setAttributeImpl(name, value);
  }
  void removeAttribute(std::string const & name)
  {
    auto it = values_.find(name);
    if (it!=values_.end()) {
      delete it->second;
      values_.erase(it);
    }
  }
  template <Type T>
  typename TypeKindToType<T>::type & get(std::string const & name);
  template <Type T>
  typename TypeKindToType<T>::type const & get(std::string const & name) const;
  int64_t & getIntAttribute(std::string const & name) 
  {
    return values_[name]->getIntValue();
  }
  double & getDoubleAttribute(std::string const & name) 
  {
    return values_[name]->getDoubleValue();
  }
  std::string & getStringAttribute(std::string const & name) 
  {
    return values_[name]->getStringValue();
  }
  std::vector<int64_t> & getIntListAttribute(std::string const & name) 
  {
    return values_[name]->getIntList();
  }
  std::vector<double> & getDoubleListAttribute(std::string const & name) 
  {
    return values_[name]->getDoubleList();
  }
  std::vector<std::string> & getStringListAttribute(std::string const & name) 
  {
    return values_[name]->getStringList();
  }
private:
  template <typename T>
  void setAttributeImpl(std::string const & name, T const & value)
  {
    auto it = values_.find(name);
    if (it==values_.end()) 
      values_[name] = new Value(value);
    else 
      it->second->setValue(value);
  }
};

template <Type T>
inline typename TypeKindToType<T>::type & get(Tuple & tuple, std::string const & name) 
{
  static_assert(true, "incorrect type passed to streamc::Tuple::get<>(std::string const &)");
  return *static_cast<typename TypeKindToType<T>::type *>(nullptr);
}
template <>
inline int64_t & get<Type::Integer>(Tuple & tuple, std::string const & name) 
{
  return tuple.getIntAttribute(name);
}
template <>
inline double & get<Type::Double>(Tuple & tuple, std::string const & name) 
{
  return tuple.getDoubleAttribute(name);
}
template <>
inline std::string & get<Type::String>(Tuple & tuple, std::string const & name) 
{
  return tuple.getStringAttribute(name);
}
template <>
inline std::vector<int64_t> & get<Type::IntList>(Tuple & tuple, std::string const & name) 
{
  return tuple.getIntListAttribute(name);
}
template <>
inline std::vector<double> & get<Type::DoubleList>(Tuple & tuple, std::string const & name) 
{
  return tuple.getDoubleListAttribute(name);
}
template <>
inline std::vector<std::string> & get<Type::StringList>(Tuple & tuple, std::string const & name) 
{
  return tuple.getStringListAttribute(name);
}
  
template <Type T>
typename TypeKindToType<T>::type & Tuple::get(std::string const & name)
{
  return streamc::get<T>(*this, name);
}

template <Type T>
typename TypeKindToType<T>::type const & Tuple::get(std::string const & name) const
{
  return streamc::get<T>(const_cast<Tuple &>(*this), name);
}



}
