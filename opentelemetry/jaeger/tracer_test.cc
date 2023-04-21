#include "trace_test.h"
#include "sampler.h"
#include "opentelemetry/context/propagation/global_propagator.h"
#include "logger/logger.h"
#include <fstream>

#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/trace/context.h"
#include "opentelemetry/context/propagation/global_propagator.h"
#include <opentelemetry/sdk/trace/samplers/trace_id_ratio.h>

using namespace std;
using namespace opentelemetry;

namespace trace      = opentelemetry::trace;
namespace nostd      = opentelemetry::nostd;
namespace trace_sdk  = opentelemetry::sdk::trace;
namespace http       = opentelemetry::ext::http;
namespace memory     = opentelemetry::exporter::memory;

TEST_F(TestTracer, Span)
{
    std::unique_ptr<memory::InMemorySpanExporter> exporter(new memory::InMemorySpanExporter());

    auto resource_attributes = opentelemetry::sdk::resource::ResourceAttributes
    {
        {"service.name", "Test"},
        {"service.instance.id", "Test-12"}
    };
    auto resource = opentelemetry::sdk::resource::Resource::Create(resource_attributes);
    
    std::shared_ptr<memory::InMemorySpanData> span_data = exporter->GetData();
    auto sampler = std::unique_ptr<CustomSampler>(new CustomSampler(0.1));
    auto processor = std::unique_ptr<trace_sdk::SpanProcessor>(new trace_sdk::SimpleSpanProcessor(std::move(exporter)));
    std::vector<std::unique_ptr<trace_sdk::SpanProcessor>> v; v.push_back(std::move(processor));
    auto m_tracer_ctx = std::make_shared<trace_sdk::TracerContext>(
                              std::move(v), 
                              resource, 
                              std::move(sampler));
        
    auto m_trace_provider = std::shared_ptr<trace::TracerProvider>(new trace_sdk::TracerProvider(m_tracer_ctx));
    auto m_tracer = m_trace_provider->GetTracer("TestTracer", "1.0.0");

    trace::StartSpanOptions options; 
    for(int i=0; i<10; i++)
    {
        std::ostringstream s;
        s << "span" << i;
        auto span = m_tracer->StartSpan(s.str(), {}, {}, options);
        span->SetAttribute("ForceSampling", true);
        span->End();
    }

    ASSERT_EQ(span_data->GetSpans().size(), 10);
}

/*
TEST_F(TestTracer, Span)
{
    Tracer tracer;
    std::unique_ptr<memory::InMemorySpanExporter> exporter(new memory::InMemorySpanExporter());
    auto span_data = tracer.InitInMemoryTracer("TestTracer", "1.0.0", std::move(exporter));
    tracer.StartSpan("Test Span");
    auto span = tracer.GetCurrentSpan();

    std::cout<<"spanId "<<spanId2Str(span->GetContext().span_id())<<"\n";
 
    //Started a span before end of first... hence this is child of first
    tracer.StartSpan("Test Child Span");
    auto child_span = tracer.GetCurrentSpan(); 
    std::cout<<"child spanId "<<spanId2Str(child_span->GetContext().span_id())<<"\n";

    ASSERT_EQ(0, span_data->GetSpans().size()); //nothing ended yet

    span->AddEvent("Test Event");

    child_span->End();
    auto exported_spans = span_data->GetSpans();
    ASSERT_EQ(1, exported_spans.size()); //after end span gets exported
    ASSERT_EQ("Test Child Span", exported_spans.at(0)->GetName());
    EXPECT_TRUE(exported_spans.at(0)->GetTraceId().IsValid());
    EXPECT_TRUE(exported_spans.at(0)->GetSpanId().IsValid());
    EXPECT_TRUE(exported_spans.at(0)->GetParentSpanId().IsValid());

    span->End();
    auto exported_spans2 = span_data->GetSpans();
    ASSERT_EQ(1, exported_spans2.size());
    ASSERT_EQ("Test Span", exported_spans2.at(0)->GetName());
    EXPECT_TRUE(exported_spans2.at(0)->GetTraceId().IsValid());
    EXPECT_TRUE(exported_spans2.at(0)->GetSpanId().IsValid());
    EXPECT_FALSE(exported_spans2.at(0)->GetParentSpanId().IsValid());

    // Verify trace and parent span id propagation
    EXPECT_EQ(exported_spans.at(0)->GetTraceId(), exported_spans2.at(0)->GetTraceId());
    EXPECT_EQ(exported_spans.at(0)->GetParentSpanId(), exported_spans2.at(0)->GetSpanId());
} 

TEST_F(TestTracer, SaveResumeSpan)
{
    Tracer tracer;
    std::unique_ptr<memory::InMemorySpanExporter> exporter(new memory::InMemorySpanExporter());
    auto span_data = tracer.InitInMemoryTracer("TestTracer", "1.0.0", std::move(exporter));
    tracer.StartSpan("Test Span");
    auto span      = tracer.GetCurrentSpan();

    span->AddEvent("Test Event");
    ASSERT_EQ(0, span_data->GetSpans().size()); 
    ASSERT_EQ(0, tracer.GetSpanMap().size());
    ASSERT_EQ(span->GetContext().span_id(),tracer.GetSpanId());

    ASSERT_EQ(tracer.GetSpanMap().size(), 0);
    auto id = tracer.SaveSpan();
    std::cout<<"id 1 "<<spanId2Str(id)<<"\n";
    ASSERT_EQ(tracer.GetSpanMap().size(), 1);
    ASSERT_TRUE(tracer.GetSpanMap().find(id) != tracer.GetSpanMap().end());

    ASSERT_EQ(1, tracer.GetSpanMap().size());
    ASSERT_EQ(0, span_data->GetSpans().size());

    tracer.StartSpan("Test Span");
    auto span2  = tracer.GetCurrentSpan();
    ASSERT_NE(id, tracer.GetSpanId());

    auto id2 = tracer.SaveSpan();
    //ASSERT_EQ(tracer.GetSpanMap().size(), 2);
    
    for(auto ele : tracer.GetSpanMap())
    {
        std::cout<<spanId2Str(ele.first)<<"\n";
    }

    ASSERT_TRUE(tracer.ResumeSpan(id));
//    ASSERT_EQ(id, tracer.GetSpanId());
} 

TEST_F(TestTracer, InjectExtract)
{
    Tracer tracer;
    std::unique_ptr<memory::InMemorySpanExporter> exporter(new memory::InMemorySpanExporter());
    auto span_data = tracer.InitInMemoryTracer("TestTracer", "1.0.0", std::move(exporter));
    tracer.StartSpan("Test Span");
    auto span      = tracer.GetCurrentSpan();
 
    auto carrier = tracer.InjectSpan();
    auto carrier_headers = carrier.Headers();

    std::cout<<"Size "<<carrier_headers.size()<<"\n";
    for(auto it : carrier_headers)
    {
        std::cout<<it.first<<":"<<it.second<<"\n";
    }

    auto remote_span = tracer.ExtractSpan(carrier);
    ASSERT_EQ(remote_span->GetContext().span_id(), span->GetContext().span_id());
}

TEST_F(TestTracer, Sampling)
{

} */
