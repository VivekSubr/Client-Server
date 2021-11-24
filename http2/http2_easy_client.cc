#include "util.h"
/*
 Here, we use libcurl easy interface for http2 client
 */

static int sockopt_callback(void *clientp, curl_socket_t curlfd, curlsocktype purpose)
{
    //std::cout<<"***Hit sockopt_callback\n";

    int *dscp = static_cast<int*>(clientp);
    if(setsockopt(curlfd, IPPROTO_IP, IP_TOS, (char*)dscp, (int32_t)sizeof(*dscp)) == -1)
    {
     //  std::cout<<"**Failed to setsockopt\n";
       return -1;
    }

    delete dscp;
    return 0;
}

bool http2POST(const std::string& api, const std::string& json, int *dscp) 
{
    auto hnd = curl_easy_init();
    if(hnd == NULL) return false;

    curl_easy_setopt(hnd, CURLOPT_URL, "http://localhost:8344");

    curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);
    //curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(hnd, CURLOPT_DEBUGFUNCTION, my_trace);

    curl_easy_setopt(hnd, CURLOPT_HTTP2_SOCKOPTFUNCTION, sockopt_callback);
    curl_easy_setopt(hnd, CURLOPT_HTTP2_SOCKOPTDATA, dscp);

    std::string url = "http://localhost:3000/" + api;
    curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, json.c_str());

    bool ret = true;
    if(curl_easy_perform(hnd) != CURLE_OK) ret = false;
    curl_easy_cleanup(hnd);
    return ret;
}

int main()
{
    curl_global_init(CURL_GLOBAL_ALL);
    
    while(true) {
       int *dscp1 = new int(46);
       int *dscp2 = new int(12);
       int *dscp3 = new int(0);

       if(!http2POST("test1", "{\name\" : \"test1\"}", dscp1)) std::cout<<"Failed test1\n";
       if(!http2POST("test2", "{\name\" : \"test2\"}", dscp2)) std::cout<<"Failed test2\n";
       if(!http2POST("test3", "{\name\" : \"test3\"}", dscp3)) std::cout<<"Failed test3\n";
       
       sleep(5);   
       std::cout<<"***********************\n";
       std::cout<<"sent to localhost:3000\n";   
       std::cout<<"***********************\n";   
    }

    curl_global_cleanup();
    return 0;
}
