#include "tracer.h"
#include "opentelemetry/exporters/jaeger/jaeger_exporter.h"
#include "opentelemetry/exporters/memory/in_memory_span_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"

namespace jaeger     = opentelemetry::exporter::jaeger;
namespace trace_sdk  = opentelemetry::sdk::trace;
namespace memory     = opentelemetry::exporter::memory;

Tracer::Tracer(const std::string& app, const std::string& ver)
{
    jaeger::JaegerExporterOptions opts;
    opts.endpoint = "localhost";
    opts.server_port = 6831;

    auto exporter  = std::unique_ptr<trace_sdk::SpanExporter>(new jaeger::JaegerExporter(opts));
    auto processor = std::unique_ptr<trace_sdk::SpanProcessor>(new trace_sdk::SimpleSpanProcessor(std::move(exporter)));
    auto provider  = nostd::shared_ptr<trace::TracerProvider>(new trace_sdk::TracerProvider(std::move(processor)));
    m_UDP_tracer = provider->GetTracer(app, ver);

    opts.server_port = 5778; 
    auto exporter2  = std::unique_ptr<trace_sdk::SpanExporter>(new jaeger::JaegerExporter(opts));
    auto processor2 = std::unique_ptr<trace_sdk::SpanProcessor>(new trace_sdk::SimpleSpanProcessor(std::move(exporter2)));
    auto provider2  = nostd::shared_ptr<trace::TracerProvider>(new trace_sdk::TracerProvider(std::move(processor2)));
    m_HTTP_tracer = provider2->GetTracer(app, ver);

    auto memory_exporter = std::unique_ptr<trace_sdk::SpanExporter>(new memory::InMemorySpanExporter);
    auto processor3 = std::unique_ptr<trace_sdk::SpanProcessor>(new trace_sdk::SimpleSpanProcessor(std::move(memory_exporter)));
    auto provider3  = nostd::shared_ptr<trace::TracerProvider>(new trace_sdk::TracerProvider(std::move(processor3)));
    m_mem_tracer = provider3->GetTracer(app, ver);
}

nostd::shared_ptr<trace_api::Span> Tracer::StartSpan(const std::string& str)
{
  return m_tracer->StartSpan(str);
}

void Tracer::SetTraceType(TraceType t)
{
   switch(t) {
     case UDP:
      m_tracer = m_UDP_tracer;
      break;
     case HTTP:
      m_tracer = m_HTTP_tracer;
      break;
     case Memory:
      m_tracer = m_mem_tracer;
      break;
   }
}

