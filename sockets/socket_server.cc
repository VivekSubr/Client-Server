#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"

int main(int argc,  char **argv)
{
    auto host = getHostFromArg(argc, argv, 1, 2);
    if(host == nullptr) retError("need to specify ip and port");

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0 ) retError("failed to create socket");
    std::cout<<"created socket : "<<sock<<"\n";

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(host->port);

    if(bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0) retError("failed to bind socket");
    std::cout<<"bound socket\n";
    
    if(listen(sock, 3) == -1) retError("failed to listen");
    std::cout<<"listening...\n";

    int client_sock, read_sz, c{sizeof(struct sockaddr_in)};
    struct sockaddr_in client;
    char buf[256];
    std::string reply = "received";
    while(true) {
        int client_sock = accept(sock, (struct sockaddr *)&client, (socklen_t*)&c);
        if(client_sock < 0 ) retError("failed to create client socket");
        std::cout<<"accepted socket : "<<client_sock<<"\n";

        while((read_sz = read(client_sock , buf , sizeof(buf))) > 0 )
        {
           write(client_sock, reply.c_str(), reply.size());
           std::cout<<"got message :"<<buf<<"\n";
           memset(&buf, 0, sizeof(buf));
        }
    }

    return 0;
}