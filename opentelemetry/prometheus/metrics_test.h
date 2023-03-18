#include "gtest/gtest.h"
#define private public
    #include "metrics.h"
#undef private

class TestMetrics : public ::testing::Test
{
public:
    virtual void SetUp(void) { }
    virtual void TearDown(void) { }
    static void SetUpTestSuite() { }
    static void TearDownTestSuite() { }
};

