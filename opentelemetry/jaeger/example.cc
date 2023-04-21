#include "tracer.h"
#include "http.h"
#include <iostream>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

int main(int argc, char *argv[])
{   
    httpClient client;
    std::string str;
    std::string sServerIP = "http://localhost:3000/";

    while(std::getline(std::cin, str)) 
    {
        trim(str);
        TraceType t = Otel_GRPC;
        if(str == "mem") t = Memory;

        bool ret = client.post(sServerIP, "test1", "{\name\" : \"test1\"}", t);
        std::cout<<"sent: "<<ret<<"\n";
    }

    return 0;
}