#pragma once
#include <string>
#include <iostream>
#include <memory>

struct host 
{
    std::string         ip;
    short unsigned int  port;

    host(std::string Ip, short unsigned int Port): ip(Ip), port(Port)
    {
    }
};

int retError(const std::string sError)
{
    std::cout<<sError<<"\n";
    return -1;
}

std::unique_ptr<host> getHostFromArg(int argc, char** argv, int ipIndex, int portIndex)
{
    if(argc < ipIndex || argc < portIndex) return nullptr;
    return std::make_unique<host>(argv[ipIndex], std::atoi(argv[portIndex]));
}

