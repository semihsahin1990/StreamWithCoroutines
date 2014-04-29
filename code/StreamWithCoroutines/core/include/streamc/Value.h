#pragma once

#include <string>
#include <iostream>
#include <inttypes.h>
#include <vector>

namespace streamc
{

/** 
 * Types supported by the Value class.
 */
enum class Type 
{
  Integer,     //! integer value type
  Double,      //! double value type
  String,      //! string value type
  IntList,     //! integer list value type
  DoubleList,  //! double list value type
  StringList,  //! string list value type
  Timestamp    //! timestamp value type
};

/**
 * Convert a value type to the corresponding C++ type.  For instance,
 * <code>ValueTypeToCppType<Type::Integer> x;</code> is equivalent to
 * <code>int64_t x;</code>.
 */
template <Type T>
struct ValueTypeToCppType {};

/// @cond hidden
template<>
struct ValueTypeToCppType<Type::Integer> 
{
  typedef int64_t type;
};
template<>
struct ValueTypeToCppType<Type::Double> 
{
  typedef double type;
};
template<>
struct ValueTypeToCppType<Type::String> 
{
  typedef std::string type;
};

template<>
struct ValueTypeToCppType<Type::IntList> 
{
  typedef std::vector<int64_t> type;
};
template<>
struct ValueTypeToCppType<Type::DoubleList> 
{
  typedef std::vector<double> type;
};
template<>
struct ValueTypeToCppType<Type::StringList> 
{
  typedef std::vector<std::string> type;
};
/// @endcond

/**
 * %Value that reprsents tuple attribute values.
 */
class Value
{    
public:
  /**
   * Construct a value from an integer.
   *
   * @param value integer value
   */
  Value(int64_t const & value)
    : type_(Type::Integer)
  {
    data.intData = value;
  }    

  /**
   * Construct a value from a double.
   *
   * @param value double value
   */
  Value(double const & value)
    : type_(Type::Double)
  {
    data.doubleData = value;
  }

  /**
   * Construct a value from a double.
   *
   * @param value double value
   */
  Value(std::chrono::high_resolution_clock::time_point const & value)
    : type_(Type::Timestamp)
  {
    data.timestampData = value;
  }    

  /**
   * Construct a value from a string.
   *
   * @param value string value
   */
  Value(std::string const & value) 
    : type_(Type::String)
  {
    data.stringPointer = new std::string(value);
  }    

  /**
   * Construct a value from an integer list.
   *
   * @param value integer list value
   */
  Value(std::vector<int64_t> const & value) 
    : type_(Type::IntList)
  {
    data.intListPointer = new std::vector<int64_t>(value);
  }    

  /**
   * Construct a value from a double list.
   *
   * @param value double list value
   */
  Value(std::vector<double> const & value) 
    : type_(Type::DoubleList)
  {
    data.doubleListPointer = new std::vector<double>(value);
  }    

  /**
   * Construct a value from a string list.
   *
   * @param value string list value
   */  
  Value(std::vector<std::string> const & value) 
    : type_(Type::StringList)
  {      
    data.stringListPointer = new std::vector<std::string>(value);
  }    

  /**
   * Construct a value from another one (aka, copy constructor).
   *
   * @param other other value to construct from.
   */
  Value(Value const & other) 
    : type_(other.type_)
  {
    allocAndSet(other);
  }

  ~Value()
  {
    free();
  }

  /**
   * Get the value type.
   *
   * @return the value type
   */
  Type getType() const
  {
    return type_;
  }

  /**
   * Assign from another value (aka asignment operator).
   *
   * @param other other value to assign from
   * @return this value
   */
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

  /**
   * Set value to the given integer. 
   *
   * @param value given integer
   */
  void setValue(int64_t const & value)
  {
    if(type_ == Type::Integer) {
      data.intData = value;
    } else {
      free();
      type_ = Type::Integer;
      data.intData = value;
    }
  } 

  /**
   * Set value to the given double.
   *
   * @param value given double
   */   
  void setValue(double const & value)
  {
    if(type_ == Type::Double) {
      data.doubleData = value;
    } else {
      free();
      type_ = Type::Double;
      data.doubleData = value;
    }
  }    

  /**
   * Set value to the given string.
   *
   * @param value given string
   */   
  void setValue(std::string const & value) 
  {
    if(type_ == Type::String) {
      *(data.stringPointer) = value;
    } else {
      free();
      type_ = Type::String;
      data.stringPointer = new std::string(value);
    }
  }    

  /**
   * Set value to the given integer list.
   *
   * @param value given integer list
   */   
  void setValue(std::vector<int64_t> const & value) 
  {
    if(type_ == Type::IntList) {
      *(data.intListPointer) = value;
    } else {
      free();
      type_ = Type::IntList;
      data.intListPointer = new std::vector<int64_t>(value);
    }
  }    

  /**
   * Set value to the given double list.
   *
   * @param value given double list
   */   
  void setValue(std::vector<double> const & value) 
  {
    if(type_ == Type::DoubleList) {
      *(data.doubleListPointer) = value;
    } else {
      free();
      type_ = Type::DoubleList;
      data.doubleListPointer = new std::vector<double>(value);
    }
  }    

  /**
   * Set value to the given string list.
   *
   * @param value given string list
   */   
  void setValue(std::vector<std::string> const & value) 
  {      
    if(type_ == Type::StringList) {
      *(data.stringListPointer) = value;
    } else {
      free();
      type_ = Type::StringList;
      data.stringListPointer = new std::vector<std::string>(value);
    }
  }        

  /**
   * Get the integer value.
   *
   * @return the integer value
   */   
  int64_t & getIntValue() 
  {
    return data.intData;
  }    

  /**
   * Get the integer value (const version).
   *
   * @return the integer value
   */   
  int64_t const & getIntValue() const
  {
    return data.intData;
  }    

  /**
   * Get the double value.
   *
   * @return the double value
   */   
  double & getDoubleValue() 
  {
    return data.doubleData;
  }    

  /**
   * Get the double value (const version).
   *
   * @return the double value
   */   
  double const & getDoubleValue() const
  {
    return data.doubleData;
  }

  /**
   * Get the timestamp value.
   *
   * @return the timestamp value
   */   
  std::chrono::high_resolution_clock::time_point & getTimestampValue()
  {
    return data.timestampData;
  }

  /**
   * Get the timestamp value (const version).
   *
   * @return the timestamp value
   */   
  std::chrono::high_resolution_clock::time_point const & getTimestampValue() const
  {
    return data.timestampData;
  }


  /**
   * Get the string value.
   *
   * @return the string value
   */   
  std::string & getStringValue() 
  {
    return *(data.stringPointer);
  }    

  /**
   * Get the string value (const version).
   *
   * @return the string value
   */   
  std::string const & getStringValue() const
  {
    return *(data.stringPointer);
  }    

  /**
   * Get the integer list value.
   *
   * @return the integer list value
   */   
  std::vector<int64_t> & getIntList() 
  {
    return *(data.intListPointer);
  }    

  /**
   * Get the integer list value (const version).
   *
   * @return the integer list value
   */   
  std::vector<int64_t> const & getIntList() const
  {
    return *(data.intListPointer);
  }    

  /**
   * Get the double list value.
   *
   * @return the double list value
   */   
  std::vector<double> & getDoubleList() 
  {
    return *(data.doubleListPointer);
  }    

  /**
   * Get the double list value (const version).
   *
   * @return the double list value
   */   
  std::vector<double> const & getDoubleList() const
  {
    return *(data.doubleListPointer);
  }    

  /**
   * Get the string list value.
   *
   * @return the string list value
   */   
  std::vector<std::string> & getStringList() 
  {
    return *(data.stringListPointer);
  } 

  /**
   * Get the string list value (const version).
   *
   * @return the string list value
   */   
  std::vector<std::string> const & getStringList() const
  {
    return *(data.stringListPointer);
  } 

  /**
   * Construct a value from a string representation.
   *
   * @param str the string representation 
   * @param type the value type
   * @return the constructed value
   */ 
  static Value fromString(std::string const & str, Type type);

  /**
   * Construct a value to the string representation.
   *
   * @param value the value
   */   
  static std::string toString(Value const & value);
private:
  void set(Value const & other)
  {
    switch(type_) {
    case Type::Integer:
      data.intData = other.data.intData;
      break;
    case Type::Double:
      data.doubleData = other.data.doubleData;
      break;
    case Type::Timestamp:
      data.timestampData = other.data.timestampData;
      break;  
    case Type::String:
      *(data.stringPointer) = *(other.data.stringPointer);
      break;	
    case Type::IntList:
      *(data.intListPointer) = *(other.data.intListPointer);
      break;        
    case Type::DoubleList:
      *(data.doubleListPointer) = *(other.data.doubleListPointer);
      break;        
    case Type::StringList:
      *(data.stringListPointer) = *(other.data.stringListPointer);
      break;
    }      
  }   
  void allocAndSet(Value const & other)
  {
    switch(type_){
    case Type::Integer:
      data.intData = other.data.intData;
      break;        
    case Type::Double:
      data.doubleData = other.data.doubleData;
      break;
    case Type::Timestamp:
      data.timestampData = other.data.timestampData;
      break;
    case Type::String:
      data.stringPointer = new std::string(*(other.data.stringPointer));
      break;	
    case Type::IntList:
      data.intListPointer = new std::vector<int64_t>(*(other.data.intListPointer));
      break;        
    case Type::DoubleList:
      data.doubleListPointer = new std::vector<double>(*(other.data.doubleListPointer));
      break;        
    case Type::StringList:
      data.stringListPointer = new std::vector<std::string>(*(other.data.stringListPointer));
      break;
    }      
  }
  void free() 
  {
    switch(type_){
    case Type::Integer:
    case Type::Double:
    case Type::Timestamp:
      break;        
    case Type::String:
      delete data.stringPointer;
      break;	
    case Type::IntList:
      delete data.intListPointer;
      break;        
    case Type::DoubleList:
      delete data.doubleListPointer;
      break;        
    case Type::StringList:
      delete data.stringListPointer;
      break;
    }
  }
private:
  Type type_;    
  union Ptr {
    Ptr() {};
    ~Ptr() {};
    int64_t intData;
    double doubleData;
    std::chrono::high_resolution_clock::time_point timestampData;
    std::string * stringPointer;
    std::vector<int64_t> * intListPointer;
    std::vector<double> * doubleListPointer;
    std::vector<std::string> * stringListPointer;
  } data;
};

} // namespace streamc
