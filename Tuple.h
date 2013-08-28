#pragma once

class Tuple
{
private:
    class Value {
        enum Type = { Integer, Double, String, /**/}
        Value(std::string const & value)
        {
            type_ = String;
            pointer_.stringPointer_ = new std::string(value);
        }
        // ...
        ~Value()
        {
            switch() {
                // typeina göre delete et
            }
        }
    private:
        Type type_;
        Ptr pointer_;
        union Ptr {
            int64_t * intPointer_;
            std::string * stringPointer_;
            // ...
        };
        int64_t & getInt64Value() {
            return &pointer_.intPointer_;
        }
        // ...
    };

  ~Tuple() 
  {
    // delete the values
  }
  void addAttribute(std::string const & name, int64_t value);
  void addAttribute(std::string const & name, double value);
  void addAttribute(std::string const & name, std::string const & value)
  {
    Value * value = new Value(value);
    attributes_[name] = value;
  }
  void addAttribute(std::string const & name, std::vector<double> value);
 
  int64_t & getInt64Attribute(std::string const & name);
  double & getDoubleAttribute(std::string const & name);
  int64_t const & getInt64Attribute(std::string const & name) const;
  double const & getDoubleAttribute(std::string const & name) const;
  std::vector<double> & getDoubleListAttribute(std::string const & name);
  std::vector<double> const & getDoubleListAttribute(std::string const & name);
 
  // std::unordered_map<std::string, Value *>

};
