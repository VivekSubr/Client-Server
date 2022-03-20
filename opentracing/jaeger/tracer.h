#pragma once
#include <string>
#include <map>
#include <iostream>
#include "opentelemetry/sdk/trace/tracer.h"
#include "opentelemetry/sdk/common/global_log_handler.h"
#include "opentelemetry/ext/http/client/http_client.h"
#include "opentelemetry/trace/propagation/http_trace_context.h"
#include "logger/logger.h"

namespace trace      = opentelemetry::trace;
namespace nostd      = opentelemetry::nostd;
namespace trace_sdk  = opentelemetry::sdk::trace;
namespace http       = opentelemetry::ext::http;

enum TraceType {
   UDP = 0,
   HTTP,
   Memory
};

class CustomLogHandler : public opentelemetry::sdk::common::internal_log::LogHandler
{
  Logger m_logger = Logger("jaeger.log");

public:
    void Handle(opentelemetry::sdk::common::internal_log::LogLevel level,
                const char *file,
                int line,
                const char *msg,
                const opentelemetry::sdk::common::AttributeMap &attributes) noexcept override

    {
      m_logger.Log(file, line, msg);
    }
};

template <typename T>
class HttpTextMapCarrier : public opentelemetry::context::propagation::TextMapCarrier
{
public:
  HttpTextMapCarrier<T>(T &headers) : headers_(headers) {}
  HttpTextMapCarrier() = default;
  virtual opentelemetry::nostd::string_view Get(
      opentelemetry::nostd::string_view key) const noexcept override
  {
    std::string key_to_compare = key.data();
    // Header's first letter seems to be  automatically capitaliazed by our test http-server, so
    // compare accordingly.
    if (key == opentelemetry::trace::propagation::kTraceParent)
    {
      key_to_compare = "Traceparent";
    }
    else if (key == opentelemetry::trace::propagation::kTraceState)
    {
      key_to_compare = "Tracestate";
    }
    auto it = headers_.find(key_to_compare);
    if (it != headers_.end())
    {
      return it->second;
    }
    return "";
  }

  virtual void Set(opentelemetry::nostd::string_view key,
                   opentelemetry::nostd::string_view value) noexcept override
  {
    headers_.insert(std::pair<std::string, std::string>(std::string(key), std::string(value)));
  }

  T headers_;
};

class Tracer 
{
 public:
  Tracer() = delete;
  Tracer(const std::string& app, const std::string& ver, TraceType t);
  ~Tracer();

  nostd::shared_ptr<trace::Span> StartSpan(const std::string& str);
  void                           SetTraceType(TraceType t);
  std::string                    GetTraceTypeStr(TraceType t) { return sTraceType.at(t); } 
  void                           InjectSpan(HttpTextMapCarrier<http::client::Headers> carrier);
  nostd::shared_ptr<trace::Span> ExtractSpan(HttpTextMapCarrier<http::client::Headers> carrier);
  
 private:
  nostd::shared_ptr<trace::Tracer>          m_tracer;
  std::shared_ptr<trace_sdk::TracerContext> m_tracer_ctx;
  std::shared_ptr<trace::TracerProvider>    m_trace_provider;
  std::unordered_map<TraceType, std::string> sTraceType = {
    {UDP, "udp"}, {HTTP, "http"}, {Memory, "memory"}
  };

  void initTracer(const std::string& app, const std::string& ver, TraceType t);
};