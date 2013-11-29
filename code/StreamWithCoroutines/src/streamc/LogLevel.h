#pragma once

#include <iostream>

namespace streamc 
{
  enum LogLevel { Trace=0, Debug, Info, Warn, Error };

inline std::ostream & operator<<(std::ostream & ostr, LogLevel const & s)
{
  static const char* strings[] =
  {
    "Trace",
    "Debug",
    "Info",
    "Warn",
    "Error"
  };
  ostr << strings[s];
  return ostr;
}

} // namespace streamc
