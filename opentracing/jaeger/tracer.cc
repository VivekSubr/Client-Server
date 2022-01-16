#include "tracer.h"

Tracer::Tracer()
{
    opentelemetry::exporter::jaeger::JaegerExporterOptions opts;
    
    // Create Jaeger exporter instance
    auto exporter  = std::unique_ptr<trace_sdk::SpanExporter>(new jaeger::JaegerExporter(opts));
    auto processor = std::unique_ptr<trace_sdk::SpanProcessor>(new trace_sdk::SimpleSpanProcessor(std::move(exporter)));
    auto provider  = nostd::shared_ptr<trace::TracerProvider>(new trace_sdk::TracerProvider(std::move(processor)));
        
    // Set the global trace provider
    trace::Provider::SetTracerProvider(provider);
}

Tracer::~Tracer()
{
}