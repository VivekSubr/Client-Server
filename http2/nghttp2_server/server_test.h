#pragma once
#include "gtest/gtest.h"

class ServerTest : public ::testing::Test
{
public:
    virtual void SetUp(void) { }
    virtual void TearDown(void) {}
    static void SetUpTestSuite();
    static void TearDownTestSuite(); 
};