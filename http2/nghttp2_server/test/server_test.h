#pragma once
#include "gtest/gtest.h"
#include "test_client.h"
#include <memory>
#include <thread>
#include <condition_variable>

class ServerTest : public ::testing::Test
{
public:
    virtual void SetUp(void) { }
    virtual void TearDown(void) {}
    static void SetUpTestSuite();
    static void TearDownTestSuite(); 

    static std::shared_ptr<std::thread> m_tServer;
    std::mutex                          m_mutex_cv;
    std::condition_variable             m_sync_cv;
};