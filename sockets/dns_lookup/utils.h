#pragma once
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>

void printAddrInfo(struct addrinfo* info)
{
    std::cout<<"***** struct addrinfo *******\n"
             <<"ai_family "<<info->ai_family<<"\n"
             <<"ai_socktype "<<info->ai_socktype<<"\n"
             <<"ai_protocol "<<info->ai_protocol<<"\n"
             <<"ai_flags "<<info->ai_flags<<"\n"
             <<"ai_addrlen "<<info->ai_addrlen<<"\n"
             <<"ai_canonname "<<(info->ai_canonname == 0?"0" : info->ai_canonname)<<"\n";
    if(info->ai_addr->sa_family == AF_INET) { 
        sockaddr_in* addr = (sockaddr_in*)info->ai_addr;
        std::cout<<"ip address "<<inet_ntoa(addr->sin_addr)<<"\n";
    } else std::cout<<"not AF_INET!\n";
    std::cout<<"*****************************\n";

   if(info->ai_next) 
   {
       std::cout<<"*****************************\n";
       printAddrInfo(info->ai_next);
   }
}

std::string getHostFromArg(int index, int argc, char **argv)
{
    std::string host;
    if(argc > index) 
    {
        host = argv[index];
    }
    else host = "google.com";

    return host;
}