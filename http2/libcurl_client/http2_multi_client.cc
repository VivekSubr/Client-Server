#include "CurlMultiInterface.h"

/*
  Here we use curl multi interface to send many requests together 
*/

#define CNT 4
CurlMulti*    CurlMulti::m_instance = nullptr;
CURLM*        CurlMulti::m_handle   = nullptr;
TimerManager* CurlMulti::m_timerManager = nullptr;

void http2POST(CURLM *cm, const std::string& json, int i)
{
  CURL *eh = curl_easy_init();
  curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(eh, CURLOPT_HEADER, 0L);
  curl_easy_setopt(eh, CURLOPT_URL, "http://localhost:3000/test1");
  curl_easy_setopt(eh, CURLOPT_PRIVATE, "http://localhost:3000/test1");
  curl_easy_setopt(eh, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
  curl_easy_setopt(eh, CURLOPT_POSTFIELDS, json.c_str());
  curl_easy_setopt(eh, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(eh, CURLOPT_DEBUGFUNCTION, my_trace);
  curl_multi_add_handle(cm, eh);
}

//based on this timer, we do curl_multi_socket_action
int CurlMulti::CurlTimerCallback(CURLM* multi_handle, long timeout_ms, void* user_pointer)
{
  CurlMulti *multiInf = CurlMulti::GetInstance();   
  multiInf->m_timerManager->CreateTimer();
  return CURLM_OK;
}

void CurlMulti::timerTriggered() 
{
  std::cout<<"Hit timerTriggered\n";
  CurlMulti *multiInf = CurlMulti::GetInstance();   
  int cnt_run = 0;
  CURLMcode  rc = curl_multi_socket_action(multiInf->getHandle(), CURL_SOCKET_TIMEOUT, 0, &cnt_run);
  if(rc != CURLM_OK) std::cout<<"curl_multi_socket_action error : "<<rc<<"\n";
}

int CurlSocketCallback(CURL* easy_handle, curl_socket_t socket, int action,
                       void* user_pointer, void* socket_pointer)
{
   return CURLM_OK;
}

int main() 
{
    curl_global_init(CURL_GLOBAL_ALL);
    CURLM *cm = curl_multi_init();
    CurlMulti *multiInf = CurlMulti::GetInstance();   
    multiInf->setHandle(cm);
    curl_multi_setopt(cm, CURLMOPT_TIMERFUNCTION, CurlMulti::CurlTimerCallback);
    curl_multi_setopt(cm, CURLMOPT_TIMERDATA, nullptr);

    for (int i = 0; i < CNT; ++i) http2POST(cm, "{\name\" : \"test1\"}", i);

    return 0;
}