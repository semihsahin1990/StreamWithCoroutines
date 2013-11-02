#pragma once

#include <string>
#include <iostream>
#include <inttypes.h>
#include <vector>

namespace streamc
{

enum class Type 
{
  Integer, 
  Double, 
  String, 
  IntList, 
  DoubleList, 
  StringList 
};

class Value
{    
public:
  Value(int64_t const & value)
    : type_(Type::Integer)
  {
    pointer.intPointer = new int64_t(value);
  }    
  Value(double const & value)
    : type_(Type::Double)
  {
    pointer.doublePointer = new double(value);
  }    
  Value(std::string const & value) 
    : type_(Type::String)
  {
    pointer.stringPointer = new std::string(value);
  }    
  Value(std::vector<int64_t> const & list) 
    : type_(Type::IntList)
  {
    pointer.intListPointer = new std::vector<int64_t>(list);
  }    
  Value(std::vector<double> const & list) 
    : type_(Type::DoubleList)
  {
    pointer.doubleListPointer = new std::vector<double>(list);
  }    
  Value(std::vector<std::string> const & list) 
    : type_(Type::StringList)
  {      
    pointer.stringListPointer = new std::vector<std::string>(list);
  }    
  Value(Value const & other) 
    : type_(other.type_)
  {
    allocAndSet(other);
  }
  ~Value()
  {
    free();
  }
  Type getType() const
  {
    return type_;
  }
  Value & operator=(Value const & other)
  {
    if (type_!=other.type_) {
      free();
      type_ = other.type_;
      allocAndSet(other);
    } else {
      set(other);
    }
    return *this;
  }
  void setValue(int64_t const & value)
  {
    if(type_ == Type::Integer) {
      *pointer.intPointer = value;
    } else {
      free();
      type_ = Type::Integer;
      pointer.intPointer = new int64_t(value);
    }
  }    
  void setValue(double const & value)
  {
    if(type_ == Type::Double) {
      *pointer.doublePointer = value;
    } else {
      free();
      type_ = Type::Double;
      pointer.doublePointer = new double(value);
    }
  }    
  void setValue(std::string const & value) 
  {
    if(type_ == Type::String) {
      *pointer.stringPointer = value;
    } else {
      free();
      type_ = Type::String;
      pointer.stringPointer = new std::string(value);
    }
  }    
  void setValue(std::vector<int64_t> const & value) 
  {
    if(type_ == Type::IntList) {
      *pointer.intListPointer = value;
    } else {
      free();
      type_ = Type::IntList;
      pointer.intListPointer = new std::vector<int64_t>(value);
    }
  }    
  void setValue(std::vector<double> const & value) 
  {
    if(type_ == Type::DoubleList) {
      *pointer.doubleListPointer = value;
    } else {
      free();
      type_ = Type::DoubleList;
      pointer.doubleListPointer = new std::vector<double>(value);
    }
  }    
  void setValue(std::vector<std::string> const & value) 
  {      
    if(type_ == Type::StringList) {
      *pointer.stringListPointer = value;
    } else {
      free();
      type_ = Type::StringList;
      pointer.stringListPointer = new std::vector<std::string>(value);
    }
  }        
  int64_t & getIntValue() 
  {
    return *pointer.intPointer;
  }    
  double & getDoubleValue() 
  {
    return *pointer.doublePointer;
  }    
  std::string & getStringValue() 
  {
    return *pointer.stringPointer;
  }    
  std::vector<int64_t> & getIntList() 
  {
      return *pointer.intListPointer;
  }    
  std::vector<double> & getDoubleList() 
  {
    return *pointer.doubleListPointer;
  }    
  std::vector<std::string> & getStringList() 
  {
    return *pointer.stringListPointer;
  } 
  static Value fromString(std::string const & str, Type type);
  static std::string toString(Value const & value);
private:
  void set(Value const & other)
  {
    switch(type_) {
    case Type::Integer:
      *pointer.intPointer = *other.pointer.intPointer;
      break;
    case Type::Double:
      *pointer.doublePointer = *other.pointer.doublePointer;
      break;        
    case Type::String:
      *pointer.stringPointer = *other.pointer.stringPointer;
      break;	
    case Type::IntList:
      *pointer.intListPointer = *other.pointer.intListPointer;
      break;        
    case Type::DoubleList:
      *pointer.doubleListPointer = *other.pointer.doubleListPointer;
      break;        
    case Type::StringList:
      *pointer.stringListPointer = *other.pointer.stringListPointer;
      break;
    }      
  }   
  void allocAndSet(Value const & other)
  {
    switch(type_){
    case Type::Integer:
      pointer.intPointer = new int64_t(*other.pointer.intPointer);
      break;        
    case Type::Double:
      pointer.doublePointer = new double(*other.pointer.doublePointer);
      break;        
    case Type::String:
      pointer.stringPointer = new std::string(*other.pointer.stringPointer);
      break;	
    case Type::IntList:
      pointer.intListPointer = new std::vector<int64_t>(*other.pointer.intListPointer);
      break;        
    case Type::DoubleList:
      pointer.doubleListPointer = new std::vector<double>(*other.pointer.doubleListPointer);
      break;        
    case Type::StringList:
      pointer.stringListPointer = new std::vector<std::string>(*other.pointer.stringListPointer);
      break;
    }      
  }
  void free() 
  {
    switch(type_){
    case Type::Integer:
      delete pointer.intPointer;
      break;        
    case Type::Double:
      delete pointer.doublePointer;
      break;        
    case Type::String:
      delete pointer.stringPointer;
      break;	
    case Type::IntList:
      delete pointer.intListPointer;
      break;        
    case Type::DoubleList:
      delete pointer.doubleListPointer;
      break;        
    case Type::StringList:
      delete pointer.stringListPointer;
      break;
    }
  }
private:
  Type type_;    
  union Ptr {
    int64_t * intPointer;
    std::string * stringPointer;
    double * doublePointer;
    std::vector<int64_t> * intListPointer;
    std::vector<double> * doubleListPointer;
    std::vector<std::string> * stringListPointer;
  } pointer;
};

} // namespace streamc
