#pragma once

#include "streamc/LogLevel.h"

#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace streamc 
{

class RuntimeLogger
{
public:
  typedef boost::log::sources::severity_channel_logger_mt<
    LogLevel,    // the type of the severity level
    std::string  // the type of the channel name
  > logger_mt;
  RuntimeLogger();
  ~RuntimeLogger();
  void setInfrastructureLogLevel(LogLevel level);
  void setApplicationLogLevel(LogLevel level);
  static logger_mt & getInfrastructureInternalLogger() 
  {
    return getLogger().infLogger_;
  }
  static logger_mt & getApplicationInternalLogger() 
  {
    return getLogger().appLogger_;
  }
  static RuntimeLogger & getLogger() 
  {
    static RuntimeLogger globalLogger;
    return globalLogger;
  }
private:
  typedef boost::log::sinks::synchronous_sink<
      boost::log::sinks::text_file_backend> sink_t;
  logger_mt infLogger_;
  logger_mt appLogger_;
  boost::shared_ptr<sink_t> infSink_;
  boost::shared_ptr<sink_t> appSink_;
};

} // namespace streamc

#define SC_LOG(lvl, body) \
  BOOST_LOG_SEV(RuntimeLogger::getInfrastructureInternalLogger(), lvl) << \
      __BASE_FILE__ << ":" << __LINE__ << " - " << body;



