#include "trace_test.h"
#include "opentelemetry/context/propagation/global_propagator.h"
#include "logger/logger.h"
#include <fstream>
using namespace std;
using namespace opentelemetry;

TEST(TestTracer, Span)
{
    Tracer tracer;
    std::unique_ptr<memory::InMemorySpanExporter> exporter(new memory::InMemorySpanExporter());
    auto span_data = tracer.InitInMemoryTracer("TestTracer", "1.0.0", std::move(exporter));
    auto span      = tracer.StartSpan("Test Span");

    ASSERT_EQ(0, span_data->GetSpans().size());
    
    span->AddEvent("Test Event");
    span->End();
}

TEST(TestTracer, SpanPropogation)
{
    struct TestTrace
    {
        std::string trace_state;
        std::string expected_trace_id;
        std::string expected_span_id;
        bool sampled;
    };

    std::vector<TestTrace> traces = {
      {
          "4bf92f3577b34da6a3ce929d0e0e4736:0102030405060708:0:00",
          "4bf92f3577b34da6a3ce929d0e0e4736",
          "0102030405060708",
          false,
      },
      {
          "4bf92f3577b34da6a3ce929d0e0e4736:0102030405060708:0:ff",
          "4bf92f3577b34da6a3ce929d0e0e4736",
          "0102030405060708",
          true,
      },
      {
          "4bf92f3577b34da6a3ce929d0e0e4736:0102030405060708:0:f",
          "4bf92f3577b34da6a3ce929d0e0e4736",
          "0102030405060708",
          true,
      },
      {
          "a3ce929d0e0e4736:0102030405060708:0:00",
          "0000000000000000a3ce929d0e0e4736",
          "0102030405060708",
          false,
      },
      {
          "A3CE929D0E0E4736:ABCDEFABCDEF1234:0:01",
          "0000000000000000a3ce929d0e0e4736",
          "abcdefabcdef1234",
          true,
      },
      {
          "ff:ABCDEFABCDEF1234:0:0",
          "000000000000000000000000000000ff",
          "abcdefabcdef1234",
          false,
      },
      {
          "4bf92f3577b34da6a3ce929d0e0e4736:0102030405060708:0102030405060708:00",
          "4bf92f3577b34da6a3ce929d0e0e4736",
          "0102030405060708",
          false,
      },

  };

  auto propagator  = context::propagation::GlobalTextMapPropagator::GetGlobalPropagator();
  for (TestTrace &test_trace : traces)
  {
    HttpTextMapCarrier<http::client::Headers> carrier;
    carrier.headers_      = {{"uber-trace-id", test_trace.trace_state}};
    context::Context ctx1 = context::Context{};
    context::Context ctx2 = propagator->Extract(carrier, ctx1);

    auto span = trace::GetSpan(ctx2)->GetContext();
    EXPECT_TRUE(span.IsValid());

    EXPECT_EQ(Hex(span.trace_id()), test_trace.expected_trace_id);
    EXPECT_EQ(Hex(span.span_id()), test_trace.expected_span_id);
    EXPECT_EQ(span.IsSampled(), test_trace.sampled);
    EXPECT_EQ(span.IsRemote(), true);
  }
}

TEST(TestTracer, Logger)
{
    Logger lg("jaeger.log");
    lg.Log("test.c", 12, "test message");

    std::ifstream iFile("jaeger.log");
    ASSERT_TRUE(iFile.good());
}