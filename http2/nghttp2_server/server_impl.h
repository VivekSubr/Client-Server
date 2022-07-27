#pragma once
#include "server_types.h"
#include "utils.h"

static void initialize_nghttp2_session(http2_session_data *session_data);

static void add_stream(http2_session_data *session_data, http2_stream_data *stream_data) 
{
  stream_data->next = session_data->root.next;
  session_data->root.next = stream_data;
  stream_data->prev = &session_data->root;
  if (stream_data->next) {
    stream_data->next->prev = stream_data;
  }
}

static void remove_stream(http2_session_data *session_data,
                          http2_stream_data *stream_data) 
{
  (void)session_data;

  stream_data->prev->next = stream_data->next;
  if (stream_data->next) {
    stream_data->next->prev = stream_data->prev;
  }
}

static http2_stream_data *create_http2_stream_data(http2_session_data *session_data, int32_t stream_id) 
{
  http2_stream_data *stream_data;
  stream_data = (http2_stream_data*)malloc(sizeof(http2_stream_data));
  memset(stream_data, 0, sizeof(http2_stream_data));
  stream_data->stream_id = stream_id;
  stream_data->fd = -1;

  add_stream(session_data, stream_data);
  return stream_data;
}

static void delete_http2_stream_data(http2_stream_data *stream_data) {
  if (stream_data->fd != -1) {
    close(stream_data->fd);
  }
  free(stream_data->request_path);
  free(stream_data);
}

static void delete_http2_session_data(http2_session_data *session_data) {
  http2_stream_data *stream_data;
  fprintf(stderr, "%s disconnected\n", session_data->client_addr);
  bufferevent_free(session_data->bev);
  nghttp2_session_del(session_data->session);
  for (stream_data = session_data->root.next; stream_data;) {
    http2_stream_data *next = stream_data->next;
    delete_http2_stream_data(stream_data);
    stream_data = next;
  }
  free(session_data->client_addr);
  free(session_data);
}

/* Serialize the frame and send (or buffer) the data to
   bufferevent. */
static int session_send(http2_session_data *session_data) {
  int rv;
  ////
 // rv = nghttp2_submit_rst_stream(session_data->session, NGHTTP2_FLAG_NONE, 
                     //            session_data->session->hd.stream_id, NGHTTP2_PROTOCOL_ERROR);
  ////
  rv = nghttp2_session_send(session_data->session);
  if (rv != 0) {
    warnx("Fatal error: %s", nghttp2_strerror(rv));
    return -1;
  }
  return 0;
}

/* Read the data in the bufferevent and feed them into nghttp2 library
   function. Invocation of nghttp2_session_mem_recv() may make
   additional pending frames, so call session_send() at the end of the
   function. */
static int session_recv(http2_session_data *session_data) 
{
  std::cout<<"Hit session_recv\n";
  ssize_t readlen;
  struct evbuffer *input = bufferevent_get_input(session_data->bev);
  size_t datalen = evbuffer_get_length(input);
  unsigned char *data = evbuffer_pullup(input, -1);
  
  if(session_data->session == NULL)
  {
    errx(1, "session_recv session NULL!");
    return -1; 
  }

  readlen = nghttp2_session_mem_recv(session_data->session, data, datalen);
  if (readlen < 0) {
    warnx("Fatal error: %s", nghttp2_strerror((int)readlen));
    return -1;
  }
  if (evbuffer_drain(input, (size_t)readlen) != 0) {
    warnx("Fatal error: evbuffer_drain failed");
    return -1;
  }
  if (session_send(session_data) != 0) {
    return -1;
  }
  return 0;
}

static http2_session_data *create_http2_session_data(app_context *app_ctx,
                                                     int fd,
                                                     struct sockaddr *addr,
                                                     int addrlen) 
{
  int rv;
  http2_session_data *session_data;
  char host[NI_MAXHOST];
  int val = 1;

  session_data = (http2_session_data*)malloc(sizeof(http2_session_data));
  memset(session_data, 0, sizeof(http2_session_data));
  session_data->app_ctx = app_ctx;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&val, sizeof(val));
  session_data->bev = bufferevent_socket_new(app_ctx->evbase, fd, 
                                             BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
  bufferevent_enable(session_data->bev, EV_READ | EV_WRITE);
  rv = getnameinfo(addr, (socklen_t)addrlen, host, sizeof(host), NULL, 0,
                   NI_NUMERICHOST);
  
  if (rv != 0) session_data->client_addr = strdup("(unknown)");
  else         session_data->client_addr = strdup(host);

  return session_data;
}

/* Send HTTP/2 client connection header, which includes 24 bytes
   magic octets and SETTINGS frame */
static int send_server_connection_header(http2_session_data *session_data) 
{
  nghttp2_settings_entry iv[1] = {
      {NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 100}};
  int rv;

  rv = nghttp2_submit_settings(session_data->session, NGHTTP2_FLAG_NONE, iv,
                               ARRLEN(iv));
  if (rv != 0) {
    warnx("Fatal error: %s", nghttp2_strerror(rv));
    return -1;
  }
  return 0;
}

/* readcb for bufferevent after client connection header was checked. */
static void readcb(struct bufferevent *bev, void *ptr) 
{
  std::cout<<"Hit readcb\n";
  http2_session_data *session_data = (http2_session_data *)ptr;
  (void)bev;

  if (session_recv(session_data) != 0) {
    delete_http2_session_data(session_data);
    return;
  }
}

/* writecb for bufferevent. To greaceful shutdown after sending or
   receiving GOAWAY, we check the some conditions on the nghttp2
   library and output buffer of bufferevent. If it indicates we have
   no business to this session, tear down the connection. If the
   connection is not going to shutdown, we call session_send() to
   process pending data in the output buffer. This is necessary
   because we have a threshold on the buffer size to avoid too much
   buffering. See send_callback(). */
static void writecb(struct bufferevent *bev, void *ptr) 
{
  std::cout<<"Hit writecb\n";
  http2_session_data *session_data = (http2_session_data *)ptr;

  if(session_data->session == NULL)
  {
    errx(1, "writecb - session null!\n");
    return;
  }

  if (evbuffer_get_length(bufferevent_get_output(bev)) > 0) {
    return;
  }
  if (nghttp2_session_want_read(session_data->session) == 0 &&
      nghttp2_session_want_write(session_data->session) == 0) 
  {
    std::cout<<"writecb - don't want read nor write... delete session\n";
    delete_http2_session_data(session_data);
    return;
  }
  if (session_send(session_data) != 0) 
  {
    std::cout<<"writecb - session_send failed\n";
    delete_http2_session_data(session_data);
    return;
  }
}

/* eventcb for bufferevent */
static void eventcb(struct bufferevent *bev, short events, void *ptr) 
{
  http2_session_data *session_data = (http2_session_data *)ptr;

  if (events & BEV_EVENT_CONNECTED) {
    std::cout<<"eventcb : event connected \n";
    const unsigned char *alpn = NULL;
    unsigned int alpnlen = 0;
    (void)bev;

    fprintf(stderr, "%s connected\n", session_data->client_addr);

    if (alpn == NULL || alpnlen != 2 || memcmp("h2", alpn, 2) != 0) {
      fprintf(stderr, "%s h2 is not negotiated\n", session_data->client_addr);
      delete_http2_session_data(session_data);
      return;
    }

    initialize_nghttp2_session(session_data);

    if (send_server_connection_header(session_data) != 0 || session_send(session_data) != 0) 
    {
      delete_http2_session_data(session_data);
      return;
    }

    return;
  }
  if (events & BEV_EVENT_EOF) {
    fprintf(stderr, "%s EOF\n", session_data->client_addr);
  } else if (events & BEV_EVENT_ERROR) {
    fprintf(stderr, "%s network error\n", session_data->client_addr);
  } else if (events & BEV_EVENT_TIMEOUT) {
    fprintf(stderr, "%s timeout\n", session_data->client_addr);
  }

  delete_http2_session_data(session_data);
}

/* callback for evconnlistener.. this is basically the entry point to server */
static void acceptcb(struct evconnlistener *listener, int fd,
                     struct sockaddr *addr, int addrlen, void *arg) 
{
  std::cout<<"Hit acceptcb\n";
  app_context *app_ctx = (app_context *)arg;
  http2_session_data *session_data;
  (void)listener;

  session_data = create_http2_session_data(app_ctx, fd, addr, addrlen);
  initialize_nghttp2_session(session_data);

  //This is a libevent api: http://www.wangafu.net/~nickm/libevent-book/Ref6_bufferevent.html
  bufferevent_setcb(session_data->bev, readcb, writecb, eventcb, session_data);
}

/******** nghttp2 callbacks ************/

/* nghttp2_on_header_callback: Called when nghttp2 library emits
   single header name/value pair. */
static int on_header_callback(nghttp2_session *session,
                              const nghttp2_frame *frame, const uint8_t *name,
                              size_t namelen, const uint8_t *value,
                              size_t valuelen, uint8_t flags, void *user_data) 
{
  std::cout<<"on_header_callback\n";
  http2_stream_data *stream_data;
  const char PATH[] = ":path";
  (void)flags;
  (void)user_data;

  switch (frame->hd.type) {
  case NGHTTP2_HEADERS:
    if (frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
      break;
    }
    stream_data =
        (http2_stream_data*)nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
    if (!stream_data || stream_data->request_path) {
      break;
    }
    if (namelen == sizeof(PATH) - 1 && memcmp(PATH, name, namelen) == 0) {
      size_t j;
      for (j = 0; j < valuelen && value[j] != '?'; ++j)
        ;
      stream_data->request_path = percent_decode(value, j);
    }
    break;
  }
  return 0;
}

static int on_begin_headers_callback(nghttp2_session *session,
                                     const nghttp2_frame *frame,
                                     void *user_data) 
{
  std::cout<<"on_begin_headers_callback\n";
  http2_session_data *session_data = (http2_session_data *)user_data;
  http2_stream_data *stream_data;

  if (frame->hd.type != NGHTTP2_HEADERS ||
      frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
    return 0;
  }
  stream_data = create_http2_stream_data(session_data, frame->hd.stream_id);
  nghttp2_session_set_stream_user_data(session, frame->hd.stream_id,
                                       stream_data);
  return 0;
}

static int on_request_recv(nghttp2_session *session,
                           http2_session_data *session_data,
                           http2_stream_data *stream_data) 
{
  std::cout<<"on_request_recv\n";

  int fd;
  nghttp2_nv hdrs[] = {MAKE_NV(":status", "200")};
  char *rel_path;

  if (!stream_data->request_path) {
    if (error_reply(session, stream_data) != 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    return 0;
  }
  fprintf(stderr, "%s GET %s\n", session_data->client_addr,
          stream_data->request_path);
  if (!check_path(stream_data->request_path)) {
    if (error_reply(session, stream_data) != 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    return 0;
  }
  for (rel_path = stream_data->request_path; *rel_path == '/'; ++rel_path)
    ;
  fd = open(rel_path, O_RDONLY);
  if (fd == -1) {
    if (error_reply(session, stream_data) != 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    return 0;
  }
  stream_data->fd = fd;

  if (send_response(session, stream_data->stream_id, hdrs, ARRLEN(hdrs), fd) !=
      0) {
    close(fd);
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  }
  return 0;
}

static int on_frame_recv_callback(nghttp2_session *session,
                                  const nghttp2_frame *frame, void *user_data) 
{
  std::cout<<"on_frame_recv_callback\n";
  http2_session_data *session_data = (http2_session_data *)user_data;
  http2_stream_data *stream_data;
  switch (frame->hd.type) {
  case NGHTTP2_DATA:
  case NGHTTP2_HEADERS:
    /* Check that the client request has finished */
    if (frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
      stream_data =
          (http2_stream_data*)nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
      /* For DATA and HEADERS frame, this callback may be called after
         on_stream_close_callback. Check that stream still alive. */
      if (!stream_data) {
        return 0;
      }
      return on_request_recv(session, session_data, stream_data);
    }
    break;
  default:
    break;
  }
  return 0;
}

static int on_stream_close_callback(nghttp2_session *session, int32_t stream_id,
                                    uint32_t error_code, void *user_data) 
{
  std::cout<<"on_stream_close_callback\n";
  http2_session_data *session_data = (http2_session_data *)user_data;
  http2_stream_data *stream_data;
  (void)error_code;

  stream_data = (http2_stream_data*)nghttp2_session_get_stream_user_data(session, stream_id);
  if (!stream_data) {
    return 0;
  }
  remove_stream(session_data, stream_data);
  delete_http2_stream_data(stream_data);
  return 0;
}

static ssize_t send_callback(nghttp2_session *session, const uint8_t *data,
                             size_t length, int flags, void *user_data) 
{ 
  std::cout<<"send_callback\n";
  http2_session_data *session_data = (http2_session_data *)user_data;
  struct bufferevent *bev = session_data->bev;
  (void)session;
  (void)flags;

  /* Avoid excessive buffering in server side. */
  if (evbuffer_get_length(bufferevent_get_output(session_data->bev)) >=
      OUTPUT_WOULDBLOCK_THRESHOLD) {
    return NGHTTP2_ERR_WOULDBLOCK;
  }
  bufferevent_write(bev, data, length);
  return (ssize_t)length;
}

static void initialize_nghttp2_session(http2_session_data *session_data) {
  nghttp2_session_callbacks *callbacks;

  nghttp2_session_callbacks_new(&callbacks);

  nghttp2_session_callbacks_set_send_callback(callbacks, send_callback);

  nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks,
                                                       on_frame_recv_callback);

  nghttp2_session_callbacks_set_on_stream_close_callback(
      callbacks, on_stream_close_callback);

  nghttp2_session_callbacks_set_on_header_callback(callbacks,
                                                   on_header_callback);

  nghttp2_session_callbacks_set_on_begin_headers_callback(
      callbacks, on_begin_headers_callback);

  nghttp2_session_server_new(&session_data->session, callbacks, session_data);

  nghttp2_session_callbacks_del(callbacks);
}

/***************************************/


/********* Server api ******************/
static void start_listen(struct event_base *evbase, const char *service, app_context *app_ctx) 
{
  int rv;
  struct addrinfo hints;
  struct addrinfo *res, *rp;

  std::cout<<"Starting server on "<<service<<"...\n";

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  rv = getaddrinfo(NULL, service, &hints, &res); 
  if (rv != 0) errx(1, "Could not resolve server address");

  for (rp = res; rp; rp = rp->ai_next) {
    struct evconnlistener *listener;
    listener = evconnlistener_new_bind(
        evbase, acceptcb, app_ctx, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
        16, rp->ai_addr, (int)rp->ai_addrlen);
    if (listener) {
      freeaddrinfo(res);
      return;
    }
  }
  errx(1, "Could not start listener");
}

static void initialize_app_context(app_context *app_ctx, struct event_base *evbase) 
{
  memset(app_ctx, 0, sizeof(app_context));
  app_ctx->evbase = evbase;
}

static void run(const char *service) 
{
  app_context app_ctx;
  struct event_base *evbase;

  evbase = event_base_new();
  initialize_app_context(&app_ctx, evbase);
  start_listen(evbase, service, &app_ctx);

  event_base_loop(evbase, 0);
  event_base_free(evbase);
}

/****************************************/