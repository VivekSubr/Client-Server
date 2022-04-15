#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
// https://beej.us/guide/bgnet/html

int main()
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags                = 0;
    hints.ai_family               = AF_INET;
    hints.ai_socktype             = SOCK_STREAM;

    if ((rv = getaddrinfo("google.com", "http", &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        char host[NI_MAXHOST];
        sockaddr_in *addr = (sockaddr_in *)p;
        getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
        std::cout << "resolved ip: " << host << "\n";
    } 

    return 0;
}