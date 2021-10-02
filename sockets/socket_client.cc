#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "utils.h"

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0 ) retError("failed to create socket");
    std::cout<<"created socket : "<<sock;

    struct sockaddr_in client;
    client.sin_family = AF_INET;
    inet_aton("127.0.0.1", &client.sin_addr);
    client.sin_port = htons(3000);

    uint8_t dscp = 46 << 2;
    if(setsockopt(sock, IPPROTO_IP, IP_TOS, &dscp, sizeof(dscp)) < 0) retError("failed to setsockopt"); 
    std::cout<<"set dscp : "<<dscp<<"\n";

    if(connect(sock, (struct sockaddr*)&client, sizeof(client)) < 0) retError("failed to connect socket"); 
    std::cout<<"connected to 127.0.0.1:3000\n";
    
    std::string msg = "test";
    while(true) {
        auto sent = send(sock, msg.c_str(), msg.size(), MSG_NOSIGNAL);
        if(send(sock, msg.c_str(), msg.size(), MSG_NOSIGNAL) == -1) 
        {
            std::cout<<"Failed to send\n";
        } else { std::cout<<"sent bytes :"<<sent<<"\n"; }

        std::cout<<"sent message\n";
        sleep(5);
    }
}