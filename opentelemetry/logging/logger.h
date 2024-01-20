#pragma once
#include <string>
#include <opentelemetry/logs/severity.h>
#include <opentelemetry/sdk/logs/logger.h>

class Logger
{
public:
    Logger(const std::string& name, const std::string& caller_name);

    void log(opentelemetry::logs::Severity dl, const std::string& log);
    
    std::string loggerName(); 

private:
    int64_t m_eventId{0};
    opentelemetry::nostd::shared_ptr<opentelemetry::logs::Logger> m_logger;
};