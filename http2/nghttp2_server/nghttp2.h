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
#include <iostream>

#define NGHTTP2_NO_SSIZE_T
#include <nghttp2/nghttp2.h>

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


#define ARRLEN(x) (sizeof(x) / sizeof(x[0]))

typedef struct {
  /* The NULL-terminated URI string to retrieve. */
  const char *uri;
  /* Parsed result of the |uri| */
  struct http_parser_url *u;
  /* The authority portion of the |uri|, not NULL-terminated */
  char *authority;
  /* The path portion of the |uri|, including query, not
     NULL-terminated */
  char *path;
  /* The length of the |authority| */
  size_t authoritylen;
  /* The length of the |path| */
  size_t pathlen;
  /* The stream ID of this stream */
  int32_t stream_id;
} http2_stream_data;

typedef struct {
  nghttp2_session   *session;
  http2_stream_data *stream_data;
} http2_session_data;

struct app_context
{

};

http2_stream_data* create_http2_stream_data(const char *uri, struct http_parser_url *u) 
{
  /* MAX 5 digits (max 65535) + 1 ':' + 1 NULL (because of snprintf) */
 //size_t extra = 7;
  http2_stream_data *stream_data = (http2_stream_data*)malloc(sizeof(http2_stream_data));

  return stream_data;
}

void delete_http2_stream_data(http2_stream_data *stream_data) 
{
  free(stream_data->path);
  free(stream_data->authority);
  free(stream_data);
}

http2_session_data *create_http2_session_data(app_context *app_ctx, int fd) 
{
  int rv;
  http2_session_data *session_data;
  int val = 1;

  session_data = (http2_session_data*)malloc(sizeof(http2_session_data));
  memset(session_data, 0, sizeof(http2_session_data));
  //session_data->app_ctx = app_ctx;

  return session_data;
}

void delete_http2_session_data(http2_session_data *session_data) 
{
  nghttp2_session_del(session_data->session);
  session_data->session = NULL;
  if (session_data->stream_data) {
    delete_http2_stream_data(session_data->stream_data);
    session_data->stream_data = NULL;
  }
  free(session_data);
}

/* Serialize the frame and send (or buffer) the data to
   bufferevent. */
int session_send(http2_session_data *session_data) 
{
  int rv;
  rv = nghttp2_session_send(session_data->session);
  if (rv != 0) {
    warnx("Fatal error: %s", nghttp2_strerror(rv));
    return -1;
  }
  return 0;
}

/* Read the data in the bufferevent and feed them into nghttp2 library
   function. Invocation of nghttp2_session_mem_recv2() may make
   additional pending frames, so call session_send() at the end of the
   function. */
int session_recv(http2_session_data *session_data) 
{
  /*nghttp2_ssize readlen;
  readlen = nghttp2_session_mem_recv2(session_data->session, data, datalen);
  if (readlen < 0) {
    warnx("Fatal error: %s", nghttp2_strerror((int)readlen));
    return -1;
  }
  /*if (evbuffer_drain(input, (size_t)readlen) != 0) {
    warnx("Fatal error: evbuffer_drain failed");
    return -1;
  }
  if (session_send(session_data) != 0) {
    return -1;
  }*/
  return 0;
}

nghttp2_ssize send_callback(nghttp2_session *session,
                                   const uint8_t *data, size_t length,
                                   int flags, void *user_data) 
{
  std::cout<<"*** Hit send_callback\n";
  /*http2_session_data *session_data = (http2_session_data *)user_data;
  struct bufferevent *bev = session_data->bev;
  (void)session;
  (void)flags;

  if (evbuffer_get_length(bufferevent_get_output(session_data->bev)) >=
      OUTPUT_WOULDBLOCK_THRESHOLD) {
    return NGHTTP2_ERR_WOULDBLOCK;
  }
  bufferevent_write(bev, data, length);
  return (nghttp2_ssize)length;*/

  return 0;
}

/* Returns nonzero if the string |s| ends with the substring |sub| */
int ends_with(const char *s, const char *sub)
{
  size_t slen = strlen(s);
  size_t sublen = strlen(sub);
  if (slen < sublen) {
    return 0;
  }
  return memcmp(s + slen - sublen, sub, sublen) == 0;
}

/* Returns int value of hex string character |c| */
uint8_t hex_to_uint(uint8_t c) 
{
  if ('0' <= c && c <= '9') {
    return (uint8_t)(c - '0');
  }
  if ('A' <= c && c <= 'F') {
    return (uint8_t)(c - 'A' + 10);
  }
  if ('a' <= c && c <= 'f') {
    return (uint8_t)(c - 'a' + 10);
  }
  return 0;
}

/* Decodes percent-encoded byte string |value| with length |valuelen|
   and returns the decoded byte string in allocated buffer. The return
   value is NULL terminated. The caller must free the returned
   string. */
char* percent_decode(const uint8_t *value, size_t valuelen) 
{
  char *res = (char*)malloc(valuelen + 1);
  if (valuelen > 3) {
    size_t i, j;
    for (i = 0, j = 0; i < valuelen - 2;) {
      if (value[i] != '%' || !isxdigit(value[i + 1]) ||
          !isxdigit(value[i + 2])) {
        res[j++] = (char)value[i++];
        continue;
      }
      res[j++] =
          (char)((hex_to_uint(value[i + 1]) << 4) + hex_to_uint(value[i + 2]));
      i += 3;
    }
    memcpy(&res[j], &value[i], 2);
    res[j + 2] = '\0';
  } else {
    memcpy(res, value, valuelen);
    res[valuelen] = '\0';
  }
  return res;
}

nghttp2_ssize file_read_callback(nghttp2_session *session,
                                        int32_t stream_id, uint8_t *buf,
                                        size_t length, uint32_t *data_flags,
                                        nghttp2_data_source *source,
                                        void *user_data) 
{
  int fd = source->fd;
  ssize_t r;
  (void)session;
  (void)stream_id;
  (void)user_data;

  while ((r = read(fd, buf, length)) == -1 && errno == EINTR)
    ;
  if (r == -1) {
    return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
  }
  if (r == 0) {
    *data_flags |= NGHTTP2_DATA_FLAG_EOF;
  }
  return (nghttp2_ssize)r;
}

int send_response(nghttp2_session *session, int32_t stream_id,
                         nghttp2_nv *nva, size_t nvlen, int fd) 
{
  std::cout<<"send_response\n";
  int rv;
  nghttp2_data_provider2 data_prd;
  data_prd.source.fd = fd;
  data_prd.read_callback = file_read_callback;

  rv = nghttp2_submit_response2(session, stream_id, nva, nvlen, &data_prd);
  if (rv != 0) {
    warnx("Fatal error: %s", nghttp2_strerror(rv));
    return -1;
  }
  return 0;
}

static const char ERROR_HTML[] = "<html><head><title>404</title></head>"
                                 "<body><h1>404 Not Found</h1></body></html>";

int error_reply(nghttp2_session *session, http2_stream_data *stream_data) 
{
  int rv;
  ssize_t writelen;
  int pipefd[2];
  nghttp2_nv hdrs[] = {MAKE_NV(":status", "404")};

  rv = pipe(pipefd);
  if (rv != 0) {
    warn("Could not create pipe");
    rv = nghttp2_submit_rst_stream(session, NGHTTP2_FLAG_NONE,
                                   stream_data->stream_id,
                                   NGHTTP2_INTERNAL_ERROR);
    if (rv != 0) {
      warnx("Fatal error: %s", nghttp2_strerror(rv));
      return -1;
    }
    return 0;
  }

  writelen = write(pipefd[1], ERROR_HTML, sizeof(ERROR_HTML) - 1);
  close(pipefd[1]);

  if (writelen != sizeof(ERROR_HTML) - 1) {
    close(pipefd[0]);
    return -1;
  }

  //stream_data->fd = pipefd[0];

  if (send_response(session, stream_data->stream_id, hdrs, ARRLEN(hdrs),
                    pipefd[0]) != 0) {
    close(pipefd[0]);
    return -1;
  }
  return 0;
}

/* nghttp2_on_header_callback: Called when nghttp2 library emits
   single header name/value pair. */
int on_header_callback(nghttp2_session *session,
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
    //if (!stream_data || stream_data->request_path) {
      //break;
    //}
    if (namelen == sizeof(PATH) - 1 && memcmp(PATH, name, namelen) == 0) {
      //size_t j;
      //for (j = 0; j < valuelen && value[j] != '?'; ++j)
        //;
      //stream_data->request_path = percent_decode(value, j);
    }
    break;
  }
  return 0;
}

int on_begin_headers_callback(nghttp2_session *session,
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
  //stream_data = create_http2_stream_data(session_data, frame->hd.stream_id);
  //nghttp2_session_set_stream_user_data(session, frame->hd.stream_id,
    //                                   stream_data);
  return 0;
}

/* Minimum check for directory traversal. Returns nonzero if it is
   safe. */
int check_path(const char *path) 
{
  /* We don't like '\' in url. */
  return path[0] && path[0] == '/' && strchr(path, '\\') == NULL &&
         strstr(path, "/../") == NULL && strstr(path, "/./") == NULL &&
         !ends_with(path, "/..") && !ends_with(path, "/.");
}

int on_request_recv(nghttp2_session *session,
                           http2_session_data *session_data,
                           http2_stream_data *stream_data) 
{
  std::cout<<"on_request_recv\n";
  /*int fd;
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
  }*/
  return 0;
}

int on_frame_recv_callback(nghttp2_session *session,const nghttp2_frame *frame, void *user_data) 
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

int on_stream_close_callback(nghttp2_session *session, int32_t stream_id,
                                    uint32_t error_code, void *user_data) 
{
  std::cout<<"on_frame_recv_callback\n";
  http2_session_data *session_data = (http2_session_data *)user_data;
  http2_stream_data *stream_data;
  (void)error_code;

  stream_data = (http2_stream_data*)nghttp2_session_get_stream_user_data(session, stream_id);
  if (!stream_data) {
    return 0;
  }
  //remove_stream(session_data, stream_data);
  delete_http2_stream_data(stream_data);
  return 0;
}

void initialize_nghttp2_session(http2_session_data *session_data) 
{
  std::cout<<"initialize_nghttp2_session\n";
  
  nghttp2_session_callbacks *callbacks;

  nghttp2_session_callbacks_new(&callbacks);

  nghttp2_session_callbacks_set_send_callback2(callbacks, send_callback);

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

/* Send HTTP/2 client connection header, which includes 24 bytes
   magic octets and SETTINGS frame */
int send_server_connection_header(http2_session_data *session_data) 
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
