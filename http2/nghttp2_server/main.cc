#include "nghttp2.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>

//max number of concurrent connections on server listening socket
#define MAX_CONN       16
//max number of events that can be queued up in epoll
#define MAX_EVENTS     32
//size of buffer used in bytes for data transfer
#define BUF_SIZE       256

int retError(const std::string sError)
{
    std::cout<<sError<<"\n";
    return -1;
}

void printReadError(int error)
{
    std::unordered_map<int, std::string> read_errors = {
        {EAGAIN, "EAGAIN"}, {EWOULDBLOCK, "EWOULDBLOCK"}, {EBADF, "EBADF"}, {EFAULT, "EFAULT"},
        {EINTR , "EINTR"}, {EINVAL, "EINVAL"}, {EIO, "EIO"}
    };

    std::string sError;
    try { sError = read_errors.at(error); }
    catch(...)
    {
      sError = "UNKNOWN_ERROR";
    }

    std::cout<<"read error, "<<sError<<" "<<strerror(error)<<"\n";
}

int main(int argc, char *argv[])
{
    if (argc < 3) return retError("required options [ip] [port]");

    std::cout<<"Starting server for "<<argv[1]<<":"<<argv[2]<<"\n";

    /*******************************************************************/
    //preliminary stuff, create a tcp ipv4 socket, set to listen non-blocking on ip:port
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0); 

    struct sockaddr_in srv_addr;
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port   = htons(std::atoi(argv[2]));
    inet_aton(argv[1], &srv_addr.sin_addr);
    bind(listen_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

    if(fcntl(listen_sock, F_SETFD, fcntl(listen_sock, F_GETFD, 0) | O_NONBLOCK) ==-1) 
    { //got the existing options, and added O_NONBLOCK: https://man7.org/linux/man-pages/man2/open.2.html
        return retError("failed to set listening socket to be non blocking");
	}

    if(listen(listen_sock, MAX_CONN) < 0)
    {
        return retError("failed to listen\n");
    }
    /********************************************************************/

    /********************************************************************/
    //epoll_create returns an fd handle to an instance of an epoll structure in kernel space
    //We then use epoll_ctl to interact with this kernel object and ask it to look out for some
    //events in listen_sock - https://man7.org/linux/man-pages/man2/epoll_ctl.2.html
    int epfd = epoll_create(1);
  
    struct epoll_event ev;
    //EPOLLIN  - available for read
    //EPOLLOUT - available for write
    //EPOLLET  - event is set to be edge triggered, if not specified default is level trigger
	ev.events  = EPOLLIN | EPOLLOUT | EPOLLET;
	ev.data.fd = listen_sock;
	if(epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) return retError("failed initial epoll_ctl");

    /********************************************************************/

    int nfds, conn_sock, read_sz;
    char buf[BUF_SIZE];
    struct sockaddr_in client_addr;
    socklen_t socklen = sizeof(client_addr);
    struct epoll_event events[MAX_EVENTS];
    app_context         app_ctx;
    http2_session_data *session_data = nullptr;
    while(true)
    {
        bzero(buf, sizeof(buf));
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1); //blocking, till even occurs on socket.
        for(int i = 0; i < nfds; i++) 
        { 
            if(events[i].data.fd == listen_sock) 
            { //new connection on socket
                conn_sock = accept(listen_sock, (struct sockaddr*)&client_addr, &socklen);

                if(fcntl(conn_sock, F_SETFD, fcntl(conn_sock, F_GETFD, 0) | O_NONBLOCK) ==-1) { 
                    return retError("failed to set client socket to be non blocking");
	            }

                inet_ntop(AF_INET, (char*)&(client_addr.sin_addr), buf, sizeof(client_addr));
                std::cout<<"connected with "<<buf<<" "<<ntohs(client_addr.sin_port)<<"\n";
        
                //note that client socket also has EPOLLRDHUP, since we expect it to be closed... the listener socket is open forever
                struct epoll_event ev;
	            ev.events =  EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP;
	            ev.data.fd = epfd;
                if(epoll_ctl(epfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
		            return retError("failed epoll_ctl");
	            }

                session_data = (http2_session_data*)malloc(sizeof(http2_session_data));
                initialize_nghttp2_session(session_data);
            } 
            else if(events[i].events & EPOLLIN) 
            { 
                do 
                {
                    bzero(buf, sizeof(buf));
                    read_sz = read(conn_sock, buf, sizeof(buf));

                    if(read_sz < 0) {
                        printReadError(errno);
                        break;
                    }
                } while(read_sz > 0);

                std::cout<<"read "<<read_sz<<" bytes\n";
                int readlen = nghttp2_session_mem_recv2(session_data->session, (const uint8_t*)buf, read_sz);
                if(readlen < 0) {
                    return retError((std::string)"Fatal error: " + nghttp2_strerror(readlen));
                }

                int rv = nghttp2_session_send(session_data->session);
                if(rv != 0) {
                    return retError((std::string)"Fatal error: " + nghttp2_strerror(rv));
                }
            } 
            else if(events[i].events & EPOLLOUT) 
            {
                if(nghttp2_session_want_read(session_data->session) == 0 &&
                   nghttp2_session_want_write(session_data->session) == 0) 
                {
                    delete_http2_session_data(session_data);
                }
                else
                {
                    int rv = nghttp2_session_send(session_data->session);
                    if(rv != 0) {
                        return retError((std::string)"Fatal error: " + nghttp2_strerror(rv));
                    }
                }
            }
            else 
            { //unknown event, shouldn't happen
                std::cout<<"unexpected event\n";
            }

            if(events[i].events & (EPOLLRDHUP | EPOLLHUP)) 
            {
			    std::cout<<"connection closed\n";

                //EPOLL_CTL_DEL deletes from kernel epoll list... we close the socket ourselves.
			    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
			    close(events[i].data.fd);
                delete_http2_session_data(session_data);  
		    }
        }
    }
}