#include "streamc/runtime/RuntimeLogger.h"

#include <cstdlib>
#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/empty_deleter.hpp>
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

typedef sinks::synchronous_sink<sinks::text_file_backend> file_sink_t;
typedef sinks::synchronous_sink< sinks::text_ostream_backend > simple_sink_t;

namespace streamc
{
  class RuntimeLoggerSinks
  {
  public:
    boost::shared_ptr<file_sink_t> infSink;
    boost::shared_ptr<file_sink_t>  appSink;
    boost::shared_ptr<simple_sink_t> clogSink;
  };
}

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", LogLevel);
BOOST_LOG_ATTRIBUTE_KEYWORD(channel, "Channel", std::string);

RuntimeLogger::RuntimeLogger()
  : infLogger_(keywords::channel = "inf"),
    appLogger_(keywords::channel = "app"),
    sinks_(new RuntimeLoggerSinks())
{
  string const logDir = "logs";

  logging::formatter fmt = expr::format("%1%: <%2%> [%3%] %4%")
            % expr::attr<boost::posix_time::ptime>("TimeStamp")            
            % expr::attr<attrs::current_thread_id::value_type>("ThreadID")
            % expr::attr<LogLevel>("Severity")
            % expr::smessage ; 

  size_t fileSize = 1024 * 1024 * 1024;
  size_t maxFiles = 10;
  boost::shared_ptr<sinks::text_file_backend> backend =
    boost::make_shared< sinks::text_file_backend >(
      keywords::file_name = logDir+"/scInf_%Y-%m-%d_%H-%M-%S.%N.log",  
      keywords::rotation_size = fileSize
      //keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0) 
    );
  backend->auto_flush(true);
  sinks_->infSink = boost::make_shared<file_sink_t>(backend);
  sinks_->infSink->set_filter(severity >= Error && channel == "inf");
  sinks_->infSink->set_formatter(fmt);
  sinks_->infSink->locked_backend()->set_file_collector(sinks::file::make_collector(
        keywords::target = logDir,                      
        keywords::max_size = maxFiles * fileSize
  ));
  logging::core::get()->add_sink(sinks_->infSink);

  backend =
    boost::make_shared< sinks::text_file_backend >(
      keywords::file_name = logDir + "/scApp_%Y-%m-%d_%H-%M-%S.%N.log",  
      keywords::rotation_size = fileSize
      //keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0) 
    );
  backend->auto_flush(true);
  sinks_->appSink = boost::make_shared<file_sink_t>(backend);
  sinks_->appSink->set_filter(severity >= Error && channel == "app");
  sinks_->appSink->set_formatter(fmt);
  sinks_->appSink->locked_backend()->set_file_collector(sinks::file::make_collector(
        keywords::target = logDir,                      
        keywords::max_size = maxFiles * fileSize
  ));
  logging::core::get()->add_sink(sinks_->appSink);

  // We have to provide an empty deleter to avoid destroying the global stream object
  sinks_->clogSink = boost::make_shared<simple_sink_t>();
  boost::shared_ptr<ostream> stream(&std::clog, logging::empty_deleter());
  sinks_->clogSink->locked_backend()->add_stream(stream);
  sinks_->clogSink->set_formatter(fmt);
  sinks_->clogSink->set_filter(severity == Error);
  logging::core::get()->add_sink(sinks_->clogSink);

  logging::add_common_attributes();
}

RuntimeLogger::~RuntimeLogger()
{
  logging::core::get()->remove_sink(sinks_->infSink);
  logging::core::get()->remove_sink(sinks_->appSink);
  logging::core::get()->remove_sink(sinks_->clogSink);
}

void RuntimeLogger::setInfrastructureLogLevel(LogLevel level)
{

  sinks_->infSink->set_filter(severity >= level && channel == "inf");
}

void RuntimeLogger::setApplicationLogLevel(LogLevel level)
{
  sinks_->appSink->set_filter(severity >= level && channel == "app");
}

