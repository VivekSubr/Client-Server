#pragma once
#include <string>
#include <map>
#include "opentelemetry/sdk/trace/tracer.h"

namespace trace = opentelemetry::trace;
namespace nostd = opentelemetry::nostd;

enum TraceType {
   UDP = 0,
   HTTP,
   Memory
};

class Tracer 
{
 public:
  Tracer() = delete;
  Tracer(const std::string& app, const std::string& ver);
  ~Tracer() = default;

  nostd::shared_ptr<trace_api::Span> StartSpan(const std::string& str);
  void                               SetTraceType(TraceType t);
  std::string                        GetTraceTypeStr(TraceType t) { return sTraceType.at(t); }   

 private:
  nostd::shared_ptr<trace::Tracer> m_UDP_tracer;
  nostd::shared_ptr<trace::Tracer> m_HTTP_tracer;
  nostd::shared_ptr<trace::Tracer> m_mem_tracer;
  
  nostd::shared_ptr<trace::Tracer> m_tracer;
  std::unordered_map<TraceType, std::string> sTraceType = {
    {UDP, "udp"}, {HTTP, "http"}, {Memory, "memory"}
  };

  void initTracer(TraceType t) { } 
};