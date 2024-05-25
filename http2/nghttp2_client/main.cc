#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <string>
#include <iostream>
#include <memory>
#include "nghttp2.h"

int retError(const std::string sError)
{
    std::cout<<sError<<"\n";
    return -1;
}

int main(int argc, char *argv[])
{
    if (argc < 3) return retError("required options [ip] [port]");

    std::cout<<"Starting client for "<<argv[1]<<":"<<argv[2]<<"\n";

    std::string sIP(argv[1]);
    uint16_t    iPort = std::atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0 ) return retError("failed to create socket");

    struct sockaddr_in client;
    client.sin_family = AF_INET;
    inet_aton(sIP.c_str(), &client.sin_addr);
    client.sin_port = htons(iPort);

    uint8_t dscp = 46 << 2;
    if(setsockopt(sock, IPPROTO_IP, IP_TOS, &dscp, sizeof(dscp)) < 0) {
        return retError((std::string)"failed to setsockopt dscp" + strerror(errno)); 
    }
    std::cout<<"set dscp : "<<dscp<<"\n";

    long on = 1L;
    if(ioctl(sock, (int)FIONBIO, (char *)&on)) return retError("ioctl FIONBIO call failed");
    std::cout<<"socket made non-blocking\n";

    int nodelay_val = 1;
    if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &nodelay_val, (socklen_t)sizeof(nodelay_val)) < 0) {
        return retError((std::string)"failed to setsockopt tcp nodelay" + strerror(errno)); 
    }

    if((connect(sock, (struct sockaddr*)&client, sizeof(client)) < 0) && errno != EINPROGRESS) {
        return retError((std::string)"failed to connect socket error " + std::to_string(errno) + ", " + strerror(errno)); 
    }
    std::cout<<"connected to "<<sIP<<":"<<iPort<<" on fd "<<sock<<"\n";

    /*** setup NG_HTTP2 ***/
    nghttp2_session_callbacks *callbacks;
    if(nghttp2_session_callbacks_new(&callbacks) != 0) return retError("nghttp2_session_callbacks_new failed"); 

    setup_nghttp2_callbacks(callbacks);
    
    nghttp2_session *session;
    struct Connection con(sock, session);

    //This just initializes session
    if(nghttp2_session_client_new(&session, callbacks, &con) != 0) return retError("nghttp2_session_client_new failed");

    nghttp2_session_callbacks_del(callbacks);

    nghttp2_settings_entry settings_nv[] = {
        {NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 1},
        {NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE,    10000},
    };

    if(nghttp2_submit_settings(session, NGHTTP2_FLAG_NONE, settings_nv, sizeof(settings_nv) / sizeof(settings_nv[0])) != 0) 
    {
        return retError("nghttp2_submit_settings failed");
    }
    
    /**********************/
 
    struct Request  req(sIP, iPort);
    nfds_t          npollfds = 1;
    struct pollfd   pollfds[1];

    submit_request(session, &req);

    pollfds[0].fd = sock;
    ctl_poll(pollfds, session);

    /* Event loop */
    while(nghttp2_session_want_read(session) || nghttp2_session_want_write(session)) 
    {
        int nfds = poll(pollfds, npollfds, -1);
        if(nfds == -1) return retError((std::string)"poll" + strerror(errno));

        if(pollfds[0].revents & (POLLIN | POLLOUT)) {
            exec_io(session);
        }

        if((pollfds[0].revents & POLLHUP) || (pollfds[0].revents & POLLERR)) {
            return retError("Connection error");
        }
    
        ctl_poll(pollfds, session);

        //sleep(1);
        submit_request(session, &req);
    }

    std::cout<<"Exiting...\n";

    /* Resource cleanup */
    nghttp2_session_del(session);
    shutdown(sock, SHUT_WR);
    close(sock);
}