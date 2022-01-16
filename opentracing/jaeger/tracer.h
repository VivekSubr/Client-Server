#pragma once
#include "opentelemetry/exporters/jaeger/jaeger_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"

namespace trace     = opentelemetry::trace;
namespace nostd     = opentelemetry::nostd;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace jaeger    = opentelemetry::exporter::jaeger;

struct Tracer 
{
    Tracer();
    ~Tracer();
};