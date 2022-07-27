#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <err.h>
#include <string.h>
#include <errno.h>
#include <iostream>

#include <event.h>
#include <event2/event.h>
#include <event2/bufferevent_ssl.h>
#include <event2/listener.h>

#include <nghttp2/nghttp2.h>

struct app_context;
typedef struct app_context app_context;

typedef struct http2_stream_data {
  struct http2_stream_data *prev, *next;
  char *request_path;
  int32_t stream_id;
  int fd;
} http2_stream_data;

typedef struct http2_session_data {
  struct http2_stream_data root;
  struct bufferevent *bev;
  app_context *app_ctx;
  nghttp2_session *session;
  char *client_addr;
} http2_session_data;

struct app_context {
  struct event_base *evbase;
};

#define MAKE_NV(NAME, VALUE)                                                   \
  {                                                                            \
    (uint8_t *)NAME, (uint8_t *)VALUE, sizeof(NAME) - 1, sizeof(VALUE) - 1,    \
        NGHTTP2_NV_FLAG_NONE                                                   \
  }

#define ARRLEN(x) (sizeof(x) / sizeof(x[0]))


static const char ERROR_HTML[] = "<html><head><title>404</title></head>"
                                 "<body><h1>404 Not Found</h1></body></html>";

#define OUTPUT_WOULDBLOCK_THRESHOLD (1 << 16)