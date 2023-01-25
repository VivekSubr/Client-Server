#include "gtest/gtest.h"
#include "tracer.h"

template <typename T>
static std::string Hex(const T &id_item)
{
  char buf[T::kSize * 2];
  id_item.ToLowerBase16(buf);
  return std::string(buf, sizeof(buf));
}

std::string spanId2Str(const opentelemetry::trace::SpanId& id)
{
  std::string ret;
  uint8_t buffer[opentelemetry::trace::SpanId::kSize];
  id.CopyBytesTo(buffer);

  for(int i=0; i<opentelemetry::trace::SpanId::kSize; i++) {
    if(i>0) ret += "-";
    ret += std::to_string(buffer[i]);
  }

  return ret;
}

class TestTracer : public ::testing::Test
{
public:
    virtual void SetUp(void) { }
    virtual void TearDown(void) { }
    static void SetUpTestSuite() { }
    static void TearDownTestSuite() { }
};