#include "fmt_logger.h"
#include <stdexcept>

std::pair<fmt::color, std::string> FmtLogger::getDebugLevel(LogLevel level) const
{
     auto level_color   = fmt::color::floral_white;
     std::string sLevel;
     switch(level)
     {
        case LogLevel::Info:
             level_color = fmt::color::floral_white;
             sLevel      = "INFO";
             break;
        
        case LogLevel::Debug:
             level_color = fmt::color::steel_blue;
             sLevel      = "DEBUG";
             break;

        case LogLevel::Error:
             level_color = fmt::color::crimson;
             sLevel      = "ERROR";
             break;

        default:
            throw std::runtime_error("Unknown log level");
     }

     return {level_color, sLevel};
}

void FmtLogger::Log(LogLevel level, const std::string& log) const
{
     auto dbg = getDebugLevel(level);
     fmt::print(fg(dbg.first), "{} ", dbg.second);
     fmt::print("{} {}\n{}\n", m_name, std::chrono::system_clock::now(), log);
}

FmtLogger::LoggerStream FmtLogger::LogStream(LogLevel level, const std::string& log)
{
     auto dbg = getDebugLevel(level);
     fmt::print(fg(dbg.first), "\n{} ", dbg.second);
     fmt::print("{} {}\n{}", m_name, std::chrono::system_clock::now(), log);

     return LoggerStream{};
}
