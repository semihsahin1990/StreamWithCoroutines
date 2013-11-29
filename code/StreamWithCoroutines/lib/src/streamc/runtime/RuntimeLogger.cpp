#include "streamc/runtime/RuntimeLogger.h"

#include <cstdlib>
#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>

using namespace std;
using namespace streamc;
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", LogLevel);
BOOST_LOG_ATTRIBUTE_KEYWORD(channel, "Channel", std::string);

RuntimeLogger::RuntimeLogger()
  : infLogger_(keywords::channel = "inf"),
    appLogger_(keywords::channel = "app")
{
  logging::formatter fmt = expr::format("%1%: <%2%> [%3%] %4%")
            % expr::attr<boost::posix_time::ptime>("TimeStamp")            
            % expr::attr<attrs::current_thread_id::value_type>("ThreadID")
            % expr::attr<LogLevel>("Severity")
            % expr::smessage  ; 

  size_t fileSize = 1024 * 1024 * 1024;
  size_t maxFiles = 10;
  boost::shared_ptr<sinks::text_file_backend> backend =
    boost::make_shared< sinks::text_file_backend >(
      keywords::file_name = "scInf_%Y-%m-%d_%H-%M-%S.%N.log",  
      keywords::rotation_size = fileSize
      //keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0) 
    );
  backend->auto_flush(true);
  infSink_.reset(new sink_t(backend));
  infSink_->set_filter(severity >= Error && channel == "inf");
  infSink_->set_formatter(fmt);
  infSink_->locked_backend()->set_file_collector(sinks::file::make_collector(
        keywords::target = "logs",                      
        keywords::max_size = maxFiles * fileSize
  ));
  logging::core::get()->add_sink(infSink_);
  
  backend =
    boost::make_shared< sinks::text_file_backend >(
      keywords::file_name = "scApp_%Y-%m-%d_%H-%M-%S.%N.log",  
      keywords::rotation_size = fileSize
      //keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0) 
    );
  backend->auto_flush(true);
  appSink_.reset(new sink_t(backend));
  appSink_->set_filter(severity >= Error && channel == "app");
  appSink_->set_formatter(fmt);
  appSink_->locked_backend()->set_file_collector(sinks::file::make_collector(
        keywords::target = "logs",                      
        keywords::max_size = maxFiles * fileSize
  ));
  logging::core::get()->add_sink(appSink_);

  logging::add_common_attributes();
}

RuntimeLogger::~RuntimeLogger()
{
  logging::core::get()->remove_sink(infSink_);
  logging::core::get()->remove_sink(appSink_);
}

void RuntimeLogger::setInfrastructureLogLevel(LogLevel level)
{
  infSink_->set_filter(severity >= level && channel == "inf");
}

void RuntimeLogger::setApplicationLogLevel(LogLevel level)
{
  appSink_->set_filter(severity >= level && channel == "app");
}

