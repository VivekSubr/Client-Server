#include "logger_test.h"
#include "logger_factory.h"
#include "fmt_logger.h"

TEST_F(TestLogger, FmtLog)
{
    FmtLogger logger("test");
    logger.Log(LogLevel::Debug, "testBody");
    logger.Break();
    logger.LogFmt(LogLevel::Debug, "{} {} {}\n", "Test", "Fmt", "Log");

    logger.LogStream(LogLevel::Info, "testStream") << "streaming1" << "streaming2";
}

TEST_F(TestLogger, Log)
{
    LoggerFactory::getInstance()->init(LoggerTypes::ExportType::OStream);

    auto logger = LoggerFactory::getInstance()->GetLogger("test", "test");
    ASSERT_TRUE(logger != nullptr);
    ASSERT_TRUE(logger->m_logger != nullptr);
    ASSERT_EQ(logger->loggerName(), "test");

    logger->log(opentelemetry::logs::Severity::kDebug, "test log");
}