#include "tracer.h"

class httpClient
{
public:
   httpClient();
   ~httpClient();
   
   bool post(const std::string& ip, const std::string& api, const std::string& json, TraceType t);          
};