#pragma once
#include <string>
#include <boost/log/core.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/logger.hpp>

class Logger
{
   public:
    Logger() = delete;
    Logger(const std::string& logName);
    ~Logger() = default;

    void Log(const char *file, int line, const char *msg);

   private:
    boost::log::sources::logger lg;
};