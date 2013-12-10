#include "streamc/Value.h"

#include <sstream>
#include <cctype>

using namespace std;
using namespace streamc;

template <typename T>
T from_string(string const & str)
{
  T value;
  istringstream sstr(str);
  sstr >> value;
  return value;
}

template <typename T>
void from_string_vec(istream & sstr, vector<T> & vecValue)
{
  T value;
  while (isspace(sstr.peek())) 
    sstr.get();
  int c = sstr.get();
  if (c!='[') {
    sstr.setstate(ios::failbit);
    return;
  }
  while (isspace(sstr.peek())) 
    sstr.get(); 
  c = sstr.peek();
  if (c==']') {
    sstr.get();
    return;    
  }
  sstr >> value; 
  if (!sstr) 
    return;
  vecValue.push_back(value);
  while(true) {
    while (isspace(sstr.peek())) 
      sstr.get();
    c = sstr.get();      
    if (c!=',' && c!=']') {
      sstr.setstate(ios::failbit);
      return;
    }
    if (c==']') 
      return;
    sstr >> value; 
    if (!sstr) 
      return;
    vecValue.push_back(value);      
  }
}

template <typename T>
std::vector<T> from_string_vec(string const & str)
{
  istringstream sstr(str);
  vector<T> vecValue;
  from_string_vec<T>(sstr, vecValue);
  return vecValue;
}

template <typename T>
string to_string_vec(vector<T> const & vecValue)
{
  ostringstream ostr;
  ostr << "[";
  if (!vecValue.empty()) {
    auto it=vecValue.begin();
    ostr << *it;
    for (++it; it!=vecValue.end(); ++it) 
      ostr << "," << *it;
  }
  ostr << "]";
  return ostr.str();
}

Value Value::fromString(string const & str, Type type) 
{
  switch(type) {
  case Type::Integer:
    return Value(from_string<int64_t>(str));
  case Type::Double:
    return Value(from_string<double>(str));
  case Type::String:
    return str;
    //TODO: return Value(from_string_literal(str));
  case Type::IntList:
    return Value(from_string_vec<int64_t>(str));
  case Type::DoubleList:
    return Value(from_string_vec<double>(str));
  case Type::StringList:
    return Value(from_string_vec<string>(str));    
  }          
}

string Value::toString(Value const & value) 
{
  switch(value.getType()) {
  case Type::Integer:
    return to_string(value.data.intData);
  case Type::Double:
    return to_string(value.data.doubleData);
  case Type::String:
    return *(value.data.stringPointer);
    //TODO: return to_string_literal(*value.stringPointer);
  case Type::IntList:
    return to_string_vec(*(value.data.intListPointer));
  case Type::DoubleList:
    return to_string_vec(*(value.data.doubleListPointer));
  case Type::StringList:
    return to_string_vec(*(value.data.stringListPointer));
  }          
}

