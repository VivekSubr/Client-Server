#include "utils.h"
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    const char* host = "google.com";
    const char* port = "80";

    struct gaicb request;
    struct gaicb* requests[] = { &request };

    // Initialize the gaicb structure
    memset(&request, 0, sizeof(request));
    request.ar_name = host;
    request.ar_service = port;

    // Start the non-blocking resolve operation
    int ret = getaddrinfo_a(GAI_NOWAIT, requests, 1, nullptr);
    if (ret != 0)
    {
        std::cerr << "Error starting DNS resolution: " << gai_strerror(ret) << std::endl;
        return 1;
    }

    // Wait for the resolution to complete
    while (true)
    {
        // Check the status of the resolution
        ret = gai_error(&request);
        if (ret == EAI_INPROGRESS)
        {
            // Resolution still in progress, continue waiting
            usleep(1000); // Sleep for a short time before checking again
            continue;
        }
        else if (ret == 0)
        {
            // Resolution completed successfully
           printAddrInfo(request.ar_result);
        }
        else
        {
            // Error occurred during resolution
            std::cerr << "Error resolving DNS: " << gai_strerror(ret) << std::endl;
        }

        break; // Resolution is complete or error occurred, exit the loop
    }

    return 0;
}