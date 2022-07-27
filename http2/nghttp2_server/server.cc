#include "server_impl.h"

int main(int argc, char *argv[])
{
    struct sigaction act;

    if (argc < 2) {
      fprintf(stderr, "Usage: libevent-server PORT\n");
      exit(EXIT_FAILURE);
    }

    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, NULL);

    run(argv[1]);
    return 0;
}