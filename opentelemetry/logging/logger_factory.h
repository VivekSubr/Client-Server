#pragma once
#include "logger.h"
#include <memory>

class LoggerFactory
{
public:
    LoggerFactory();
    ~LoggerFactory();

    std::shared_ptr<Logger> GetLogger(const std::string& name, const std::string& caller_name);
    
private:
    
};