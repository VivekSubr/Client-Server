#include "opentelemetry/exporters/otlp/otlp_grpc_metric_exporter_factory.h"
#include "metrics.h"
#include "metrics_logger.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader.h"
#include "opentelemetry/metrics/provider.h"

namespace otlp_exporter = opentelemetry::exporter::otlp;

Metrics::Metrics(const std::string& name, const std::string& ver, const std::string& url): 
                                                                   m_metricsName(name), m_metricsVer(ver), m_url(url), 
                                                                   m_meterProvider(new metric_sdk::MeterProvider())
{
    if(url.empty()) {
        m_meter = m_meterProvider->GetMeter(m_metricsName, m_metricsVer, "");
        std::cout<<"Empty url!\n";
        return;
    } 

    otlp_exporter::OtlpGrpcMetricExporterOptions grpc_options;
    grpc_options.endpoint = m_url;
    grpc_options.use_ssl_credentials = false;
    auto exporter = otlp_exporter::OtlpGrpcMetricExporterFactory::Create(grpc_options);

    metric_sdk::PeriodicExportingMetricReaderOptions options;
    options.export_interval_millis = std::chrono::milliseconds(1000);
    options.export_timeout_millis  = std::chrono::milliseconds(500);
    std::unique_ptr<metric_sdk::MetricReader> reader{new metric_sdk::PeriodicExportingMetricReader(std::move(exporter), options)};
            
    m_meterProvider->AddMetricReader(std::move(reader));
    m_meter = m_meterProvider->GetMeter(m_metricsName, m_metricsVer, "");

    ot_log::GlobalLogHandler::SetLogHandler(nostd::shared_ptr<MetricsLogHandler>());
    ot_log::GlobalLogHandler::SetLogLevel(ot_log::LogLevel::Debug);
}

Metrics::~Metrics()
{
    m_syncInstruments.clear(); //need to clear before shutdown
}

metrics::SynchronousInstrument* Metrics::CreateSyncInstrument(Metrics::InstrumentType t, const std::string& name, 
                                                            const std::string& description, const std::string& unit)
{
    SyncInstrument instrument;
    switch(t) 
    {
        case InstrumentType::UIntCounter:   
        { 
            instrument = m_meter->CreateUInt64Counter(name, description, unit);  
        } break;
        case InstrumentType::DoubleCounter: 
        { 
            instrument = m_meter->CreateDoubleCounter(name, description, unit);
        } break;
        case InstrumentType::UIntGauge:     
        { 
            instrument = m_meter->CreateInt64UpDownCounter(name, description, unit);
        } break;
        case InstrumentType::DoubleGauge:
        {
            instrument = m_meter->CreateDoubleUpDownCounter(name, description, unit);
        } break;
        case InstrumentType::UIntHistogram:
        {
            instrument = m_meter->CreateUInt64Histogram(name, description, unit);
        } break;
        case InstrumentType::DoubleHistogram:
        {
            instrument = m_meter->CreateDoubleHistogram(name, description, unit);
        } break;

        default: return nullptr;
    }

    auto ret = instrument.get();
    m_syncInstruments.insert({name, std::move(instrument)});   
    return ret;
}

metrics::SynchronousInstrument* Metrics::GetSyncInstrument(const std::string& name)
{
   auto it = m_syncInstruments.find(name);
   if(it == m_syncInstruments.end()) return nullptr;
   return it->second.get();
}

Metrics::ObservableInstrument Metrics::CreateObservableInstrument(Metrics::InstrumentType t, const std::string& name, 
                                                const std::string& description, const std::string& unit,
                                                metrics::ObservableCallbackPtr cb)
{
    ObservableInstrument instrument;
    switch(t)
    {
        case InstrumentType::IntCounter:   
        { 
            instrument = m_meter->CreateInt64ObservableCounter(name, description, unit);   
        } break;
        case InstrumentType::DoubleCounter: 
        { 
            instrument = m_meter->CreateDoubleObservableCounter(name, description, unit);
        } break;
        case InstrumentType::IntGauge:     
        { 
            instrument = m_meter->CreateInt64ObservableGauge(name, description, unit);
        } break;
        case InstrumentType::DoubleGauge:
        {
            instrument = m_meter->CreateDoubleObservableGauge(name, description, unit);
        } break;
        case InstrumentType::UIntHistogram:
        {
            instrument = m_meter->CreateInt64ObservableGauge(name, description, unit);
        } break;
        case InstrumentType::DoubleHistogram:
        {
            instrument = m_meter->CreateDoubleObservableGauge(name, description, unit);
        } break;

        default: return ObservableInstrument();
    }

    instrument->AddCallback(cb, nullptr);
    m_observableInstruments.insert({name, ObservableInstrumentStruct(t, instrument, cb)});
    return instrument;
}