#include "metrics.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/exporters/prometheus/exporter.h"

Metrics::Metrics(const std::string& url): m_meterProvider(metrics::Provider::GetMeterProvider())
{
    m_meter = m_meterProvider->GetMeter("prometheus");

    metrics_exporter::PrometheusExporterOptions opts;
    opts.url = url;

    std::unique_ptr<metrics_sdk::PushMetricExporter> exporter{new metrics_exporter::PrometheusExporter(opts)};

    // Initialize and set the global MeterProvider
    metrics_sdk::PeriodicExportingMetricReaderOptions options;
    options.export_interval_millis = std::chrono::milliseconds(30000);
    options.export_timeout_millis  = std::chrono::milliseconds(2000);

    std::unique_ptr<metrics_sdk::MetricReader> reader{new metrics_sdk::PeriodicExportingMetricReader(std::move(exporter), options)};
    m_meterProvider->AddMetricReader(std::move(reader));
}

Metrics::~Metrics()
{
}

void Metrics::CreateSyncInstrument(InstrumentType t, const std::string& name, const std::string& description, const std::string& unit)
{
    switch(t) 
    {
        case UIntCounter:   { m_syncInstruments.insert(m_meter->CreateUInt64Counter(name, description, unit));      } break;
        case DoubleCounter: { m_syncInstruments.insert(m_meter->CreateDoubleCounter(name, description, unit));      } break;
        case UIntGauge:     { m_syncInstruments.insert(m_meter->CreateInt64UpDownCounter(name, description, unit)); } break;

        case DoubleGauge:
        {

        } break;

        case UIntHistogram:
        {

        } break;

        case DoubleHistogram:
        {

        } break;
    }
}