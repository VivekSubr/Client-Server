#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <string.h>
#include <iostream>

int answer_to_connection (void *cls, struct MHD_Connection *connection,
                          const char *url,
                          const char *method, const char *version,
                          const char *upload_data,
                          size_t *upload_data_size, void **con_cls)
{
    const char *reply  = "libmicrohttpd replied";
    struct MHD_Response *response = MHD_create_response_from_buffer (strlen (reply),
                                            (void*) reply, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return ret;
}
