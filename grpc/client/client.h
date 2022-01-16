#pragma once
#include <string>
#include "hello.grpc.pb.h"

class ClientImpl
{
   public:
    ClientImpl() = default;
 
    bool CallRPC(const std::string& msg)
    {
        return true;
    }

    ~ClientImpl() = default;    

   private:
    hello::Example rpc;
};