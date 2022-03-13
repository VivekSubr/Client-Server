#include <memory>
#include <vector>
#include "tracer.h"
#include "opentelemetry/exporters/jaeger/jaeger_exporter.h"
#include "opentelemetry/exporters/memory/in_memory_span_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/trace/context.h"
#include "opentelemetry/trace/propagation/jaeger.h"
#include "opentelemetry/context/propagation/global_propagator.h"

namespace jaeger      = opentelemetry::exporter::jaeger;
namespace memory      = opentelemetry::exporter::memory;
namespace log         = opentelemetry::sdk::common::internal_log;
namespace propagation = opentelemetry::context::propagation;

Tracer::Tracer(const std::string& app, const std::string& ver, TraceType t)
{
  initTracer(app, ver, t);
  
  log::GlobalLogHandler::SetLogHandler(nostd::shared_ptr<CustomLogHandler>());
  log::GlobalLogHandler::SetLogLevel(log::LogLevel::Debug);

  propagation::GlobalTextMapPropagator::SetGlobalPropagator(nostd::shared_ptr<trace::propagation::JaegerPropagator>(new trace::propagation::JaegerPropagator()));
}

Tracer::~Tracer() 
{
  std::static_pointer_cast<trace_sdk::TracerProvider>(m_trace_provider)->Shutdown();
}

void Tracer::initTracer(const std::string& app, const std::string& ver, TraceType t)
{
  jaeger::JaegerExporterOptions opts;
  opts.endpoint = "localhost";

  auto resource_attributes = opentelemetry::sdk::resource::ResourceAttributes
  {
        {"service.name", "Test"},
        {"service.instance.id", "Test-12"}
  };
  auto resource = opentelemetry::sdk::resource::Resource::Create(resource_attributes);
  auto always_on_sampler = std::unique_ptr<trace_sdk::AlwaysOnSampler>(new trace_sdk::AlwaysOnSampler);

  switch(t)
  {
    case UDP:
      {
        opts.server_port = 6831;

        auto exporter  = std::unique_ptr<trace_sdk::SpanExporter>(new jaeger::JaegerExporter(opts));
        auto processor = std::unique_ptr<trace_sdk::SpanProcessor>(new trace_sdk::SimpleSpanProcessor(std::move(exporter)));
        std::vector<std::unique_ptr<trace_sdk::SpanProcessor>> v; v.push_back(std::move(processor));
        m_tracer_ctx = std::make_shared<trace_sdk::TracerContext>(
                              std::move(v), 
                              resource, 
                              std::move(always_on_sampler));
        
        m_trace_provider =  std::shared_ptr<trace::TracerProvider>(new trace_sdk::TracerProvider(m_tracer_ctx));
        m_tracer = m_trace_provider->GetTracer(app, ver);
      } break;

    case HTTP:
      {
        opts.server_port = 5778;

        auto exporter  = std::unique_ptr<trace_sdk::SpanExporter>(new jaeger::JaegerExporter(opts));
        auto processor = std::unique_ptr<trace_sdk::SpanProcessor>(new trace_sdk::SimpleSpanProcessor(std::move(exporter)));
        std::vector<std::unique_ptr<trace_sdk::SpanProcessor>> v; v.push_back(std::move(processor));
        m_tracer_ctx = std::make_shared<trace_sdk::TracerContext>(
                              std::move(v), 
                              resource, 
                              std::move(always_on_sampler));
        
        m_trace_provider =  std::shared_ptr<trace::TracerProvider>(new trace_sdk::TracerProvider(m_tracer_ctx));
        m_tracer = m_trace_provider->GetTracer(app, ver);
      } break;

    case Memory:
      {
        auto memory_exporter = std::unique_ptr<trace_sdk::SpanExporter>(new memory::InMemorySpanExporter);
        auto processor = std::unique_ptr<trace_sdk::SpanProcessor>(new trace_sdk::SimpleSpanProcessor(std::move(memory_exporter)));
        std::vector<std::unique_ptr<trace_sdk::SpanProcessor>> v; v.push_back(std::move(processor));
        m_tracer_ctx = std::make_shared<trace_sdk::TracerContext>(
                              std::move(v), 
                              resource, 
                              std::move(always_on_sampler));
        
        m_trace_provider =  std::shared_ptr<trace::TracerProvider>(new trace_sdk::TracerProvider(m_tracer_ctx));
        m_tracer = m_trace_provider->GetTracer(app, ver);
      } break;

    default:
      exit(-1);
  }
}

nostd::shared_ptr<trace::Span> Tracer::StartSpan(const std::string& str)
{
  return m_tracer->StartSpan(str);
}

void Tracer::InjectSpan(HttpTextMapCarrier<opentelemetry::ext::http::client::Headers> carrier)
{
  auto propagator = propagation::GlobalTextMapPropagator::GetGlobalPropagator();
  auto current_ctx = opentelemetry::context::RuntimeContext::GetCurrent();
  propagator->Inject(carrier, current_ctx);
}

nostd::shared_ptr<trace::Span> Tracer::ExtractSpan(HttpTextMapCarrier<opentelemetry::ext::http::client::Headers> carrier)
{
  auto propagator  = propagation::GlobalTextMapPropagator::GetGlobalPropagator();
  auto current_ctx = opentelemetry::context::RuntimeContext::GetCurrent();
  auto new_context = propagator->Extract(carrier, current_ctx);
  auto remote_span = trace::GetSpan(new_context);

  return remote_span;
}