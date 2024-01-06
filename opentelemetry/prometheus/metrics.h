#pragma once
#include <unordered_map>
#include "opentelemetry/metrics/meter.h"
#include "opentelemetry/metrics/meter_provider.h"
#include "opentelemetry/metrics/sync_instruments.h"
#include "opentelemetry/metrics/async_instruments.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"

namespace metrics    = opentelemetry::metrics;
namespace metric_sdk = opentelemetry::sdk::metrics;
namespace nostd      = opentelemetry::nostd;

class Metrics
{
public:
    using SyncInstrument       = nostd::unique_ptr<metrics::SynchronousInstrument>;
    using SyncInstrumentMap    = std::unordered_map<std::string, SyncInstrument>;
    using ObservableInstrument = nostd::shared_ptr<metrics::ObservableInstrument>;

    enum class InstrumentType {
        UIntCounter = 0,
        IntCounter,
        DoubleCounter,
        IntGauge,
        UIntGauge,
        DoubleGauge,
        UIntHistogram,
        DoubleHistogram
    };
    
    struct ObservableInstrumentStruct {
        InstrumentType                 type;
        ObservableInstrument           instrument;
        metrics::ObservableCallbackPtr callback;

        ObservableInstrumentStruct(InstrumentType t, 
                                   ObservableInstrument inst, 
                                   metrics::ObservableCallbackPtr cb): type(t), instrument(inst), callback(cb) 
        {
        }
    };

    using ObservableInstrumentMap = std::unordered_map<std::string, ObservableInstrumentStruct>;

    Metrics() = delete; 
    Metrics(const std::string& name, const std::string& ver, const std::string& url);
    ~Metrics();

    void AddMetricReader(std::shared_ptr<metric_sdk::MetricReader> reader) { m_meterProvider->AddMetricReader(reader); }
    void AddView(std::unique_ptr<metric_sdk::View>& view, 
                 std::unique_ptr<metric_sdk::InstrumentSelector>& inst_selector, 
                 std::unique_ptr<metric_sdk::MeterSelector>& meter_selector)
    {
        m_meterProvider->AddView(std::move(inst_selector), std::move(meter_selector), std::move(view));
    }

    metrics::SynchronousInstrument* CreateSyncInstrument(InstrumentType t, const std::string& name, 
                                                            const std::string& description, const std::string& unit);
    void                            DestroySyncInstrument(const std::string& name) { m_syncInstruments.erase(name); }
    metrics::SynchronousInstrument* GetSyncInstrument(const std::string& name); 
    const SyncInstrumentMap&        GetSyncInstruments() { return m_syncInstruments; }

    ObservableInstrument CreateObservableInstrument(InstrumentType t, const std::string& name, 
                                                            const std::string& description, const std::string& unit,
                                                            metrics::ObservableCallbackPtr cb);
    void                           DestroyObservableInstrument(const std::string& name) { m_observableInstruments.erase(name); }
    const ObservableInstrumentMap& GetObservableInstruments() { return m_observableInstruments; }

private:
    const std::string m_metricsName;
    const std::string m_metricsVer;
    const std::string m_url;
    nostd::shared_ptr<metric_sdk::MeterProvider> m_meterProvider;
    nostd::shared_ptr<metrics::Meter>            m_meter;

    SyncInstrumentMap       m_syncInstruments;
    ObservableInstrumentMap m_observableInstruments;
};