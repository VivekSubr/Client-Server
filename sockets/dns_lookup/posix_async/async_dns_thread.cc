#include "utils.h"
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
//Design Doc: https://akkadia.org/drepper/asynchnl.pdf

void dnsCallback(sigval_t sigval)
{
    if(sigval.sival_ptr == NULL)
    {
        std::cout<<"sival_ptr NULL!\n";
        return;
    }

    std::cout<<"****Hit dnsCallback\n";
    struct gaicb* resp = reinterpret_cast<struct gaicb*>(sigval.sival_ptr);
    if(resp == NULL)
    {
       std::cout<<"cast NULL!\n";
       return;
    }

    for(auto p = resp->ar_result; p != NULL; p = p->ai_next)
    {
        printAddrInfo(p);
    }
    std::cout<<"******************\n\n";
}

int main(int argc, char **argv)
{
    std::string host = getHostFromArg(1, argc, argv);
    std::cout<<"Resolving "<<host<<"\n";
    
    struct addrinfo pollHints, servinfo;
    struct sigevent sev;
    struct gaicb**  dnsReqs;
    
    std::memset(&pollHints, 0, sizeof(pollHints));
    pollHints.ai_flags     = 0;
    pollHints.ai_family    = AF_INET;
    pollHints.ai_socktype  = SOCK_STREAM;

    dnsReqs    = (struct gaicb**)malloc(sizeof(struct gaicb*));
    dnsReqs[0] = (struct gaicb*)malloc(sizeof(struct gaicb));
    dnsReqs[0]->ar_name    = host.c_str();
    dnsReqs[0]->ar_service = "http";
    dnsReqs[0]->ar_request = &pollHints;
    dnsReqs[0]->ar_result  = &servinfo;

    std::memset(&sev, 0, sizeof(sev));
    sev.sigev_notify_function = &dnsCallback;
    sev.sigev_value.sival_ptr = dnsReqs[0];
    sev.sigev_notify = SIGEV_THREAD;

    int rv = getaddrinfo_a(GAI_NOWAIT, dnsReqs, 1, &sev);
    switch(rv)
    {
        case EAI_AGAIN:
            std::cout<<"getaddrinfo_a failed! "<<gai_error(dnsReqs[0])<<"\n"; 
            return -1;

        case EAI_MEMORY:
            std::cout<<"getaddrinfo_a failed! Out of Memory\n"; 
            return -1;

        case EAI_SYSTEM: 
            std::cout<<"getaddrinfo_a failed! EAI_SYSTEM\n"; 
            return -1;
    }

    struct timespec timeout;
    timeout.tv_sec = 15;
    rv = gai_suspend(dnsReqs, 1, &timeout);
    switch(rv)
    {
        case EAI_AGAIN:
            std::cout<<"gai_suspend timeout \n";
            break;
 
        case EAI_ALLDONE:
            std::cout<<"gai_suspend no requests pending\n";
            break;

        case EAI_INTR:
            std::cout<<"gai_suspend interupt\n";
            break;
    }

    free(dnsReqs[0]);
    free(dnsReqs);
    return 0;
}