#include "logger.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
namespace logging  = boost::log;
namespace keywords = boost::log::keywords;

Logger::Logger(const std::string& logName)
{
    logging::add_file_log(logName);
}

void Logger::Log(const char *file, int line, const char *msg)
{
    logging::record rec = lg.open_record();
    if (rec)
    {
        logging::record_ostream strm(rec);
        strm << "FILE: "<<file <<" : "<<line<<"\n";
        strm << msg;
        strm.flush();
        lg.push_record(boost::move(rec));
    }
}