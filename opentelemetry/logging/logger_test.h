#pragma once
#include "gtest/gtest.h"
#define private public
    #include "logger.h"
#undef private

class TestLogger : public ::testing::Test
{
public:
    virtual void SetUp(void) { }
    virtual void TearDown(void) { }
    static void SetUpTestSuite() { }
    static void TearDownTestSuite() { }
};