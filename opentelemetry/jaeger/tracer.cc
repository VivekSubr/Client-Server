#include <memory>
#include <vector>
#include "tracer.h"
#include "opentelemetry/exporters/jaeger/jaeger_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/trace/context.h"
#include "opentelemetry/trace/propagation/jaeger.h"
#include "opentelemetry/context/propagation/global_propagator.h"

//https://opentelemetry.io/docs/instrumentation/cpp/manual/

namespace jaeger      = opentelemetry::exporter::jaeger;
namespace ot_log      = opentelemetry::sdk::common::internal_log;
namespace propagation = opentelemetry::context::propagation;

//nostd::shared_ptr<trace::Span> Tracer::EMPTY_SPAN = opentelemetry::trace::NoopSpan;

Tracer::Tracer(const std::string& app, const std::string& ver, TraceType t)
{
  initTracer(app, ver, t);
  
  ot_log::GlobalLogHandler::SetLogHandler(nostd::shared_ptr<CustomLogHandler>());
  ot_log::GlobalLogHandler::SetLogLevel(ot_log::LogLevel::Debug);

  propagation::GlobalTextMapPropagator::SetGlobalPropagator(nostd::shared_ptr<trace::propagation::JaegerPropagator>(new trace::propagation::JaegerPropagator()));
}

Tracer::~Tracer() 
{
  m_spanMap.clear();
  m_parentSpans.clear();
  m_activeScopes.clear();
  std::static_pointer_cast<trace_sdk::TracerProvider>(m_trace_provider)->Shutdown();
}

opentelemetry::sdk::resource::Resource Tracer::createResources()
{
  auto resource_attributes = opentelemetry::sdk::resource::ResourceAttributes
  {
        {"service.name", "Test"},
        {"service.instance.id", "Test-12"}
  };
  auto resource = opentelemetry::sdk::resource::Resource::Create(resource_attributes);
  return resource;
}

void Tracer::initTracer(const std::string& app, const std::string& ver, TraceType t)
{
  jaeger::JaegerExporterOptions opts;
  opts.endpoint = "localhost";

  auto resource = createResources();
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
        opts.transport_format = opentelemetry::exporter::jaeger::TransportFormat::kThriftHttp;
        opts.server_port      = 14268;
        opts.headers          = {{}}; // optional headers

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
        std::unique_ptr<memory::InMemorySpanExporter> exporter(new memory::InMemorySpanExporter());
        InitInMemoryTracer(app, ver, std::move(exporter));
      } break;

    default:
      exit(-1);
  }
}

std::shared_ptr<memory::InMemorySpanData> Tracer::InitInMemoryTracer(const std::string& app, 
                                                              const std::string& ver,
                                                              std::unique_ptr<memory::InMemorySpanExporter> exporter)
{
  auto resource = createResources();
  std::shared_ptr<memory::InMemorySpanData> span_data = exporter->GetData();
  auto always_on_sampler = std::unique_ptr<trace_sdk::AlwaysOnSampler>(new trace_sdk::AlwaysOnSampler);
  auto processor = std::unique_ptr<trace_sdk::SpanProcessor>(new trace_sdk::SimpleSpanProcessor(std::move(exporter)));
  std::vector<std::unique_ptr<trace_sdk::SpanProcessor>> v; v.push_back(std::move(processor));
  m_tracer_ctx = std::make_shared<trace_sdk::TracerContext>(
                              std::move(v), 
                              resource, 
                              std::move(always_on_sampler));
        
  m_trace_provider = std::shared_ptr<trace::TracerProvider>(new trace_sdk::TracerProvider(m_tracer_ctx));
  m_tracer = m_trace_provider->GetTracer(app, ver);
  return span_data;
}

HttpTextMapCarrier<http::client::Headers> Tracer::InjectSpan()
{
  HttpTextMapCarrier<http::client::Headers> carrier;
  auto propagator  = propagation::GlobalTextMapPropagator::GetGlobalPropagator();
  auto current_ctx = opentelemetry::context::RuntimeContext::GetCurrent();
  propagator->Inject(carrier, current_ctx);

  return carrier;
}

nostd::shared_ptr<trace::Span> Tracer::ExtractSpan(HttpTextMapCarrier<opentelemetry::ext::http::client::Headers>& carrier)
{
  auto propagator  = propagation::GlobalTextMapPropagator::GetGlobalPropagator();
  auto current_ctx = opentelemetry::context::RuntimeContext::GetCurrent();
  auto new_context = propagator->Extract(carrier, current_ctx);
  auto remote_span = trace::GetSpan(new_context);

  return remote_span;
}