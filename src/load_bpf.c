#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "load_bpf.h"
#include "xdp.skel.h"


int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args) {
    if (level == LIBBPF_DEBUG)
        return 0;
    return vfprintf(stderr, format, args);
}

void bump_memlock_rlimit() {
    struct rlimit rlim_new = {
            .rlim_cur	= RLIM_INFINITY,
            .rlim_max	= RLIM_INFINITY,
    };
    if (setrlimit(RLIMIT_MEMLOCK, &rlim_new)) {
        fprintf(stderr, "Failed to increase RLIMIT_MEMLOCK limit!\n");
        exit(1);
    }
}

struct xdp_bpf* load_bpf(){
    struct xdp_bpf* skel = xdp_bpf__open();
    if (!skel) {
        fprintf(stderr, "Failed to open skeleton\n");
        exit(1);
    }
    int err = xdp_bpf__load(skel);
    if (err) {
        fprintf(stderr, "Failed to load skeleton: %s\n", strerror(err));
        exit(1);
    }
    return skel;
}

void unload_bpf(struct xdp_bpf *skel){
    xdp_bpf__destroy(skel);
}

struct xdp_bpf* attach_bpf(){
    struct xdp_bpf *skel = load_bpf();
    /* Attach RX XDP */
    int rx_fd = bpf_program__fd(skel->progs.xdp_rx);
    int err = bpf_xdp_attach(RX_IFINDEX, rx_fd, XDP_LOADING_MODE, 0);
    if (err) {
        fprintf(stderr, "Failed to attach XDP program to RX: %s\n", strerror(err));
        exit(1);
    }
    /* Attach TX XDP */
    int tx_fd = bpf_program__fd(skel->progs.xdp_tx);
    err = bpf_xdp_attach(TX_IFINDEX, tx_fd, XDP_LOADING_MODE, 0);
    if (err) {
        fprintf(stderr, "Failed to attach XDP program to TX: %s\n", strerror(err));
        exit(1);
    }
    return skel;
}

void detach_bpf(struct xdp_bpf* skel){
    bpf_xdp_detach(RX_IFINDEX, 0, 0);
    bpf_xdp_detach(TX_IFINDEX, 0, 0);
    unload_bpf(skel);
}
