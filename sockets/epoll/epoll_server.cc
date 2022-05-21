#include <iostream>
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

/*
  let's implement a simple tcp server using epoll
*/

#define SERVER_IP      INADDR_ANY
#define SERVER_PORT    3000
//max number of concurrent connections on server listening socket
#define MAX_CONN       16
//max number of events that can be queued up in epoll
#define MAX_EVENTS     32
//size of buffer used in bytes for data transfer
#define BUF_SIZE       256

int main(int argc, char *argv[])
{
  /*******************************************************************/
  //preliminary stuff, create a tcp ipv4 socket, set to listen non-blocking on 0.0.0.0:port
  int listen_sock = socket(AF_INET, SOCK_STREAM, 0); 

  struct sockaddr_in srv_addr;
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = SERVER_IP;
	srv_addr.sin_port = htons(SERVER_PORT);
  bind(listen_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

  if(fcntl(listen_sock, F_SETFD, fcntl(listen_sock, F_GETFD, 0) | O_NONBLOCK) ==-1) 
  { //got the existing options, and added O_NONBLOCK: https://man7.org/linux/man-pages/man2/open.2.html
    std::cout<<"failed to set listening socket to be non blocking\n";
		return -1;
	}

  listen(listen_sock, MAX_CONN);
  /********************************************************************/

  /********************************************************************/
  //epoll_create returns an fd handle to an instance of an epoll structure in kernel space
  //We then use epoll_ctl to interact with this kernel object and ask it to look out for some
  //events in listen_sock - https://man7.org/linux/man-pages/man2/epoll_ctl.2.html
  int epfd = epoll_create(1);
  
  struct epoll_event ev;
  //EPOLLIN - available for read
  //EPOLLOUT - available for write
  //EPOLLET - event is set to be edge triggered, if not specified default is level trigger
	ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
	ev.data.fd = listen_sock;
	if(epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
		std::cout<<"failed initial epoll_ctl\n";
    return -1;
	}
  /********************************************************************/

  int nfds, conn_sock, read_sz;
  char buf[BUF_SIZE];
  struct sockaddr_in client_addr;
  socklen_t socklen = sizeof(client_addr);
  struct epoll_event events[MAX_EVENTS];
  while(true)
  {
    bzero(buf, sizeof(buf));
    nfds = epoll_wait(epfd, events, MAX_EVENTS, -1); //blocking, till even occurs on socket.
    for(int i = 0; i < nfds; i++) { 
      if(events[i].data.fd == listen_sock) { //new connection on socket
        conn_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &socklen);

        if(fcntl(conn_sock, F_SETFD, fcntl(conn_sock, F_GETFD, 0) | O_NONBLOCK) ==-1) { 
          std::cout<<"failed to set client socket to be non blocking\n";
		      return -1;
	      }

        inet_ntop(AF_INET, (char *)&(client_addr.sin_addr), buf, sizeof(client_addr));
        std::cout<<"connected with "<<buf<<" "<<ntohs(client_addr.sin_port)<<"\n";
        
        //note that client socket also has EPOLLRDHUP, since we expect it to be closed... the listener socket is open forever
        struct epoll_event ev;
	      ev.events =  EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP;
	      ev.data.fd = epfd;
        if(epoll_ctl(epfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
		        std::cout<<"failed epoll_ctl\n";
            return -1;
	      }

      } else if(events[i].events & EPOLLIN) { 
        while(true) 
        {
          bzero(buf, sizeof(buf));
          read_sz = read(events[i].data.fd, buf, sizeof(buf));
          
          if(read_sz == 0) break;
          if(read_sz < 0) {
            std::cout<<"failed to read\n";
            break;
          }
          else {
            std::cout<<"data "<<buf<<"\n";
						write(events[i].data.fd, buf, read_sz);
          }
        }
      } else { //unknown event, shouldn't happen
        std::cout<<"unexpected event\n";
      }

      if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
				std::cout<<"connection closed\n";
				epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
				close(events[i].data.fd);
        //EPOLL_CTL_DEL deletes from kernel epoll list... we close the socket ourselves.
			}
    }
  }
  
  return 0;
}