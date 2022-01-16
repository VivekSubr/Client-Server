#pragma once
#include <chrono>
#include <map>
#include <thread>
#include <functional>
#include <iostream>
#define CURL_TIMER_MS 2000
#define TRY_TIMER_CB(cb)\
    try { cb(); }\
    catch(std::bad_function_call) { std::cout<<"timer callback, std::bad_function_call\n"; }\
    catch(...) { std::cout<<"timer callback, unhandled exceptin\n"; }\

class Timer {
    int  m_id = -1;
    bool m_clear = false;
    std::function<void()>  m_callback;

public:
    Timer(int id, 
          std::function<void()> callback): m_id(id), m_callback(callback)  { }
    ~Timer() = default;

    void setTimeout(int delay);
    void setInterval(int interval);
  
    void stop() { m_clear = true; }
    int  id()   { return m_id;    }
};

class TimerManager {
 public:
    void CreateTimer() {
        int id;
        if(m_Timers.size() > 0) id = m_Timers.rbegin()->first;
        else                    id = 0;

        Timer t(id + 1, m_callback);
        t.setTimeout(CURL_TIMER_MS + id*1000);
        m_Timers.insert({t.id(), t}); 
    }

    void setCallback(std::function<void()> callback) { m_callback = callback; }
    
  private:
      std::function<void()>  m_callback;
      std::map<int, Timer>   m_Timers;
};

void Timer::setTimeout(int delay) {
    m_clear = false;
    std::thread t([=]() {
        if(this->m_clear) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        if(this->m_clear) return;
        std::cout<<"timer finish "<<m_id<<"\n";
        TRY_TIMER_CB(m_callback)
    });
    t.detach();
}

void Timer::setInterval(int interval) {
    m_clear = false;
    std::thread t([=]() {
        while(true) {
            if(m_clear) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            if(m_clear) return;
            std::cout<<"timer finish "<<m_id<<"\n";
            TRY_TIMER_CB(m_callback)
        }
    });
    t.detach();
}