#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
// https://beej.us/guide/bgnet/html

void printAddrInfo(struct addrinfo* info)
{
    std::cout<<"***** struct addrinfo *******\n"
             <<"ai_family "<<info->ai_family<<"\n"
             <<"ai_socktype "<<info->ai_socktype<<"\n"
             <<"ai_protocol "<<info->ai_protocol<<"\n"
             <<"ai_flags "<<info->ai_flags<<"\n"
             <<"ai_addrlen "<<info->ai_addrlen<<"\n"
             <<"ai_canonname "<<(info->ai_canonname == 0?"0" : info->ai_canonname)<<"\n";
    if(info->ai_addr->sa_family == AF_INET) { 
        sockaddr_in* addr = (sockaddr_in*)info->ai_addr;
        std::cout<<"ip address "<<inet_ntoa(addr->sin_addr)<<"\n";
    } else std::cout<<"not AF_INET!\n";
    std::cout<<"*****************************\n";

   if(info->ai_next) 
   {
       std::cout<<"*****************************\n";
       printAddrInfo(info->ai_next);
   }
}

struct addrinfo* ip2AddrInfo(const std::string& ip)
{
    struct addrinfo* addr = (struct addrinfo*)malloc(sizeof(struct addrinfo));
    addr->ai_family = AF_INET;
    addr->ai_socktype = SOCK_STREAM;
    addr->ai_protocol = 6;
    addr->ai_flags = 0;
    addr->ai_canonname = 0;
    addr->ai_addrlen = ip.length();

    addr->ai_addr = (struct sockaddr*)malloc(sizeof(struct sockaddr));
    struct sockaddr_in* addr_in = (struct sockaddr_in*)(addr->ai_addr);
    inet_aton(ip.c_str(), &addr_in->sin_addr);
    std::cout<<"ip address "<<inet_ntoa(addr_in->sin_addr)<<"\n";

    addr->ai_next = nullptr;
    return addr;
}

int main()
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags    = 0;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo("google.com", "http", &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

//    printAddrInfo(servinfo);
//    std::cout<<"\n\n";
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        char host[NI_MAXHOST];
        getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
        std::cout << "resolved ip: " << host << "\n";
    } 

    freeaddrinfo(servinfo);
    return 0;
}
