#pragma once

#include "streamc/Value.h"

#include <unordered_map>

namespace streamc
{

/** %Tuple represending a set of named values.
 *
 */
class Tuple
{
public:
  /**
   * Construct a tuple (aka default constructor).
   *
   */
  Tuple() {}

  /** 
   * Construct from another tuple (aka copy constructor).
   *
   * @param other other tuple to construct from
   */
  Tuple(Tuple const & other) 
  {
    for(auto it=other.values_.begin(); it!=other.values_.end(); ++it) 
      values_[it->first] = new Value(*(it->second));
  }

  ~Tuple() {
    for (auto it=values_.begin(); it!=values_.end(); ++it)
      delete it->second;
  }

  /**
   * Get the tuple attributes.
   *
   * @return the tuple attributes
   */
  std::unordered_map<std::string, Value *> const & getAttributes() const
  {
    return values_;
  }

  /**
   * Assign from another tuple (aka assignment operator).
   *
   * @param other other tuple to assign from
   * @return this tuple
   */
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

  /**
   * Append a tuple to this tuple 
   *
   * New attributes are added as copies, existing ones are overwritten.
   * @param tuple tuple to be appended to this tuple
   */
  void append(Tuple const & tuple)
  {
    auto tupleValues = tuple.getAttributes();
    for(auto it = tupleValues.begin(); it!= tupleValues.end(); it++)
      setAttribute(it->first, *(it->second));
  }

  /**
   * Set value of an attribute.
   * 
   * @param name name of the attribute
   * @param value value to set to
   */
  void setAttribute(std::string const & name, Value const & value) 
  {
    auto it = values_.find(name);
    if (it==values_.end()) 
      values_[name] = new Value(value);
    else 
      *(it->second) = value;
  }

  /**
   * Set value of an attribute to an integer.
   * 
   * @param name name of the attribute
   * @param value integer value to set to
   */
  void setAttribute(std::string const & name, int64_t const & value) 
  {
    setAttributeImpl(name, value);
  }

  /**
   * Set value of an attribute to a double.
   * 
   * @param name name of the attribute
   * @param value double value to set to
   */
  void setAttribute(std::string const & name, double const & value) 
  {
    setAttributeImpl(name, value);
  }  

  /**
   * Set value of an attribute to a string.
   * 
   * @param name name of the attribute
   * @param value string value to set to
   */
  void setAttribute(std::string const & name, std::string const & value) 
  {
    setAttributeImpl(name, value);
  }

  /**
   * Set value of an attribute to an integer list.
   * 
   * @param name name of the attribute
   * @param value integer list value to set to
   */
  void setAttribute(std::string const & name, std::vector<int64_t> const & value) 
  {
    setAttributeImpl(name, value);
  }

  /**
   * Set value of an attribute to a double list.
   * 
   * @param name name of the attribute
   * @param value double list value to set to
   */
  void setAttribute(std::string const & name, std::vector<double> const & value) 
  {
    setAttributeImpl(name, value);
  }

  /**
   * Check whether an attribute with the given name exist.
   * 
   * @param name the attribute
   * @return true if the attribute exists, false otherwise 
   */ 
  bool hasAttribute(std::string const & name)
  {
    return (values_.count(name) > 0);
  }

  /**
   * Set value of an attribute to a string list.
   * 
   * @param name name of the attribute
   * @param value string list value to set to
   */
  void setAttribute(std::string const & name, std::vector<std::string> const & value) 
  {
    setAttributeImpl(name, value);
  }

  /**
   * Remove a given attrobute
   * 
   * @param name name of the attribute
   */
  void removeAttribute(std::string const & name)
  {
    auto it = values_.find(name);
    if (it!=values_.end()) {
      delete it->second;
      values_.erase(it);
    }
  }

  /**
   * Get the value of the attribute with the given type and name.
   * 
   * @param name name of the attribute
   * @return the value of the attribute 
   */
  template <Type T>
  typename ValueTypeToCppType<T>::type & get(std::string const & name);

  /**
   * Get the value of the attribute with a given type and name (const version).
   * 
   * @param name name of the attribute
   * @return the value of the attribute 
   */
  template <Type T>
  typename ValueTypeToCppType<T>::type const & get(std::string const & name) const;

  /**
   * Get the integer value of the attribute with the given name.
   * 
   * @param name name of the attribute
   * @return the integer value of the attribute 
   */
  int64_t & getIntAttribute(std::string const & name) 
  {
    return values_[name]->getIntValue();
  }

  /**
   * Get the integer value of the attribute with the given name (const version).
   * 
   * @param name name of the attribute
   * @return the integer value of the attribute 
   */
  int64_t const & getIntAttribute(std::string const & name) const
  {
    return values_.find(name)->second->getIntValue();
  }

  /**
   * Get the double value of the attribute with the given name.
   * 
   * @param name name of the attribute
   * @return the double value of the attribute 
   */
  double & getDoubleAttribute(std::string const & name) 
  {
    return values_[name]->getDoubleValue();
  }

  /**
   * Get the double value of the attribute with the given name (const version).
   * 
   * @param name name of the attribute
   * @return the double value of the attribute 
   */
  double const & getDoubleAttribute(std::string const & name) const
  {
    return values_.find(name)->second->getDoubleValue();
  }

  /**
   * Get the string value of the attribute with the given name.
   * 
   * @param name name of the attribute
   * @return the string value of the attribute 
   */
  std::string & getStringAttribute(std::string const & name) 
  {
    return values_[name]->getStringValue();
  }

  /**
   * Get the string value of the attribute with the given name (const version).
   * 
   * @param name name of the attribute
   * @return the string value of the attribute 
   */
  std::string const & getStringAttribute(std::string const & name) const
  {
    return values_.find(name)->second->getStringValue();
  }

  /**
   * Get the integer list value of the attribute with the given name.
   * 
   * @param name name of the attribute
   * @return the integer list value of the attribute 
   */
  std::vector<int64_t> & getIntListAttribute(std::string const & name) 
  {
    return values_[name]->getIntList();
  }

  /**
   * Get the integer list value of the attribute with the given name (const version).
   * 
   * @param name name of the attribute
   * @return the integer list value of the attribute 
   */
  std::vector<int64_t> const & getIntListAttribute(std::string const & name) const
  {
    return values_.find(name)->second->getIntList();
  }

  /**
   * Get the double list value of the attribute with the given name.
   * 
   * @param name name of the attribute
   * @return the double list value of the attribute 
   */
  std::vector<double> & getDoubleListAttribute(std::string const & name) 
  {
    return values_[name]->getDoubleList();
  }

  /**
   * Get the double list value of the attribute with the given name (const version).
   * 
   * @param name name of the attribute
   * @return the double list value of the attribute 
   */
  std::vector<double> const & getDoubleListAttribute(std::string const & name) const
  {
    return values_.find(name)->second->getDoubleList();
  }

  /**
   * Get the string list value of the attribute with the given name.
   * 
   * @param name name of the attribute
   * @return the string list value of the attribute 
   */
  std::vector<std::string> & getStringListAttribute(std::string const & name) 
  {
    return values_[name]->getStringList();
  }

  /**
   * Get the string list value of the attribute with the given name (const version).
   * 
   * @param name name of the attribute
   * @return the string list value of the attribute 
   */
  std::vector<std::string> const & getStringListAttribute(std::string const & name) const
  {
    return values_.find(name)->second->getStringList();
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
private:
  std::unordered_map<std::string, Value *> values_;
};

/// @cond hidden
template <Type T>
inline typename ValueTypeToCppType<T>::type & get(Tuple & tuple, std::string const & name) 
{
  static_assert(true, "incorrect type passed to streamc::Tuple::get<>(std::string const &)");
  return *static_cast<typename ValueTypeToCppType<T>::type *>(nullptr);
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
typename ValueTypeToCppType<T>::type & Tuple::get(std::string const & name)
{
  return streamc::get<T>(*this, name);
}

template <Type T>
typename ValueTypeToCppType<T>::type const & Tuple::get(std::string const & name) const
{
  return streamc::get<T>(const_cast<Tuple &>(*this), name);
}
/// @endcond

} // namespace streamc
