#ifndef LOAD_BPF_H
#define LOAD_BPF_H

#include <argp.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include "env.h"


struct xdp_bpf* attach_bpf();
void detach_bpf(struct xdp_bpf* skel);
void bump_memlock_rlimit();
int libbpf_print_fn();

#endif