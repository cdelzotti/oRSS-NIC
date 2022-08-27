#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "load_bpf.h"
#include "xdp.skel.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t looping = 1;

void handle_interrupt(int sig) {
    printf("Caught interrupt signal, exiting...\n");
    looping = 0;
}

int user_space_prog(int connections_map_fd){
    // Contains the user space logic
    // Typically, should be a forever looping program

    printf("Everything loaded correctly!\n");
    while (looping)
    {
        printf("Looping...\n");
        sleep(1);
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    // Initial setup
    libbpf_set_print(libbpf_print_fn);
    bump_memlock_rlimit();
    signal(SIGINT, handle_interrupt);
    // Load BPF
    struct xdp_bpf *skel = attach_bpf();
    // Setup maps
    int connections_fd = bpf_map__fd(skel->maps.connections);
    if (connections_fd < 0) {
        fprintf(stderr, "Failed to get connections map FD: %s\n", strerror(errno));
        detach_bpf(skel);
        exit(1);
    }
    user_space_prog(connections_fd);
    // Detach BPF
    detach_bpf(skel);
    printf("Everything unloaded correctly!\n");
    return 0;
}