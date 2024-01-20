#pragma once
#include "gtest/gtest.h"
#include "logger.h"

class TestLogger : public ::testing::Test
{
public:
    virtual void SetUp(void) { }
    virtual void TearDown(void) { }
    static void SetUpTestSuite() { }
    static void TearDownTestSuite() { }
};