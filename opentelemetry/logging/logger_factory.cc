#include "logger_factory.h"
#include <opentelemetry/exporters/ostream/log_record_exporter.h>
#include <opentelemetry/logs/provider.h>
#include <opentelemetry/sdk/logs/logger_provider_factory.h>
#include <opentelemetry/sdk/logs/processor.h>
#include <opentelemetry/sdk/logs/simple_log_record_processor_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_log_record_exporter_factory.h>
#include <ios>

namespace logs_api      = opentelemetry::logs;
namespace logs_sdk      = opentelemetry::sdk::logs;
namespace logs_exporter = opentelemetry::exporter::logs;

bool LoggerFactory::init(LoggerTypes::ExportType transportType)
{
  std::unique_ptr<logs_sdk::LogRecordExporter> exporter;
  switch(transportType)
  {
    case LoggerTypes::ExportType::OStream: 
      std::cout<<"**getting ostream exporter\n";
      exporter  = std::unique_ptr<logs_sdk::LogRecordExporter>(new logs_exporter::OStreamLogRecordExporter);
      break;

    case LoggerTypes::ExportType::Http: 
      exporter  = opentelemetry::exporter::otlp::OtlpHttpLogRecordExporterFactory::Create();
      break;
  }
  
  auto processor = logs_sdk::SimpleLogRecordProcessorFactory::Create(std::move(exporter));
  std::shared_ptr<logs_api::LoggerProvider> provider(logs_sdk::LoggerProviderFactory::Create(std::move(processor)));

  // Set the global logger provider
  logs_api::Provider::SetLoggerProvider(provider);

  //Optimizations
  std::ios_base::sync_with_stdio(false); //don't sync with stdio
  std::cin.tie(NULL); //don't sync cout and cin

  return true;
}

LoggerFactory::~LoggerFactory()
{
  std::shared_ptr<logs_api::LoggerProvider> none;
  logs_api::Provider::SetLoggerProvider(none);
}

std::shared_ptr<Logger> LoggerFactory::GetLogger(const std::string& name, const std::string& caller_name)
{
  return std::make_shared<Logger>(name, caller_name); 
}

