#include "server.h"
#include <stdio.h>
#define PORT 8888

int main()
{
    struct MHD_Daemon *daemon = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                                                    &answer_to_connection, NULL, MHD_OPTION_END);
    
    if(daemon == NULL) return -1;
    
    std::cout<<"listening on "<<PORT<<" hit ENTER to exit\n";
    getchar();
    MHD_stop_daemon (daemon);
    return 0;
}