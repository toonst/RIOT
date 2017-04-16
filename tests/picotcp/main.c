
#include "shell.h"

#include "pico_icmp4.h"
#include "pico_ipv4.h"

#define NUM_PING 1

/* gets called when the ping receives a reply, or encounters a problem */
void _cb_ping(struct pico_icmp4_stats *s)
{
    char host[30];
    pico_ipv4_to_string(host, s->dst.addr);
    if (s->err == 0) {
        /* if all is well, print some pretty info */
        printf("%lu bytes from %s: icmp_req=%lu ttl=%lu time=%lu ms\n", s->size,
                host, s->seq, s->ttl, (long unsigned int)s->time);
    } else {
        /* if something went wrong, print it and signal we want to stop */
        printf("PING %lu to %s: Error %d\n", s->seq, host, s->err);
    }
}
static int _ping(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int id;

    id = pico_icmp4_ping("192.168.5.5", NUM_PING, 1000, 10000, 64, _cb_ping);

    if (id == -1) {
        return -1;
    } else {
        return 0;
    }
}

static const shell_command_t shell_commands[] = {
    { "ping", "Send pings", _ping },
    { NULL, NULL, NULL }
};
int main(void)
{
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
