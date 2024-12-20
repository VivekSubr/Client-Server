#include "logger.h"
#include <opentelemetry/logs/provider.h>
#include <iostream>

Logger::Logger(const std::string& name, const std::string& caller_name):
        m_logger(opentelemetry::logs::Provider::GetLoggerProvider()->GetLogger(name, caller_name))
{
  std::cout<<"***Logger::Logger\n";
}

//EmitLogRecord can take several types of args,
//https://github.com/open-telemetry/opentelemetry-cpp/blob/c4f39f2be8109fd1a3e047677c09cf47954b92db/api/include/opentelemetry/logs/logger.h#L100

void Logger::log(opentelemetry::logs::Severity dl, const std::string& log)
{
  ++m_eventId;
  m_logger->EmitLogRecord(dl, log, opentelemetry::common::SystemTimestamp(std::chrono::system_clock::now()));
}

std::string Logger::loggerName()
{
  return std::string(m_logger->GetName());
}