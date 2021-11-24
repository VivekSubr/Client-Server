#pragma once
#include "util.h"
#include "timer.h"

class CurlMulti {
   static CurlMulti    *m_instance;
   static CURLM        *m_handle;
   static TimerManager *m_timerManager;
   
   CurlMulti() = default;

  public:
    static CurlMulti* GetInstance() {
       if(m_instance) return m_instance;
       m_instance = new CurlMulti();
       m_timerManager = new TimerManager();
       m_instance->m_timerManager->setCallback(timerTriggered);
       return m_instance;
    }

    static int    CurlTimerCallback(CURLM* multi_handle, long timeout_ms, void* user_pointer);
    static void   timerTriggered();
    static void   setHandle(CURLM *cm) { m_handle = cm; }
    static CURLM* getHandle() { return m_handle; }
};