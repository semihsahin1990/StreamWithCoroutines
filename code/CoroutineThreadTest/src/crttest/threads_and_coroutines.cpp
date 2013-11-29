#include <atomic>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <thread>
#include <string>

#include <boost/log/attributes.hpp>
#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/log/attributes/current_thread_id.hpp>

using namespace std;
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

enum severity_level { trace, debug, info, error };

inline ostream & operator<<(ostream & ostr, severity_level const & s)
{
  switch(s) {
  case trace:
    ostr << "trace";
    break;
  case debug:
    ostr << "debug";
    break;
  case info:
    ostr << "info";
    break;
  case error:
    ostr << "error";
    break;
  }
  return ostr;
}


BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(sysLogger_, my_logger_mt)
{
    return my_logger_mt(keywords::channel = "sys");
}
BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(appLogger_, my_logger_mt)
{
    return my_logger_mt(keywords::channel = "app");
}
//BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(logger_, src::severity_logger_mt<severity_level>);
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level);
BOOST_LOG_ATTRIBUTE_KEYWORD(tag_attr, "Tag", std::string);
BOOST_LOG_ATTRIBUTE_KEYWORD(channel, "Channel", std::string);

void init_logger(string const & sysLogFile, severity_level sysLogLevel,
                 string const & appLogFile, severity_level appLogLevel)
{
  logging::formatter fmt = expr::format("%1%: <%2%> [%3%] - %4%")
            % expr::attr<boost::posix_time::ptime>("TimeStamp")            
            % expr::attr<attrs::current_thread_id::value_type>("ThreadID")
            % expr::attr<severity_level>("Severity")
            % expr::smessage  ; 

  typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;

  boost::shared_ptr<sinks::text_file_backend> backend =
    boost::make_shared< sinks::text_file_backend >(
      keywords::file_name = (sysLogFile+"_%Y-%m-%d_%H-%M-%S.%N.log").c_str(),  
      keywords::rotation_size = 10 * 1024 * 1024
      //keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0) 
    );
  backend->auto_flush(true);
  typedef sinks::synchronous_sink<sinks::text_file_backend> sink_t;
  boost::shared_ptr<sink_t> sink(new sink_t(backend));
  sink->set_filter(severity >= sysLogLevel && expr::has_attr(channel) && channel == "sys");
  sink->set_formatter(fmt);
  logging::core::get()->add_sink(sink);

  backend =
    boost::make_shared< sinks::text_file_backend >(
      keywords::file_name = (appLogFile+"_%Y-%m-%d_%H-%M-%S.%N.log").c_str(),  
      keywords::rotation_size = 10 * 1024 * 1024
      //keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0) 
    );
  backend->auto_flush(true);
  sink.reset(new sink_t(backend));
  sink->set_filter(severity >= sysLogLevel && expr::has_attr(channel) && channel == "app");
  sink->set_formatter(fmt);
  logging::core::get()->add_sink(sink);

  logging::add_common_attributes();
}

atomic<bool> done(false);

void work1()
{
  while(!done.load()) {
    //BOOST_LOG_SCOPED_THREAD_TAG("Tag", "sys");
    BOOST_LOG_SEV(appLogger_::get(), info) << "AAA";
  }
}

void work2()
{
  while(!done.load()) {
    //BOOST_LOG_SCOPED_THREAD_TAG("Tag", "app");    
    BOOST_LOG_SEV(sysLogger_::get(), info) << "BBB";
  }
}

int main(void)
{
  init_logger("sys", info, "app", trace);
  thread thread1(work1);
  thread thread2(work2);  
  this_thread::sleep_for(chrono::seconds(5));
  done.store(true);
  thread1.join();
  thread2.join();

  return EXIT_SUCCESS;
}
