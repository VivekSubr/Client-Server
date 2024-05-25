#pragma once 
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <err.h>
#include <signal.h>
#include <string.h>
#include <string>

#define NGHTTP2_NO_SSIZE_T
#include <nghttp2/nghttp2.h>


//Based on https://github.com/nghttp2/nghttp2/blob/master/examples/client.c

#define MAKE_NV(NAME, VALUE)                                                   \
  {                                                                            \
    (uint8_t *)NAME, (uint8_t *)VALUE, sizeof(NAME) - 1, sizeof(VALUE) - 1,    \
        NGHTTP2_NV_FLAG_NONE                                                   \
  }

#define MAKE_NV_CS(NAME, VALUE)                                                \
  {                                                                            \
    (uint8_t *)NAME, (uint8_t *)VALUE, sizeof(NAME) - 1, strlen(VALUE),        \
        NGHTTP2_NV_FLAG_NONE                                                   \
  }

/*
 * Prints error containing the function name |func| and error code
 * |error_code| and exit.
 */
static void diec(const char *func, int error_code) 
{
  fprintf(stderr, "FATAL: %s: error_code=%d, msg=%s\n", func, error_code, nghttp2_strerror(error_code));
  exit(EXIT_FAILURE);
}

struct Connection
{
  Connection(int sock, nghttp2_session* ngs): fd(sock), session(ngs) 
  {
  }

  int              fd;
  nghttp2_session *session;
};

struct Request 
{
  Request(const std::string& h, uint16_t p): host(h), port(p), hostport(host + ":"  + std::to_string(port)), stream_id(-1)
  {
  }

  std::string host;
  std::string hostport;   // This is the concatenation of host and port with ":" in between
  int32_t     stream_id;  // Stream ID for this request. 
  uint16_t    port;
};


/*
 * The implementation of nghttp2_send_callback2 type. Here we write
 * |data| with size |length| to the network and return the number of
 * bytes actually written. See the documentation of
 * nghttp2_send_callback for the details.
 */
static nghttp2_ssize send_callback(nghttp2_session *session,
                                   const uint8_t *data, size_t length,
                                   int flags, void *user_data)
{
  std::cout<<"Hit send_callback, data length "<<length<<" ";
   
  struct Connection *connection = (struct Connection *)user_data;
  int rv = write(connection->fd, data, (int)length);
  if(rv <= 0)
  {
    if(nghttp2_session_want_read(session) || nghttp2_session_want_write(session)) {
      std::cout<<" EAGAIN, want read/write\n";
      return NGHTTP2_ERR_WOULDBLOCK;
    }
    else {
      std::cout<<" ERR_CALLBACK_FAILURE\n";
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    
    std::cout<<"send_callback, failed to write "<<strerror(errno)<<"\n";
    return -1;
  }
  
  std::cout<<"written bytes "<<rv<<"\n";
  return rv;
}

/*
 * The implementation of nghttp2_recv_callback2 type. Here we read
 * data from the network and write them in |buf|. The capacity of
 * |buf| is |length| bytes. Returns the number of bytes stored in
 * |buf|. See the documentation of nghttp2_recv_callback for the
 * details.
 */
static nghttp2_ssize recv_callback(nghttp2_session *session, uint8_t *buf, size_t length, int flags, void *user_data) 
{
  struct Connection *connection = (struct Connection *)user_data;
  
  std::cout<<"Hit recv_callback, fd "<<connection->fd;
  int rv = read(connection->fd, buf, length);
  if(rv < 0)
  {
    if(errno == EAGAIN) 
    {
      if(nghttp2_session_want_read(session) || nghttp2_session_want_write(session)) {
        std::cout<<" EAGAIN, want read/write\n";
        return NGHTTP2_ERR_WOULDBLOCK;
      }
      else {
        std::cout<<" ERR_CALLBACK_FAILURE\n";
        return NGHTTP2_ERR_CALLBACK_FAILURE;
      }
    }
    else 
    {
      std::cout<<" recv_callback, failed to read, error "<<errno<<" ,"<<strerror(errno)<<"\n";
      return -1;
    }
  }
  else if(rv == 0)
  {
    std::cout<<" NGHTTP2_ERR_EOF\n";
    return NGHTTP2_ERR_EOF;
  }

  std::cout<<" read bytes: "<<rv<<"\n";
  return rv;
}

static int on_frame_send_callback(nghttp2_session *session, const nghttp2_frame *frame, void *user_data)                               
{
  std::cout<<"Hit on_frame_send_callback\n";
  
  switch (frame->hd.type) 
  {
    case NGHTTP2_DATA:
      printf("[INFO] C ----------------------------> S (DATA)\n");
      break;

    case NGHTTP2_HEADERS:
      if(nghttp2_session_get_stream_user_data(session, frame->hd.stream_id)) 
      {
        const nghttp2_nv *nva = frame->headers.nva;
        printf("[INFO] C ----------------------------> S (HEADERS)\n");
        for(int i = 0; i < frame->headers.nvlen; ++i) 
        {
          fwrite(nva[i].name, 1, nva[i].namelen, stdout);
          printf(": ");
          fwrite(nva[i].value, 1, nva[i].valuelen, stdout);
          printf("\n");
        }
      } break;

    case NGHTTP2_RST_STREAM:
      printf("[INFO] C ----------------------------> S (RST_STREAM)\n");
      break;
  
    case NGHTTP2_GOAWAY:
      printf("[INFO] C ----------------------------> S (GOAWAY)\n");
      break;

    case NGHTTP2_PRIORITY:
      printf("[INFO] C ----------------------------> S (PRIORITY)\n");
      break;

    case NGHTTP2_SETTINGS:
      printf("[INFO] C ----------------------------> S (SETTINGS)\n");
      break;

    case NGHTTP2_PUSH_PROMISE:
      printf("[INFO] C ----------------------------> S (PUSH PROMISE)\n");
      break;

    case NGHTTP2_WINDOW_UPDATE:
      printf("[INFO] C ----------------------------> S (WINDOW UPDATE)\n");
      break;
  }

  return 0;
}

static int on_frame_recv_callback(nghttp2_session *session, const nghttp2_frame *frame, void *user_data)
{
  std::cout<<"Hit on_frame_recv_callback\n";

  switch(frame->hd.type) 
  {
    case NGHTTP2_DATA:
      printf("[INFO] C <---------------------------- S (DATA)\n");
      break;

    case NGHTTP2_HEADERS:
      if(frame->headers.cat == NGHTTP2_HCAT_RESPONSE) 
      {
        const nghttp2_nv *nva = frame->headers.nva;
        struct Request *req = (struct Request*)nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
        if(req) 
        {
          printf("[INFO] C <---------------------------- S (HEADERS)\n");
          for(int i = 0; i < frame->headers.nvlen; ++i) 
          {
            fwrite(nva[i].name, 1, nva[i].namelen, stdout);
            printf(": ");
            fwrite(nva[i].value, 1, nva[i].valuelen, stdout);
            printf("\n");
          }
        }
      } break;
  
    case NGHTTP2_RST_STREAM:
      printf("[INFO] C <---------------------------- S (RST_STREAM)\n");
      break;
  
    case NGHTTP2_GOAWAY:
      printf("[INFO] C <---------------------------- S (GOAWAY)\n");
      break;

    case NGHTTP2_PRIORITY:
      printf("[INFO] C <---------------------------- S (PRIORITY)\n");
      break;

    case NGHTTP2_SETTINGS:
      printf("[INFO] C <---------------------------- S (SETTINGS)\n");
      break;

    case NGHTTP2_PUSH_PROMISE:
      printf("[INFO] C <---------------------------- S (PUSH PROMISE)\n");
      break;

    case NGHTTP2_WINDOW_UPDATE:
      printf("[INFO] C <---------------------------- S (WINDOW UPDATE)\n");
      break;
  }
  
  return 0;
}

/*
 * The implementation of nghttp2_on_stream_close_callback type. We use
 * this function to know the response is fully received. Since we just
 * fetch 1 resource in this program, after reception of the response,
 * we submit GOAWAY and close the session.
 */
static int on_stream_close_callback(nghttp2_session *session, int32_t stream_id,
                                    uint32_t error_code, void *user_data)
{
  std::cout<<"Hit on_stream_close_callback\n";

  struct Request *req = (struct Request*)nghttp2_session_get_stream_user_data(session, stream_id);
  if(req) 
  {
    int rv = nghttp2_session_terminate_session(session, NGHTTP2_NO_ERROR);

    if (rv != 0) {
      diec("nghttp2_session_terminate_session", rv);
    }
  }

  return 0;
}

/*
 * The implementation of nghttp2_on_data_chunk_recv_callback type. We
 * use this function to print the received response body.
 */
static int on_data_chunk_recv_callback(nghttp2_session *session, uint8_t flags,
                                       int32_t stream_id, const uint8_t *data,
                                       size_t len, void *user_data) 
{
  std::cout<<"Hit on_data_chunk_recv_callback\n";

  struct Request *req = (struct Request*)nghttp2_session_get_stream_user_data(session, stream_id);
  if(req) 
  {
    printf("[INFO] C <---------------------------- S (DATA chunk)\n"
           "%lu bytes\n",
           (unsigned long int)len);
    fwrite(data, 1, len, stdout);
    printf("\n");
  }

  return 0;
}

/*
 * Setup callback functions. nghttp2 API offers many callback
 * functions, but most of them are optional. The send_callback is
 * always required. Since we use nghttp2_session_recv(), the
 * recv_callback is also required.
 */
static void setup_nghttp2_callbacks(nghttp2_session_callbacks *callbacks) 
{
  nghttp2_session_callbacks_set_send_callback2(callbacks, send_callback);

  nghttp2_session_callbacks_set_recv_callback2(callbacks, recv_callback);

  nghttp2_session_callbacks_set_on_frame_send_callback(callbacks,
                                                       on_frame_send_callback);

  nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks,
                                                       on_frame_recv_callback);

  nghttp2_session_callbacks_set_on_stream_close_callback(
      callbacks, on_stream_close_callback);

  nghttp2_session_callbacks_set_on_data_chunk_recv_callback(
      callbacks, on_data_chunk_recv_callback);
}

/*
 * Update |pollfd| based on the state of |session|.
 */
static void ctl_poll(struct pollfd *pollfd, nghttp2_session *session) 
{
  pollfd->events = 0;
  if(nghttp2_session_want_read(session))   pollfd->events |= POLLIN;
  if(nghttp2_session_want_write(session))  pollfd->events |= POLLOUT;
}

/*
 * function does not send packets; just append the request to the internal queue in |session|.
*/
static void submit_request(nghttp2_session *session, struct Request *req) 
{
  std::cout<<"submit_request\n";
  /* Make sure that the last item is NULL */
  const nghttp2_nv nva[] = {MAKE_NV(":method", "GET"),
                            MAKE_NV_CS(":path", "/test"),
                            MAKE_NV(":scheme", "https"),
                            MAKE_NV_CS(":authority", req->hostport.c_str()),
                            MAKE_NV("accept", "*/*"),
                            MAKE_NV("user-agent", "nghttp2/" NGHTTP2_VERSION)};

  int32_t stream_id = nghttp2_submit_request2(session, NULL, nva,
                                      sizeof(nva) / sizeof(nva[0]), NULL, req);

  if (stream_id < 0) {
    diec("nghttp2_submit_request", stream_id);
  }

  req->stream_id = stream_id;
  printf("[INFO] Stream ID = %d\n", stream_id);
}


/*
 * Performs the network I/O.
 */
static void exec_io(nghttp2_session *session) 
{
  int rv;
  
  rv = nghttp2_session_recv(session);
  if (rv != 0) {
    diec("nghttp2_session_recv", rv);
  }
  
  rv = nghttp2_session_send(session);
  if (rv != 0) {
    diec("nghttp2_session_send", rv);
  }
}