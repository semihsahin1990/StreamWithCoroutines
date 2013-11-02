#include "streamc/Value.h"

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
T from_string_vec(string const & str)
{
  istringstream sstr(str);
  vector<T> vecValue;
  from_string_vec<T>(sstr, vecValue);
  return vecValue;
}

template <typename T>
void from_string_vec(istream & sstr, vector<T> & vecValue)
{
  while (is_space(sstr.peek())) 
    sstr.get();
  int c = sstr.get();
  if (c!='[') {
    sstr.setstate(ios::failbit);
    return;
  }
  while (is_space(sstr.peek())) 
    sstr.get(); 
  c = sstr.peek();
  if (c==']') {
    sstr.get();
    return vecValue;    
  }
  sstr >> value; 
  if (!sstr) 
    return;
  vecValue.push_back(value);
  while(true) {
    while (is_space(sstr.peek())) 
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
}

Value Value::fromString(string const & str, Type type) 
{
  switch(type) {
  case Type::Integer:
    return Value(from_string<uint64_t>(str));
  case Type::Double:
    return Value(from_string<double>(str));
  case Type::String:
    return str;
    //TODO: return Value(from_string_literal(str));
  case Type::IntList:
    return Value(from_string_vec<uint64_t>(str));
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
    return to_string(*value.intPointer);
  case Type::Double:
    return to_string(*value.doublePointer);
  case Type::String:
    return *value.stringPointer;
    //TODO: return to_string_literal(*value.stringPointer);
  case Type::IntList:
    retur to_string_vec(*value.intListPointer);
  case Type::DoubleList:
    retur to_string_vec(*value.doubleListPointer);
  case Type::StringList:
    retur to_string_vec(*value.stringListPointer);
  }          
}

