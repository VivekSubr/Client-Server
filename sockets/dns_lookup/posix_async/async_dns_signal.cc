#include "utils.h"
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
//Design Doc: https://akkadia.org/drepper/asynchnl.pdf
//signalfd:   https://unixism.net/2021/02/making-signals-less-painful-under-linux/
//            https://github.com/kazuho/examples/blob/master/getaddrinfo_a%2Bsignalfd.c

//max number of events that can be queued up in epoll
#define MAX_EVENTS 32

int signalfd_setup(int signal, int& sfd) 
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, signal);
    sigprocmask(SIG_BLOCK, &mask, NULL); //we block the signal
    
    sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    std::cout<<"Setup signalfd "<<sfd<<"\n";

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
    std::string host = getHostFromArg(1, argc, argv);
    std::cout<<"Resolving "<<host<<"\n";
    
    struct addrinfo pollHints, servinfo;
    struct sigevent sev;
    struct gaicb**  dnsReqs;
    int    signal = SIGRTMIN + 3;
    
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
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo  = signal;
    sev.sigev_value.sival_ptr = dnsReqs[0];

    int sfd = -1;
    int efd = signalfd_setup(signal, sfd);
    if(efd == -1 || sfd == -1) return -1;

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
 
    struct epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(efd, events, MAX_EVENTS, -1); //blocking, till even occurs on socket.
    std::cout<<"Got nfds "<<nfds<<"\n";
    for(int i = 0; i < nfds; i++) 
    {
        std::cout<<"Reading file descriptor "<<events[i].data.fd<<"\n";
        if(events[i].data.fd == sfd)
        {
            struct signalfd_siginfo sfd_si;
            if (read(sfd, &sfd_si, sizeof(sfd_si)) > 0)
            {
               if(sfd_si.ssi_signo == signal)
               {
                  std::cout<<"Got signal from getaddrinfo_a\n";
                  for(auto p = dnsReqs[0]->ar_result; p != NULL; p = p->ai_next)
                  {
                    printAddrInfo(p);
                  }
               }
            }
            else perror("Error :");
        }
    }

    free(dnsReqs[0]);
    free(dnsReqs);
    return 0;
}