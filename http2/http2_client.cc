#include <string>
#include <iostream>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void dump(const char *text, FILE *stream, unsigned char *ptr, size_t size)
{
  size_t i;
  size_t c;
  unsigned int width=0x10;
 
  fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n",
          text, (long)size, (long)size);
 
  for(i=0; i<size; i+= width) {
    fprintf(stream, "%4.4lx: ", (long)i);
 
    /* show hex to the left */
    for(c = 0; c < width; c++) {
      if(i+c < size)
        fprintf(stream, "%02x ", ptr[i+c]);
      else
        fputs("   ", stream);
    }
 
    /* show data on the right */
    for(c = 0; (c < width) && (i+c < size); c++) {
      char x = (ptr[i+c] >= 0x20 && ptr[i+c] < 0x80) ? ptr[i+c] : '.';
      fputc(x, stream);
    }
 
    fputc('\n', stream); /* newline */
  }
}

static int my_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp)
{
  const char *text;
  (void)handle; /* prevent compiler warning */
  (void)userp;
 
  switch (type) {
  case CURLINFO_TEXT:
    fprintf(stderr, "== Info: %s", data);
  default: /* in case a new one is introduced to shock us */
    return 0;
 
  case CURLINFO_HEADER_OUT:
    text = "=> Send header";
    break;
  case CURLINFO_DATA_OUT:
    text = "=> Send data";
    break;
  case CURLINFO_SSL_DATA_OUT:
    text = "=> Send SSL data";
    break;
  case CURLINFO_HEADER_IN:
    text = "<= Recv header";
    break;
  case CURLINFO_DATA_IN:
    text = "<= Recv data";
    break;
  case CURLINFO_SSL_DATA_IN:
    text = "<= Recv SSL data";
    break;
  }
 
  dump(text, stdout, (unsigned char *)data, size);
  return 0;
}

static int sockopt_callback(void *clientp, curl_socket_t curlfd, curlsocktype purpose)
{
    std::cout<<"***Hit sockopt_callback\n";

    int *dscp = static_cast<int*>(clientp);
    if(setsockopt(curlfd, IPPROTO_IP, IP_TOS, (char*)dscp, (int32_t)sizeof(*dscp)) == -1)
    {
       std::cout<<"**Failed to setsockopt\n";
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
