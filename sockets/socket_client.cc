#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"

int main(int argc,  char **argv)
{
    auto host = getHostFromArg(argc, argv, 1, 2);
    if(host == nullptr) retError("need to specify ip and port");

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0 ) retError("failed to create socket");
    std::cout<<"created socket : "<<sock<<"\n";

    struct sockaddr_in client;
    client.sin_family = AF_INET;
    inet_aton(host->ip.c_str(), &client.sin_addr);
    client.sin_port = htons(host->port);

    uint8_t dscp = 46 << 2;
    if(setsockopt(sock, IPPROTO_IP, IP_TOS, &dscp, sizeof(dscp)) < 0) retError("failed to setsockopt"); 
    std::cout<<"set dscp : "<<dscp<<"\n";

    if(connect(sock, (struct sockaddr*)&client, sizeof(client)) < 0) retError("failed to connect socket"); 
    
    std::string msg = "test";
    while(true) {
        auto sent = send(sock, msg.c_str(), msg.size(), MSG_NOSIGNAL);
        if(sent == -1) 
        {
            std::cout<<"Failed to send, errno: "<<errno<<" "<<strerror(errno)<<"\n";
        } else { std::cout<<"sent bytes :"<<sent<<"\n"; }

        std::cout<<"sent message\n";
        sleep(5);
    }
}