#pragma once
#include <math.h>
#include "opentelemetry/sdk/trace/sampler.h"

namespace trace_sdk  = opentelemetry::sdk::trace;

class CustomSampler : public trace_sdk::Sampler
{
public:
  explicit CustomSampler(double ratio): threshold_(CalculateThreshold(ratio))
  {
    if(ratio > 1.0) ratio = 1.0;
    if(ratio < 0.0) ratio = 0.0;
    description_ = "CustomSampler{" + std::to_string(ratio) + "}";
  }

  /**
   * @return Returns either RECORD_AND_SAMPLE or DROP based on current
   * sampler configuration and provided trace_id and ratio. trace_id
   * is used as a pseudorandom value in conjunction with the predefined
   * ratio to determine whether this trace should be sampled
   */
  trace_sdk::SamplingResult ShouldSample(
      const opentelemetry::trace::SpanContext& parent_context,
      opentelemetry::trace::TraceId trace_id,
      nostd::string_view name,
      opentelemetry::trace::SpanKind span_kind,
      const opentelemetry::common::KeyValueIterable& attributes,
      const opentelemetry::trace::SpanContextKeyValueIterable& links) noexcept override
    {
        std::cout<<"***Hit ShouldSample "<<name<<"\n";
        if(threshold_ == 0) return {trace_sdk::Decision::DROP, nullptr, {}};
        
        if(CheckIfForceSampling(attributes) || CalculateThresholdFromBuffer(trace_id) <= threshold_)
        {
            return {trace_sdk::Decision::RECORD_AND_SAMPLE, nullptr, {}};
        }

        return {trace_sdk::Decision::DROP, nullptr, {}};
    }

    nostd::string_view GetDescription() const noexcept override 
    {
        return description_;
    }

private:
    std::string    description_;
    const uint64_t threshold_;

    //Converts a ratio in [0, 1] to a threshold in [0, UINT64_MAX]
    uint64_t CalculateThreshold(double ratio) noexcept
    {
        if (ratio <= 0.0) return 0;
        if (ratio >= 1.0) return UINT64_MAX;

        // We can't directly return ratio * UINT64_MAX.
        // UINT64_MAX is (2^64)-1, but as a double rounds up to 2^64.
        // For probabilities >= 1-(2^-54), the product wraps to zero! Instead, calculate the high and low 32 bits separately.
        const double product = UINT32_MAX * ratio;
        double hi_bits, lo_bits = ldexp(modf(product, &hi_bits), 32) + product;
        return (static_cast<uint64_t>(hi_bits) << 32) + static_cast<uint64_t>(lo_bits);
    }

    uint64_t CalculateThresholdFromBuffer(const trace_api::TraceId &trace_id) noexcept
    {
        uint64_t res = 0;
        std::memcpy(&res, &trace_id, 8);

        double ratio = (double)res / (double)UINT64_MAX;
        return CalculateThreshold(ratio);
    }

    bool CheckIfForceSampling(const opentelemetry::common::KeyValueIterable& attributes)
    {
        std::cout<<"***CheckIfForceSampling "<<attributes.size()<<"\n";
        return !attributes.ForEachKeyValue([](nostd::string_view key, opentelemetry::common::AttributeValue value) -> bool {
            std::cout<<"***ForEachKeyValue :"<<key<<"\n";
            if(key == "ForceSampling" && absl::get<bool>(value) == true) return false;
            
            return true;
        });
    }
};