// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */
#include "../../vmlinux/vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_endian.h>
#include "xdp.bpf.h"


char LICENSE[] SEC("license") = "Dual BSD/GPL";

// Create redirection map for XDP_REDIRECT action
struct {
    __uint(type, BPF_MAP_TYPE_DEVMAP);
    __uint(key_size, sizeof(int));
    __uint(value_size, sizeof(int));
    __uint(max_entries, 100);
} map_redir SEC(".maps");

// Create a per CPU hash map to store the connection state
struct {
    __uint(type, BPF_MAP_TYPE_PERCPU_HASH);
    __uint(key_size, sizeof(struct FiveTuple));
    __uint(value_size, sizeof(struct ConnectionState));
    __uint(max_entries, 4096*64);
} connections SEC(".maps");

static inline int forward(void* map, u32 key, u64 flags){
    if (XDP_FORWARDING)
        return bpf_redirect_map(map, key, flags);
    return XDP_PASS;
}

static enum TCP_FLAGS parse_headers(struct xdp_md_copy *ctx, struct FiveTuple *tuple){
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;
    struct ethhdr *eth = data;
    if (data + sizeof(*eth) > data_end)
        return ERROR;
    if (eth->h_proto == bpf_htons(ETH_P_IP)){
        struct iphdr *ip = data + sizeof(struct ethhdr);
        if (data + sizeof(*ip) + sizeof(*eth) > data_end)
            return ERROR;
        tuple->src_ip = ip->saddr;
        tuple->dst_ip = ip->daddr;
        tuple->proto = ip->protocol;
        if (ip->protocol == IPPROTO_TCP){
            struct tcphdr *tcp = data + sizeof(struct ethhdr) + sizeof(struct iphdr);
            if (data + sizeof(*ip) + sizeof(*eth) + sizeof(*tcp) > data_end)
                return ERROR;
            tuple->src_port = bpf_ntohs(tcp->source);
            tuple->dst_port = bpf_ntohs(tcp->dest);
            if (tcp->fin) {
                bpf_printk("FIN packet detected\n");
                return FIN;
            } else 
            if (tcp->syn && !tcp->ack){
                bpf_printk("SYN");
                return SYN;
            } else if (tcp->ack && tcp->syn){
                bpf_printk("SYNACK");
                return SYNACK;
            } else if (tcp->ack && !tcp->syn){
                // bpf_printk("ACK");
                return ACK;
            } else
                return NO_FLAGS;
        } else if (ip->protocol == IPPROTO_UDP){
            struct udphdr *udp = data + sizeof(struct ethhdr) + sizeof(struct iphdr);
            if (data + sizeof(*ip) + sizeof(*eth) + sizeof(*udp) > data_end)
                return ERROR;
            tuple->src_port = bpf_ntohs(udp->source);
            tuple->dst_port = bpf_ntohs(udp->dest);
            return UDP;
        } else {
            return NOT_TCP;
        }
    }
    return NOT_TCP;
}

static inline void update_connection_state(struct FiveTuple *tuple, struct ConnectionState *state){
    struct ConnectionState *old_state = bpf_map_lookup_elem(&connections, tuple);
    if (old_state){
        if (state->SYN)
            old_state->SYN = 1;
        if (state->SYNACK)
            old_state->SYNACK = 1;
        if (state->ACK)
            old_state->ACK = 1;
        if (state->FIN)
            old_state->FIN = 1;
        bpf_map_update_elem(&connections, tuple, old_state, BPF_ANY);
    } else {
        bpf_map_update_elem(&connections, tuple, state, BPF_ANY);
    }
}

static inline void swap_tuple(struct FiveTuple *tuple){
    uint32_t tmp_ip = tuple->src_ip;
    uint16_t tmp_port = tuple->src_port;
    tuple->src_ip = tuple->dst_ip;
    tuple->src_port = tuple->dst_port;
    tuple->dst_ip = tmp_ip;
    tuple->dst_port = tmp_port;
}

SEC("xdp")
int xdp_rx(struct xdp_md_copy *ctx)
{
    struct FiveTuple tuple = {0};
    enum TCP_FLAGS flags = parse_headers(ctx, &tuple);
    if (flags == ERROR || 
        flags == NO_FLAGS ||
        (flags == NOT_TCP && tuple.proto != IPPROTO_UDP))
        return forward(&map_redir, ctx->ingress_ifindex,0);
    struct ConnectionState state = {0};
    if (flags == UDP){
        state.SYN = 1;
        state.SYNACK = 1;
        state.ACK = 1;
    } else {
        if (flags == SYN)
            state.SYN = 1;
        if (flags == SYNACK)
            state.SYNACK = 1;
        if (flags == ACK)
            state.ACK = 1;
        // if (flags == FIN)
        //     state.FIN = 1;
    }
    update_connection_state(&tuple, &state);
    return forward(&map_redir, ctx->ingress_ifindex,0);
}

SEC("xdp")
int xdp_tx(struct xdp_md_copy *ctx)
{
    struct FiveTuple tuple = {0};
    enum TCP_FLAGS flags = parse_headers(ctx, &tuple);
    if (flags == ERROR || 
        flags == NO_FLAGS ||
        (flags == NOT_TCP && tuple.proto != IPPROTO_UDP))
        return forward(&map_redir, ctx->ingress_ifindex,0);
    struct ConnectionState state = {0};
    // On TX side, must swap src and dst
    swap_tuple(&tuple);
    if (flags == UDP){
        state.SYN = 1;
        state.SYNACK = 1;
        state.ACK = 1;
    } else {
        if (flags == SYN)
            state.SYN = 1;
        if (flags == SYNACK)
            state.SYNACK = 1;
        if (flags == ACK)
            state.ACK = 1;
        update_connection_state(&tuple, &state);
    }
    return forward(&map_redir, ctx->ingress_ifindex,0);
}
