#include "utils.h"
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
//Design Doc: https://akkadia.org/drepper/asynchnl.pdf

//max number of events that can be queued up in epoll
#define MAX_EVENTS 32

void dnsCallback(sigval_t sigval)
{
    if(sigval.sival_ptr == NULL)
    {
        std::cout<<"sival_ptr NULL!\n";
        return;
    }

    addrinfo* servinfo = reinterpret_cast<addrinfo*>(sigval.sival_ptr);
    std::cout<<"****Hit dnsCallback\n";
    for(auto p = servinfo; p != NULL; p = p->ai_next)
    {
        printAddrInfo(p);
    }
    std::cout<<"******************\n\n";
}

int signalfd_setup() 
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGRTMIN + 3);
    sigprocmask(SIG_BLOCK, &mask, NULL); //we block the signal
    int sfd = signalfd(-1, &mask, 0);
    //create and add to event queue
    int epfd = epoll_create(1);
 
    struct epoll_event ev;
	ev.events  = EPOLLIN | EPOLLOUT | EPOLLET;
	ev.data.fd = sfd;
	if(epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev) == -1) {
		std::cout<<"failed initial epoll_ctl\n";
        return -1;
	}

    return epfd;
}

int main(int argc, char **argv)
{
    std::string host = getHostFromArg(2, argc, argv);
    std::string type = "thread";
    if(argc > 1) type = argv[1];

    std::cout<<"Resolving "<<host<<" type "<<type<<"\n";
    
    struct addrinfo pollHints, *servinfo{nullptr};
    struct sigevent sev;
    struct gaicb**  dnsReqs;
    
    std::memset(&pollHints, 0, sizeof(pollHints));
    pollHints.ai_flags     = 0;
    pollHints.ai_family    = AF_INET;
    pollHints.ai_socktype  = SOCK_STREAM;

    std::memset(&sev, 0, sizeof(sev));
    sev.sigev_notify_function = &dnsCallback;
    sev.sigev_value.sival_ptr = servinfo;
    if(type == "thread")
    {
        sev.sigev_notify = SIGEV_THREAD;
    }
    else if(type == "signal")
    {
        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo  = SIGRTMIN + 3;
    }

    dnsReqs    = (struct gaicb**)malloc(sizeof(struct gaicb*));
    dnsReqs[0] = (struct gaicb*)malloc(sizeof(struct gaicb));
    dnsReqs[0]->ar_name    = host.c_str();
    dnsReqs[0]->ar_service = "http";
    dnsReqs[0]->ar_request = &pollHints;
    dnsReqs[0]->ar_result  = servinfo;

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

    if(type == "signal")
    {
       int efd = signalfd_setup();
       if(efd == -1) return -1;
 
       struct epoll_event events[MAX_EVENTS];
       int nfds = epoll_wait(efd, events, MAX_EVENTS, -1); //blocking, till even occurs on socket.
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