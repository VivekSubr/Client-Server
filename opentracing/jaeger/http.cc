#include "http.h"
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>

httpClient::httpClient()
{
    curl_global_init(CURL_GLOBAL_ALL);
}

httpClient::~httpClient()
{
    curl_global_cleanup();
}

bool httpClient::post(const std::string& ip, const std::string& api, const std::string& json, TraceType t)
{
    auto m_tracer = std::make_unique<Tracer>("httpClient", "1.0.0", t);
    auto span = m_tracer->StartSpan("http post " + m_tracer->GetTraceTypeStr(t));
    auto hnd = curl_easy_init();
    if(hnd == NULL) return false;

    span->AddEvent("sending to localhost:8344");
    curl_easy_setopt(hnd, CURLOPT_URL, "http://localhost:8344");

    curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);

    std::string url = ip + api;
    curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, json.c_str());

    bool ret = true;
    if(curl_easy_perform(hnd) != CURLE_OK) ret = false;
    curl_easy_cleanup(hnd);

    span->AddEvent("completed, ret: " + std::to_string(ret));
    span->End();
    return ret;
}