#include "metrics_test.h"
#include "metrics_mock.h"

TEST_F(TestMetrics, SyncInstruments)
{
    Metrics m("testMeter", "v1", "");

    std::unique_ptr<MockMetricExporter>       exporter(new MockMetricExporter());
    std::shared_ptr<metric_sdk::MetricReader> reader{new MockMetricReader(std::move(exporter))};
    m.AddMetricReader(reader);

    std::unique_ptr<metric_sdk::View> view{new metric_sdk::View("view1", "view1_description", metric_sdk::AggregationType::kSum)};
    std::unique_ptr<metric_sdk::InstrumentSelector> instrument_selector{
            new metric_sdk::InstrumentSelector(metric_sdk::InstrumentType::kCounter, "testCounter")};
    std::unique_ptr<metric_sdk::MeterSelector> meter_selector{new metric_sdk::MeterSelector("testMeter", "v1", "schema1")};

    auto inst = m.CreateSyncInstrument(Metrics::InstrumentType::UIntCounter, "testCounter", "test", "test");
    ASSERT_TRUE(inst != nullptr);

   auto counter = static_cast<metrics::Counter<uint64_t>*>(inst);
    counter->Add(5);

    std::vector<metric_sdk::SumPointData> actuals;
    reader->Collect([&](metric_sdk::ResourceMetrics &rm) {
    for (const metric_sdk::ScopeMetrics &smd : rm.scope_metric_data_)
    {
      for (const metric_sdk::MetricData &md : smd.metric_data_)
      {
        for (const metric_sdk::PointDataAttributes &dp : md.point_data_attr_)
        {
          actuals.push_back(opentelemetry::nostd::get<metric_sdk::SumPointData>(dp.point_data));
        }
      }
    }
    return true;
  });

  ASSERT_EQ(1, actuals.size());
  const auto &actual = actuals.at(0);
  ASSERT_EQ(opentelemetry::nostd::get<int64_t>(actual.value_), 5); 
}

TEST_F(TestMetrics, ObservableInstruments)
{
  Metrics m("testMeter", "v1", "");

  std::unique_ptr<MockMetricExporter>       exporter(new MockMetricExporter());
  std::shared_ptr<metric_sdk::MetricReader> reader{new MockMetricReader(std::move(exporter))};
  m.AddMetricReader(reader);

  std::unique_ptr<metric_sdk::View> view{new metric_sdk::View("view1", "view1_description", metric_sdk::AggregationType::kSum)};
  std::unique_ptr<metric_sdk::InstrumentSelector> instrument_selector{
            new metric_sdk::InstrumentSelector(metric_sdk::InstrumentType::kCounter, "testCounter")};
  std::unique_ptr<metric_sdk::MeterSelector> meter_selector{new metric_sdk::MeterSelector("testMeter", "v1", "schema1")};

  auto instCB = [](metrics::ObserverResult observer, void* /* state */) { 
          std::cout<<"***Hit Callback!\n";
          auto observer_long = nostd::get<nostd::shared_ptr<opentelemetry::metrics::ObserverResultT<int64_t>>>(observer);
          observer_long->Observe(10);
  };

  m.CreateObservableInstrument(Metrics::InstrumentType::IntCounter, "testCounter", "test", "test", instCB);

  size_t count = 0;
  reader->Collect([&count](metric_sdk::ResourceMetrics &metric_data) {
    EXPECT_EQ(metric_data.scope_metric_data_.size(), 1);
    if (metric_data.scope_metric_data_.size())
    {
      EXPECT_EQ(metric_data.scope_metric_data_[0].metric_data_.size(), 1);
      if (metric_data.scope_metric_data_.size())
      {
        count += metric_data.scope_metric_data_[0].metric_data_.size();
        EXPECT_EQ(count, 1);
      }
    }
    return true;
  });
}