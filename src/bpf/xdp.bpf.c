// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */
#include "../../vmlinux/vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_endian.h>
#include "xdp.bpf.h"

// Create a per CPU hash map to store the connection state
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(key_size, sizeof(struct FiveTuple));
    __uint(value_size, sizeof(struct ConnectionState));
    __uint(max_entries, 4096*64);
} connections SEC(".maps");

SEC("xdp")
int xdp_rx(struct xdp_md *ctx)
{
    // Returns the program to the TX interface
    return bpf_redirect(TX_IFINDEX, 0);
}

SEC("xdp")
int xdp_tx(struct xdp_md *ctx)
{
    // Returns the program to the RX interface
    return bpf_redirect(RX_IFINDEX, 0);
}