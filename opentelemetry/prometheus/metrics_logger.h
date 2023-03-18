#pragma once
#include <unordered_map>
#include <iostream>
#include <opentelemetry/sdk/common/global_log_handler.h>

namespace ot_log = opentelemetry::sdk::common::internal_log;

class MetricsLogHandler : public ot_log::LogHandler
{
public: 
    void Handle(ot_log::LogLevel level,
                const char *file,
                int line,
                const char *msg,
                const opentelemetry::sdk::common::AttributeMap &attributes) noexcept override

    {
      const std::string logMsg = std::string(file) + ":" + std::to_string(line) + " - " + msg;
      std::cout<<logMsg<<std::endl;
    }
};
