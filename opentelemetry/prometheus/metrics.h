#pragma once
#include <unordered_map>
#include "opentelemetry/metrics/meter.h"
#include "opentelemetry/metrics/meter_provider.h"
#include "opentelemetry/metrics/sync_instruments.h"

namespace metrics = opentelemetry::metrics;
namespace nostd   = opentelemetry::nostd;

class Metrics 
{
public:
    enum class InstrumentType {
        UIntCounter = 0,
        DoubleCounter,
        UIntGauge,
        DoubleGauge,
        UIntHistogram,
        DoubleHistogram
    };

    Metrics(const std::string& url);
    ~Metrics();

    void CreateSyncInstrument(InstrumentType t, const std::string& name, const std::string& description, const std::string& unit);

private:
    nostd::shared_ptr<metrics::MeterProvider> m_meterProvider;
    nostd::shared_ptr<metrics::Meter>         m_meter;

    std::unordered_map<std::string, metrics::SynchronousInstrument> m_syncInstruments;
};