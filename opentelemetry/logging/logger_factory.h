#pragma once
#include "logger.h"
#include <memory>

namespace LoggerTypes 
{
    enum class ExportType {
        OStream = 1,
        Http 
    };
}

class LoggerFactory
{
private:
    LoggerFactory() {} 

public:
    ~LoggerFactory();

    static LoggerFactory* getInstance() 
    {
        static LoggerFactory temp;
        return &temp;
    }

    bool init(LoggerTypes::ExportType transportType);

    std::shared_ptr<Logger> GetLogger(const std::string& name, const std::string& caller_name);
    
private:
    LoggerTypes::ExportType m_exportType;
};