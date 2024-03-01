#pragma once
#include <string>
#include <cstdarg>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/chrono.h>

enum class LogLevel
{
    Info = 0,
    Debug,
    Error
};

class FmtLogger
{
public:
    FmtLogger(const std::string& name): m_name(name)
    {
    }

    /*
        Log format:
        Line 1: DEBUG_LEVEL, LOGGER-NAME, TIMESTAMP (HH-MM-SS : DD-MM-YYYY) (ie, all meta data)
        Line 2: Log Body
    */
    void Log(LogLevel level, const std::string& log) const;

    /*
       Line 1: DEBUG_LEVEL, LOGGER-NAME, TIMESTAMP (HH-MM-SS : DD-MM-YYYY) (ie, all meta data)
       Line 2: Log body, as built by format string and args
    */
    template <typename... Args>
    void LogFmt(LogLevel level, const std::string& log_fmt, const Args&... args) const
    {
        auto dbg = getDebugLevel(level);
        fmt::print(fg(dbg.first), "{} {} {}\n", dbg.second, m_name, std::chrono::system_clock::now());
        fmt::print(log_fmt, args...);
    }

    struct LoggerStream 
    {
        template <typename T>
        LoggerStream operator<<(const T& log) 
        {
            fmt::print("  {}", log); 
            return LoggerStream{};
        }
    };
   
    // This api supports chaining logs with << operator
    LoggerStream LogStream(LogLevel level, const std::string& log);

    void Break()     const { fmt::print("{}\n", m_breakStr); }
    void LineBreak() const { fmt::print("\n"); }

    std::string GetName() const { return m_name; }
    void SetBreakStr(const std::string& breakStr) { m_breakStr = breakStr; }

private:
    const std::string m_name;
    std::string       m_breakStr = "**********************************";

    std::pair<fmt::color, std::string> getDebugLevel(LogLevel level) const;
};